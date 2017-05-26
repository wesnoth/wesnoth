/*
   Copyright (C) 2014 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/core/point.hpp"
#include "sdl/rect.hpp"
#include "video.hpp"

namespace sdl
{
bool point_in_rect(int x, int y, const SDL_Rect& rect)
{
	return x >= rect.x && y >= rect.y && x < rect.x + rect.w && y < rect.y + rect.h;
}

bool point_in_rect(const gui2::point& point, const SDL_Rect& rect)
{
	return point_in_rect(point.x, point.y, rect);
}

bool rects_overlap(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	return (rect1.x < rect2.x+rect2.w && rect2.x < rect1.x+rect1.w &&
			rect1.y < rect2.y+rect2.h && rect2.y < rect1.y+rect1.h);
}

SDL_Rect intersect_rects(SDL_Rect const &rect1, SDL_Rect const &rect2)
{
	SDL_Rect res;
	res.x = std::max<int>(rect1.x, rect2.x);
	res.y = std::max<int>(rect1.y, rect2.y);
	int w = std::min<int>(rect1.x + rect1.w, rect2.x + rect2.w) - res.x;
	int h = std::min<int>(rect1.y + rect1.h, rect2.y + rect2.h) - res.y;
	if (w <= 0 || h <= 0) return empty_rect;
	res.w = w;
	res.h = h;
	return res;
}

SDL_Rect union_rects(SDL_Rect const &rect1, SDL_Rect const &rect2)
{
	if (rect1.w == 0 || rect1.h == 0) return rect2;
	if (rect2.w == 0 || rect2.h == 0) return rect1;
	SDL_Rect res;
	res.x = std::min<int>(rect1.x, rect2.x);
	res.y = std::min<int>(rect1.y, rect2.y);
	res.w = std::max<int>(rect1.x + rect1.w, rect2.x + rect2.w) - res.x;
	res.h = std::max<int>(rect1.y + rect1.h, rect2.y + rect2.h) - res.y;
	return res;
}

void draw_rectangle(const SDL_Rect& rect, const color_t& color)
{
	SDL_Renderer* renderer = *CVideo::get_singleton().get_window();

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawRect(renderer, &rect);
}

void fill_rectangle(const SDL_Rect& rect, const color_t& color)
{
	SDL_Renderer* renderer = *CVideo::get_singleton().get_window();

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &rect);
}

} // namespace sdl

bool operator==(const SDL_Rect& a, const SDL_Rect& b)
{
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

bool operator!=(const SDL_Rect& a, const SDL_Rect& b)
{
	return !operator==(a,b);
}
