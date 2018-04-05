/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "ogl/sprite.hpp"

namespace gl
{

void sprite::normalize_coordinates(const std::pair<int, int>& texture_size)
{
	x_min = static_cast<float>(x_min_px) / static_cast<float>(texture_size.first);
	x_max = static_cast<float>(x_max_px) / static_cast<float>(texture_size.first);
	y_min = static_cast<float>(y_min_px) / static_cast<float>(texture_size.second);
	y_max = static_cast<float>(y_max_px) / static_cast<float>(texture_size.second);

	half_width = (x_max_px - x_min_px) / 2.0f;
	half_height = (y_max_px - y_min_px) / 2.0f;
}

}
