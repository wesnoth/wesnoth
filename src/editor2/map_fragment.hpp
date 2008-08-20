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

#ifndef EDITOR2_MAP_FRAGMENT_HPP_INCLUDED
#define EDITOR2_MAP_FRAGMENT_HPP_INCLUDED

#include "editor_map.hpp"

#include "../config.hpp"

#include <set>

namespace editor2 {

/**
 * This represents a tile along with information about it, namely the terrain,
 * possibly other information. It is a less compact representation that what
 * is used in the map, but is more convenient in some situations.
 */
struct tile_info
{
	/**
	 * Create a tile info -- the constructor grabs required data from the map
	 */
	tile_info(const gamemap& map, const gamemap::location& offset)
	: offset(offset), terrain(map.get_terrain(offset))
	{
	}
	
	gamemap::location offset;
	t_translation::t_terrain terrain;
};

/**
 * A map fragment -- a collection of locations and information abut them.
 */
class map_fragment
{
	public:
		/**
		 * Create an empty map fragment.
		 */
		map_fragment();
		
		/**
		 * Create a map fragment from the specified locations on the map.
		 */
		map_fragment(const gamemap& map, const std::set<gamemap::location>& area);
		
		/**
		 * Add a single location and pull its info from the map.
		 */
		void add_tile(const gamemap& map, const gamemap::location& loc);
		
		/**
		 * Add many locations and pull their info from the map.
		 */
		void add_tiles(const gamemap& map, const std::set<gamemap::location>& loc);
		
		/**
		 * Get the tile_info vector.
		 */
		const std::vector<tile_info>& get_items() const { return items_; }
		
		/**
		 * Get the area covered by this map fragment.
		 */
		std::set<gamemap::location> get_area() const;
		
		/**
		 * Get the area covered by this map fragment, shifted by an offset.
		 */
		std::set<gamemap::location> get_offset_area(const gamemap::location& offset) const;
		
		/**
		 * Paste the map fragment into the map, treating loc as the (0,0) point (offset).
		 */
		void paste_into(gamemap& map, const gamemap::location& loc) const;
		
		/**
		 * Shift all tiles in the map fragment by the specified offset.
		 */
		void shift(const gamemap::location& offset);
		
		/**
		 * Get the center of the map fragment, bounds-wise.
		 */
		gamemap::location center_of_bounds() const;

		/**
		 * Get the center of the map fragment, mass-wise.
		 */
		gamemap::location center_of_mass() const;
		
		/**
		 * Shift the map fragment so it is roughly centered around the (0,0) point, bounds-wise.
		 */
		void center_by_bounds();
		
		/**
		 * Shift the map fragment so it is roughly centered around the (0,0) point, mass-wise.
		 */
		void center_by_mass();
		
		/**
		 * @return true if the map_fragment is empty
		 */
		bool empty() const;
		
		/**
		 * Rotate the map fragment 60 degrees clockwise around (0,0)
		 */
		void rotate_60_cw();

		/**
		 * Rotate the map fragment 60 degrees counter-clockwise around (0,0)
		 */
		void rotate_60_ccw();
	protected:
		std::vector<tile_info> items_;
};

} //end namespace editor2

#endif
