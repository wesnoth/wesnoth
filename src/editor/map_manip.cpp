/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#include "../map.hpp"

#include "map_manip.hpp"

#include <vector>
#include <map>
#include <algorithm>
#include <set>

namespace map_editor {

std::vector<gamemap::location> get_tiles(const gamemap &map,
										 const gamemap::location& a,
										 const unsigned int radius) {
	const unsigned int distance = radius - 1;
	std::vector<gamemap::location> res;
	res.push_back(a);
	for (unsigned int d = 1; d <= distance; d++) {
		gamemap::location loc = a;
		unsigned int i;
		// Get the starting point.
		for (i = 1; i <= d; i++) {
			loc = loc.get_direction(gamemap::location::NORTH);
		}
		// Get all the tiles clockwise with distance d.
		const gamemap::location::DIRECTION direction[6] =
			{gamemap::location::SOUTH_EAST, gamemap::location::SOUTH, gamemap::location::SOUTH_WEST,
			 gamemap::location::NORTH_WEST, gamemap::location::NORTH, gamemap::location::NORTH_EAST};
		for (i = 0; i < 6; i++) {
			for (unsigned int j = 1; j <= d; j++) {
				loc = loc.get_direction(direction[i]);
				if (map.on_board(loc)) {
					res.push_back(loc);
				}
			}
		}
	}
	return res;
}


void flood_fill(gamemap &map, const gamemap::location &start_loc,
				const gamemap::TERRAIN fill_with, terrain_log *log) {
	gamemap::TERRAIN terrain_to_fill = map.get_terrain(start_loc);
	if (fill_with == terrain_to_fill) {
		return;
	}
	std::set<gamemap::location> to_fill = get_component(map, start_loc);
	std::set<gamemap::location>::iterator it;
	for (it = to_fill.begin(); it != to_fill.end(); it++) {
		gamemap::location loc = *it;
		if (log != NULL) {
			log->push_back(std::make_pair(loc, terrain_to_fill));
		}
		map.set_terrain(loc, fill_with);
	}
}

std::set<gamemap::location>
get_component(const gamemap &map, const gamemap::location &start_loc) {
	gamemap::TERRAIN terrain_to_fill = map.get_terrain(start_loc);
	std::set<gamemap::location> to_fill;
	std::set<gamemap::location> filled;
	std::set<gamemap::location>::iterator it;
	// Insert the start location in a set. Chose an element in the set,
	// mark this element, and add all adjacent elements that are not
	// marked. Continue until the set is empty.
	to_fill.insert(start_loc);
	while (!to_fill.empty()) {
		it = to_fill.begin();
		gamemap::location loc = *it;
		to_fill.erase(it);
		filled.insert(loc);
		// Find all adjacent tiles with the terrain that should be
		// filled and add these to the to_fill vector.
		std::vector<gamemap::location> adj = get_tiles(map, loc, 2);
		for (std::vector<gamemap::location>::iterator it2 = adj.begin();
			 it2 != adj.end(); it2++) {
			if (map.on_board(*it2) && map.get_terrain(*it2) == terrain_to_fill
				&& filled.find(*it2) == filled.end()) {
				to_fill.insert(*it2);
			}
		}
	}
	return filled;
}





}
