/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "font/pango/stream_ops.hpp"

#include "gettext.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/core/log.hpp"
#include "sdl/point.hpp"
#include "sdl/utils.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "preferences/general.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace font {

pango_text::pango_text()
	: context_(pango_font_map_create_context(pango_cairo_font_map_get_default()), g_object_unref)
	, layout_(pango_layout_new(context_.get()), g_object_unref)
	, rect_()
	, surface_()
	, text_()
	, markedup_text_(false)
	, link_aware_(false)
	, link_color_()
	, font_class_(font::FONT_SANS_SERIF)
	, font_size_(14)
	, font_style_(STYLE_NORMAL)
	, foreground_color_() // solid white
	, add_outline_(false)
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
	pango_cairo_context_set_resolution(context_.get(), 72.0);

	pango_layout_set_ellipsize(layout_.get(), ellipse_mode_);
	pango_layout_set_alignment(layout_.get(), alignment_);
	pango_layout_set_wrap(layout_.get(), PANGO_WRAP_WORD_CHAR);

	/*
	 * Set the pango spacing a bit bigger since the default is deemed to small
	 * https://www.wesnoth.org/forum/viewtopic.php?p=358832#p358832
	 */
	pango_layout_set_spacing(layout_.get(), 4 * PANGO_SCALE);

	cairo_font_options_t *fo = cairo_font_options_create();
	cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_FULL);
	cairo_font_options_set_hint_metrics(fo, CAIRO_HINT_METRICS_ON);
	cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_DEFAULT);

	pango_cairo_context_set_font_options(context_.get(), fo);
	cairo_font_options_destroy(fo);
}

surface& pango_text::render()
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

point pango_text::get_size() const
{
	this->recalculate();

	return point(rect_.width, rect_.height);
}

bool pango_text::is_truncated() const
{
	this->recalculate();

	return (pango_layout_is_ellipsized(layout_.get()) != 0);
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
	const std::string insert = text.substr(0, utf8::index(text, len));
	std::string tmp = text_;
	this->set_text(utf8::insert(tmp, offset, insert), false);
	// report back how many characters were actually inserted (e.g. to move the cursor selection)
	return len;
}

bool pango_text::insert_unicode(const unsigned offset, char32_t unicode)
{
	return this->insert_unicode(offset, std::u32string(1, unicode)) == 1;
}

unsigned pango_text::insert_unicode(const unsigned offset, const std::u32string& unicode)
{
	const std::string insert = unicode_cast<std::string>(unicode);
	return this->insert_text(offset, insert);
}

point pango_text::get_cursor_position(
		const unsigned column, const unsigned line) const
{
	this->recalculate();

	// First we need to determine the byte offset, if more routines need it it
	// would be a good idea to make it a separate function.
	std::unique_ptr<PangoLayoutIter, std::function<void(PangoLayoutIter*)>> itor(
		pango_layout_get_iter(layout_.get()), pango_layout_iter_free);

	// Go the wanted line.
	if(line != 0) {
		if(pango_layout_get_line_count(layout_.get()) >= static_cast<int>(line)) {
			return point(0, 0);
		}

		for(std::size_t i = 0; i < line; ++i) {
			pango_layout_iter_next_line(itor.get());
		}
	}

	// Go the wanted column.
	for(std::size_t i = 0; i < column; ++i) {
		if(!pango_layout_iter_next_char(itor.get())) {
			// It seems that the documentation is wrong and causes and off by
			// one error... the result should be false if already at the end of
			// the data when started.
			if(i + 1 == column) {
				break;
			}
			// We are beyond data.
			return point(0, 0);
		}
	}

	// Get the byte offset
	const int offset = pango_layout_iter_get_index(itor.get());

	// Convert the byte offset in a position.
	PangoRectangle rect;
	pango_layout_get_cursor_pos(layout_.get(), offset, &rect, nullptr);

	return point(PANGO_PIXELS(rect.x), PANGO_PIXELS(rect.y));
}

