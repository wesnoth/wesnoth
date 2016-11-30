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

const color_t
	NORMAL_COLOR    {0xDD, 0xDD, 0xDD, SDL_ALPHA_OPAQUE},
	GRAY_COLOR      {0x77, 0x77, 0x77, SDL_ALPHA_OPAQUE},
	LOBBY_COLOR     {0xBB, 0xBB, 0xBB, SDL_ALPHA_OPAQUE},
	GOOD_COLOR      {0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE},
	BAD_COLOR       {0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE},
	BLACK_COLOR     {0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE},
	YELLOW_COLOR    {0xFF, 0xFF, 0x00, SDL_ALPHA_OPAQUE},
	BUTTON_COLOR    {0xBC, 0xB0, 0x88, SDL_ALPHA_OPAQUE},
	PETRIFIED_COLOR {0xA0, 0xA0, 0xA0, SDL_ALPHA_OPAQUE},
	TITLE_COLOR     {0xBC, 0xB0, 0x88, SDL_ALPHA_OPAQUE},
	LABEL_COLOR     {0x6B, 0x8C, 0xFF, SDL_ALPHA_OPAQUE},
	BIGMAP_COLOR    {0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE};

const color_t DISABLED_COLOR = inverse(PETRIFIED_COLOR);

const color_t
	weapon_color           { 245, 230, 193, SDL_ALPHA_OPAQUE },
	good_dmg_color         { 130, 240, 50,  SDL_ALPHA_OPAQUE },
	bad_dmg_color          { 250, 140, 80,  SDL_ALPHA_OPAQUE },
	weapon_details_color   { 166, 146, 117, SDL_ALPHA_OPAQUE },
	inactive_details_color { 146, 146, 146, SDL_ALPHA_OPAQUE },
	inactive_ability_color { 146, 146, 146, SDL_ALPHA_OPAQUE },
	unit_type_color        { 245, 230, 193, SDL_ALPHA_OPAQUE },
	race_color             { 166, 146, 117, SDL_ALPHA_OPAQUE };
}
