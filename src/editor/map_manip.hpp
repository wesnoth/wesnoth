/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

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

namespace map_editor {

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
				

}

#endif // MAP_MANIP_H_INCLUDED
