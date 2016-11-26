/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "font/text.hpp"

#include "font/font_config.hpp"

#include "font/pango/escape.hpp"
#include "font/pango/font.hpp"
#include "font/pango/hyperlink.hpp"
#include "font/pango/iter.hpp"
#include "font/pango/stream_ops.hpp"

#include "gettext.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/core/log.hpp"
#include "gui/core/point.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "preferences.hpp"

#include <cassert>
#include <cstring>

#include "video.hpp"

namespace font {

pango_text::pango_text()
#if PANGO_VERSION_CHECK(1,22,0)
	: context_(pango_font_map_create_context(pango_cairo_font_map_get_default()))
#else
	: context_(pango_cairo_font_map_create_context((
		reinterpret_cast<PangoCairoFontMap*>(pango_cairo_font_map_get_default()))))
#endif
	, layout_(pango_layout_new(context_))
	, rect_()
	, surface_()
	, text_()
	, markedup_text_(false)
	, link_aware_(false)
	, link_color_()
	, font_class_(font::FONT_SANS_SERIF)
	, font_size_(14)
	, font_style_(STYLE_NORMAL)
	, foreground_color_(0xFFFFFFFF) // solid white
	, maximum_width_(-1)
	, characters_per_line_(0)
	, maximum_height_(-1)
	, ellipse_mode_(PANGO_ELLIPSIZE_END)
	, alignment_(PANGO_ALIGN_LEFT)
	, maximum_length_(std::string::npos)
	, calculation_dirty_(true)
	, length_(0)
	, surface_dirty_(true)
	, surface_buffer_()
{
	// With 72 dpi the sizes are the same as with SDL_TTF so hardcoded.
	pango_cairo_context_set_resolution(context_, 72.0);

	pango_layout_set_ellipsize(layout_, ellipse_mode_);
	pango_layout_set_alignment(layout_, alignment_);
	pango_layout_set_wrap(layout_, PANGO_WRAP_WORD_CHAR);

	/*
	 * Set the pango spacing a bit bigger since the default is deemed to small
	 * http://www.wesnoth.org/forum/viewtopic.php?p=358832#p358832
	 */
	pango_layout_set_spacing(layout_, 4 * PANGO_SCALE);

	cairo_font_options_t *fo = cairo_font_options_create();
	cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_FULL);
	cairo_font_options_set_hint_metrics(fo, CAIRO_HINT_METRICS_ON);
	cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_DEFAULT);

	pango_cairo_context_set_font_options(context_, fo);
	cairo_font_options_destroy(fo);
}

pango_text::~pango_text()
{
	if(context_) {
		g_object_unref(context_);
	}
	if(layout_) {
		g_object_unref(layout_);
	}
	surface_.assign(nullptr);
}

surface pango_text::render() const
{
	this->rerender();
	return surface_;
}


int pango_text::get_width() const
{
	return this->get_size().x;
}

int pango_text::get_height() const
{
	return this->get_size().y;
}

gui2::point pango_text::get_size() const
{
	this->recalculate();

	return gui2::point(rect_.width, rect_.height);
}

bool pango_text::is_truncated() const
{
	this->recalculate();

	return (pango_layout_is_ellipsized(layout_) != 0);
}

unsigned pango_text::insert_text(const unsigned offset, const std::string& text)
{
	if (text.empty() || length_ == maximum_length_) {
		return 0;
	}

	// do we really need that assert? utf8::insert will just append in this case, which seems fine
	assert(offset <= length_);

	unsigned len = utf8::size(text);
	if (length_ + len > maximum_length_) {
		len = maximum_length_ - length_;
	}
	const utf8::string insert = text.substr(0, utf8::index(text, len));
	utf8::string tmp = text_;
	this->set_text(utf8::insert(tmp, offset, insert), false);
	// report back how many characters were actually inserted (e.g. to move the cursor selection)
	return len;
}

bool pango_text::insert_unicode(const unsigned offset, ucs4::char_t unicode)
{
	return this->insert_unicode(offset, ucs4::string(1, unicode)) == 1;
}

