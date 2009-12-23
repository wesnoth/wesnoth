/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "text.hpp"

#include "gettext.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/auxiliary/log.hpp"
#include "font.hpp"
#include "serialization/string_utils.hpp"

#include <cassert>
#include <cstring>

#ifndef PANGO_VERSION_CHECK
#define PANGO_VERSION_CHECK(a,b,c) 0
#endif

namespace font {

namespace {

/**
 * Small helper wrapper for PangoLayoutIter*.
 *
 * Needed to make sure it gets freed properly.
 */
class titor
{
	titor(const titor&);
	titor& operator=(const titor&);
public:
	titor(PangoLayout* layout_) :
		itor_(pango_layout_get_iter(layout_))
	{
	}

	~titor() { pango_layout_iter_free(itor_); }

	operator PangoLayoutIter*() { return itor_; }

private:
	PangoLayoutIter* itor_;
};

} // namespace

const unsigned ttext::STYLE_NORMAL = TTF_STYLE_NORMAL;
const unsigned ttext::STYLE_BOLD = TTF_STYLE_BOLD;
const unsigned ttext::STYLE_ITALIC = TTF_STYLE_ITALIC;
const unsigned ttext::STYLE_UNDERLINE = TTF_STYLE_UNDERLINE;

ttext::ttext() :
#if PANGO_VERSION_CHECK(1,22,0)
	context_(pango_font_map_create_context(pango_cairo_font_map_get_default())),
#else
	context_(pango_cairo_font_map_create_context((
		reinterpret_cast<PangoCairoFontMap*>(pango_cairo_font_map_get_default())))),
#endif
	layout_(pango_layout_new(context_)),
	rect_(),
	surface_(),
	text_(),
	markedup_text_(false),
	font_size_(14),
	font_style_(STYLE_NORMAL),
	foreground_colour_(0xFFFFFFFF), // solid white
	maximum_width_(-1),
	maximum_height_(-1),
	ellipse_mode_(PANGO_ELLIPSIZE_END),
	maximum_length_(std::string::npos),
	calculation_dirty_(true),
	length_(0),
	surface_dirty_(true),
	surface_buffer_(NULL)
{
	// With 72 dpi the sizes are the same as with SDL_TTF so hardcoded.
	pango_cairo_context_set_resolution(context_, 72.0);

	pango_layout_set_ellipsize(layout_, ellipse_mode_);

	/*
	 * Set the pango spacing a bit bigger since the default is deemed to small
	 * http://www.wesnoth.org/forum/viewtopic.php?p=358832#p358832
	 */
	pango_layout_set_spacing(layout_, 2 * PANGO_SCALE);

	cairo_font_options_t *fo = cairo_font_options_create();
	cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_FULL);
	cairo_font_options_set_hint_metrics(fo, CAIRO_HINT_METRICS_ON);
	pango_cairo_context_set_font_options(context_, fo);
	cairo_font_options_destroy(fo);
}

ttext::~ttext()
{
	if(context_) {
		g_object_unref(context_);
	}
	if(layout_) {
		g_object_unref(layout_);
	}
	if(surface_buffer_) {
		surface_.assign(NULL);
		delete[] surface_buffer_;
	}
}

surface ttext::render() const
{
	rerender();
	return surface_;
}

int ttext::get_width() const
{
	return get_size().x;
}

int ttext::get_height() const
{
	return get_size().y;
}

gui2::tpoint ttext::get_size() const
{
	recalculate();

	return gui2::tpoint(rect_.width, rect_.height);
}

bool ttext::is_truncated() const
{
	recalculate();

#if PANGO_VERSION_CHECK(1,16,0)
	return (pango_layout_is_ellipsized(layout_) != 0);
#else
	return false;
#endif
}

unsigned ttext::insert_text(const unsigned offset, const std::string& text)
{
	if(text.empty()) {
		return 0;
	}

	return insert_unicode(offset, utils::string_to_wstring(text));
}

bool ttext::insert_unicode(const unsigned offset, const wchar_t unicode)
{
	return (insert_unicode(offset, wide_string(1, unicode)) == 1);
}

