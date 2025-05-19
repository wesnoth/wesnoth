/*
	Copyright (C) 2021 - 2025
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

#pragma once

/**
 * @file
 * Transitional API for porting SDL_ttf-based code to Pango. Do NOT use in new code!
 *
 * @note GUI1 markup is not supported by this transitional API for cost-benefit reasons.
 * Not only does implementing it require a lot more work to go over text line by line,
 * it also had major design flaws -- namely, only applying to whole lines with variable
 * spans that would be decided by the layout algorithm depending on available space,
 * rather than on a physical line basis (markup start till EOL) or fixed span basis (e.g.
 * the special markup used by the Help browser, or Pango markup).
 */

#include "font/text.hpp"

struct rect;
class texture;

namespace font {

/**
 * Returns a SDL texture containing the rendered text.
 */
texture pango_render_text(const std::string& text, int size, const color_t& color, font::pango_text::FONT_STYLE style = font::pango_text::STYLE_NORMAL, bool use_markup = false, int max_width = -1);

/**
 * Determine the width and height of a line of text given a certain font size.
 */
std::pair<int, int> pango_line_size(const std::string& line, int font_size, font::pango_text::FONT_STYLE font_style = font::pango_text::STYLE_NORMAL);

/**
 * Uses Pango to word wrap text.
 */
std::string pango_word_wrap(const std::string& unwrapped_text, int font_size, int max_width, int max_height = -1, int max_lines = -1, bool partial_line = false);

/**
 * Draws text on the screen.
 *
 * The text will be clipped to area.  If the text runs outside of area
 * horizontally, an ellipsis will be displayed at the end of it.
 * If area is empty, the text will not be clipped.
 *
 * A bounding rectangle of the text is returned. If actually_draw is true
 * the text will also be drawn to the screen. Otherwise only the bounding
 * rectangle is returned.
 */
rect pango_draw_text(bool actually_draw, const rect& area, int size, const color_t& color, const std::string& text, int x, int y);

} // end namespace font
