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

#include "../pathutils.hpp"

#include <deque>


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

std::set<gamemap::location> editor_map::get_contigious_terrain_tiles(const gamemap::location& start) const
{
	t_translation::t_terrain terrain = get_terrain(start);
	std::set<gamemap::location> result;
	std::deque<gamemap::location> queue;
	result.insert(start);
	queue.push_back(start);
	//this is basically a breadth-first search along adjacent hexes
	do {
		gamemap::location adj[6];
		get_adjacent_tiles(queue.front(), adj);
		for (int i = 0; i < 6; ++i) {
			if (on_board_with_border(adj[i]) && get_terrain(adj[i]) == terrain
			&& result.find(adj[i]) == result.end()) {
				result.insert(adj[i]);
				queue.push_back(adj[i]);
			}
		}
		queue.pop_front();
	} while (!queue.empty());
	return result;
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
