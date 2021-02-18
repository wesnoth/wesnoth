/*
   Copyright (C) 2021 by Iris Morelle <shadowm@wesnoth.org>
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

#include "log.hpp"
#include "serialization/unicode.hpp"

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace font {

surface pango_render_text(const std::string& text, int size, const color_t& color, font::pango_text::FONT_STYLE style, bool use_markup, int max_width)
{
	font::pango_text ptext;
	ptext.set_text(text, use_markup);
	ptext.set_font_size(size)
		 .set_font_style(style)
		 .set_foreground_color(color);

	if(max_width > 0) {
		ptext.set_maximum_width(max_width)
			 .set_ellipse_mode(PANGO_ELLIPSIZE_END);
	}

	return ptext.render().clone();
}

std::pair<int, int> pango_line_size(const std::string& line, int font_size, font::pango_text::FONT_STYLE font_style)
{
	font::pango_text ptext;
	ptext.set_text(line, false);
	ptext.set_font_size(font_size)
		 .set_font_style(font_style)
		 .set_maximum_height(-1, false);

	auto& s = ptext.render();
	return { s->w, s->h };
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
		WRN_FT << "Invalid UTF-8 string: \"" << text << "\"\n";
		return "";
	}

	return text; // Should not happen
}

} // end namespace font
