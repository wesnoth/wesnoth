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

#include "sdl/renderer.hpp"

#include "image.hpp"
#include "log.hpp"

static lg::log_domain log_sdl("sdl_renderer");
#define LOG_SDL LOG_STREAM(info, log_sdl)

namespace sdl {

/***** ***** ***** ***** ***** DRAWING PRIMITIVES ***** ***** ***** ***** *****/

void set_renderer_color(SDL_Renderer* renderer, Uint32 color)
{
	SDL_SetRenderDrawColor(renderer,
		(color & 0xFF000000) >> 24,
		(color & 0x00FF0000) >> 16,
		(color & 0x0000FF00) >> 8,
		(color & 0x000000FF));
}

void draw_line(
		surface& surface,
		SDL_Renderer* renderer,
		Uint32 color,
		const unsigned x1,
		const unsigned y1,
		const unsigned x2,
		const unsigned y2)
{
	unsigned w = surface->w;

	LOG_SDL << "Shape: draw line from " << x1 << ',' << y1 << " to " << x2
	        << ',' << y2 << " surface width " << w << " surface height "
	        << surface->h << ".\n";

	assert(static_cast<int>(x1) < surface->w);
	assert(static_cast<int>(x2) < surface->w);
	assert(static_cast<int>(y1) < surface->h);
	assert(static_cast<int>(y2) < surface->h);

	set_renderer_color(renderer, color);

	if(x1 == x2 && y1 == y2) {
		// Handle single-pixel lines properly
		SDL_RenderDrawPoint(renderer, x1, y1);
	} else {
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
}

void draw_circle(
		surface& surface,
		SDL_Renderer* renderer,
		Uint32 color,
		const int x_center,
		const int y_center,
		const int radius)
{
	unsigned w = surface->w;

	LOG_SDL << "Shape: draw circle at " << x_center << ',' << y_center
	        << " with radius " << radius << " surface width " << w
	        << " surface height " << surface->h << ".\n";

	assert(static_cast<int>(x_center + radius) < surface->w);
	assert(static_cast<int>(x_center - radius) >= 0);
	assert(static_cast<int>(y_center + radius) < surface->h);
	assert(static_cast<int>(y_center - radius) >= 0);

	set_renderer_color(renderer, color);

	// Algorithm based on
	// http://de.wikipedia.org/wiki/Rasterung_von_Kreisen#Methode_von_Horn
	// version of 2011.02.07.
	int d = -static_cast<int>(radius);
	int x = radius;
	int y = 0;

	std::vector<SDL_Point> points;

	while(!(y > x)) {
		points.push_back({x_center + x, y_center + y});
		points.push_back({x_center + x, y_center - y});
		points.push_back({x_center - x, y_center + y});
		points.push_back({x_center - x, y_center - y});

		points.push_back({x_center + y, y_center + x});
		points.push_back({x_center + y, y_center - x});
		points.push_back({x_center - y, y_center + x});
		points.push_back({x_center - y, y_center - x});

		d += 2 * y + 1;
		++y;
		if(d > 0) {
			d += -2 * x + 2;
			--x;
		}
	}

	SDL_RenderDrawPoints(renderer, points.data(), points.size());
}

}
