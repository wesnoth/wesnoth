/*
   Copyright (C) 2008 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_EDITOR_MAP_HPP_INCLUDED
#define EDITOR_EDITOR_MAP_HPP_INCLUDED

#include "editor/editor_common.hpp"

#include "map/map.hpp"

#include <deque>

namespace editor {

struct editor_map_operation_exception : public editor_exception
{
	editor_map_operation_exception()
	: editor_exception("Map operation error. Check debug log for details.")
	{
	}
};

struct editor_map_integrity_error : public editor_exception
{
	editor_map_integrity_error()
	: editor_exception("Map integrity error. Check debug log for details.")
	{
	}
};

struct editor_map_load_exception : public editor_exception
{
	editor_map_load_exception(const std::string& fn, const std::string& msg)
	: editor_exception(msg), filename(fn)
	{
	}
	~editor_map_load_exception() NOEXCEPT {}
	std::string filename;
};

struct editor_map_save_exception : public editor_exception
{
	editor_map_save_exception(const std::string& msg)
	: editor_exception(msg)
	{
	}
	~editor_map_save_exception() NOEXCEPT {}
};


/**
 * Exception wrapping utility
 */
editor_map_load_exception wrap_exc(const char* type, const std::string& e_msg, const std::string& filename);

/**
 * This class adds extra editor-specific functionality to a normal gamemap.
 */
class editor_map : public gamemap
{
public:

	/**
	 * Empty map constructor
	 */
	explicit editor_map(const config& terrain_cfg);

	/**
	 * Create an editor map from a map data string
	 */
	editor_map(const config& terrain_cfg, const std::string& data);

	/**
	 * Wrapper around editor_map(cfg, data) that catches possible exceptions
	 * and wraps them in a editor_map_load_exception
	 */
	static editor_map from_string(const config& terrain_cfg, const std::string& data);

	/**
	 * Create an editor map with the given dimensions and filler terrain
	 */
	editor_map(const config& terrain_cfg, size_t width, size_t height, const t_translation::terrain_code & filler);

	/**
	 * Create an editor_map by upgrading an existing gamemap. The map data is
	 * copied. Marked "explicit" to avoid potentially harmful automatic conversions.
	 */
	explicit editor_map(const gamemap& map);

	/**
	 * editor_map destructor
	 */
	~editor_map();

	/**
	 * Debugging aid. Check if the widths and heights correspond to the actual map data sizes.
	 */
	void sanity_check();

	/**
	 * Get a contiguous set of tiles having the same terrain as the starting location.
	 * Useful for flood fill or magic wand selection
	 * @return a contiguous set of locations that will always contain at least the starting element
	 */
	std::set<map_location> get_contiguous_terrain_tiles(const map_location& start) const;

	/**
	 * Set labels for staring positions in the given display object.
	 * @return the locations where the labels were added
	 */
	std::set<map_location> set_starting_position_labels(display& disp);

	/**
	 * @return true when the location is part of the selection, false otherwise
	 */
	bool in_selection(const map_location& loc) const;

	/**
	 * Add a location to the selection. The location should be valid (i.e. on the map)
	 * @return true if the selected hexes set was modified
	 */
	bool add_to_selection(const map_location& loc);

	/**
	 * Remove a location to the selection. The location does not actually have to be selected
	 * @return true if the selected hexes set was modified
	 */
	bool remove_from_selection(const map_location& loc);

	/**
	 * Select the given area.
	 * @param area to select.
	 */
	bool set_selection(const std::set<map_location>& area);

	/**
	 * Return the selection set.
	 */
	const std::set<map_location>& selection() const { return selection_; }

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

	/**
	 * @return true if the entire map is selected, false otherwise
	 */
	bool everything_selected() const;

	/**
	 * Resize the map. If the filler is NONE, the border terrain will be copied
	 * when expanding, otherwise the filler terrain will be inserted there
	 */
	void resize(int width, int height, int x_offset, int y_offset,
		const t_translation::terrain_code & filler = t_translation::NONE_TERRAIN);

	/**
	 * A sort-of diff operation returning a mask that, when applied to the current editor_map,
	 * will transform it into the target map.
	 */
	gamemap mask_to(const gamemap& target) const;

	/**
	 * A precondition to several map operations
	 * @return true if this map has the same dimensions as the other map
	 */
	bool same_size_as(const gamemap& other) const;

protected:
	//helper functions for resizing
	void expand_right(int count, const t_translation::terrain_code & filler);
	void expand_left(int count, const t_translation::terrain_code & filler);
	void expand_top(int count, const t_translation::terrain_code & filler);
	void expand_bottom(int count, const t_translation::terrain_code & filler);
	void shrink_right(int count);
	void shrink_left(int count);
	void shrink_top(int count);
	void shrink_bottom(int count);

	/**
	 * The selected hexes
	 */
	std::set<map_location> selection_;
};


} //end namespace editor

#endif
