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

/**
 * @file
 * Generate ranges of colors, and color palettes.
 * Used e.g. to color HP, XP.
 */

#include "color_range.hpp"

#include "game_config.hpp"
#include "map/map.hpp"

#include <iomanip>
#include <unordered_set>

color_range_map recolor_range(const color_range& new_range, const std::vector<color_t>& old_rgb)
{
	color_range_map map_rgb;

	const uint16_t new_red   = new_range.mid().r;
	const uint16_t new_green = new_range.mid().g;
	const uint16_t new_blue  = new_range.mid().b;

	const uint16_t max_red   = new_range.max().r;
	const uint16_t max_green = new_range.max().g;
	const uint16_t max_blue  = new_range.max().b;

	const uint16_t min_red   = new_range.min().r;
	const uint16_t min_green = new_range.min().g;
	const uint16_t min_blue  = new_range.min().b;

	// Map first color in vector to exact new color
	const color_t temp_rgb = old_rgb.empty() ? color_t() : old_rgb[0];

	const uint16_t reference_avg = (temp_rgb.r + temp_rgb.g + temp_rgb.b) / 3;

	for(const auto& old_c : old_rgb) {
		const uint16_t old_avg = (old_c.r + old_c.g + old_c.b) / 3;

		// Calculate new color
		uint32_t new_r = 0, new_g = 0, new_b = 0;

		if(reference_avg && old_avg <= reference_avg) {
			float old_ratio = static_cast<float>(old_avg) / reference_avg;

			new_r = static_cast<uint32_t>(old_ratio * new_red   + (1 - old_ratio) * min_red);
			new_g = static_cast<uint32_t>(old_ratio * new_green + (1 - old_ratio) * min_green);
			new_b = static_cast<uint32_t>(old_ratio * new_blue  + (1 - old_ratio) * min_blue);
		} else if(reference_avg != 255) {
			float old_ratio = (255.0f - static_cast<float>(old_avg)) / (255.0f - reference_avg);

			new_r = static_cast<uint32_t>(old_ratio * new_red   + (1 - old_ratio) * max_red);
			new_g = static_cast<uint32_t>(old_ratio * new_green + (1 - old_ratio) * max_green);
			new_b = static_cast<uint32_t>(old_ratio * new_blue  + (1 - old_ratio) * max_blue);
		} else {
			// Should never get here.
			// Would imply old_avg > reference_avg = 255
			assert(false);
		}

		new_r = std::min<uint32_t>(new_r, 255);
		new_g = std::min<uint32_t>(new_g, 255);
		new_b = std::min<uint32_t>(new_b, 255);

		map_rgb[old_c] = {static_cast<uint8_t>(new_r), static_cast<uint8_t>(new_g), static_cast<uint8_t>(new_b)};
	}

	return map_rgb;
}

std::vector<color_t> palette(const color_range& cr)
{
	std::vector<color_t> temp, res;
	std::unordered_set<color_t> clist;

	// Use blue to make master set of possible colors
	for(int i = 255; i != 0; i--) {
		const int j = 255 - i;

		temp.emplace_back(static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(i));
		temp.emplace_back(static_cast<uint8_t>(j), static_cast<uint8_t>(j), static_cast<uint8_t>(255));
	}

	// Use recolor function to generate list of possible colors.
	// Could use a special function, would be more efficient, but harder to maintain.
	color_range_map cmap = recolor_range(cr, temp);
	for(const auto& cm : cmap) {
		clist.insert(cm.second);
	}

	res.push_back(cmap[{0,0,255}]);

	for(const auto& cs : clist) {
		if(cs != res[0] && !cs.null() && cs != color_t(255, 255, 255)) {
			res.push_back(cs);
		}
	}

	return res;
}

std::string color_range::debug() const
{
	std::ostringstream o;

	o << '{' << mid_.to_hex_string()
	  << ',' << max_.to_hex_string()
	  << ',' << min_.to_hex_string()
	  << ',' << rep_.to_hex_string()
	  << '}';

	return o.str();
}
