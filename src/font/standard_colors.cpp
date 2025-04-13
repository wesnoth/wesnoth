/*
	Copyright (C) 2008 - 2025
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "font/standard_colors.hpp"
#include "color.hpp"

#include <pango/pango-color.h>

namespace font {

const color_t
	NORMAL_COLOR    {221, 221, 221},
	GRAY_COLOR      {136, 136, 136},
	GOOD_COLOR      {0  , 181, 26 },
	BAD_COLOR       {255, 0  , 0  },
	YELLOW_COLOR    {255, 255, 0  },
	TITLE_COLOR     {186, 172, 125},
	LABEL_COLOR     {107, 140, 255},
	INACTIVE_COLOR  {150, 150, 150};

const color_t
	weapon_color           {245, 230, 193},
	good_dmg_color         {130, 240, 50 },
	bad_dmg_color          {250, 140, 80 },
	weapon_details_color   {196, 176, 147},
	inactive_details_color { 86,  86,  86},
	unit_type_color        {245, 230, 193};

color_t string_to_color(const std::string& color_str)
{
	PangoColor color;
	if(pango_color_parse(&color, color_str.c_str())) {
		return color_t::from_48bit_color(color.red, color.green, color.blue);
	} else if(color_str.size() == 6) {
		// rrggbb color, wesnoth format
		return color_t::from_hex_string(color_str);
	}
	return font::NORMAL_COLOR;
}

} // namespace font

