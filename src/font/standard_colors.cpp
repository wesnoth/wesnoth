/*
 * Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

namespace font {

const color_t
	NORMAL_COLOR    {221, 221, 221},
	GRAY_COLOR      {119, 119, 119},
	LOBBY_COLOR     {187, 187, 187},
	GOOD_COLOR      {0  , 255, 0  },
	BAD_COLOR       {255, 0  , 0  },
	BLACK_COLOR     {0  , 0  , 0  },
	YELLOW_COLOR    {255, 255, 0  },
	BUTTON_COLOR    {186, 172, 125},
	PETRIFIED_COLOR {160, 160, 160},
	TITLE_COLOR     {186, 172, 125},
	LABEL_COLOR     {107, 140, 255},
	BIGMAP_COLOR    {255, 255, 255};

const color_t DISABLED_COLOR = PETRIFIED_COLOR.inverse();

const color_t
	weapon_color           {245, 230, 193},
	good_dmg_color         {130, 240, 50 },
	bad_dmg_color          {250, 140, 80 },
	weapon_details_color   {166, 146, 117},
	inactive_details_color {146, 146, 146},
	inactive_ability_color {146, 146, 146},
	unit_type_color        {245, 230, 193},
	race_color             {166, 146, 117};
}
