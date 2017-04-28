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
#include "sdl/surface.hpp"

namespace sdl
{

SDL_Rect create_rect(const int x, const int y, const int w, const int h)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	return rect;
}

bool point_in_rect(int x, int y, const SDL_Rect& rect)
{
	return x >= rect.x && y >= rect.y && x < rect.x + rect.w && y < rect.y + rect.h;
}

bool point_in_rect(const gui2::point& point, const SDL_Rect& rect)
{
	return point.x >= rect.x && point.y >= rect.y && point.x < rect.x + rect.w && point.y < rect.y + rect.h;
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

void fill_rect_alpha(SDL_Rect &rect, Uint32 color, Uint8 alpha, surface target)
{
	if(alpha == SDL_ALPHA_OPAQUE) {
		sdl::fill_rect(target,&rect,color);
		return;
	} else if(alpha == SDL_ALPHA_TRANSPARENT) {
		return;
	}

	surface tmp(create_compatible_surface(target,rect.w,rect.h));
	if(tmp == nullptr) {
		return;
	}

	SDL_SetSurfaceBlendMode (tmp, SDL_BLENDMODE_BLEND);

	SDL_Rect r {0,0,rect.w,rect.h};
	sdl::fill_rect(tmp,&r,color);
	adjust_surface_alpha(tmp, alpha);
	sdl_blit(tmp,nullptr,target,&rect);
}

void draw_rectangle(int x, int y, int w, int h, Uint32 color, surface target)
{

	SDL_Rect top = create_rect(x, y, w, 1);
	SDL_Rect bot = create_rect(x, y + h - 1, w, 1);
	SDL_Rect left = create_rect(x, y, 1, h);
	SDL_Rect right = create_rect(x + w - 1, y, 1, h);

	sdl::fill_rect(target,&top,color);
	sdl::fill_rect(target,&bot,color);
	sdl::fill_rect(target,&left,color);
	sdl::fill_rect(target,&right,color);
}

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
								 int r, int g, int b,
								 double alpha, surface target)
{

	SDL_Rect rect = create_rect(x, y, w, h);
	fill_rect_alpha(rect,SDL_MapRGB(target->format,r,g,b),Uint8(alpha*255),target);
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
