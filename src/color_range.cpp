/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "utils/span.hpp"

#include <array>
#include <cassert>
#include <sstream>

namespace
{
std::vector<color_t> recolor_palette(const color_range& new_range, utils::span<const color_t> old_rgb)
{
	std::vector<color_t> clist;
	clist.reserve(old_rgb.size());

	const color_t mid_c = new_range.mid();
	const color_t max_c = new_range.max();
	const color_t min_c = new_range.min();

	// Map first color in vector to exact new color
	const uint8_t reference_avg = old_rgb.empty()
		? 255u
		: (old_rgb[0].r + old_rgb[0].g + old_rgb[0].b) / 3;

	for(const color_t& old_c : old_rgb) {
		const uint8_t old_avg = (old_c.r + old_c.g + old_c.b) / 3;

		if(reference_avg && old_avg <= reference_avg) {
			float old_ratio = static_cast<float>(old_avg) / reference_avg;

			clist.emplace_back(
				std::min(255u, static_cast<uint32_t>(old_ratio * mid_c.r + (1 - old_ratio) * min_c.r)),
				std::min(255u, static_cast<uint32_t>(old_ratio * mid_c.g + (1 - old_ratio) * min_c.g)),
				std::min(255u, static_cast<uint32_t>(old_ratio * mid_c.b + (1 - old_ratio) * min_c.b))
			);
		} else if(reference_avg != 255) {
			float old_ratio = (255.0f - static_cast<float>(old_avg)) / (255.0f - reference_avg);

			clist.emplace_back(
				std::min(255u, static_cast<uint32_t>(old_ratio * mid_c.r + (1 - old_ratio) * max_c.r)),
				std::min(255u, static_cast<uint32_t>(old_ratio * mid_c.g + (1 - old_ratio) * max_c.g)),
				std::min(255u, static_cast<uint32_t>(old_ratio * mid_c.b + (1 - old_ratio) * max_c.b))
			);
		}
	}

	return clist;
}

constexpr auto base_palette = []() {
	// Two entries per color, except on the first iteration.
	std::array<color_t, (255 * 2) - 1> res;
	std::size_t index = 0;

	// Use blue to make master set of possible colors
	for(uint8_t i = 255u; i != 0; --i) {
		res[index++] = {0, 0, i};

		// Avoid duplicate entry on the first pass when j == 0
		if(uint8_t j = 255u - i; j != 0) {
			res[index++] = {j, j, 255};
		}
	}

	return res;
}();

} // namespace

color_mapping generate_color_mapping(const color_range& new_range, const std::vector<color_t>& old_rgb)
{
	std::vector new_rgb = recolor_palette(new_range, old_rgb);
	assert(new_rgb.size() == old_rgb.size());

	color_mapping res;
	for(std::size_t i = 0; i < new_rgb.size(); ++i) {
		res[old_rgb[i]] = new_rgb[i];
	}

	return res;
}

std::vector<color_t> generate_reference_palette(const color_range& cr)
{
	return recolor_palette(cr, base_palette);
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
