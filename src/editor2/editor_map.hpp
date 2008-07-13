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

#ifndef EDITOR2_EDITOR_MAP_HPP_INCLUDED
#define EDITOR2_EDITOR_MAP_HPP_INCLUDED

#include "../map.hpp"

namespace editor2 {

/**
 * This class adds extra editor-specific functionality to a normal gamemap.
 */
	
class editor_map : public gamemap 
{
public:
	editor_map(const config& terrain_cfg, const std::string& data);
	static editor_map new_map(const config& terrain_cfg, size_t width, size_t height, t_translation::t_terrain filler);
	
	/**
	 * Get a contigious set of tiles having the same terrain as the starting location.
	 * Useful for flood fill or magic wand selection
	 * @return a contigious set of locations that will always contain at least the starting element
	 */
	std::set<gamemap::location> get_contigious_terrain_tiles(const gamemap::location& start) const;
	
	/**
	 * @return true when the location is part of the selection, false otherwise
	 */
	bool in_selection(const gamemap::location& loc) const;
	
	/**
	 * Add a location to the selection. The location should be valid (i.e. on the map)
	 * @return true if the selected hexes set was modified
	 */
	bool add_to_selection(const gamemap::location& loc);
	
	/**
	 * Remove a location to the selection. The location does not actually have to be selected
	 * @return true if the selected hexes set was modified
	 */
	bool remove_from_selection(const gamemap::location& loc);
	
	/**
	 * Return the selection set.
	 */
	const std::set<gamemap::location> selection() const { return selection_; }
	
	/**
	 * Clear the selection
	 */
	void clear_selection();
	
	/**
	 * Invert the selection, i.e. select all the map hexes that were not selected.
	 */
	void invert_selection();
	
	/**
	 * Select all map hexes
	 */
	void select_all();
	
protected:
	/**
	 * The selected hexes
	 */
	std::set<gamemap::location> selection_;
};


} //end namespace editor2

#endif

