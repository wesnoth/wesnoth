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
#include <cmath>

#include "utils/general.hpp"

namespace utils {
	color_t pick_color_range(const std::vector<color_t>& color_scale, int value) {
		assert(!color_scale.empty());

		value = utils::clamp(value, 0, 100);
		const int idx = color_scale.size() * value / 101;

		return color_scale[idx];
	}

	color_t interpolate_color_range(const std::vector<color_t>& color_scale, double value) {
		assert(!color_scale.empty());

		value = utils::clamp(value, 0., 100.);

		const double stepsize = 100.0 / (color_scale.size() - 1);
		const double idx_d = value / stepsize;
		const long idx_l = std::lround(std::floor(idx_d));
		const int idx_i = static_cast<int>(idx_l);
		const double remainder = std::fmod(value, stepsize);
		const double spline_fraction = remainder / stepsize;

		if(idx_i >= static_cast<signed>(color_scale.size()) - 1){
			return color_scale.back();
		}

		const color_t& left = color_scale[idx_i];
		const color_t& right = color_scale[idx_i + 1];
		return {
			static_cast<uint8_t>(std::lround(left.r * (1 - spline_fraction) + right.r * spline_fraction)),
			static_cast<uint8_t>(std::lround(left.g * (1 - spline_fraction) + right.g * spline_fraction)),
			static_cast<uint8_t>(std::lround(left.b * (1 - spline_fraction) + right.b * spline_fraction)),
			static_cast<uint8_t>(std::lround(left.a * (1 - spline_fraction) + right.a * spline_fraction)),
		};
	}
};
