/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file marked-up_text.hpp  */

#ifndef MARKED_UP_TEXT_HPP_INCLUDED
#define MARKED_UP_TEXT_HPP_INCLUDED


#include "sdl_utils.hpp"
class CVideo;
#include <SDL_video.h>
#include <string>

namespace font {

/** Standard markups for color, size, font, images. */
extern const char LARGE_TEXT, SMALL_TEXT, BOLD_TEXT, NORMAL_TEXT, NULL_MARKUP, BLACK_TEXT, GRAY_TEXT,
                  GOOD_TEXT, BAD_TEXT, GREEN_TEXT, RED_TEXT, COLOR_TEXT, YELLOW_TEXT, IMAGE;

extern const std::string weapon, weapon_details, unit_type, race;

extern const SDL_Color weapon_color, weapon_details_color, unit_type_color, race_color;

/** Parses the markup-tags at the front of a string. */
std::string::const_iterator parse_markup(std::string::const_iterator i1,
												std::string::const_iterator i2,
												int* font_size,
												SDL_Color* colour, int* style);

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
 *  - any line beginning in # will be displayed in BAD_COLOUR  (red)
 *  - any line beginning in @ will be displayed in GOOD_COLOUR (green)
 *  - any line beginning in + will be displayed with size increased by 2
 *  - any line beginning in - will be displayed with size decreased by 2
 *  - any line beginning with 0x0n will be displayed in the colour of side n
 *
 * The above special characters can be quoted using a C-style backslash.
 *
 * A bounding rectangle of the text is returned. If dst is NULL, then the
 * text will not be drawn, and a bounding rectangle only will be returned.
 */
SDL_Rect draw_text(surface dst, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y, bool use_tooltips = false, int style = 0);

/** wrapper of the previous function, gui can also be NULL */
SDL_Rect draw_text(CVideo* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
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
 * Determine if a wchar_t is a CJK character
 *
 * @retval true                   Input-char is a CJK char
 * @retval false                  Input-char is a not CJK char.
 */
bool is_cjk_char(const wchar_t ch);

/** Create string of color-markup, such as "<255,255,0>" for yellow. */
std::string color2markup(const SDL_Color &color);

/** Creates the hexadecimal string of a color, such as "#ffff00" for yellow. */
std::string color2hexa(const SDL_Color &color);

/**
 * Wrap text.
 *
 * - If the text exceedes the specified max width, wrap it one a word basis.
 * - If this is not possible, e.g. the word is too big to fit, wrap it on a
 * - char basis.
 */
std::string word_wrap_text(const std::string& unwrapped_text, int font_size,
	int max_width, int max_height = -1, int max_lines = -1, bool partial_line = false);

/**
 * Draw text on the screen, fit text to maximum width, no markup, no tooltips.
 *
 * This method makes sure that the text fits within a given maximum width.
 * If a line exceedes this width, it will be wrapped
 * on a word basis if possible, otherwise on a char basis.
 * This method is otherwise similar to the draw_text method,
 * but it doesn't support special markup or tooltips.
 *
 * @returns                       A bounding rectangle of the text.
 */
SDL_Rect draw_wrapped_text(CVideo* gui, const SDL_Rect& area, int font_size,
			     const SDL_Color& colour, const std::string& text,
			     int x, int y, int max_width);

} // end namespace font

#endif // MARKED_UP_TEXT_HPP_INCLUDED

