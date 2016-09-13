/*
   Copyright (C) 2007 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SDL_RENDERER_INCLUDED
#define SDL_RENDERER_INCLUDED

#include "sdl/utils.hpp"

#include <SDL.h>

namespace sdl {

void set_renderer_color(SDL_Renderer* renderer, Uint32 color);

/**
 * Draws a line on a surface.
 *
 * @pre                   The caller needs to make sure the entire line fits on
 *                        the @p surface.
 * @pre                   @p x2 >= @p x1
 * @pre                   The @p surface is locked.
 *
 * @param surface          The surface to draw upon, the caller should lock the
 *                        surface before calling.
 * @param color           The color of the line to draw.
 * @param x1              The start x coordinate of the line to draw.
 * @param y1              The start y coordinate of the line to draw.
 * @param x2              The end x coordinate of the line to draw.
 * @param y2              The end y coordinate of the line to draw.
 */
void draw_line(
	surface& surface,
	SDL_Renderer* renderer,
	Uint32 color,
	const unsigned x1,
	const unsigned y1,
	const unsigned x2,
	const unsigned y2);

/**
 * Draws a circle on a surface.
 *
 * @pre                   The circle must fit on the surface.
 * @pre                   The @p surface is locked.
 *
 * @param surface          The surface to draw upon, the caller should lock the
 *                        surface before calling.
 * @param color           The color of the circle to draw.
 * @param x_center        The x coordinate of the center of the circle to draw.
 * @param y_center        The y coordinate of the center of the circle to draw.
 * @param radius          The radius of the circle to draw.
 */
void draw_circle(
	surface& surface,
	SDL_Renderer* renderer,
	Uint32 color,
	const int x_center,
	const int y_center,
	const int radius);

} // namespace sdl

#endif
