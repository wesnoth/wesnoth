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

#pragma once

/**
 * @file
 * Contains the SDL_Rect helper code.
 */

#include "sdl/point.hpp"

#include <SDL2/SDL_rect.h>

#include <algorithm>

namespace sdl
{

constexpr const SDL_Rect empty_rect { 0, 0, 0, 0 };

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
	constexpr rect() : SDL_Rect{0, 0, 0, 0} {}

	/** There's nothing extra when converting an SDL_Rect. */
	constexpr rect(const SDL_Rect& r) : SDL_Rect{r} {}

	/** Specify via (x, y, w, h). */
	constexpr rect(int x, int y, int w, int h) : SDL_Rect{x, y, w, h} {}

	/** Specify via top-left corner position and size. */
	constexpr rect(const point& pos, const point& size)
		: SDL_Rect{pos.x, pos.y, size.x, size.y}
	{}

	// subcomponent access
	constexpr point origin() const { return {x, y}; }
	constexpr point size() const { return {w, h}; }

	// Comparisons
	bool operator==(const rect& r) const;
	bool operator==(const SDL_Rect& r) const;

	// Scalar multiplication and division
	constexpr rect operator*(int s) const
	{
		return {x * s, y * s, w * s, h * s};
	}

	constexpr rect& operator*=(int s)
	{
		x *= s;
		y *= s;
		w *= s;
		h *= s;
		return *this;
	}

	constexpr rect operator/(int s) const
	{
		return {x / s, y / s, w / s, h / s};
	}

	constexpr rect& operator/=(int s)
	{
		x /= s;
		y /= s;
		w /= s;
		h /= s;
		return *this;
	}

	/** The area of this rectangle, in square pixels. */
	constexpr int area() const { return w * h; }

	/** The center point of the rectangle, accounting for origin. */
	constexpr point center() const
	{
		return {x + w / 2, y + h / 2};
	}

	/** False if both w and h are > 0, true otherwise. */
	bool empty() const;

	/** Whether the given point lies within the rectangle. */
	bool contains(int x, int y) const;
	bool contains(const point& p) const;

	/** Whether the given rectangle is completely contained by this one. */
	bool contains(const SDL_Rect& r) const;

	/** Whether the given rectangle and this rectangle overlap. */
	bool overlaps(const SDL_Rect& r) const;

	/**
	 * Calculates the minimal rectangle that completely contains both
	 * this rectangle and the given rectangle.
	 */
	rect minimal_cover(const SDL_Rect& r) const;

	/** Minimally expand this rect to fully contain another. */
	rect& expand_to_cover(const SDL_Rect& r);

	/**
	 * Calculates the intersection of this rectangle and another;
	 * that is, the maximal rectangle that is contained by both.
	 */
	rect intersect(const SDL_Rect& r) const;

	/**
	 * Clip this rectangle by the given rectangle.
	 *
	 * This rectangle will be reduced to the intersection of both rectangles.
	 */
	void clip(const SDL_Rect& r);

	/**
	 * Shift the rectangle by the given relative position.
	 *
	 * The point's X and Y coordinates will be added to the rectangle's.
	 */
	void shift(const point& p);

	/** Returns a new rectangle shifted by the given relative position. */
	rect shifted_by(int x, int y) const;
	rect shifted_by(const point& p) const;

	/** Returns a new rectangle with @a dx horizontal padding and @a dy vertical padding. */
	constexpr rect padded_by(int dx, int dy) const
	{
		return { x - dx, y - dy, w + dx * 2, h + dy * 2 };
	}

	/** Returns a new rectangle with equal @a amount horizontal and vertical padding. */
	constexpr rect padded_by(int amount) const
	{
		return padded_by(amount, amount);
	}

	/** Returns the proper point that corresponds to the given [0.0, 1.0] coordinates. */
	point point_at(double x, double y) const;
};

std::ostream& operator<<(std::ostream&, const rect&);

namespace sdl
{
#ifdef __cpp_concepts
template<typename T>
concept Rectangle = requires(T r)
{
	r.x;
	r.y;
	r.w;
	r.h;
};

/** Returns the sub-rect bounded to the top left and bottom right by the given [0.0, 1.0] coordinates. */
constexpr SDL_FRect precise_subrect(const Rectangle auto& base, const SDL_FPoint& tl, const SDL_FPoint& br)
#else
template<typename Rect>
constexpr SDL_FRect precise_subrect(const Rect& base, const SDL_FPoint& tl, const SDL_FPoint& br)
#endif
{
	const auto point_at = [&base](auto x, auto y) -> SDL_FPoint {
		return {
			base.x + base.w * std::clamp(x, 0.0f, 1.0f),
			base.y + base.h * std::clamp(y, 0.0f, 1.0f)
		};
	};

	SDL_FPoint p1 = point_at(tl.x, tl.y);
	SDL_FPoint p2 = point_at(br.x, br.y);

	return { p1.x, p1.y, p2.x - p1.x, p2.y - p1.y };
}

} // namespace sdl
