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
#include "../config.hpp"
#include "../util.hpp"
#include "serialization/string_utils.hpp"

#include "map_manip.hpp"

#include <vector>
#include <map>
#include <algorithm>
#include <set>

namespace {
	
	// Grow the map, represented by lines, one step. If grow_height is
	// true, the height is increased and every other column is shifted
	// one step downwards. Otherwise the map is increased in width one
	// step. The terrain used as padding is deducted from the surrounded
	// terrain.
	void grow_and_pad(const gamemap &map, std::vector<std::string> &lines,
					  bool grow_height) {
		int i, j;
		std::vector<gamemap::TERRAIN> terrains;
		gamemap::TERRAIN chosen_terrain;
		if (grow_height) {
			lines.push_back(std::string(lines[0].size(),
										gamemap::FOREST));
			// Shift terrain on odd numbered columns one step downwards.
			for (i = 0; (unsigned)i < lines[0].size(); i += 2) {
				for (j = lines.size() - 2; j >= 0; j--) {
					lines[j + 1][i] = lines[j][i];
				}
			}
			// Set the terrain for the hexes that was used as padding.
			for (i = 0; (unsigned)i < lines[0].size(); i++) {
				terrains.clear();
				if (is_even(i)) {
					terrains.push_back(lines[1][i]);
					terrains.push_back(i == 0 ? terrains[0] : lines[0][i - 1]);
					terrains.push_back((unsigned)i == lines[0].size() - 1
									   ? terrains[0] : lines[0][i + 1]);
				}
				else {
					terrains.push_back(lines[lines.size() - 2][i]);
					terrains.push_back(i == 0
									   ? terrains[0] :
									   lines[lines.size() - 1][i - 1]);
					terrains.push_back((unsigned)i == lines[0].size() - 1 ?
						terrains[0] : lines[lines.size() - 1][i + 1]);
				}
				if (terrains[1] == terrains[2]) {
					chosen_terrain = terrains[2];
				}
				else {
					chosen_terrain = terrains[0];
				}
				if (map.is_village(chosen_terrain)) {
					for (j = 0; j < 3; j++) {
						if (!map.is_village(terrains[j])) {
							chosen_terrain = terrains[j];
							break;
						}
					}
				}
				if (map.is_village(chosen_terrain)) {
					chosen_terrain = gamemap::FOREST;
				}
				if (is_even(i)) {
					lines[0][i] = chosen_terrain;
				}
				else {
					lines[lines.size() - 1][i] = chosen_terrain;
				}
			}
		}
		else {
			for (i = 0; (unsigned)i < lines.size(); i++) {
				int change;
				terrains.clear();
				terrains.push_back(lines[i][lines[i].length() - 1]);
				if (is_even(lines[i].size()+1)) {
					change = 1;
				}
				else {
					change = -1;
				}
				if (i + change > 0 && (unsigned)(i + change) < lines.size()) {
					terrains.push_back(lines[i + change][lines[i].length() - 1]);
				}
				else {
					terrains.push_back(terrains[0]);
				}
				chosen_terrain = map.is_village(terrains[0])
					? terrains[1] : terrains[0];
				chosen_terrain = map.is_village(chosen_terrain)
					? gamemap::FOREST : chosen_terrain;
				lines[i].resize(lines[i].length() + 1, chosen_terrain);
			}
		}
	}
}

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

std::string resize_map(const gamemap &map, const unsigned new_w,
					   const unsigned new_h, const gamemap::TERRAIN fill_with) {
	std::string str_map = map.write();
	std::vector<std::string> lines = utils::split(str_map, '\n');
	bool map_changed = false;
	const unsigned old_w = (unsigned)map.x();
	const unsigned old_h = (unsigned)map.y();
	if (old_h != new_h) {
		const std::string one_row(old_w, fill_with);
		lines.resize(new_h, one_row);
		map_changed = true;
	}
	if (new_w != old_w) {
		for (std::vector<std::string>::iterator it = lines.begin();
			 it != lines.end(); it++) {
			(*it).resize(new_w, fill_with);
		}
		map_changed = true;
	}
	if (map_changed) {
		return utils::join(lines, '\n');
	}
	else {
		return "";
	}
}


std::string flip_map(const gamemap &map, const FLIP_AXIS axis) {
	const std::string str_map = map.write();
	if (str_map == "") {
		return str_map;
	}
	std::vector<std::string> lines = utils::split(str_map, '\n');
	std::vector<std::string> new_lines;
	if (axis == FLIP_Y) {
		if (is_even(lines[0].size())) {
			grow_and_pad(map, lines, false);
		}
		new_lines.resize(lines.size());
		std::vector<std::string>::iterator new_line_it = new_lines.begin();
		for (std::vector<std::string>::const_iterator it = lines.begin();
			 it != lines.end(); it++) {
			for (std::string::const_reverse_iterator sit = (*it).rbegin();
				 sit != (*it).rend(); sit++) {
				push_back(*new_line_it,*sit);
			}
			new_line_it++;
		}
	}
	else if (axis == FLIP_X) {
		std::vector<std::string>::reverse_iterator it;
		for (it = lines.rbegin(); it != lines.rend(); it++) {
			new_lines.push_back(*it);
		}
		grow_and_pad(map, new_lines, true);
	}
	else {
		new_lines = lines;
	}
	return utils::join(new_lines, '\n');
}

bool valid_mapdata(const std::string &data, const config &cfg) {
	bool res = data.size() != 0;
	// Create a map and see if we get an exception. Not very efficient,
	// but simple as things are implemented now.
	try {
		const gamemap m(cfg, data);
		// Having a zero size map may cause floating point exceptions at
		// some places later on.
		res = m.x() != 0 && m.y() != 0;
	}
	catch (gamemap::incorrect_format_exception) {
		res = false;
	}
	return res;
}
	

}
