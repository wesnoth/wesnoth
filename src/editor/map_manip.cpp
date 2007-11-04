/* $Id$ */
/*
  Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

//! @file editor/map_manip.cpp
//! Operations on a game-map.

#include "map_manip.hpp"

#include "../gettext.hpp"
#include "../map.hpp"
#include "../config.hpp"
#include "../construct_dialog.hpp"
#include "../util.hpp"
#include "../wassert.hpp"
#include "serialization/string_utils.hpp"

#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <sstream>

std::string editormap::resize(const size_t width, const size_t height,
	const int x_offset, const int y_offset,
	const bool do_expand, t_translation::t_letter filler)
{

	const unsigned old_w = static_cast<unsigned>(w());
	const unsigned old_h = static_cast<unsigned>(h());

	// no changes leave
	if(old_w == width && old_h == height && x_offset == 0 && y_offset == 0 ) {
		return "";
	}

	if(do_expand) {
		filler = t_translation::NONE_TERRAIN;
	}

	// Determine the amount of resizing is required
	const int left_resize = -x_offset;
	const int right_resize = (width - old_w) + x_offset;
	const int top_resize = -y_offset;
	const int bottom_resize = (height - old_h) + y_offset;

//	std::cerr << "resize y " << left_resize << " x " << right_resize
//		<< " top " << top_resize << " bottom " << bottom_resize << '\n';

	if(right_resize > 0) {
		add_tiles_right(right_resize, filler);
	} else if(right_resize < 0) {
		remove_tiles_right(-right_resize);
	}

	if(bottom_resize > 0) {
		add_tiles_bottom(bottom_resize, filler);
	} else if(bottom_resize < 0) {
		remove_tiles_bottom(-bottom_resize);
	}

	if(left_resize > 0) {
		add_tiles_left(left_resize, filler);
	} else if(left_resize < 0) {
		remove_tiles_left(-left_resize);
	}

	if(top_resize > 0) {
		add_tiles_top(top_resize, filler);
	} else if(top_resize < 0) {
		remove_tiles_top(-top_resize);
	}

	// fix the starting positions
	if(x_offset || y_offset) {
		for(size_t i = 0; i < STARTING_POSITIONS; ++i) {
			if(startingPositions_[i] != gamemap::location()) {
				startingPositions_[i].x -= x_offset;
				startingPositions_[i].y -= y_offset;
			}
		}
	}


	return write();
}

std::string editormap::flip(const map_editor::FLIP_AXIS axis)
{
	if(axis !=  map_editor::FLIP_X && axis != map_editor::FLIP_Y) {
		return "";
	}

	if(axis == map_editor::FLIP_X) {
		// Due to the hexes we need some mirror tricks when mirroring over the
		// X axis. We resize the map and fill it. The odd columns will be extended
		// with the data in row 0 the even columns are extended with the data in
		// the last row
		const size_t middle = (tiles_[0].size() / 2); // the middle if reached we flipped all
		const size_t end = tiles_[0].size() - 1; // the last row _before_ resizing
		for(size_t x = 0; x < tiles_.size(); ++x) {
			if(x % 2) {
				// odd lines
				tiles_[x].resize(tiles_[x].size() + 1, tiles_[x][0]);

				for(size_t y1 = 0, y2 = end; y1 < middle; ++y1, --y2) {
					swap_starting_position(x, y1, x, y2);
					std::swap(tiles_[x][y1], tiles_[x][y2]);
				}
			} else {
				// even lines
				tiles_[x].resize(tiles_[x].size() + 1, tiles_[x][end]);

				for(size_t y1 = 0, y2 = end + 1; y1 < middle; ++y1, --y2) {
					swap_starting_position(x, y1, x, y2);
					std::swap(tiles_[x][y1], tiles_[x][y2]);
				}

			}
		}
	} else { // FLIP_Y
		// Flipping on the Y axis requires no resize,
		// so the code is much simpler.
		const size_t middle = (tiles_.size() / 2);
		const size_t end = tiles_.size() - 1;
		for(size_t y = 0; y < tiles_[0].size(); ++y) {
			for(size_t x1 = 0, x2 = end; x1 < middle; ++x1, --x2) {
				swap_starting_position(x1, y, x2, y);
				std::swap(tiles_[x1][y], tiles_[x2][y]);
			}
		}
	}

	return write();
}

void editormap::set_starting_position(const int pos, const location loc) {
	startingPositions_[pos] = loc;
}

void editormap::swap_starting_position(const size_t x1, const size_t y1,
	const size_t x2, const size_t y2)
{
	const int pos1 = is_starting_position(location(x1, y1));
	const int pos2 = is_starting_position(location(x2, y2));

	if(pos1 != -1) {
		set_starting_position(pos1 + 1, location(x2, y2));
	}

	if(pos2 != -1) {
		set_starting_position(pos2 + 1, location(x1, y1));
	}
}

void editormap::add_tiles_right(
	const unsigned count, const t_translation::t_letter& filler)
{
	for(size_t x = 1; x <= count; ++x) {
		t_translation::t_list one_row(tiles_[1].size());
		for(size_t y = 0; y < tiles_[1].size(); ++y) {
			one_row[y] =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[x - 1][y];

			wassert(one_row[y] != t_translation::NONE_TERRAIN);
		}
		tiles_.push_back(one_row);
	}
}

void editormap::add_tiles_left(
	const unsigned count, const t_translation::t_letter& filler)
{
	for(size_t i = 1; i <= count; ++i) {
		t_translation::t_list one_row(tiles_[1].size());
		for(size_t y = 0; y < tiles_[1].size(); ++y) {
			one_row[y] =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[0][y];

			wassert(one_row[y] != t_translation::NONE_TERRAIN);
		}
		tiles_.insert(tiles_.begin(), 1, one_row);
		clear_border_cache();
	}
}

void editormap::remove_tiles_right(const unsigned count)
{
	if(count > tiles_.size()) {
		std::stringstream sstr;
		sstr << _("Can't resize the map, the requested size is bigger "
			"than the maximum, size=") << count << _(" maximum=")
			<< tiles_.size();

		throw incorrect_format_exception(sstr.str().c_str());
	}

	tiles_.resize(tiles_.size() - count);
}

void editormap::remove_tiles_left(const unsigned count)
{
	if(count > tiles_.size()) {
		std::stringstream sstr;
		sstr << _("Can't resize the map, the requested size is bigger "
			"than the maximum, size=") << count << _(" maximum=")
			<< tiles_.size();

		throw incorrect_format_exception(sstr.str().c_str());
	}

	tiles_.erase(tiles_.begin(), tiles_.begin() + count);
}

void editormap::add_tiles_top(
	const unsigned count, const t_translation::t_letter& filler)
{
	for(size_t i =  1; i <= count; ++i) {
		for(size_t y = 0; y < tiles_.size(); ++y) {
			t_translation::t_letter terrain =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[y][0];

			wassert(terrain != t_translation::NONE_TERRAIN);
			tiles_[y].insert(tiles_[y].begin(), 1, terrain);
			clear_border_cache();
		}
	}
}

void editormap::add_tiles_bottom(
	const unsigned count, const t_translation::t_letter& filler)
{
	for(size_t i =  1; i <= count; ++i) {
		for(size_t x = 0; x < tiles_.size(); ++x) {
			t_translation::t_letter terrain =
				filler != t_translation::NONE_TERRAIN ?
				filler :
				tiles_[x][i - 1];

			wassert(terrain != t_translation::NONE_TERRAIN);
			tiles_[x].push_back(terrain);
		}
	}
}

void editormap::remove_tiles_top(const unsigned count)
{
	if(count > tiles_[0].size()) {
		std::stringstream sstr;
		sstr << _("Can't resize the map, the requested size is bigger "
			"than the maximum, size=") << count << _(" maximum=")
			<< tiles_[0].size();

		throw incorrect_format_exception(sstr.str().c_str());
	}

	for(size_t x = 0; x < tiles_.size(); ++x) {
		tiles_[x].erase(tiles_[x].begin(), tiles_[x].begin() + count);
	}
}

void editormap::remove_tiles_bottom(const unsigned count)
{
	if(count > tiles_[0].size()) {
		std::stringstream sstr;
		sstr << _("Can't resize the map, the requested size is bigger "
			"than the maximum, size=") << count << _(" maximum=")
			<< tiles_[0].size();

		throw incorrect_format_exception(sstr.str().c_str());
	}

	for(size_t x = 0; x < tiles_.size(); ++x) {
		tiles_[x].erase(tiles_[x].end() - count, tiles_[x].end());
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
			loc = loc.get_direction(gamemap::location::NORTH, 1);
		}
		// Get all the tiles clockwise with distance d.
		const gamemap::location::DIRECTION direction[6] =
			{gamemap::location::SOUTH_EAST, gamemap::location::SOUTH, gamemap::location::SOUTH_WEST,
			 gamemap::location::NORTH_WEST, gamemap::location::NORTH, gamemap::location::NORTH_EAST};
		for (i = 0; i < 6; i++) {
			for (unsigned int j = 1; j <= d; j++) {
				loc = loc.get_direction(direction[i], 1);
				if (map.on_board(loc, true)) {
					res.push_back(loc);
				}
			}
		}
	}
	return res;
}

void flood_fill(gamemap &map, const gamemap::location &start_loc,
				const t_translation::t_letter fill_with, terrain_log *log)
{
	t_translation::t_letter terrain_to_fill = map.get_terrain(start_loc);
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

std::set<gamemap::location> get_component(const gamemap &map,
		const gamemap::location &start_loc)
{
	t_translation::t_letter terrain_to_fill = map.get_terrain(start_loc);
	std::set<gamemap::location> to_fill;
	std::set<gamemap::location> filled;
	std::set<gamemap::location>::iterator it;
	// Insert the start location in a set.
	// Chose an element in the set, mark this element,
	// and add all adjacent elements that are not marked.
	// Continue until the set is empty.
	to_fill.insert(start_loc);
	while (!to_fill.empty()) {
		it = to_fill.begin();
		gamemap::location loc = *it;
		to_fill.erase(it);
		filled.insert(loc);
		// Find all adjacent tiles with the terrain that should
		// be filled and add these to the to_fill vector.
		std::vector<gamemap::location> adj = get_tiles(map, loc, 2);
		for (std::vector<gamemap::location>::iterator it2 = adj.begin();
			 it2 != adj.end(); it2++) {
			if (map.on_board(*it2, true) && map.get_terrain(*it2) == terrain_to_fill
				&& filled.find(*it2) == filled.end()) {
				to_fill.insert(*it2);
			}
		}
	}
	return filled;
}

std::string resize_map(editormap &map, const unsigned new_w,
	const unsigned new_h, const int off_x, const int off_y,
	const bool do_expand, const t_translation::t_letter fill_with)
{
	return map.resize(new_w, new_h, off_x, off_y, do_expand, fill_with);
}


std::string flip_map(editormap &map, const FLIP_AXIS axis) {
	return map.flip(axis);
}

bool valid_mapdata(const std::string &data, const config &cfg) {
	bool res = data.size() != 0;
	// Create a map and see if we get an exception. Not very efficient,
	// but simple as things are implemented now.
	try {
		const gamemap m(cfg, data, gamemap::SINGLE_TILE_BORDER, gamemap::IS_MAP);
		// Having a zero size map may cause floating point exceptions
		// at some places later on.
		res = m.w() != 0 && m.h() != 0;
	}
	catch (gamemap::incorrect_format_exception) {
		res = false;
	}
	return res;
}

std::string new_map(const size_t width, const size_t height, const t_translation::t_letter filler)
{
	const t_translation::t_list column(height, filler);
	const t_translation::t_map map(width, column);

	return gamemap::default_map_header + t_translation::write_game_map(map);

}

} // end namespace map_editor