std::size_t pango_text::get_maximum_length() const
{
	return maximum_length_;
}

std::string pango_text::get_token(const point & position, const char * delim) const
{
	this->recalculate();

	// Get the index of the character.
	int index, trailing;
	if (!pango_layout_xy_to_index(layout_.get(), position.x * PANGO_SCALE,
		position.y * PANGO_SCALE, &index, &trailing)) {
		return "";
	}

	std::string txt = pango_layout_get_text(layout_.get());

	std::string d(delim);

	if (index < 0 || (static_cast<std::size_t>(index) >= txt.size()) || d.find(txt.at(index)) != std::string::npos) {
		return ""; // if the index is out of bounds, or the index character is a delimiter, return nothing
	}

	std::size_t l = index;
	while (l > 0 && (d.find(txt.at(l-1)) == std::string::npos)) {
		--l;
	}

	std::size_t r = index + 1;
	while (r < txt.size() && (d.find(txt.at(r)) == std::string::npos)) {
		++r;
	}

	return txt.substr(l,r-l);
}

std::string pango_text::get_link(const point & position) const
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

point pango_text::get_column_line(const point& position) const
{
	this->recalculate();

	// Get the index of the character.
	int index, trailing;
	pango_layout_xy_to_index(layout_.get(), position.x * PANGO_SCALE,
		position.y * PANGO_SCALE, &index, &trailing);

	// Extract the line and the offset in pixels in that line.
	int line, offset;
	pango_layout_index_to_line_x(layout_.get(), index, trailing, &line, &offset);
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
	for(std::size_t i = 0; ; ++i) {
		const int pos = this->get_cursor_position(i, line).x;

		if(pos == offset) {
			return  point(i, line);
		}
	}
}

