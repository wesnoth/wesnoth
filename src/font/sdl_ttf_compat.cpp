/*
	Copyright (C) 2021 - 2024
	by Iris Morelle <shadowm@wesnoth.org>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "font/sdl_ttf_compat.hpp"

#include "draw.hpp"
#include "font/standard_colors.hpp"
#include "log.hpp"
#include "sdl/point.hpp"
#include "sdl/texture.hpp"
#include "sdl/utils.hpp"
#include "serialization/unicode.hpp"
#include "tooltips.hpp"

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace font {

namespace {

pango_text& private_renderer()
{
	// Use a separate renderer for GUI1 in case of shenanigans.
	static pango_text text_renderer;
	return text_renderer;
}

}

texture pango_render_text(const std::string& text, int size, const color_t& color, font::pango_text::FONT_STYLE style, bool use_markup, int max_width)
{
	auto& ptext = private_renderer();

	ptext.set_text(text, use_markup);
	ptext.set_family_class(font::FONT_SANS_SERIF)
		 .set_font_size(size)
		 .set_font_style(style)
		 .set_maximum_height(-1, false)
		 .set_foreground_color(color)
		 .set_maximum_width(max_width)
		 .set_ellipse_mode(max_width > 0 ? PANGO_ELLIPSIZE_END : PANGO_ELLIPSIZE_NONE);

	return ptext.render_and_get_texture();
}

std::pair<int, int> pango_line_size(const std::string& line, int font_size, font::pango_text::FONT_STYLE font_style)
{
	auto& ptext = private_renderer();

	ptext.set_text(line, false);
	ptext.set_family_class(font::FONT_SANS_SERIF)
		 .set_font_size(font_size)
		 .set_font_style(font_style)
		 .set_maximum_height(-1, false)
		 .set_maximum_width(-1)
		 .set_ellipse_mode(PANGO_ELLIPSIZE_NONE);

	auto s = ptext.get_size();
	return { s.x, s.y };
}

std::string pango_line_ellipsize(const std::string& text, int font_size, int max_width, font::pango_text::FONT_STYLE font_style)
{
	if(pango_line_width(text, font_size, font_style) <= max_width) {
		return text;
	}
	if(pango_line_width(font::ellipsis, font_size, font_style) > max_width) {
		return "";
	}

	std::string current_substring;

	try {
		utf8::iterator itor(text);
		for(; itor != utf8::iterator::end(text); ++itor) {
			std::string tmp = current_substring;
			tmp.append(itor.substr().first, itor.substr().second);

			if(pango_line_width(tmp + font::ellipsis, font_size, font_style) > max_width) {
				return current_substring + font::ellipsis;
			}

			current_substring = std::move(tmp);
		}
	} catch(const utf8::invalid_utf8_exception&) {
		WRN_FT << "Invalid UTF-8 string: \"" << text << "\"";
		return "";
	}

	return text; // Should not happen
}

std::string pango_word_wrap(const std::string& unwrapped_text, int font_size, int max_width, int max_height, int max_lines, bool /*partial_line*/)
{
	// FIXME: what the hell does partial_line do in the SDL_ttf version?

	if(max_lines == 0) {
		return "";
	}

	auto& ptext = private_renderer();

	ptext.set_text(unwrapped_text, false);
	ptext.set_family_class(font::FONT_SANS_SERIF)
		 .set_font_size(font_size)
		 .set_font_style(font::pango_text::STYLE_NORMAL)
		 .set_maximum_height(max_height, true)
		 .set_maximum_width(max_width)
		 .set_ellipse_mode(PANGO_ELLIPSIZE_NONE);

	std::string res;
	const auto& lines = ptext.get_lines();

	for(const auto& line : lines) {
		if(!res.empty())
			res += '\n';
		res += line;
		if(--max_lines == 0)
			break;
	}

	return res;
}

rect pango_draw_text(bool actually_draw, const rect& area, int size, const color_t& color, const std::string& text, int x, int y, bool use_tooltips, pango_text::FONT_STYLE style)
{
	auto& ptext = private_renderer();

	ptext.set_text(text, false);
	ptext.set_family_class(font::FONT_SANS_SERIF)
		 .set_font_size(size)
		 .set_font_style(style)
		 .set_maximum_width(-1)
		 .set_foreground_color(color)
		 .set_ellipse_mode(PANGO_ELLIPSIZE_END);

	if(!area.empty()) {
		ptext.set_maximum_height(area.h, true);
	}

	auto extents = ptext.get_size();
	bool ellipsized = false;

	if(!area.empty() && extents.x > area.w) {
		ptext.set_maximum_width(area.w);
		ellipsized = true;
	}

	auto t = ptext.render_and_get_texture();

	SDL_Rect res = {x, y, t.w(), t.h()};

	if(actually_draw) {
		draw::blit(t, res);
	}

	if(ellipsized && use_tooltips) {
		tooltips::add_tooltip(res, text);
	}

	return res;
}

} // end namespace font
