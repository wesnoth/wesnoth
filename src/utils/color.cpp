/*
   Copyright (C) 2003 - 2019 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "utils/color.hpp"

#include <cassert>

#include "utils/general.hpp"

namespace utils {
	color_t pick_color_range(const std::vector<color_t>& color_scale, int value) {
		assert(!color_scale.empty());

		value = utils::clamp(value, 0, 100);
		const int idx = color_scale.size() * value / 101;

		return color_scale[idx];
	}
};
