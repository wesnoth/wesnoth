/*
   Copyright (C) 2014 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sdl/point.hpp"
#include "sdl/rect.hpp"
#include "sdl/window.hpp"
#include "video.hpp"

namespace sdl
{
bool point_in_rect(int x, int y, const SDL_Rect& rect)
{
	SDL_Point p {x, y};
	return SDL_PointInRect(&p, &rect) != SDL_FALSE;
}

bool point_in_rect(const point& point, const SDL_Rect& rect)
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
	if(!SDL_IntersectRect(&rect1, &rect2, &res)) {
		return empty_rect;
	}

	return res;
}

SDL_Rect union_rects(SDL_Rect const &rect1, SDL_Rect const &rect2)
{
	SDL_Rect res;
	SDL_UnionRect(&rect1, &rect2, &res);

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
	return SDL_RectEquals(&a, &b) != SDL_FALSE;
}

bool operator!=(const SDL_Rect& a, const SDL_Rect& b)
{
	return !operator==(a,b);
}

std::ostream& operator<<(std::ostream& s, const SDL_Rect& rect)
{
	s << "x: " << rect.x << ", y: " << rect.y << ", w: " << rect.w << ", h: " << rect.h;
	return s;
}