bool pango_text::set_text(const std::string& text, const bool markedup)
{
	if(markedup != markedup_text_ || text != text_) {
		if(layout_ == nullptr) {
			layout_.reset(pango_layout_new(context_.get()));
		}

		const std::u32string wide = unicode_cast<std::u32string>(text);
		const std::string narrow = unicode_cast<std::string>(wide);
		if(text != narrow) {
			ERR_GUI_L << "pango_text::" << __func__
					<< " text '" << text
					<< "' contains invalid utf-8, trimmed the invalid parts.\n";
		}
		if(markedup) {
			if(!this->set_markup(narrow, *layout_)) {
				return false;
			}
		} else {
			/*
			 * pango_layout_set_text after pango_layout_set_markup might
			 * leave the layout in an undefined state regarding markup so
			 * clear it unconditionally.
			 */
			pango_layout_set_attributes(layout_.get(), nullptr);
			pango_layout_set_text(layout_.get(), narrow.c_str(), narrow.size());
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

pango_text& pango_text::set_foreground_color(const color_t& color)
{
	if(color != foreground_color_) {
		foreground_color_ = color;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_maximum_width(int width)
{
	if(width <= 0) {
		width = -1;
	}

	if(width != maximum_width_) {
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

		pango_layout_set_height(layout_.get(), !multiline ? -1 : height * PANGO_SCALE);
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

		pango_layout_set_ellipsize(layout_.get(), ellipse_mode);
		ellipse_mode_ = ellipse_mode;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text &pango_text::set_alignment(const PangoAlignment alignment)
{
	if (alignment != alignment_) {
		pango_layout_set_alignment(layout_.get(), alignment);
		alignment_ = alignment;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_maximum_length(const std::size_t maximum_length)
{
	if(maximum_length != maximum_length_) {
		maximum_length_ = maximum_length;
		if(length_ > maximum_length_) {
			std::string tmp = text_;
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

pango_text& pango_text::set_link_color(const color_t& color)
{
	if(color != link_color_) {
		link_color_ = color;
		calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

pango_text& pango_text::set_add_outline(bool do_add)
{
	if(do_add != add_outline_) {
		add_outline_ = do_add;
		//calculation_dirty_ = true;
		surface_dirty_ = true;
	}

	return *this;
}

void pango_text::recalculate(const bool force) const
{
	if(calculation_dirty_ || force) {
		assert(layout_ != nullptr);

		calculation_dirty_ = false;
		surface_dirty_ = true;

		rect_ = calculate_size(*layout_);
	}
}

PangoRectangle pango_text::calculate_size(PangoLayout& layout) const
{
	PangoRectangle size;

	p_font font{ get_font_families(font_class_), font_size_, font_style_ };
	pango_layout_set_font_description(&layout, font.get());

	if(font_style_ & pango_text::STYLE_UNDERLINE) {
		PangoAttrList *attribute_list = pango_attr_list_new();
		pango_attr_list_insert(attribute_list
			, pango_attr_underline_new(PANGO_UNDERLINE_SINGLE));

		pango_layout_set_attributes(&layout, attribute_list);
		pango_attr_list_unref(attribute_list);
	}

	int maximum_width = 0;
	if(characters_per_line_ != 0) {
		PangoFont* f = pango_font_map_load_font(
			pango_cairo_font_map_get_default(),
			context_.get(),
			font.get());

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

	pango_layout_set_width(&layout, maximum_width == -1
		? -1
		: maximum_width * PANGO_SCALE);
	pango_layout_get_pixel_extents(&layout, nullptr, &size);

	DBG_GUI_L << "pango_text::" << __func__
		<< " text '" << gui2::debug_truncate(text_)
		<< "' maximum_width " << maximum_width
		<< " width " << size.x + size.width
		<< ".\n";

	DBG_GUI_L << "pango_text::" << __func__
		<< " text '" << gui2::debug_truncate(text_)
		<< "' font_size " << font_size_
		<< " markedup_text " << markedup_text_
		<< " font_style " << std::hex << font_style_ << std::dec
		<< " maximum_width " << maximum_width
		<< " maximum_height " << maximum_height_
		<< " result " << size
		<< ".\n";
	if(maximum_width != -1 && size.x + size.width > maximum_width) {
		DBG_GUI_L << "pango_text::" << __func__
			<< " text '" << gui2::debug_truncate(text_)
			<< " ' width " << size.x + size.width
			<< " greater as the wanted maximum of " << maximum_width
			<< ".\n";
	}

	return size;
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

	unsigned operator[](uint8_t i) const { return values[i]; }
};

static const inverse_table inverse_table_;

/***
 * Helper function for un-premultiplying alpha
 * Div should be the high-precision inverse for the alpha value.
 */
static void unpremultiply(uint8_t & value, const unsigned div) {
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
static void from_cairo_format(uint32_t & c)
{
	uint8_t a = (c >> 24) & 0xff;
	uint8_t r = (c >> 16) & 0xff;
	uint8_t g = (c >> 8) & 0xff;
	uint8_t b = c & 0xff;

	const unsigned div = inverse_table_[a];
	unpremultiply(r, div);
	unpremultiply(g, div);
	unpremultiply(b, div);

	c = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

void pango_text::render(PangoLayout& layout, const PangoRectangle& rect, const std::size_t surface_buffer_offset, const unsigned stride)
{
	int width = rect.x + rect.width;
	int height = rect.y + rect.height;
	if(maximum_width_  > 0) { width = std::min(width, maximum_width_); }
	if(maximum_height_ > 0) { height = std::min(height, maximum_height_); }

	cairo_format_t format = CAIRO_FORMAT_ARGB32;

	uint8_t* buffer = &surface_buffer_[surface_buffer_offset];

	std::unique_ptr<cairo_surface_t, std::function<void(cairo_surface_t*)>> cairo_surface(
		cairo_image_surface_create_for_data(buffer, format, width, height, stride), cairo_surface_destroy);
	std::unique_ptr<cairo_t, std::function<void(cairo_t*)>> cr(cairo_create(cairo_surface.get()), cairo_destroy);

	if(cairo_status(cr.get()) == CAIRO_STATUS_INVALID_SIZE) {
		throw std::length_error("Text is too long to render");
	}

	//
	// TODO: the outline may be slightly cut off around certain text if it renders too
	// close to the surface's edge. That causes the outline to extend just slightly
	// outside the surface's borders. I'm not sure how best to deal with this. Obviously,
	// we want to increase the surface size, but we also don't want to invalidate all
	// the placement and size calculations. Thankfully, it's not very noticeable.
	//
	// -- vultraz, 2018-03-07
	//
	if(add_outline_) {
		// Add a path to the cairo context tracing the current text.
		pango_cairo_layout_path(cr.get(), &layout);

		// Set color for background outline (black).
		cairo_set_source_rgba(cr.get(), 0.0, 0.0, 0.0, 1.0);

		cairo_set_line_join(cr.get(), CAIRO_LINE_JOIN_ROUND);
		cairo_set_line_width(cr.get(), 3.0); // Adjust as necessary

		// Stroke path to draw outline.
		cairo_stroke(cr.get());
	}

	// Set main text color.
	cairo_set_source_rgba(cr.get(),
		foreground_color_.r / 255.0,
		foreground_color_.g / 255.0,
		foreground_color_.b / 255.0,
		foreground_color_.a / 255.0
	);

	pango_cairo_show_layout(cr.get(), &layout);
}

void pango_text::rerender(const bool force)
{
	if(surface_dirty_ || force) {
		assert(layout_.get());

		this->recalculate(force);
		surface_dirty_ = false;

		int width  = rect_.x + rect_.width;
		int height = rect_.y + rect_.height;
		if(maximum_width_  > 0) { width  = std::min(width, maximum_width_); }
		if(maximum_height_ > 0) { height = std::min(height, maximum_height_); }

		cairo_format_t format = CAIRO_FORMAT_ARGB32;
		const unsigned stride = cairo_format_stride_for_width(format, width);

		this->create_surface_buffer(stride * height);

		if (surface_buffer_.empty()) {
			surface_ = surface(0, 0);
			return;
		}

		try {
			// Try rendering the whole text in one go
			render(*layout_, rect_, 0u, stride);
		} catch (std::length_error&) {
			// Try rendering line-by-line, this is a workaround for cairo
			// surfaces being limited to approx 2**15 pixels in height. If this
			// also throws a length_error then leave it to the caller to
			// handle.
			std::size_t cumulative_height = 0u;

			auto start_of_line = text_.cbegin();
			while (start_of_line != text_.cend()) {
				auto end_of_line = std::find(start_of_line, text_.cend(), '\n');

				auto part_layout = std::unique_ptr<PangoLayout, std::function<void(void*)>> { pango_layout_new(context_.get()), g_object_unref};
				auto line = utils::string_view(&*start_of_line, std::distance(start_of_line, end_of_line));
				set_markup(line, *part_layout);
				copy_layout_properties(*layout_, *part_layout);

				auto part_rect = calculate_size(*part_layout);
				render(*part_layout, part_rect, cumulative_height * stride, stride);
				cumulative_height += part_rect.height;

				start_of_line = end_of_line;
				if (start_of_line != text_.cend()) {
					// skip over the \n
					++start_of_line;
				}
			}
		}

		// The cairo surface is in CAIRO_FORMAT_ARGB32 which uses
		// pre-multiplied alpha. SDL doesn't use that so the pixels need to be
		// decoded again.
		uint32_t * pixels = reinterpret_cast<uint32_t *>(&surface_buffer_[0]);

		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < width; ++x) {
				from_cairo_format(pixels[y * width + x]);
			}
		}

#if SDL_VERSION_ATLEAST(2, 0, 6)
		surface_ = SDL_CreateRGBSurfaceWithFormatFrom(
			&surface_buffer_[0], width, height, 32, stride, SDL_PIXELFORMAT_ARGB8888);
#else
		surface_ = SDL_CreateRGBSurfaceFrom(
			&surface_buffer_[0], width, height, 32, stride, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
#endif
	}
}

void pango_text::create_surface_buffer(const std::size_t size) const
{
	// Clear surface.
	surface_ = nullptr;

	// Resize buffer appropriately and clear all existing data (essentially sets all pixel values to 0).
	surface_buffer_.assign(size, 0);
}

bool pango_text::set_markup(utils::string_view text, PangoLayout& layout) {
	char* raw_text;
	std::string semi_escaped;
	bool valid = validate_markup(text, &raw_text, semi_escaped);
	if(semi_escaped != "") {
		text = semi_escaped;
	}

	if(valid) {
		if(link_aware_) {
			std::vector<std::string> links = find_links(raw_text);
			std::string final_text = text.to_string();
			format_links(final_text, links);
			pango_layout_set_markup(&layout, final_text.c_str(), final_text.size());
		} else {
			pango_layout_set_markup(&layout, text.data(), text.size());
		}
	} else {
		ERR_GUI_L << "pango_text::" << __func__
			<< " text '" << text
			<< "' has broken markup, set to normal text.\n";
		set_text(_("The text contains invalid Pango markup: ") + text.to_string(), false);
	}

	return valid;
}

std::vector<std::string> pango_text::find_links(utils::string_view text) const {
	const std::string delim = " \n\r\t";
	std::vector<std::string> links;

	int last_delim = -1;
	for (std::size_t index = 0; index < text.size(); ++index) {
		if (delim.find(text[index]) != std::string::npos) {
			// want to include chars from range since last token, don't want to include any delimiters
			utils::string_view token = text.substr(last_delim + 1, index - last_delim - 1);
			if(looks_like_url(token)) {
				links.push_back(token.to_string());
			}
			last_delim = index;
		}
	}
	if (last_delim < static_cast<int>(text.size()) - 1) {
		utils::string_view token = text.substr(last_delim + 1, text.size() - last_delim - 1);
		if(looks_like_url(token)) {
			links.push_back(token.to_string());
		}
	}

	return links;
}

void pango_text::format_links(std::string& text, const std::vector<std::string>& links) const
{
	for(const std::string& link : links) {
		boost::algorithm::replace_first(text, link, format_as_link(link, link_color_));
	}
}

bool pango_text::validate_markup(utils::string_view text, char** raw_text, std::string& semi_escaped) const
{
	if(pango_parse_markup(text.data(), text.size(),
		0, nullptr, raw_text, nullptr, nullptr)) {
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
	semi_escaped = semi_escape_text(text.to_string());

	/*
	 * If at least one ampersand is replaced the semi-escaped string
	 * is longer than the original. If this isn't the case then the
	 * markup wasn't (only) broken by ampersands in the first place.
	 */
	if(text.size() == semi_escaped.size()
			|| !pango_parse_markup(semi_escaped.c_str(), semi_escaped.size()
				, 0, nullptr, raw_text, nullptr, nullptr)) {

		/* Fixing the ampersands didn't work. */
		return false;
	}

	/* Replacement worked, still warn the user about the error. */
	WRN_GUI_L << "pango_text::" << __func__
			<< " text '" << text
			<< "' has unescaped ampersands '&', escaped them.\n";

	return true;
}

void pango_text::copy_layout_properties(PangoLayout& src, PangoLayout& dst)
{
	pango_layout_set_alignment(&dst, pango_layout_get_alignment(&src));
	pango_layout_set_height(&dst, pango_layout_get_height(&src));
	pango_layout_set_ellipsize(&dst, pango_layout_get_ellipsize(&src));
}

pango_text& get_text_renderer()
{
	static pango_text text_renderer;
	return text_renderer;
}

} // namespace font
