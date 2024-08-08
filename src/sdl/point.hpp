/*
	Copyright (C) 2008 - 2024
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

#pragma once

#include <SDL2/SDL_rect.h>

#include <iosfwd>
#include <tuple>

/** Holds a 2D point. This is a thin wrapper over SDL_Point. */
struct point : SDL_Point
{
	/** Initialize to 0 by default. */
	constexpr point() : SDL_Point{0, 0} {}

	constexpr point(int x, int y) : SDL_Point{x, y} {}

	constexpr point(const SDL_Point& p) : SDL_Point{p} {}

	constexpr bool operator==(const point& point) const
	{
		return x == point.x && y == point.y;
	}

	constexpr bool operator!=(const point& point) const
	{
		return !operator==(point);
	}

	constexpr bool operator<(const point& point) const
	{
		return std::tie(x, y) < std::tie(point.x, point.y);
	}

	constexpr bool operator<=(const point& point) const
	{
		return x < point.x || (x == point.x && y <= point.y);
	}

	constexpr point operator+(const point& point) const
	{
		return {x + point.x, y + point.y};
	}

	constexpr point& operator+=(const point& point)
	{
		x += point.x;
		y += point.y;
		return *this;
	}

	constexpr point operator-() const
	{
		return {-x, -y};
	}

	constexpr point operator-(const point& point) const
	{
		return {x - point.x, y - point.y};
	}

	constexpr point& operator-=(const point& point)
	{
		x -= point.x;
		y -= point.y;
		return *this;
	}

	constexpr point operator*(int s) const
	{
		return {x * s, y * s};
	}

	constexpr point& operator*=(int s)
	{
		x *= s;
		y *= s;
		return *this;
	}

	constexpr point operator/(int s) const
	{
		return {x / s, y / s};
	}

	constexpr point& operator/=(int s)
	{
		x /= s;
		y /= s;
		return *this;
	}

	// Multiplication and division of points works elementwise.

	constexpr point operator*(const point& p) const
	{
		return {x * p.x, y * p.y};
	}

	constexpr point& operator*=(const point& p)
	{
		x *= p.x;
		y *= p.y;
		return *this;
	}

	constexpr point operator/(const point& p) const
	{
		return {x / p.x, y / p.y};
	}

	constexpr point& operator/=(const point& p)
	{
		x /= p.x;
		y /= p.y;
		return *this;
	}

};

std::ostream& operator<<(std::ostream& stream, const point& point);
