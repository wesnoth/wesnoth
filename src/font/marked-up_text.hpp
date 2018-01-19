/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

#include "color.hpp"

class CVideo;
class surface;

#include <string>
#include "serialization/unicode_types.hpp"

#include <SDL_rect.h>

namespace font {

/** Standard markups for color, size, font, images. */
extern const char LARGE_TEXT, SMALL_TEXT, BOLD_TEXT, NORMAL_TEXT, NULL_MARKUP, BLACK_TEXT, GRAY_TEXT,
                  GOOD_TEXT, BAD_TEXT, GREEN_TEXT, RED_TEXT, COLOR_TEXT, IMAGE;

/** Parses the markup-tags at the front of a string. */
std::string::const_iterator parse_markup(std::string::const_iterator i1,
												std::string::const_iterator i2,
												int* font_size,
												color_t* color, int* style);

/**
 * Function to draw text on a surface.
 *
 * The text will be clipped to area.  If the text runs outside of area
 * horizontally, an ellipsis will be displayed at the end of it.
 *
 * If use_tooltips is true, then text with an ellipsis will have a tooltip
 * set for it equivalent to the entire contents of the text.
 *
 * Some very basic 'markup' will be done on the text:
 *  - any line beginning in # will be displayed in BAD_COLOR  (red)
 *  - any line beginning in @ will be displayed in GOOD_COLOR (green)
 *  - any line beginning in + will be displayed with size increased by 2
 *  - any line beginning in - will be displayed with size decreased by 2
 *  - any line beginning with 0x0n will be displayed in the color of side n
 *
 * The above special characters can be quoted using a C-style backslash.
 *
 * A bounding rectangle of the text is returned. If dst is nullptr, then the
 * text will not be drawn, and a bounding rectangle only will be returned.
 */
SDL_Rect draw_text(surface& dst, const SDL_Rect& area, int size,
                   const color_t& color, const std::string& text,
                   int x, int y, bool use_tooltips = false, int style = 0);

/** wrapper of the previous function, gui can also be nullptr */
SDL_Rect draw_text(CVideo* gui, const SDL_Rect& area, int size,
                   const color_t& color, const std::string& text,
                   int x, int y, bool use_tooltips = false, int style = 0);


/** Calculate the size of a text (in pixels) if it were to be drawn. */
SDL_Rect text_area(const std::string& text, int size, int style=0);

/** Copy string, but without tags at the beginning */
std::string del_tags(const std::string& text);

/**
 * Determine if char is one of the special chars used as markup.
 *
 * @retval true                   Input-char is a markup-char.
 * @retval false                  Input-char is a normal char.
 */
bool is_format_char(char c);

/**
 * Determine if a ucs4::char_t is a CJK character
 *
 * @retval true                   Input-char is a CJK char
 * @retval false                  Input-char is a not CJK char.
 */
bool is_cjk_char(const ucs4::char_t ch);

/**
 * Wrap text.
 *
 * - If the text exceeds the specified max width, wrap it on a word basis.
 * - If this is not possible, e.g. the word is too big to fit, wrap it on a
 * - char basis.
 */
std::string word_wrap_text(const std::string& unwrapped_text, int font_size,
	int max_width, int max_height = -1, int max_lines = -1, bool partial_line = false);

} // end namespace font