unsigned pango_text::insert_unicode(const unsigned offset, const ucs4::string& unicode)
{
	const utf8::string insert = unicode_cast<utf8::string>(unicode);
	return this->insert_text(offset, insert);
}

gui2::point pango_text::get_cursor_position(
		const unsigned column, const unsigned line) const
{
	this->recalculate();

	// First we need to determine the byte offset, if more routines need it it
	// would be a good idea to make it a separate function.
	p_itor itor{layout_};

	// Go the wanted line.
	if(line != 0) {
		if(pango_layout_get_line_count(layout_) >= static_cast<int>(line)) {
			return gui2::point(0, 0);
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
			// We are beyond data.
			return gui2::point(0, 0);
		}
	}

	// Get the byte offset
	const int offset = pango_layout_iter_get_index(itor);

	// Convert the byte offset in a position.
	PangoRectangle rect;
	pango_layout_get_cursor_pos(layout_, offset, &rect, nullptr);

	return gui2::point(PANGO_PIXELS(rect.x), PANGO_PIXELS(rect.y));
}

std::string pango_text::get_token(const gui2::point & position, const char * delim) const
{
	this->recalculate();

	// Get the index of the character.
	int index, trailing;
	if (!pango_layout_xy_to_index(layout_, position.x * PANGO_SCALE,
		position.y * PANGO_SCALE, &index, &trailing)) {
		return "";
	}

	std::string txt = pango_layout_get_text(layout_);

	std::string d(delim);

	if (index < 0 || (static_cast<size_t>(index) >= txt.size()) || d.find(txt.at(index)) != std::string::npos) {
		return ""; // if the index is out of bounds, or the index character is a delimiter, return nothing
	}

	size_t l = index;
	while (l > 0 && (d.find(txt.at(l-1)) == std::string::npos)) {
		--l;
	}

	size_t r = index + 1;
	while (r < txt.size() && (d.find(txt.at(r)) == std::string::npos)) {
		++r;
	}

	return txt.substr(l,r-l);
}

std::string pango_text::get_link(const gui2::point & position) const
{
	if (!link_aware_) {
		return "";
	}

	std::string tok = this->get_token(position, " \n\r\t");

	if (looks_like_url(tok)) {
		return tok;
	} else {
		return "";
	}
}

gui2::point pango_text::get_column_line(const gui2::point& position) const
{
	this->recalculate();

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
		const int pos = this->get_cursor_position(i, line).x;

		if(pos == offset) {
			return  gui2::point(i, line);
		}
	}
}

