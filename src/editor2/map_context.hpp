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

#ifndef EDITOR2_MAP_CONTEXT_HPP_INCLUDED
#define EDITOR2_MAP_CONTEXT_HPP_INCLUDED

#include "editor_common.hpp"
#include "editor_map.hpp"

#include <boost/utility.hpp>

#include <deque>

namespace editor2 {

/**
 * This class wraps around a map to provide a conscise interface for the editor to work with
 */	
class map_context : private boost::noncopyable
{
public:
	map_context(const editor_map& map);
	~map_context();
	
	editor_map& get_map() { return map_; };
	const editor_map& get_map() const { return map_; }
	
	void draw_terrain(t_translation::t_terrain terrain, const gamemap::location& loc, 
		bool one_layer_only = false);
	
	void draw_terrain(t_translation::t_terrain terrain, const std::set<gamemap::location>& locs, 
		bool one_layer_only = false);

	bool needs_reload() const { return needs_reload_; }
	void set_needs_reload(bool value=true) { needs_reload_ = value; }
	
	bool needs_terrain_rebuild() const { return needs_terrain_rebuild_; }
	void set_needs_terrain_rebuild(bool value=true) { needs_terrain_rebuild_ = value; }
	
	bool needs_labels_reset() const { return needs_labels_reset_; }
	void set_needs_labels_reset(bool value=true) { needs_labels_reset_ = value; }
	
	const std::set<gamemap::location> changed_locations() const { return changed_locations_; }
	void clear_changed_locations();
	void add_changed_location(const gamemap::location& loc);
	void add_changed_location(const std::set<gamemap::location>& locs);
	void set_everything_changed();
	bool everything_changed() const;
	
	void clear_starting_position_labels(display& disp);
	
	void set_starting_position_labels(display& disp);
	
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
	
protected:
	/**
	 * The map object of this map_context.
	 */
	editor_map map_;
	
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
	
	std::set<gamemap::location> starting_position_label_locs_;
	
	bool needs_reload_;
	bool needs_terrain_rebuild_;
	bool needs_labels_reset_;
	std::set<gamemap::location> changed_locations_;
	bool everything_changed_;
};


} //end namespace editor2

#endif
