/* $Id$ */
/*
   Copyright (C) 2008 - 2012 by Tomasz Sniatowski <kailoran@gmail.com>
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

#include "../editor_common.hpp"

#include "../../map.hpp"
#include "../../map_label.hpp"
#include "unit_map.hpp"
#include "tod_manager.hpp"
#include "gamestatus.hpp"

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
	~editor_map_load_exception() throw() {}
	std::string filename;
};

struct editor_map_save_exception : public editor_exception
{
	editor_map_save_exception(const std::string& msg)
	: editor_exception(msg)
	{
	}
	~editor_map_save_exception() throw() {}
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

	/** Adds a new side to the map */
	void new_side() {
    	teams_.push_back(team());
    }

	/** Get the team from the current map context object */
	std::vector<team>& get_teams() {
		return teams_;
	}

	/** Get the unit map from the current map context object */
	unit_map& get_units() {
		return units_;
	}

	const unit_map& get_units() const {
		return units_;
	}

	tod_manager& get_time_manager() {
		return tod_manager_;
	}

	game_state& get_game_state() {
		return state_;
	}


	/**
	 * Empty map constructor
	 */
	explicit editor_map(const config& terrain_cfg, const display& disp);

	/**
	 * Create an editor map from a map data string
	 */
	editor_map(const config& terrain_cfg, const config& level, const display& disp);

	editor_map(const config& terrain_cfg, const std::string& data, const display& disp);

	/**
	 * Wrapper around editor_map(cfg, data) that catches possible exceptions
	 * and wraps them in a editor_map_load_exception
	 */
	static editor_map from_string(const config& terrain_cfg, const std::string& data, const display& disp);

	/**
	 * Create an editor map with the given dimensions and filler terrain
	 */
	editor_map(const config& terrain_cfg, size_t width, size_t height, t_translation::t_terrain filler, const display& disp);

	/**
	 * Create an editor_map by upgrading an existing gamemap. The map data is
	 * copied. Marked "explicit" to avoid potentially harmful automatic conversions.
	 */
	explicit editor_map(const gamemap& map, const display& disp);

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
	 * @return the map labels of the map
	 */
	map_labels& get_map_labels() { return labels_; };


	const map_labels& get_map_labels() const { return labels_; };

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
		t_translation::t_terrain filler = t_translation::NONE_TERRAIN);

	/**
	 * A sort-of diff operation returning a mask that, when applied to the current editor_map,
	 * will transform it into the target map.
	 */
	editor_map mask_to(const editor_map& target) const;

	/**
	 * A precondition to several map operations
	 * @return true if this map has the same dimensions as the other map
	 */
	bool same_size_as(const gamemap& other) const;

	void write(config&) const;

protected:
	t_translation::t_list clone_column(int x, t_translation::t_terrain filler);

	//helper functions for resizing
	void expand_right(int count, t_translation::t_terrain filler);
	void expand_left(int count, t_translation::t_terrain filler);
	void expand_top(int count, t_translation::t_terrain filler);
	void expand_bottom(int count, t_translation::t_terrain filler);
	void shrink_right(int count);
	void shrink_left(int count);
	void shrink_top(int count);
	void shrink_bottom(int count);

	/**
	 * The selected hexes
	 */
	std::set<map_location> selection_;

private:

	/**
	 * The labels of this map.
	 */
	map_labels labels_;

	/**
	 * TODO
	 */
	unit_map units_;

	std::vector<team> teams_;

	tod_manager tod_manager_;

	game_state state_;

};


} //end namespace editor

#endif
