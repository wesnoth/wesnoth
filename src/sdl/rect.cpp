/*
	Copyright (C) 2014 - 2024
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

#include <algorithm>
#include <ostream>

bool operator==(const SDL_Rect& a, const SDL_Rect& b)
{
	return SDL_RectEquals(&a, &b) != SDL_FALSE;
}

bool operator!=(const SDL_Rect& a, const SDL_Rect& b)
{
	return !operator==(a,b);
}

std::ostream& operator<<(std::ostream& s, const SDL_Rect& r)
{
	s << '[' << r.x << ',' << r.y << '|' << r.w << ',' << r.h << ']';
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

bool rect::contains(const SDL_Rect& r) const
{
	if(this->x > r.x) return false;
	if(this->y > r.y) return false;
	if(this->x + this->w < r.x + r.w) return false;
	if(this->y + this->h < r.y + r.h) return false;
	return true;
}

bool rect::overlaps(const SDL_Rect& r) const
{
	return SDL_HasIntersection(this, &r);
}

rect rect::minimal_cover(const SDL_Rect& other) const
{
	rect result;
	SDL_UnionRect(this, &other, &result);
	return result;
}

rect& rect::expand_to_cover(const SDL_Rect& other)
{
	SDL_UnionRect(this, &other, this);
	return *this;
}

rect rect::intersect(const SDL_Rect& other) const
{
	rect result;
	if(!SDL_IntersectRect(this, &other, &result)) {
		return rect();
	}
	return result;
}

void rect::clip(const SDL_Rect& other)
{
	*this = this->intersect(other);
}

void rect::shift(const point& other)
{
	this->x += other.x;
	this->y += other.y;
}

rect rect::shifted_by(int x, int y) const
{
	rect res = *this;
	res.x += x;
	res.y += y;
	return res;
}

rect rect::shifted_by(const point& other) const
{
	return shifted_by(other.x, other.y);
}

point rect::point_at(double x, double y) const
{
	return {
		static_cast<int>(this->x + this->w * std::clamp(x, 0.0, 1.0)),
		static_cast<int>(this->y + this->h * std::clamp(y, 0.0, 1.0))
	};
}

std::ostream& operator<<(std::ostream& s, const rect& r)
{
	s << '[' << r.x << ',' << r.y << '|' << r.w << ',' << r.h << ']';
	return s;
}
