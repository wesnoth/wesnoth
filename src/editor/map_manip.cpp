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

namespace map_editor {

std::vector<gamemap::location> get_tiles(const gamemap &map,
										 const gamemap::location& a,
										 const unsigned int radius) {
	const unsigned int distance = radius - 1;
	std::vector<gamemap::location> res;
	res.push_back(a);
	for (int d = 1; d <= distance; d++) {
		gamemap::location loc = a;
		int i;
		// Get the starting point.
		for (i = 1; i <= d; i++) {
			loc = loc.get_direction(gamemap::location::NORTH);
		}
		// Get all the tiles clockwise with distance d.
		const gamemap::location::DIRECTION direction[6] =
			{gamemap::location::SOUTH_EAST, gamemap::location::SOUTH, gamemap::location::SOUTH_WEST,
			 gamemap::location::NORTH_WEST, gamemap::location::NORTH, gamemap::location::NORTH_EAST};
		for (i = 0; i < 6; i++) {
			for (int j = 1; j <= d; j++) {
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
	std::vector<gamemap::location> to_fill;
	// First push the starting location onto a stack. Then, in every
	// iteration, pop one element from the stack, fill this tile and add
	// all adjacent tiles that have the terrain that should be
	// filled. Continue until the stack is empty.
	to_fill.push_back(start_loc);
	while (!to_fill.empty()) {
		gamemap::location loc = to_fill.back();
		to_fill.pop_back();
		if (log != NULL) {
			log->push_back(std::make_pair(loc, terrain_to_fill));
		}
		map.set_terrain(loc, fill_with);
		// Find all adjacent tiles with the terrain that should be
		// filled and add these to the to_fill vector.
		std::vector<gamemap::location> adj = get_tiles(map, loc, 2);
		for (std::vector<gamemap::location>::iterator it2 = adj.begin();
			 it2 != adj.end(); it2++) {
			if (map.get_terrain(*it2) == terrain_to_fill && map.on_board(*it2)) {
				to_fill.push_back(*it2);
			}
		}
		// Remove duplicates.
		std::sort(to_fill.begin(), to_fill.end());
		std::vector<gamemap::location>::iterator end_of_unique =
			std::unique(to_fill.begin(), to_fill.end());
		to_fill.erase(end_of_unique, to_fill.end());
	}
}

}
