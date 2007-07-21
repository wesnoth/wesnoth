/* $Id$ */
/*
  Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#ifndef MAP_MANIP_H_INCLUDED
#define MAP_MANIP_H_INCLUDED

#include "global.hpp"

#include "../map.hpp"

#include <vector>
#include <set>

namespace map_editor {
enum FLIP_AXIS {NO_FLIP, FLIP_X, FLIP_Y};
}

//This object modifies the internal structure of a map, most of the time this is
//done on the real map data. This means that the display and mapdata are heavily out
//of sync. Our callers throw a new_map_exception which will invalide the entire
//map object. These callers expect a string with the new raw map data.
//So it's "save" but really not clean. -- Mordante
class editormap : public gamemap
{
public:
	editormap(const config& terrain_cfg, const std::string& data):gamemap(terrain_cfg,data){};
	~editormap(){};

	/**
	 * Resizes the map
	 *
	 * @param width 	the new width
	 * @param height	the new height
	 * @param filler	if the map is enlarged the new tiles are set to this terrain
	 *
	 * @return			if there's been a modification to the map the new map data as string
	 * 					else an empty string
	 */
	std::string resize(const size_t width, const size_t height, const t_translation::t_letter filler); 
	
	/**
	 * Flips the map over an axis
	 *
	 * @param axis 		the axis to flip the map over
	 *
	 * @return			if there's been a modification to the map the new map data as string
	 * 					else an empty string
	 */
	std::string flip(const map_editor::FLIP_AXIS axis);

	/**
	 * sets the starting position of a player
	 *
	 * @param pos 		the starting position, 1 = player 1
	 * @location		a location (same as gamemap location)
	 */
	void set_starting_position(const int pos, const location loc);

private:

	/**
	 * Exchanges starting positions, if there's a starting location on x1, xy
	 * it will be moved to x2, y2. If x2, y2 contains a starting location this
	 * is moved to x1, y1. The function also works if both locations contain
	 * a starting position.
	 */
	void swap_starting_position(const size_t x1, const size_t y1,
								const size_t x2, const size_t y2);
};

namespace map_editor {

/// Return the tiles that are within radius from the location.
std::vector<gamemap::location> get_tiles(const gamemap &map,
										 const gamemap::location& a,
										 const unsigned int radius);

typedef std::vector<std::pair<gamemap::location, t_translation::t_letter> > terrain_log;

/// Flood fill the map with the terrain fill_with starting from the
/// location start_loc. If log is non-null it will contain the positions
/// of the changed tiles and the terrains they had before the filling
/// started.
void flood_fill(gamemap &map, const gamemap::location &start_loc,
		const t_translation::t_letter fill_with, terrain_log *log = NULL);

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
std::string resize_map(editormap &map, const unsigned new_w,
   const unsigned new_h, const t_translation::t_letter fill_with);

/// Return the string representation of the map after it has been
/// flipped around the axis.
std::string flip_map(editormap &map, const FLIP_AXIS axis);

/// Return true if the data is valid to create a map with, othwerwise
/// false.
bool valid_mapdata(const std::string &data, const config &cfg);


// Returns a string representating a new empty map
// of width by height of the terrain filler
std::string new_map(const size_t width, const size_t height, const t_translation::t_letter filler);

} //namespace


#endif // MAP_MANIP_H_INCLUDED
