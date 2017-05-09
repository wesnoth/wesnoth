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

#pragma once

#include "color.hpp"

namespace font {

//
// TODO: these should probably all be constexpr
//

extern const color_t
	// These are GUI1 formatting colors and should be removed when GUI1 is
	NORMAL_COLOR,
	GRAY_COLOR,
	LOBBY_COLOR,
	GOOD_COLOR,
	BAD_COLOR,
	BLACK_COLOR,
	YELLOW_COLOR,
	BUTTON_COLOR,
	BIGMAP_COLOR,
	PETRIFIED_COLOR,
	TITLE_COLOR,
	DISABLED_COLOR,
	LABEL_COLOR,

	// General purpose color values
	weapon_color,
	good_dmg_color,
	bad_dmg_color,
	weapon_details_color,
	inactive_details_color,
	inactive_ability_color,
	unit_type_color,
	race_color;
}
