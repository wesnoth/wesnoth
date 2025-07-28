/*
	Copyright (C) 2025 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "sdl/point.hpp"

#include <cairo.h>
#include <memory>

namespace cairo
{
using surface_ptr = std::unique_ptr<cairo_surface_t, void(*)(cairo_surface_t*)>;
using context_ptr = std::unique_ptr<cairo_t, void(*)(cairo_t*)>;

/** Color format for cairo surfaces. Should be equivalent to the format used by SDL. */
constexpr cairo_format_t format = CAIRO_FORMAT_ARGB32;

inline surface_ptr create_surface(uint8_t* buffer, const point& size)
{
	const auto& [width, height] = size;
	const int stride = cairo_format_stride_for_width(format, width);

	return {
		cairo_image_surface_create_for_data(buffer, format, width, height, stride),
		cairo_surface_destroy
	};
}

inline context_ptr create_context(const surface_ptr& surf)
{
	return {
		cairo_create(surf.get()),
		cairo_destroy
	};
}

} // namespace cairo
