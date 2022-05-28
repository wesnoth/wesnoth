/*
	Copyright (C) 2022
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "draw.hpp"

#include "color.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"
#include "video.hpp"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

static SDL_Renderer* renderer()
{
	return CVideo::get_singleton().get_renderer();
}

/**************************************/
/* basic drawing and pixel primatives */
/**************************************/

void draw::fill(
	const SDL_Rect& area,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_SetRenderDrawColor(renderer(), r, g, b, a);
	SDL_RenderFillRect(renderer(), &area);
}

void draw::fill(
	const SDL_Rect& area,
	uint8_t r, uint8_t g, uint8_t b)
{
	draw::fill(area, r, g, b, SDL_ALPHA_OPAQUE);
}

void draw::fill(const SDL_Rect& area, const color_t& c)
{
	draw::fill(area, c.r, c.g, c.b, c.a);
}

void draw::fill(const SDL_Rect& area)
{
	SDL_RenderFillRect(renderer(), &area);
}

void draw::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_SetRenderDrawColor(renderer(), r, g, b, a);
}

void draw::set_color(uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetRenderDrawColor(renderer(), r, g, b, SDL_ALPHA_OPAQUE);
}

void draw::set_color(const color_t& c)
{
	SDL_SetRenderDrawColor(renderer(), c.r, c.g, c.b, c.a);
}

void draw::rect(const SDL_Rect& rect)
{
	SDL_RenderDrawRect(renderer(), &rect);
}

void draw::rect(const SDL_Rect& rect,
	uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_SetRenderDrawColor(renderer(), r, g, b, a);
	SDL_RenderDrawRect(renderer(), &rect);
}

void draw::rect(const SDL_Rect& rect, uint8_t r, uint8_t g, uint8_t b)
{
	draw::rect(rect, r, g, b, SDL_ALPHA_OPAQUE);
}

void draw::rect(const SDL_Rect& rect, const color_t& c)
{
	draw::rect(rect, c.r, c.g, c.b, c.a);
}

// TODO: highdpi - canvas previously had code which special cased single-point lines. Is this necessary?
/* this was the code of draw_line() in canvas.cpp:
	DBG_GUI_D << "Shape: draw line from " << x1 << ',' << y1
	          << " to " << x2 << ',' << y2 << ".\n";

	draw::set_color(color);

	if(x1 == x2 && y1 == y2) {
		// Handle single-pixel lines properly
		draw::point(x1, y1);
	} else {
		draw::line(x1, y1, x2, y2);
	}
*/

void draw::line(int from_x, int from_y, int to_x, int to_y)
{
	SDL_RenderDrawLine(renderer(), from_x, from_y, to_x, to_y);
}

void draw::line(int from_x, int from_y, int to_x, int to_y, const color_t& c)
{
	SDL_SetRenderDrawColor(renderer(), c.r, c.g, c.b, c.a);
	SDL_RenderDrawLine(renderer(), from_x, from_y, to_x, to_y);
}

void draw::points(const std::vector<SDL_Point>& points)
{
	SDL_RenderDrawPoints(renderer(), points.data(), points.size());
}

void draw::point(int x, int y)
{
	SDL_RenderDrawPoint(renderer(), x, y);
}


/*******************/
/* texture drawing */
/*******************/


void draw::blit(const texture& tex, const SDL_Rect& dst, const SDL_Rect& src)
{
	SDL_RenderCopy(renderer(), tex, &src, &dst);
}

void draw::blit(const texture& tex, const SDL_Rect& dst)
{
	SDL_RenderCopy(renderer(), tex, nullptr, &dst);
}

void draw::blit(const texture& tex)
{
	SDL_RenderCopy(renderer(), tex, nullptr, nullptr);
}


void draw::flipped(
	const texture& tex,
	const SDL_Rect& dst,
	const SDL_Rect& src,
	bool flip_h,
	bool flip_v)
{
	SDL_RendererFlip flip =
		flip_h ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
		| flip_v ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
	SDL_RenderCopyEx(renderer(), tex, &src, &dst, 0.0, nullptr, flip);
}

void draw::flipped(
	const texture& tex,
	const SDL_Rect& dst,
	bool flip_h,
	bool flip_v)
{
	SDL_RendererFlip flip =
		flip_h ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
		| flip_v ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
	SDL_RenderCopyEx(renderer(), tex, nullptr, &dst, 0.0, nullptr, flip);
}

void draw::flipped(const texture& tex, bool flip_h, bool flip_v)
{
	SDL_RendererFlip flip =
		flip_h ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
		| flip_v ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
	SDL_RenderCopyEx(renderer(), tex, nullptr, nullptr, 0.0, nullptr, flip);
}


void draw::tiled(const texture& tex, const SDL_Rect& dst, bool centered)
{
	// TODO: highdpi - should this draw at full res? Or game res? For now it's using game res to ensure consistency of the result.
	// TODO: highdpi - does this ever need to clip the source texture? It doesn't seem so

	const int xoff = centered ? (dst.w - tex.w()) / 2 : 0;
	const int yoff = centered ? (dst.h - tex.h()) / 2 : 0;

	// Just blit the image however many times is necessary.
	SDL_Rect t{dst.x - xoff, dst.y - yoff, tex.w(), tex.h()};
	for (; t.y < dst.y + dst.h; t.y += tex.h()) {
		for (t.x = dst.x - xoff; t.x < dst.x + dst.w; t.x += tex.w()) {
			draw::blit(tex, t);
		}
	}
}