bool pango_text::set_text(const std::string& text, const bool markedup)
{
	if(markedup != markedup_text_ || text != text_) {
		// assert(layout_);

		const ucs4::string wide = unicode_cast<ucs4::string>(text);
		const std::string narrow = unicode_cast<utf8::string>(wide);
		if(text != narrow) {
			ERR_GUI_L << "pango_text::" << __func__
					<< " text '" << text
					<< "' contains invalid utf-8, trimmed the invalid parts.\n";
		}
		if(markedup) {
			if(!this->set_markup(narrow)) {
				return false;
			}
		} else {
			/*
			 * pango_layout_set_text after pango_layout_set_markup might
			 * leave the layout in an undefined state regarding markup so
			 * clear it unconditionally.
			 */
			pango_layout_set_attributes(layout_, nullptr);
			pango_layout_set_text(layout_, narrow.c_str(), narrow.size());
		}
		text_ = narrow;
		length_ = wide.size();
		markedup_text_ = markedup;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return true;
}

pango_text& pango_text::set_family_class(font::family_class fclass)
{
	if(fclass != font_class_) {
		font_class_ = fclass;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_font_size(const unsigned font_size)
{
	unsigned int actual_size = preferences::font_scaled(font_size);
	if(actual_size != font_size_) {
		font_size_ = actual_size;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_font_style(const pango_text::FONT_STYLE font_style)
{
	if(font_style != font_style_) {
		font_style_ = font_style;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_foreground_color(const Uint32 color)
{
	if(color != foreground_color_) {
		foreground_color_ = color;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text &pango_text::set_foreground_color(const SDL_Color color)
{
	return this->set_foreground_color((color.r << 24) + (color.g << 16) + (color.b << 8) + color.a);
}

pango_text& pango_text::set_maximum_width(int width)
{
	if(width <= 0) {
		width = -1;
	}

	if(width != maximum_width_) {
		// assert(context_);
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

pango_text& pango_text::set_characters_per_line(const unsigned characters_per_line)
{
	if(characters_per_line != characters_per_line_) {
		characters_per_line_ = characters_per_line;

		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_maximum_height(int height, bool multiline)
{
	if(height <= 0) {
		height = -1;
		multiline = false;
	}

	if(height != maximum_height_) {
		// assert(context_);

		pango_layout_set_height(layout_, !multiline ? -1 : height * PANGO_SCALE);
		maximum_height_ = height;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_ellipse_mode(const PangoEllipsizeMode ellipse_mode)
{
	if(ellipse_mode != ellipse_mode_) {
		// assert(context_);

		pango_layout_set_ellipsize(layout_, ellipse_mode);
		ellipse_mode_ = ellipse_mode;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text &pango_text::set_alignment(const PangoAlignment alignment)
{
	if (alignment != alignment_) {
		pango_layout_set_alignment(layout_, alignment);
		alignment_ = alignment;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_maximum_length(const size_t maximum_length)
{
	if(maximum_length != maximum_length_) {
		maximum_length_ = maximum_length;
		if(length_ > maximum_length_) {
			utf8::string tmp = text_;
			this->set_text(utf8::truncate(tmp, maximum_length_), false);
		}
	}

	return *this;
}

pango_text& pango_text::set_link_aware(bool b)
{
	if (link_aware_ != b) {
		calculation_dirty_ = true;
		surface_dirty_ = true;
		link_aware_ = b;
	}
	return *this;
}

pango_text& pango_text::set_link_color(const std::string & color)
{
	if(color != link_color_) {
		link_color_ = color;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}


void pango_text::recalculate(const bool force) const
{
	if(calculation_dirty_ || force) {
		// assert(layout_);

		calculation_dirty_ = false;
		surface_dirty_ = true;

		p_font font{get_font_families(font_class_), font_size_, font_style_};
		pango_layout_set_font_description(layout_, font.get());

		if(font_style_ & pango_text::STYLE_UNDERLINE) {
			PangoAttrList *attribute_list = pango_attr_list_new();
			pango_attr_list_insert(attribute_list
					, pango_attr_underline_new(PANGO_UNDERLINE_SINGLE));

			pango_layout_set_attributes (layout_, attribute_list);
			pango_attr_list_unref(attribute_list);
		}

		int maximum_width = 0;
		if(characters_per_line_ != 0) {
			PangoFont* f = pango_font_map_load_font(
					  pango_cairo_font_map_get_default()
					, context_
					, font.get());

			PangoFontMetrics* m = pango_font_get_metrics(f, nullptr);

			int w = pango_font_metrics_get_approximate_char_width(m);
			w *= characters_per_line_;

			maximum_width = ceil(pango_units_to_double(w));
		} else {
			maximum_width = maximum_width_;
		}

		if(maximum_width_ != -1) {
			maximum_width = std::min(maximum_width, maximum_width_);
		}

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
			pango_layout_set_width(layout_, maximum_width == -1
					? -1
					: (maximum_width + hack) * PANGO_SCALE);
			pango_layout_get_pixel_extents(layout_, nullptr, &rect_);

			DBG_GUI_L << "pango_text::" << __func__
					<< " text '" << gui2::debug_truncate(text_)
					<< "' maximum_width " << maximum_width
					<< " hack " << hack
					<< " width " << rect_.x + rect_.width
					<< ".\n";

			--hack;
		} while(maximum_width != -1
				&& hack >= 0 && rect_.x + rect_.width > maximum_width);

		DBG_GUI_L << "pango_text::" << __func__
				<< " text '" << gui2::debug_truncate(text_)
				<< "' font_size " << font_size_
				<< " markedup_text " << markedup_text_
				<< " font_style " << std::hex << font_style_ << std::dec
				<< " maximum_width " << maximum_width
				<< " maximum_height " << maximum_height_
				<< " result " <<  rect_
				<< ".\n";
		if(maximum_width != -1 && rect_.x + rect_.width > maximum_width) {
			DBG_GUI_L << "pango_text::" << __func__
					<< " text '" << gui2::debug_truncate(text_)
					<< " ' width " << rect_.x + rect_.width
					<< " greater as the wanted maximum of " << maximum_width
					<< ".\n";
		}
	}
}

/***
 * Inverse table
 *
 * Holds a high-precision inverse for each number i, that is, a number x such that x * i / 256 is close to 255.
 */
struct inverse_table
{
	unsigned values[256];

	inverse_table()
	{
		values[0] = 0;
		for (int i = 1; i < 256; ++i) {
			values[i] = (255 * 256) / i;
		}
	}

	unsigned operator[](Uint8 i) const { return values[i]; }
};

static const inverse_table inverse_table_;

/***
 * Helper function for un-premultiplying alpha
 * Div should be the high-precision inverse for the alpha value.
 */
static void unpremultiply(Uint8 & value, const unsigned div) {
	unsigned temp = (value * div) / 256u;
	// Note: It's always the case that alpha * div < 256 if div is the inverse
	// for alpha, so if cairo is computing premultiplied alpha by rounding down,
	// this min is not necessary. However, if cairo generates illegal output,
	// the min may be selected.
	// It's probably not worth removing the min, since branch prediction will
	// make it essentially free if one of the branches is never actually
	// selected.
	value = std::min(255u, temp);
}

/**
 * Converts from cairo-format ARGB32 premultiplied alpha to plain alpha.
 * @param c a uint32 representing the color
 */
static void from_cairo_format(Uint32 & c)
{
	Uint8 a = (c >> 24) & 0xff;
	Uint8 r = (c >> 16) & 0xff;
	Uint8 g = (c >> 8) & 0xff;
	Uint8 b = c & 0xff;

	const unsigned div = inverse_table_[a];
	unpremultiply(r, div);
	unpremultiply(g, div);
	unpremultiply(b, div);

	c = (static_cast<Uint32>(a) << 24) | (static_cast<Uint32>(r) << 16) | (static_cast<Uint32>(g) << 8) | static_cast<Uint32>(b);
}

void pango_text::rerender(const bool force) const
{
	if(surface_dirty_ || force) {
		// assert(layout_);

		this->recalculate(force);
		surface_dirty_ = false;

		// TODO: This looks broken to me. If rect_.x is negative, shouldn't that increase width?
		// I think it maybe should be
		// int width  = (-rect_.x) + rect_.width;
		int width  = rect_.x + rect_.width;
		int height = rect_.y + rect_.height;
		if(maximum_width_  > 0) { width  = std::min(width, maximum_width_); }
		if(maximum_height_ > 0) { height = std::min(height, maximum_height_); }

		cairo_format_t format = CAIRO_FORMAT_ARGB32;
		const unsigned stride = cairo_format_stride_for_width(format, width);

		this->create_surface_buffer(stride * height);

		if (!surface_buffer_.size()) {
			// surface_.assign(nullptr);
			surface_.assign(create_neutral_surface(0, 0));
			return;
		}

		cairo_surface_t* cairo_surface = cairo_image_surface_create_for_data(&surface_buffer_[0], format, width, height, stride);
		cairo_t* cr = cairo_create(cairo_surface);

		/* set color (used for foreground). */
		cairo_set_source_rgba(cr,
			 (foreground_color_ >> 24)         / 256.0,
			((foreground_color_ >> 16) & 0xFF) / 256.0,
			((foreground_color_ >> 8)  & 0xFF) / 256.0,
			(foreground_color_         & 0xFF) / 256.0);

		pango_cairo_show_layout(cr, layout_);

		static_assert(sizeof(Uint32) == 4, "Something is wrong with our typedefs");

		// The cairo surface is in CAIRO_FORMAT_ARGB32 which uses
		// pre-multiplied alpha. SDL doesn't use that so the pixels need to be
		// decoded again.
		Uint32 * pixels = reinterpret_cast<Uint32 *>(&surface_buffer_[0]);

		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < width; ++x) {
				from_cairo_format(pixels[y * width + x]);
			}
		}

		surface_.assign(SDL_CreateRGBSurfaceFrom(
			&surface_buffer_[0], width, height, 32, stride, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
		cairo_destroy(cr);
		cairo_surface_destroy(cairo_surface);
	}
}

void pango_text::create_surface_buffer(const size_t size) const
{
	// clear old buffer
	surface_.assign(nullptr);

	surface_buffer_.resize(size);

	// Not sure why this is needed, perhaps SDL assumes it?
	for (auto & c : surface_buffer_) { c = 0; }
}

bool pango_text::set_markup(const std::string & text) {
	return this->set_markup_helper(link_aware_ ? this->format_link_tokens(text) : text);
}

std::string pango_text::format_link_tokens(const std::string & text) const {
	std::string delim = " \n\r\t";
	// Tokenize according to these delimiters, and stream the results of `handle_token` on each token to get the new text.

	std::string result;

	int last_delim = -1;
	for (size_t index = 0; index < text.size(); ++index) {
		if (delim.find(text.at(index)) != std::string::npos) {
			// want to include chars from range since last token, dont want to include any delimiters
			result += this->handle_token(text.substr(last_delim + 1, index - last_delim - 1));
			result += text.at(index);
			last_delim = index;
		}
	}
	if (last_delim < static_cast<int>(text.size()) - 1) {
		result += this->handle_token(text.substr(last_delim + 1, text.size() - last_delim - 1));
	}

	return result;
}

std::string pango_text::handle_token(const std::string & token) const
{
	if (looks_like_url(token)) {
		return format_as_link(token, link_color_);
	} else {
		return token;
	}
}

bool pango_text::set_markup_helper(const std::string& text)
{
	if(pango_parse_markup(text.c_str(), text.size()
			, 0, nullptr, nullptr, nullptr, nullptr)) {

		/* Markup is valid so set it. */
		pango_layout_set_markup(layout_, text.c_str(), text.size());
		return true;
	}

	/*
	 * The markup is invalid. Try to recover.
	 *
	 * The pango engine tested seems to accept stray single quotes »'« and
	 * double quotes »"«. Stray ampersands »&« seem to give troubles.
	 * So only try to recover from broken ampersands, by simply replacing them
	 * with the escaped version.
	 */
	std::string semi_escaped{semi_escape_text(text)};

	/*
	 * If at least one ampersand is replaced the semi-escaped string
	 * is longer than the original. If this isn't the case then the
	 * markup wasn't (only) broken by ampersands in the first place.
	 */
	if(text.size() == semi_escaped.size()
			|| !pango_parse_markup(semi_escaped.c_str(), semi_escaped.size()
				, 0, nullptr, nullptr, nullptr, nullptr)) {

		/* Fixing the ampersands didn't work. */
		ERR_GUI_L << "pango_text::" << __func__
				<< " text '" << text
				<< "' has broken markup, set to normal text.\n";

		this->set_text(_("The text contains invalid markup: ") + text, false);
		return false;
	}

	/* Replacement worked, still warn the user about the error. */
	ERR_GUI_L << "pango_text::" << __func__
			<< " text '" << text
			<< "' has unescaped ampersands '&', escaped them.\n";

	pango_layout_set_markup(layout_, semi_escaped.c_str(), semi_escaped.size());
	return true;
}

} // namespace font
