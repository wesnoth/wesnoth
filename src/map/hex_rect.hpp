/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "map/location.hpp"

#include <array>
#include <iterator>

/**
 * Rectangular area of hexes.
 *
 * Allows decising how the top and bottom edges handles the vertical shift
 * for parity with the x coordinate.
 *
 * @todo Improve this documentation because I'm not sure best how to describe
 * the function of this class.
 *
 * -- vultraz, 2018-03-24
 */
struct rect_of_hexes
{
	/** Default ctor. Will evaluate as begin() == end(). */
	rect_of_hexes()
		: left(0)
		, right(-1) // end is right + 1
		, top{{0,0}}
		, bottom{{0,0}}
	{
	}

	int left;
	int right;

	std::array<int, 2> top; // for even and odd values of x, respectively
	std::array<int, 2> bottom;

	/** Very simple iterator to walk into the rect_of_hexes */
	struct iterator
	{
		iterator(const map_location& loc, const rect_of_hexes& rect)
			: loc_(loc)
			, rect_(rect)
		{
		}

		/** Increment y first, then when reaching bottom, increment x. */
		iterator& operator++()
		{
			if(loc_.y < rect_.bottom[loc_.x & 1]) {
				++loc_.y;
			} else {
				++loc_.x;
				loc_.y = rect_.top[loc_.x & 1];
			}

			return *this;
		}

		bool operator==(const iterator& that) const
		{
			return that.loc_ == loc_;
		}

		bool operator!=(const iterator& that) const
		{
			return that.loc_ != loc_;
		}

		const map_location& operator*() const
		{
			return loc_;
		}

		using iterator_category = std::forward_iterator_tag;
		using value_type = map_location;
		using difference_type = int;
		using pointer = const map_location*;
		using reference = const map_location&;

	private:
		map_location loc_;
		const rect_of_hexes& rect_;
	};

	using const_iterator = iterator;

	iterator begin() const
	{
		return iterator(map_location(left, top[left & 1]), *this);
	}

	iterator end() const
	{
		return iterator(map_location(right + 1, top[(right + 1) & 1]), *this);
	}
};
