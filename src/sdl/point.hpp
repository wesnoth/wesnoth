/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <SDL_rect.h>

#include <iosfwd>

/** Holds a 2D point. */
struct point
{
	point()
		: x(0)
		, y(0)
	{
	}

	point(const int x_, const int y_)
		: x(x_)
		, y(y_)
	{
	}

	point(const SDL_Point& p)
		: x(p.x)
		, y(p.y)
	{
	}

	/** x coordinate. */
	int x;

	/** y coordinate. */
	int y;

	/** Allow implicit conversion to SDL_Point. */
	operator SDL_Point() const;

	bool operator==(const point& point) const
	{
		return x == point.x && y == point.y;
	}

	bool operator!=(const point& point) const
	{
		return !operator==(point);
	}

	bool operator<(const point& point) const
	{
		return x < point.x || (x == point.x && y < point.y);
	}

	bool operator<=(const point& point) const
	{
		return x < point.x || (x == point.x && y <= point.y);
	}

	point operator+(const point& point) const
	{
		return {x + point.x, y + point.y};
	}

	point& operator+=(const point& point);

	point operator-(const point& point) const
	{
		return {x - point.x, y - point.y};
	}

	point& operator-=(const point& point);
};

std::ostream& operator<<(std::ostream& stream, const point& point);
