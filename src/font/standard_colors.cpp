/*
 * Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
 * Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 *
 * See the COPYING file for more details.
 */

#include "font/standard_colors.hpp"

#include "sdl/utils.hpp"

namespace font {
	const SDL_Color NORMAL_COLOR = {0xDD,0xDD,0xDD,0},
		GRAY_COLOR   = {0x77,0x77,0x77,0},
		LOBBY_COLOR  = {0xBB,0xBB,0xBB,0},
		GOOD_COLOR   = {0x00,0xFF,0x00,0},
		BAD_COLOR    = {0xFF,0x00,0x00,0},
		BLACK_COLOR  = {0x00,0x00,0x00,0},
		YELLOW_COLOR = {0xFF,0xFF,0x00,0},
		BUTTON_COLOR = {0xBC,0xB0,0x88,0},
		PETRIFIED_COLOR = {0xA0,0xA0,0xA0,0},
		TITLE_COLOR  = {0xBC,0xB0,0x88,0},
		LABEL_COLOR  = {0x6B,0x8C,0xFF,0},
		BIGMAP_COLOR = {0xFF,0xFF,0xFF,0};

	const SDL_Color DISABLED_COLOR = inverse(PETRIFIED_COLOR);
}
