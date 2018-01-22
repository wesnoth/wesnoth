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

#pragma once

/**
 * @file
 * Contains the SDL_Rect helper code.
 */

#include "global.hpp"
#include "utils.hpp"

#include <SDL_rect.h>

struct point;

namespace sdl
{

CONSTEXPR const SDL_Rect empty_rect { 0, 0, 0, 0 };

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

/**
 * Draw a rectangle outline.
 *
 * @param rect                    The dimensions of the rectangle.
 * @param color                   The color of the rectangle.
 */
void draw_rectangle(const SDL_Rect& rect, const color_t& color);

/**
 * Draws a filled rectangle.
 *
 * @param rect                    The dimensions of the rectangle.
 * @param color                   The color of the rectangle.
 */
void fill_rectangle(const SDL_Rect& rect, const color_t& color);

/**
 * Fill a rectangle on a given surface. Alias for SDL_FillRect.
 *
 * @param dst                     The surface to operate on.
 * @param dst_rect                The rectangle to fill.
 * @param color                   Color of the rectangle.
 */
inline void fill_surface_rect(surface& dst, SDL_Rect* dst_rect, const uint32_t color)
{
	SDL_FillRect(dst, dst_rect, color);
}

} // namespace sdl

bool operator==(const SDL_Rect& a, const SDL_Rect& b);
bool operator!=(const SDL_Rect& a, const SDL_Rect& b);

std::ostream& operator<<(std::ostream& s, const SDL_Rect& rect);
