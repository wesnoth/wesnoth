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

#pragma once

#include <vector>

#include "../color.hpp"

namespace utils {
	/**
	 * Return a color from the scale, with 0 being the first, and 100 the last.
	 *
	 * @param color_scale   The scale to index the color from
	 * @param value         The 0-100 range integer used to index the scale
	 * @return              The color value corresponding to the value
	 */
	color_t pick_color_range(const std::vector<color_t>& color_scale, int value);

	/**
	 * Return a color interpolated from the spline defined by the scale.
	 * As the interpolation happens in RGB, the colors on the scale should not be too far apart.
	 *
	 * @param color_scale   The scale to interpolate the color from
	 * @param value         The 0-100 range integer used to interpolate from the scale
	 * @return              The color value corresponding to the value
	 */
	color_t interpolate_color_range(const std::vector<color_t>& color_scale, double value);
};
