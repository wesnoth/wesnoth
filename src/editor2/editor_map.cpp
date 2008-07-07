/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor_map.hpp"

namespace editor2 {

editor_map::editor_map(const config& terrain_cfg, const std::string& data)
: gamemap(terrain_cfg, data)
{
}

editor_map editor_map::new_map(const config& terrain_cfg, size_t width, size_t height, t_translation::t_terrain filler)
{
	const t_translation::t_list column(height, filler);
	const t_translation::t_map map(width, column);
	return editor_map(terrain_cfg, gamemap::default_map_header + t_translation::write_game_map(map));
}
	
std::vector<gamemap::location> editor_map::get_tiles_in_radius(const gamemap::location& center, const unsigned int radius) {
	const unsigned int distance = radius - 1;
	std::vector<gamemap::location> res;
	res.push_back(center);
	for (unsigned int d = 1; d <= distance; d++) {
		gamemap::location loc = center;
		unsigned int i;
		// Get the starting point.
		for (i = 1; i <= d; i++) {
			loc = loc.get_direction(gamemap::location::NORTH, 1);
		}
		// Get all the tiles clockwise with distance d.
		const gamemap::location::DIRECTION direction[6] =
			{gamemap::location::SOUTH_EAST, gamemap::location::SOUTH, gamemap::location::SOUTH_WEST,
			 gamemap::location::NORTH_WEST, gamemap::location::NORTH, gamemap::location::NORTH_EAST};
		for (i = 0; i < 6; i++) {
			for (unsigned int j = 1; j <= d; j++) {
				loc = loc.get_direction(direction[i], 1);
				if (on_board_with_border(loc)) {
					res.push_back(loc);
				}
			}
		}
	}
	return res;
}

bool editor_map::in_selection(const gamemap::location& loc) const
{
	return selection_.find(loc) != selection_.end();
}

bool editor_map::add_to_selection(const gamemap::location& loc)
{
	return selection_.insert(loc).second;
}

bool editor_map::remove_from_selection(const gamemap::location& loc)
{
	return selection_.erase(loc);
}

void editor_map::clear_selection()
{
	selection_.clear();
}

void editor_map::invert_selection()
{
	std::set<gamemap::location> new_selection;
	for (int x = 0; x < w(); ++x) {
		for (int y = 0; y < h(); ++y) {
			if (selection_.find(gamemap::location(x, y)) == selection_.end()) {
				new_selection.insert(gamemap::location(x, y));
			}
		}
	}
	selection_.swap(new_selection);
}

void editor_map::select_all()
{
	clear_selection();
	invert_selection();
}

} //end namespace editor2
