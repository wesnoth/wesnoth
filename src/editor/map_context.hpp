/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_MAP_CONTEXT_HPP_INCLUDED
#define EDITOR_MAP_CONTEXT_HPP_INCLUDED

#include "editor_common.hpp"
#include "editor_map.hpp"

#include <boost/utility.hpp>

#include <deque>

namespace editor {

/**
 * This class wraps around a map to provide a conscise interface for the editor to work with.
 * The actual map object can change rapidly (be assigned to), the map context persists
 * data (like the undo stacks) in this case. The functionality is here, not in editor_controller
 * as e.g. the undo stack is part of the map, not the editor as a whole. This might allow many
 * maps to be open at the same time.
 */
class map_context : private boost::noncopyable
{
public:
	/**
	 * Create a map context from an existing map. The filename is set to be
	 * empty, indicating a new map.
	 * Marked "explicit" to avoid automatic conversions.
	 */
	explicit map_context(const editor_map& map);

	/**
	 * Create map_context from a map file. If the map cannot be loaded, an
	 * exception will be thrown and the object will not be constructed. If the
	 * map file is a scenario, the map specified in its map_data key will be
	 * loaded, and the stored filename updated accordingly. Maps embedded
	 * inside scenarios do not change the filename, but set the "embedded" flag
	 * instead.
	 */
	map_context(const config& game_config, const std::string& filename);

	/**
	 * Map context destructor
	 */
	~map_context();

	/**
	 * Map accesor
	 */
	editor_map& get_map() { return map_; };

	/**
	 * Map accesor - const version
	 */
	const editor_map& get_map() const { return map_; }

	/**
	 * Draw a terrain on a single location on the map.
	 * Sets the refresh flags accordingly.
	 */
	void draw_terrain(t_translation::t_terrain terrain, const map_location& loc,
		bool one_layer_only = false);

	/**
	 * Actual drawing function used by both overloaded variants of draw_terrain.
	 */
	void draw_terrain_actual(t_translation::t_terrain terrain, const map_location& loc,
		bool one_layer_only = false);

	/**
	 * Draw a terrain on a set of locations on the map.
	 * Sets the refresh flags accordingly.
	 */
	void draw_terrain(t_translation::t_terrain terrain, const std::set<map_location>& locs,
		bool one_layer_only = false);

	/**
	 * Getter for the reload flag. Reload is the highest level of required refreshing,
	 * set when the map size has changed or the map was reassigned.
	 */
	bool needs_reload() const { return needs_reload_; }

	/**
	 * Setter for the reload flag
	 */
	void set_needs_reload(bool value=true) { needs_reload_ = value; }

	/**
	 * Getter for the terrain rebuild flag. Set whenever any terrain has changed.
	 */
	bool needs_terrain_rebuild() const { return needs_terrain_rebuild_; }

	/**
	 * Setter for the terrain rebuild flag
	 */
	void set_needs_terrain_rebuild(bool value=true) { needs_terrain_rebuild_ = value; }

	/**
	 * Getter fo the labels reset flag. Set when the labels need to be refreshed.
	 */
	bool needs_labels_reset() const { return needs_labels_reset_; }

	/**
	 * Setter for the labels reset flag
	 */
	void set_needs_labels_reset(bool value=true) { needs_labels_reset_ = value; }

	const std::set<map_location> changed_locations() const { return changed_locations_; }
	void clear_changed_locations();
	void add_changed_location(const map_location& loc);
	void add_changed_location(const std::set<map_location>& locs);
	void set_everything_changed();
	bool everything_changed() const;

	void clear_starting_position_labels(display& disp);

	void set_starting_position_labels(display& disp);

	void reset_starting_position_labels(display& disp);

	const std::string& get_filename() const { return filename_; }

	void set_filename(const std::string& fn) { filename_ = fn; }

	const std::string& get_map_data_key() const { return map_data_key_; }

	bool is_embedded() const { return embedded_; }

	void set_embedded(bool v) { embedded_ = v; }

	/**
	 * Saves the map under the current filename. Filename must be valid.
	 * May throw an exception on failure.
	 */
	bool save();

	void set_map(const editor_map& map);

	/**
	 * Performs an action (thus modyfying the map). An appropriate undo action is added to
	 * the undo stack. The redo stack is cleared. Note that this may throw, use caution
	 * when calling this with a dereferennced pointer that you own (i.e. use a smart pointer).
	 */
	void perform_action(const editor_action& action);

	/**
	 * Performs a partial action, assumes that the top undo action has been modified to
	 * maintain coherent state of the undo stacks, and so a new undo action is not
	 * created.
	 */
	void perform_partial_action(const editor_action& action);

	/** @return whether the map was modified since the last save */
	bool modified() const;

	/** Clear the modified state */
	void clear_modified();

	/** @return true when undo can be performed, false otherwise */
	bool can_undo() const;

	/** @return true when redo can be performed, false otherwise */
	bool can_redo() const;

	/** @return a pointer to the last undo action or NULL if the undo stack is empty */
	editor_action* last_undo_action();

	/** @return a pointer to the last redo action or NULL if the undo stack is empty */
	editor_action* last_redo_action();

	/** const version of last_undo_action */
	const editor_action* last_undo_action() const;

	/** const version of last_redo_action */
	const editor_action* last_redo_action() const;

	/** Un-does the last action, and puts it in the redo stack for a possible redo */
	void undo();

	/** Re-does a previousle undid action, and puts it back in the undo stack. */
	void redo();

	/**
	 * Un-does a single step from a undo action chain. The action is separated
	 * from the chain and it's undo (the redo) is added as a standalone action
	 * to the redo stack.
	 * Precodnition: the last undo action has to actually be an action chain.
	 */
	void partial_undo();

	/**
	 * Clear the undo and redo stacks
	 */
	void clear_undo_redo();

protected:
	/**
	 * The actual filename of this map. An empty string indicates a new map.
	 */
	std::string filename_;

	/**
	 * When a scenario file is loaded, the referenced map is loaded instead.
	 * The verbatim form of the reference is kept here.
	 */
	std::string map_data_key_;

	/**
	 * Whether the map context refers to a map embedded in a scenario file.
	 * This distinction is important in order to avoid overwriting the scenario.
	 */
	bool embedded_;

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
	static const size_t max_action_stack_size_;

	/**
	 * Number of actions performed since the map was saved. Zero means the map was not modified.
	 */
	int actions_since_save_;

	/**
	 * Cache of set starting position labels. Necessary for removing them.
	 */
	std::set<map_location> starting_position_label_locs_;

	/**
	 * Refresh flag indicating the map in this context should be completely reloaded by the display
	 */
	bool needs_reload_;

	/**
	 * Refresh flag indicating the terrain in the map has changed and requires a rebuild
	 */
	bool needs_terrain_rebuild_;

	/**
	 * Refresh flag indicating the labels in the map have changed
	 */
	bool needs_labels_reset_;

	std::set<map_location> changed_locations_;
	bool everything_changed_;
};


} //end namespace editor

#endif
