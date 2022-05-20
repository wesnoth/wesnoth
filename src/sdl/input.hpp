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

#include <cstdint>

#pragma once

/**
 * @file
 * Contains functions for cleanly handling SDL input.
 */

struct SDL_Point;

namespace sdl
{

/**
 * A wrapper for SDL_GetMouseState that gives coordinates in draw space.
 */
uint32_t get_mouse_state(int *x, int *y);

/** Returns the current mouse button mask */
uint32_t get_mouse_button_mask();

/** Returns the currnet mouse location in draw space. */
SDL_Point get_mouse_location();

/**
 * Update the cached drawing area and input area sizes. These correspond to
 * the size of the drawing surface in pixels, and the size of the window in
 * display coordinates.
 *
 * This should be called every time the window is resized, or the pixel scale
 * multiplier changes.
 *
 * @param draw_width         The width of the drawing surface, in pixels
 * @param draw_height        The height of the drawing surface, in pixels
 * @param input_width        The width of the input surface, in display coordinates
 * @param input_height       The height of the input surface, in display coordinates
 */
void update_input_dimensions(
	int draw_width, int draw_height,
	int input_width, int input_height
);
void update_input_dimensions(SDL_Point draw_size, SDL_Point input_size);


} // namespace sdl
