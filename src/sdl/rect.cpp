/*
	Copyright (C) 2014 - 2022
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <iostream>

namespace sdl
{
SDL_Rect intersect_rects(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	SDL_Rect res;
	if(!SDL_IntersectRect(&rect1, &rect2, &res)) {
		return empty_rect;
	}

	return res;
}

SDL_Rect union_rects(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	SDL_Rect res;
	SDL_UnionRect(&rect1, &rect2, &res);

	return res;
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

bool rect::operator==(const rect& r) const
{
	return SDL_RectEquals(this, &r) != SDL_FALSE;
}

bool rect::operator==(const SDL_Rect& r) const
{
	return SDL_RectEquals(this, &r) != SDL_FALSE;
}

bool rect::empty() const
{
	return SDL_RectEmpty(this);
}

bool rect::contains(int x, int y) const
{
	SDL_Point p{x, y};
	return SDL_PointInRect(&p, this) != SDL_FALSE;
}

bool rect::contains(const point& point) const
{
	return SDL_PointInRect(&point, this) != SDL_FALSE;
}

bool rect::overlaps(const SDL_Rect& r) const
{
	return (r.x < x + w && x < r.x + r.w &&
	        r.y < y + h && y < r.y + r.h);
}

std::ostream& operator<<(std::ostream& s, const rect& r)
{
	s << '[' << r.x << ',' << r.y << '|' << r.w << ',' << r.h << ']';
	return s;
}