unsigned ttext::insert_unicode(const unsigned offset, const wide_string& unicode)
{
	assert(offset <= length_);

	if(length_ == maximum_length_) {
		return 0;
	}

	const unsigned len = length_ + unicode.size() > maximum_length_
		? maximum_length_ - length_  : unicode.size();

	wide_string tmp = utils::string_to_wstring(text_);
	tmp.insert(tmp.begin() + offset, unicode.begin(), unicode.begin() + len);

	set_text(utils::wstring_to_string(tmp), false);

	return len;
}

gui2::tpoint ttext::get_cursor_position(
		const unsigned column, const unsigned line) const
{
	recalculate();

	// First we need to determine the byte offset, if more routines need it it
	// would be a good idea to make it a separate function.
	titor itor(layout_);

	// Go the the wanted line.
	if(line != 0) {
		if(pango_layout_get_line_count(layout_) >= static_cast<int>(line)) {
			return gui2::tpoint(0, 0);
		}

		for(size_t i = 0; i < line; ++i) {
			pango_layout_iter_next_line(itor);
		}
	}

	// Go the wanted column.
	for(size_t i = 0; i < column; ++i) {
		if(!pango_layout_iter_next_char(itor)) {
			// It seems that the documentation is wrong and causes and off by
			// one error... the result should be false if already at the end of
			// the data when started.
			if(i + 1 == column) {
				break;
			}
			// beyound data.
			return gui2::tpoint(0, 0);
		}
	}

	// Get the byte offset
	const int offset = pango_layout_iter_get_index(itor);

	// Convert the byte offset in a position.
	PangoRectangle rect;
	pango_layout_get_cursor_pos(layout_, offset, &rect, NULL);

	return gui2::tpoint(PANGO_PIXELS(rect.x), PANGO_PIXELS(rect.y));
}

gui2::tpoint ttext::get_column_line(const gui2::tpoint& position) const
{
	recalculate();

	// Get the index of the character.
	int index, trailing;
	pango_layout_xy_to_index(layout_, position.x * PANGO_SCALE,
		position.y * PANGO_SCALE, &index, &trailing);

	// Extract the line and the offset in pixels in that line.
	int line, offset;
	pango_layout_index_to_line_x(layout_, index, trailing, &line, &offset);
	offset = PANGO_PIXELS(offset);

	// Now convert this offset to a column, this way is a bit hacky but haven't
	// found a better solution yet.

	/**
	 * @todo There's still a bug left. When you select a text which is in the
	 * ellipses on the right side the text gets reformatted with ellipses on
	 * the left and the selected character is not the one under the cursor.
	 * Other widget toolkits don't show ellipses and have no indication more
	 * text is available. Haven't found what the best thing to do would be.
	 * Until that time leave it as is.
	 */
	for(size_t i = 0; ; ++i) {
		const int pos = get_cursor_position(i, line).x;

		if(pos == offset) {
			return  gui2::tpoint(i, line);
		}
	}
}

