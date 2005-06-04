/*
  Copyright (C) 2003 by David White <davidnwhite@comcast.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#ifndef MAP_MANIP_H_INCLUDED
#define MAP_MANIP_H_INCLUDED

#include "../map.hpp"

#include <vector>
#include <set>

namespace map_editor {

/// Return the tiles that are within radius from the location.
std::vector<gamemap::location> get_tiles(const gamemap &map,
										 const gamemap::location& a,
										 const unsigned int radius);

typedef std::vector<std::pair<gamemap::location, gamemap::TERRAIN> > terrain_log;

/// Flood fill the map with the terrain fill_with starting from the
/// location start_loc. If log is non-null it will contain the positions
/// of the changed tiles and the terrains they had before the filling
/// started.
void flood_fill(gamemap &map, const gamemap::location &start_loc,
				const gamemap::TERRAIN fill_with, terrain_log *log = NULL);

/// Return the area that would be flood filled if a flood fill was
/// requested.
std::set<gamemap::location>
get_component(const gamemap &map, const gamemap::location &start_loc);

/// Return the string representation of the map after it has been
/// resized to new_w X new_h. If the new dimensions are smaller than the
/// current ones, the map will be cropped from the bottom and from the
/// right. If the map becomes larger than the current dimensions, the
/// new map area appeard at the bottom and/or the right and is filled
/// with the terrain fill_with.
std::string resize_map(const gamemap &map, const unsigned new_w,
					   const unsigned new_h, const gamemap::TERRAIN fill_with);

enum FLIP_AXIS {NO_FLIP, FLIP_X, FLIP_Y};
/// Return the string representation of the map after it has been
/// flipped around the axis.
std::string flip_map(const gamemap &map, const FLIP_AXIS axis);

/// Return true if the data is valid to create a map with, othwerwise
/// false.
bool valid_mapdata(const std::string &data, const config &cfg);
}

#endif // MAP_MANIP_H_INCLUDED
