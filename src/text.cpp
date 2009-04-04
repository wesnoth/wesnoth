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

#include "gui/widgets/helper.hpp"
#include "font.hpp"

#include <cassert>
#include <cstring>

namespace font {

namespace internal {

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
	context_(pango_cairo_font_map_create_context((
		reinterpret_cast<PangoCairoFontMap*>(pango_cairo_font_map_get_default())))),
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

#if defined(PANGO_VERSION_CHECK)
#if PANGO_VERSION_CHECK(1,16,0)
	return (pango_layout_is_ellipsized(layout_) == TRUE);
#endif
#endif
	return false;
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

void ttext::clone()
{
	context_ = pango_cairo_font_map_create_context((
		reinterpret_cast<PangoCairoFontMap*>(pango_cairo_font_map_get_default())));
	// With 72 dpi the sizes are the same as with SDL_TTF so hardcoded.
	pango_cairo_context_set_resolution(context_, 72.0);

	layout_ = pango_layout_new(context_);
	pango_layout_set_ellipsize(layout_, ellipse_mode_);

	surface_dirty_ = true;
	surface_buffer_ = 0;
}

ttext& ttext::set_text(const std::string& text, const bool markedup)
{
	if(markedup != markedup_text_ || text != text_) {
		assert(layout_);

		if(markedup) {
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

	return *this;
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

		pango_layout_set_width(layout_, width == -1 ? -1 : width * PANGO_SCALE);
		maximum_width_ = width;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

ttext& ttext::set_maximum_height(int height)
{
	if(height <= 0) {
		height = -1;
	}

	if(height != maximum_height_) {
		assert(context_);

/**
 * @todo See whether we can make pango 1.20 mandatory before Wesnoth 1.6 is
 * released.
 */
#if defined(PANGO_VERSION_CHECK)
#if PANGO_VERSION_CHECK(1,20,0)
		pango_layout_set_height(layout_, height == -1 ? -1 : height * PANGO_SCALE);
#endif
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

		tfont font(get_fonts(), font_size_, font_style_);
		pango_layout_set_font_description(layout_, font.get());

		pango_layout_get_pixel_extents(layout_, NULL, &rect_);
	}
}

/**
 * Helper function to decode pixels.
 *
 * @param alpha                   The value of the alpha channel, this value
 *                                must not be zero.
 * @param sub_pixel               The address of a sub pixel, either red,
 *                                green or blue.
 */
static void decode_pixel(const unsigned char alpha, unsigned char *sub_pixel)
{
	unsigned colour = *sub_pixel;
	// Sature at 255.
	*sub_pixel = colour >= alpha
			? 255
			: static_cast<unsigned char>((colour << 8) / alpha);
}

void ttext::rerender(const bool force) const
{
	if(surface_dirty_ || force) {
		assert(layout_);

		recalculate(force);
		surface_dirty_ = false;

		const unsigned width = rect_.x + rect_.width;
		const unsigned height = rect_.y + rect_.height;
		const unsigned stride = width * 4;
		create_surface_buffer(stride * height);

		cairo_surface_t *cairo_surface =
			cairo_image_surface_create_for_data(surface_buffer_,
				CAIRO_FORMAT_ARGB32, width, height, stride);
		cairo_t *cr = cairo_create(cairo_surface);

		pango_cairo_update_context (cr, context_); // Needed?

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
		for(size_t y = 0; y < height; ++y) {
			for(size_t x = 0; x < width; ++x) {

				unsigned char *pixel = &surface_buffer_[(y * width + x) * 4];

// Assume everything not compiled with gcc to be on a little endian platform.
#if defined(__GNUC__) && defined(__BIG_ENDIAN__)
				const unsigned char alpha = *(pixel + 0);
#else
				const unsigned char alpha = *(pixel + 3);
#endif
				if(alpha == 0) {
					continue;
				}

// Assume everything not compiled with gcc to be on a little endian platform.
#if defined(__GNUC__) && defined(__BIG_ENDIAN__)
				decode_pixel(alpha, pixel + 3);
#else
				decode_pixel(alpha, pixel + 0);
#endif
				decode_pixel(alpha, pixel + 1);
				decode_pixel(alpha, pixel + 2);
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

} // namespace internal

} // namespace font