bool ttext::set_text(const std::string& text, const bool markedup)
{
	if(markedup != markedup_text_ || text != text_) {
		assert(layout_);

		if(markedup) {
			if(!pango_parse_markup(text.c_str(), text.size()
						, 0, NULL, NULL, NULL, NULL)) {

				ERR_GUI_L << "ttext::" << __func__
						<< " text '" << text
						<< "' has broken markup, set to normal text.\n";
				/** @todo Enable after 1.8. */
//				set_text(_("The text contains invalid markup: ") + text, false);
				set_text(text, false);
				return false;
			}
			pango_layout_set_markup(layout_, text.c_str(), text.size());
		} else {
			/*
			 * pango_layout_set_text after pango_layout_set_markup might
			 * leave the layout in an undefined state regarding markup so
			 * clear it unconditionally.
			 */
			pango_layout_set_attributes(layout_, NULL);
			pango_layout_set_text(layout_, text.c_str(), text.size());
		}
		text_ = text;
		length_ = utils::string_to_wstring(text_).size();
		markedup_text_ = markedup;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return true;
}

ttext& ttext::set_font_size(const unsigned font_size)
{
	if(font_size != font_size_) {
		font_size_ = font_size;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_font_style(const unsigned font_style)
{
	if(font_style != font_style_) {
		font_style_ = font_style;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_foreground_colour(const Uint32 colour)
{
	if(colour != foreground_colour_) {
		foreground_colour_ = colour;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_width(int width)
{
	if(width <= 0) {
		width = -1;
	}

	if(width != maximum_width_) {
		assert(context_);
#if 0
		/**
		 * todo Adding 4 extra pixels feels a bit hacky.
		 *
		 * For some reason it's needed since the following scenario fails:
		 * - pango_layout_set_width(value)
		 * - pango_layout_get_pixel_extents() -> max_width_1
		 * - pango_layout_set_width(max_width_1)
		 * - pango_layout_get_pixel_extents() -> max_width_2
		 *
		 * Now it can happen max_width_2 < max_width_1. Adding the 4 seems to
		 * "fix" the problem.
		 */
		pango_layout_set_width(layout_, width == -1
				? -1
				: (width + 4) * PANGO_SCALE);
#endif
		maximum_width_ = width;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_height(int height, bool multiline)
{
	if(height <= 0) {
		height = -1;
		multiline = false;
	}

	if(height != maximum_height_) {
		assert(context_);

/**
 * @todo See whether we can make pango 1.20 mandatory before Wesnoth 1.6 is
 * released.
 */
#if PANGO_VERSION_CHECK(1,20,0)
		pango_layout_set_height(layout_, !multiline ? -1 : height * PANGO_SCALE);
#endif
		maximum_height_ = height;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_ellipse_mode(const PangoEllipsizeMode ellipse_mode)
{
	if(ellipse_mode != ellipse_mode_) {
		assert(context_);

		pango_layout_set_ellipsize(layout_, ellipse_mode);
		ellipse_mode_ = ellipse_mode;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_length(const size_t maximum_length)
{
	if(maximum_length != maximum_length_) {
		maximum_length_ = maximum_length;
		if(length_ > maximum_length_) {

			wide_string tmp = utils::string_to_wstring(text_);
			tmp.resize(maximum_length_);
			set_text(utils::wstring_to_string(tmp), false);
		}
	}

	return *this;
}

namespace {

/** Small helper class to make sure the font object is destroyed properly. */
class tfont
{
	tfont(const tfont&);
	tfont& operator=(const tfont&);
public:
	tfont(const std::string& name, const unsigned size, const unsigned style) :
		font_(pango_font_description_new())
	{
		pango_font_description_set_family(font_, name.c_str());
		pango_font_description_set_size(font_, size * PANGO_SCALE);

		if(style != ttext::STYLE_NORMAL) {
			if(style & ttext::STYLE_ITALIC) {
				pango_font_description_set_style(font_, PANGO_STYLE_ITALIC);
			}
			if(style & ttext::STYLE_BOLD) {
				pango_font_description_set_weight(font_, PANGO_WEIGHT_BOLD);
			}
			if(style & ttext::STYLE_UNDERLINE) {
				assert(false); // Not implemented yet
			}
		}
	}

	~tfont() { pango_font_description_free(font_); }

	PangoFontDescription* get() { return font_; }

private:
	PangoFontDescription *font_;
};

std::ostream& operator<<(std::ostream& s, const PangoRectangle &rect)
{
	s << rect.x << ',' << rect.y << " x " << rect.width << ',' << rect.height;
	return s;
}

} // namespace

void ttext::recalculate(const bool force) const
{
	if(calculation_dirty_ || force) {
		assert(layout_);

		calculation_dirty_ = false;
		surface_dirty_ = true;

		tfont font(get_font_families(), font_size_, font_style_);
		pango_layout_set_font_description(layout_, font.get());

		/*
		 * See set_maximum_width for some more background info as well.
		 * In order to fix the problem first set a width which seems to render
		 * correctly then lower it to fit. For the campaigns the 4 does "the
		 * right thing" for the terrain labels it should use the value 0 to set
		 * the ellipse properly. Need to see whether this is a bug in pango or
		 * a bug in my understanding of the pango api.
		 */
		int hack = 4;
		do {
			pango_layout_set_width(layout_, maximum_width_ == -1
					? -1
					: (maximum_width_ + hack) * PANGO_SCALE);
			pango_layout_get_pixel_extents(layout_, NULL, &rect_);

			DBG_GUI_L << "ttext::" << __func__
					<< " text '" << gui2::debug_truncate(text_)
					<< "' maximum_width " << maximum_width_
					<< " hack " << hack
					<< " width " << rect_.width
					<< ".\n";

			--hack;
		} while(maximum_width_ != -1
				&& hack >= 0 && rect_.width > maximum_width_);

		DBG_GUI_L << "ttext::" << __func__
				<< " text '" << gui2::debug_truncate(text_)
				<< "' font_size " << font_size_
				<< " markedup_text " << markedup_text_
				<< " font_style " << std::hex << font_style_ << std::dec
				<< " maximum_width " << maximum_width_
				<< " maximum_height " << maximum_height_
				<< " result " <<  rect_
				<< ".\n";
		if(maximum_width_ != -1 && rect_.width > maximum_width_) {
			DBG_GUI_L << "ttext::" << __func__
					<< " text '" << gui2::debug_truncate(text_)
					<< " ' width " << rect_.width
					<< " greater as the wanted maximum of " << maximum_width_
					<< ".\n";
		}
	}
}

struct decode_table
{
	// 1-based, from 1 to 255.
	unsigned values[255];
	decode_table()
	{
		for (int i = 1; i < 256; ++i) values[i - 1] = (255 * 256) / i;
	}
};

static decode_table decode_table;

/**
 * Converts from premultiplied alpha to plain alpha.
 * @param p pointer to a 4-byte endian-dependent color.
 */
static void decode_pixel(unsigned char *p)
{
// Assume everything not compiled with gcc to be on a little endian platform.
#if defined(__GNUC__) && defined(__BIG_ENDIAN__)
	int alpha = p[0];
#else
	int alpha = p[3];
#endif
	if (alpha == 0) return;

	int div = decode_table.values[alpha - 1];

#define DECODE(i) \
	do { \
		unsigned color = p[i]; \
		color = color * div / 256; \
		if (color > 255) color = 255; \
		p[i] = color; \
	} while (0)

#if defined(__GNUC__) && defined(__BIG_ENDIAN__)
	DECODE(3);
#else
	DECODE(0);
#endif
	DECODE(1);
	DECODE(2);
}

void ttext::rerender(const bool force) const
{
	if(surface_dirty_ || force) {
		assert(layout_);

		recalculate(force);
		surface_dirty_ = false;

		int width = rect_.x + rect_.width;
		int height = rect_.y + rect_.height;
		if (maximum_width_  > 0 && width  > maximum_width_ ) width  = maximum_width_;
		if (maximum_height_ > 0 && height > maximum_height_) height = maximum_height_;
		const unsigned stride = width * 4;
		create_surface_buffer(stride * height);

		cairo_surface_t *cairo_surface =
			cairo_image_surface_create_for_data(surface_buffer_,
				CAIRO_FORMAT_ARGB32, width, height, stride);
		cairo_t *cr = cairo_create(cairo_surface);

		/* set colour (used for foreground). */
		cairo_set_source_rgba(cr,
			 (foreground_colour_ >> 24)         / 256.0,
			((foreground_colour_ >> 16) & 0xFF) / 256.0,
			((foreground_colour_ >> 8)  & 0xFF) / 256.0,
			(foreground_colour_         & 0xFF) / 256.0);

		pango_cairo_show_layout(cr, layout_);

#ifndef _WIN32

		// The cairo surface is in CAIRO_FORMAT_ARGB32 which uses
		// pre-multiplied alpha. SDL doesn't use that so the pixels need to be
		// decoded again.
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x)
			{
				unsigned char *pixel = &surface_buffer_[(y * width + x) * 4];
				decode_pixel(pixel);
			}
		}
#else
		// The solution code above doesn't seem to work properly on windows so
		// use the old trick of drawing the same text a few times.
		pango_cairo_show_layout(cr, layout_);
		pango_cairo_show_layout(cr, layout_);
		pango_cairo_show_layout(cr, layout_);
#endif
		surface_.assign(SDL_CreateRGBSurfaceFrom(
			surface_buffer_, width, height, 32, stride,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
		cairo_destroy(cr);
		cairo_surface_destroy(cairo_surface);
	}
}

void ttext::create_surface_buffer(const size_t size) const
{
	// clear old buffer
	if(surface_buffer_) {
		surface_.assign(NULL);
		delete[] surface_buffer_;
	}

	surface_buffer_ = new unsigned char [size];
	memset(surface_buffer_, 0, size);
}

} // namespace font

