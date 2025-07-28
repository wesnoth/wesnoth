/*
	Copyright (C) 2018 - 2025
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
#include <set>

/**
 * Function that will add to @a result all elements of @a locs, plus all
 * on-board (that is: all locs that match @a pred1) locations matching @a pred2
 * that are connected to elements of
 * locs by a chain of at most @a radius tiles, each of which matches @a pred2.
 * @a result must be a std::set of locations.
 *
 * @a pred1 a fast predicate (used before cachecheck).
 * @a pred2 a slow predicate (used after cachecheck).
 */
template<typename FPred1, typename FPred2>
void get_tiles_radius(std::set<map_location>&& locs, std::size_t radius, std::set<map_location>& result, const FPred1& pred1, const FPred2& pred2)
{
	typedef std::set<map_location> location_set;
	location_set must_visit, filtered_out;
	location_set not_visited = std::move(locs);

	for ( ; radius != 0  &&  !not_visited.empty(); --radius )
	{
		location_set::const_iterator it = not_visited.begin();
		location_set::const_iterator it_end = not_visited.end();

		result.insert(it, it_end);
		for(; it != it_end; ++it) {
			for(const map_location& loc : get_adjacent_tiles(*it)) {
				if( pred1(loc) ) {
					if( !result.count(loc) && !filtered_out.count(loc) ) {
						if( pred2(loc) ) {
							must_visit.insert(loc);
						}
						else {
							filtered_out.insert(loc);
						}
					}
				}
			}
		}

		not_visited.swap(must_visit);
		must_visit.clear();
	}

	result.insert(not_visited.begin(), not_visited.end());
}
