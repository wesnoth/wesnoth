/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "color.hpp"

class CVideo;
class surface;

#include <string>

#include <SDL2/SDL_rect.h>

namespace font {

/** Standard markups for color, size, font, images. */
extern const char LARGE_TEXT, SMALL_TEXT, BOLD_TEXT, NORMAL_TEXT, NULL_MARKUP, BLACK_TEXT, GRAY_TEXT,
                  GOOD_TEXT, BAD_TEXT, GREEN_TEXT, RED_TEXT, COLOR_TEXT, IMAGE;

/** Parses the markup-tags at the front of a string. */
std::string::const_iterator parse_markup(std::string::const_iterator i1,
												std::string::const_iterator i2,
												int* font_size,
												color_t* color, int* style);

/** Copy string, but without tags at the beginning */
std::string del_tags(const std::string& text);

/**
 * Determine if a char32_t is a CJK character
 *
 * @retval true                   Input-char is a CJK char
 * @retval false                  Input-char is a not CJK char.
 */
bool is_cjk_char(const char32_t ch);

} // end namespace font
