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

#pragma once

/**
 * @file
 * Transitional API for porting SDL_ttf-based code to Pango. Do NOT use in new code!
 */

#include "font/text.hpp"

namespace font {

/**
 * Returns a SDL surface containing the rendered text.
 */
surface pango_render_text(const std::string& text, int size, const color_t& color, font::pango_text::FONT_STYLE style, bool use_markup = false, int max_width = -1);

/**
 * Determine the width and height of a line of text given a certain font size.
 */
std::pair<int, int> pango_line_size(const std::string& line, int font_size, font::pango_text::FONT_STYLE font_style);

/**
 * Determine the width of a line of text given a certain font size.
 */
inline int pango_line_width(const std::string& line, int font_size, font::pango_text::FONT_STYLE font_style)
{
	return pango_line_size(line, font_size, font_style).first;
}

/**
 * If the text exceeds the specified max width, end it with an ellipsis (...)
 */
std::string pango_line_ellipsize(const std::string& text, int font_size, int max_width, font::pango_text::FONT_STYLE font_style = font::pango_text::STYLE_NORMAL);

} // end namespace font
