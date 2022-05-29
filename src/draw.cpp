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

void draw::circle(int cx, int cy, int r, const color_t& c, uint8_t octants)
{
	draw::set_color(c);
	draw::circle(cx, cy, r, octants);
}

void draw::circle(int cx, int cy, int r, uint8_t octants)
{
	// Algorithm based on
	// http://de.wikipedia.org/wiki/Rasterung_von_Kreisen#Methode_von_Horn
	// version of 2011.02.07.
	int d = -r;
	int x = r;
	int y = 0;

	std::vector<SDL_Point> points;

	while(!(y > x)) {
		if(octants & 0x04) points.push_back({cx + x, cy + y});
		if(octants & 0x02) points.push_back({cx + x, cy - y});
		if(octants & 0x20) points.push_back({cx - x, cy + y});
		if(octants & 0x40) points.push_back({cx - x, cy - y});

		if(octants & 0x08) points.push_back({cx + y, cy + x});
		if(octants & 0x01) points.push_back({cx + y, cy - x});
		if(octants & 0x10) points.push_back({cx - y, cy + x});
		if(octants & 0x80) points.push_back({cx - y, cy - x});

		d += 2 * y + 1;
		++y;
		if(d > 0) {
			d += -2 * x + 2;
			--x;
		}
	}

	draw::points(points);
}

void draw::disc(int cx, int cy, int r, const color_t& c, uint8_t octants)
{
	draw::set_color(c);
	draw::disc(cx, cy, r, octants);
}

void draw::disc(int cx, int cy, int r, uint8_t octants)
{
	int d = -r;
	int x = r;
	int y = 0;

	while(!(y > x)) {
		// I use the formula of Bresenham's line algorithm
		// to determine the boundaries of a segment.
		// The slope of the line is always 1 or -1 in this case.
		if(octants & 0x04)
			// x2 - 1 = y2 - (cy + 1) + cx
			draw::line(cx + x, cy + y + 1, cx + y + 1, cy + y + 1);
		if(octants & 0x02)
			// x2 - 1 = cy - y2 + cx
			draw::line(cx + x, cy - y, cx + y + 1, cy - y);
		if(octants & 0x20)
			// x2 + 1 = (cy + 1) - y2 + (cx - 1)
			draw::line(cx - x - 1, cy + y + 1, cx - y - 2, cy + y + 1);
		if(octants & 0x40)
			// x2 + 1 = y2 - cy + (cx - 1)
			draw::line(cx - x - 1, cy - y, cx - y - 2, cy - y);

		if(octants & 0x08)
			// y2 = x2 - cx + (cy + 1)
			draw::line(cx + y, cy + x + 1, cx + y, cy + y + 1);
		if(octants & 0x01)
			// y2 = cx - x2 + cy
			draw::line(cx + y, cy - x, cx + y, cy - y);
		if(octants & 0x10)
			// y2 = (cx - 1) - x2 + (cy + 1)
			draw::line(cx - y - 1, cy + x + 1, cx - y - 1, cy + y + 1);
		if(octants & 0x80)
			// y2 = x2 - (cx - 1) + cy
			draw::line(cx - y - 1, cy - x, cx - y - 1, cy - y);

		d += 2 * y + 1;
		++y;
		if(d > 0) {
			d += -2 * x + 2;
			--x;
		}
	}
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
