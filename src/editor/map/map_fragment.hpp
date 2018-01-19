/*
   Copyright (C) 2008 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "editor/map/editor_map.hpp"

namespace editor {

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
	tile_info(const gamemap& map, const map_location& offset)
	: offset(offset), terrain(map.get_terrain(offset))
	{
	}

	map_location offset;
	t_translation::terrain_code terrain;
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
		map_fragment(const gamemap& map, const std::set<map_location>& area);

		/**
		 * Add a single location and pull its info from the map.
		 */
		void add_tile(const gamemap& map, const map_location& loc);

		/**
		 * Add many locations and pull their info from the map.
		 */
		void add_tiles(const gamemap& map, const std::set<map_location>& loc);

		/**
		 * Get the tile_info vector.
		 */
		const std::vector<tile_info>& get_items() const { return items_; }

		/**
		 * Get the area covered by this map fragment.
		 */
		std::set<map_location> get_area() const;

		/**
		 * Get the area covered by this map fragment, shifted by an offset.
		 */
		std::set<map_location> get_offset_area(const map_location& offset) const;

		/**
		 * Paste the map fragment into the map, treating loc as the (0,0) point (offset).
		 */
		void paste_into(gamemap& map, const map_location& loc) const;

		/**
		 * Shift all tiles in the map fragment by the specified offset.
		 */
		void shift(const map_location& offset);

		/**
		 * Get the center of the map fragment, mass-wise.
		 */
		map_location center_of_mass() const;

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

		/**
		 * Flip the map fragment horizontally
		 */
		void flip_horizontal();

		/**
		 * Flip the map fragment vertically
		 */
		void flip_vertical();

		/**
		 * Debug dump to a string
		 */
		std::string dump() const;

	protected:
		/**
		 * The data of this map_fragment
		 */
		std::vector<tile_info> items_;
		std::set<map_location> area_;
};

} //end namespace editor
