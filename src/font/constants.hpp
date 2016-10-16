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

#pragma once

#include <SDL.h>
#include "global.hpp"

namespace font {

//various standard colors
extern const SDL_Color NORMAL_COLOR, GRAY_COLOR, LOBBY_COLOR, GOOD_COLOR, BAD_COLOR,
                       BLACK_COLOR, YELLOW_COLOR, BUTTON_COLOR, BIGMAP_COLOR,
                       PETRIFIED_COLOR, TITLE_COLOR, DISABLED_COLOR, LABEL_COLOR;

// font sizes, to be made theme parameters
CONSTEXPR int SIZE_NORMAL = 14;
// automatic computation of other font sizes, to be made a default for theme-provided values
CONSTEXPR int
	SIZE_TINY       = 10 * SIZE_NORMAL / 14,
	SIZE_SMALL      = 12 * SIZE_NORMAL / 14,

	SIZE_15         = 15 * SIZE_NORMAL / 14,
	SIZE_PLUS       = 16 * SIZE_NORMAL / 14,
	SIZE_LARGE      = 18 * SIZE_NORMAL / 14,
	SIZE_TITLE      = 20 * SIZE_NORMAL / 14,
	SIZE_XLARGE     = 24 * SIZE_NORMAL / 14
  ;
// For arbitrary scaling:
// (Not used in defining the SIZE_* consts because of spurious compiler warnings.)
CONSTEXPR inline int relative_size(int size)
{
	return (SIZE_NORMAL * size / 14);
}

// GUI1 built-in maximum
CONSTEXPR size_t max_text_line_width = 4096;

} // end namespace font
