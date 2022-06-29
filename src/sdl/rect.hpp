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

#pragma once

/**
 * @file
 * Contains the SDL_Rect helper code.
 */

#include "sdl/point.hpp"

#include <SDL2/SDL_rect.h>

namespace sdl
{

constexpr const SDL_Rect empty_rect { 0, 0, 0, 0 };

/**
 * Creates an SDL_Rect with the given dimensions.
 *
 * This is a simple wrapper in order to avoid the narrowing conversion warnings
 * that occur when using aggregate initialization and non-int values.
 */
inline SDL_Rect create_rect(const int x, const int y, const int w, const int h)
{
	return {x, y, w, h};
}

/**
 * Tests whether a point is inside a rectangle.
 *
 * @param x                       The x coordinate of the point.
 * @param y                       The y coordinate of the point.
 * @param rect                    The rectangle.
 *
 * @return                        True if point (x;y) is inside or on the border
 *                                of rect, false otherwise
 */
bool point_in_rect(int x, int y, const SDL_Rect& rect);

bool point_in_rect(const point& point, const SDL_Rect& rect);

/**
 * Tests whether two rectangles overlap.
 *
 * @param rect1                   One rectangle.
 * @param rect2                   Another rectangle.
 *
 * @return                        True if rect1 and rect2 intersect, false if
 *                                not. Touching borders don't overlap.
 */
bool rects_overlap(const SDL_Rect& rect1, const SDL_Rect& rect2);

/**
 * Calculates the intersection of two rectangles.
 *
 * @param rect1                   One rectangle.
 * @param rect2                   Another rectangle
 * @return                        The intersection of rect1 and rect2, or
 *                                empty_rect if they don't overlap.
 */
SDL_Rect intersect_rects(const SDL_Rect& rect1, const SDL_Rect& rect2);

/**
 * Calculates the union of two rectangles. Note: "union" here doesn't mean the
 * union of the sets of points of the two polygons, but rather the minimal
 * rectangle that supersets both rectangles.
 *
 * @param rect1                   One rectangle.
 * @param rect2                   Another rectangle.
 *
 * @return                        The union of rect1 and rect2.
 */
SDL_Rect union_rects(const SDL_Rect &rect1, const SDL_Rect &rect2);

} // namespace sdl

bool operator==(const SDL_Rect& a, const SDL_Rect& b);
bool operator!=(const SDL_Rect& a, const SDL_Rect& b);

std::ostream& operator<<(std::ostream& s, const SDL_Rect& rect);

/**
 * An abstract description of a rectangle with integer coordinates.
 *
 * This is a thin wrapper over SDL_Rect, furnished with utility functions.
 *
 * As for SDL_Rect, member variables x, y, w and h are public.
 */
struct rect : SDL_Rect
{
public:
	/** Explicitly initialize rects to 0. */
	rect() : SDL_Rect{0, 0, 0, 0} {}

	/** There's nothing extra when converting an SDL_Rect. */
	rect(const SDL_Rect& r) : SDL_Rect{r} {}

	/** Specify via (x, y, w, h). */
	rect(int x, int y, int w, int h) : SDL_Rect{x, y, w, h} {}

	/** Specify via top-left corner position and size. */
	rect(const point& pos, const point& size)
		: SDL_Rect{pos.x, pos.y, size.x, size.y}
	{}

	// Comparisons
	bool operator==(const rect& r) const;
	bool operator==(const SDL_Rect& r) const;

	// Scalar multiplication and division
	rect operator*(int s) const { return {x*s, y*s, w*s, h*s}; }
	rect& operator*=(int s) { x*=s; y*=s; w*=s; h*=s; return *this; }
	rect operator/(int s) const { return {x/s, y/s, w/s, h/s}; }
	rect& operator/=(int s) { x/=s; y/=s; w/=s; h/=s; return *this; }

	/** False if both w and h are > 0, true otherwise. */
	bool empty() const;
};

std::ostream& operator<<(std::ostream&, const rect&);
