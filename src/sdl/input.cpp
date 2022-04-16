/*
	Copyright (C) 2022
	by Thomas Iorns <mesilliac@tomanui.nz>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "sdl/input.hpp"
#include "sdl/point.hpp"

#include <SDL2/SDL_mouse.h>

namespace {
	SDL_Point drawing_surface_size;
	SDL_Point window_size;
}

namespace sdl
{

uint32_t get_mouse_state(int *x, int *y)
{
	uint32_t buttons = SDL_GetMouseState(x, y);
	if (window_size.x == 0 || window_size.y == 0) {
		// This will give bad results, but will not outright crash.
		return buttons;
	}
	if (window_size.x != drawing_surface_size.x) {
		*x = (*x * drawing_surface_size.x) / window_size.x;
	}
	if (window_size.y != drawing_surface_size.y) {
		*y = (*y * drawing_surface_size.y) / window_size.y;
	}
	return buttons;
}

uint32_t get_mouse_button_mask()
{
	return SDL_GetMouseState(nullptr, nullptr);
}

SDL_Point get_mouse_location()
{
	SDL_Point p;
	get_mouse_state(&p.x, &p.y);
	return p;
}

void update_input_dimensions(
	int draw_width, int draw_height,
	int input_width, int input_height
) {
	drawing_surface_size.x = draw_width;
	drawing_surface_size.y = draw_height;
	window_size.x = input_width;
	window_size.y = input_height;
}

void update_input_dimensions(SDL_Point draw_size, SDL_Point input_size)
{
	drawing_surface_size = draw_size;
	window_size = input_size;
}

} // namespace sdl
