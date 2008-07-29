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

#include "editor_common.hpp"

#include "../map.hpp"

#include <deque>

namespace editor2 {

struct editor_map_operation_exception : public editor_exception
{
};

/**
 * This class adds extra editor-specific functionality to a normal gamemap.
 */	
class editor_map : public gamemap 
{
public:

	editor_map(const config& terrain_cfg, const std::string& data);
	editor_map(const config& terrain_cfg, size_t width, size_t height, t_translation::t_terrain filler);
	
	~editor_map();
	
	
	/**
	 * Get a contigious set of tiles having the same terrain as the starting location.
	 * Useful for flood fill or magic wand selection
	 * @return a contigious set of locations that will always contain at least the starting element
	 */
	std::set<gamemap::location> get_contigious_terrain_tiles(const gamemap::location& start) const;
	
	void clear_starting_position_labels(display& disp) const;
	
	void set_starting_position_labels(display& disp) const;
	
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
	
	const std::string& get_filename() const { return filename_; }
	
	void set_filename(const std::string& fn) { filename_ = fn; }
	
	bool save();
	
	/**
	 * Performs an action (thus modyfying the map). An appropriate undo action is added to
	 * the undo stack. The redo stack is cleared.
	 */
	void perform_action(const editor_action& action);
	
	void perform_partial_action(const editor_action& action);
	
	/** @return whether the map was modified since the last save */
	bool modified() const;

	/** @return true when undo can be performed, false otherwise */
	bool can_undo() const;
	
	editor_action* last_undo_action();

	/** @return true when redo can be performed, false otherwise */
	bool can_redo() const;

	/** Un-does an action, and puts it in the redo stack for a possible redo */
	void undo();

	/** Re-does a previousle undid action, and puts it back in the undo stack. */
	void redo();
	
	/** 
	 * Resize the map. If the filler is NONE, the border terrain will be copied
	 * when expanding, otherwise the fill er terrain will be inserted there
	 */
	void resize(int width, int height, int x_offset, int y_offset,
		t_translation::t_terrain filler = t_translation::NONE_TERRAIN);

	void flip_x();
	void flip_y();

protected:
	void set_starting_position(int pos, const location& loc);
	void swap_starting_position(int x1, int y1, int x2, int y2);
	t_translation::t_list clone_column(int x, t_translation::t_terrain filler);
	void expand_right(int count, t_translation::t_terrain filler);
	void expand_left(int count, t_translation::t_terrain filler);
	void expand_top(int count, t_translation::t_terrain filler);
	void expand_bottom(int count, t_translation::t_terrain filler);
	void shrink_right(int count);
	void shrink_left(int count);
	void shrink_top(int count);
	void shrink_bottom(int count);

	/**
	 * Container type used to store actions in the undo and redo stacks
	 */
	typedef std::deque<editor_action*> action_stack;

	/**
	 * Checks if an action stack reached its capacity and removes the front element if so.
	 */
	void trim_stack(action_stack& stack);

	/**
	 * Clears an action stack and deletes all its contents. Helper function used when the undo
	 * or redo stack needs to be cleared
	 */
	void clear_stack(action_stack& stack);

	/**
	 * Perform an action at the back of one stack, and then move it to the back of the other stack.
	 * This is the implementation of both undo and redo which only differ in the direction.
	 */
	void perform_action_between_stacks(action_stack& from, action_stack& to);

	/**
	 * The selected hexes
	 */
	std::set<gamemap::location> selection_;
	
	/**
	 * The actual filename of this map. An empty string indicates a new map.
	 */
	std::string filename_;
	
	/**
	 * The undo stack. A double-ended queues due to the need to add items to one end,
	 * and remove from both when performing the undo or when trimming the size. This container owns
	 * all contents, i.e. no action in the stack shall be deleted, and unless otherwise noted the contents 
	 * could be deleted at an time during normal operation of the stack. To work on an action, either
	 * remove it from the container or make a copy. Actions are inserted at the back of the container
	 * and disappear from the front when the capacity is exceeded.
	 * @todo Use boost's pointer-owning container?
	 */
	action_stack undo_stack_;
	
	/**
	 * The redo stack. @see undo_stack_
	 */
	action_stack redo_stack_;
	
	/**
	 * Action stack (i.e. undo and redo) maximum size
	 */
	static const int max_action_stack_size_;
	
	/**
	 * Number of actions performed since the map was saved. Zero means the map was not modified.
	 */
	int actions_since_save_;	
};


} //end namespace editor2

#endif

