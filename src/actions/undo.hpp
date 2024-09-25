/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 * Various functions that implement the undoing (and redoing) of in-game commands.
 */

#pragma once

#include "vision.hpp"
#include "map/location.hpp"
#include "units/ptr.hpp"
#include "undo_action.hpp"

#include <vector>

namespace actions {


/** Class to store the actions that a player can undo and redo. */
class undo_list {

	typedef std::unique_ptr<undo_action_container> action_ptr_t;
	typedef std::vector<action_ptr_t> action_list;
	typedef std::vector<std::unique_ptr<config>> redos_list;

public:
	undo_list(const undo_list&) = delete;
	undo_list& operator=(const undo_list&) = delete;

	undo_list();
	~undo_list();

	// Functions related to managing the undo stack:

	/** Adds an auto-shroud toggle to the undo stack. */
	void add_auto_shroud(bool turned_on);
	/** Adds a dismissal to the undo stack. */
	void add_dismissal(const unit_const_ptr& u);
	/** Adds a move to the undo stack. */
	void add_move(const unit_const_ptr& u,
	              const std::vector<map_location>::const_iterator & begin,
	              const std::vector<map_location>::const_iterator & end,
	              int start_moves,
	              const map_location::direction dir=map_location::direction::indeterminate);
	/** Adds a recall to the undo stack. */
	void add_recall(const unit_const_ptr& u, const map_location& loc,
	                const map_location& from);
	/** Adds a recruit to the undo stack. */
	void add_recruit(const unit_const_ptr& u, const map_location& loc,
	                 const map_location& from);

	template<class T, class... Args>
	void add_custom(Args&&... args)
	{
		add(std::make_unique<T>(std::forward<Args>(args)...));
	}
private:
public:
	/** Clears the stack of undoable (and redoable) actions. */
	void clear();
	/**
	 * Updates fog/shroud based on the undo stack, then updates stack as needed.
	 * Returns true if somethign was cleared.
	 */
	bool commit_vision();
	/**
	 * Performs some initializations and error checks when starting a new
	 * side-turn.
	 */
	void new_side_turn(int side);
	/** Returns true if the player has performed any actions this turn. */
	bool player_acted() const { return committed_actions_ || !undos_.empty(); }
	/** Read the undo_list from the provided config. */
	void read(const config & cfg, int current_side);
	/** Write the undo_list into the provided config. */
	void write(config & cfg) const;

	// Functions related to using the undo stack:

	/** True if there are actions that can be undone. */
	bool can_undo() const  { return !undos_.empty(); }
	/** True if there are actions that can be redone. */
	bool can_redo() const  { return !redos_.empty(); }

	/** called before a user action, starts collecting undo steps for the new action. */
	void init_action();
	/** called after a user action, pushes the collected undo steps on the undo stack. */
	void finish_action(bool can_undo);
	/** called after a user action, removes empty actions */
	void cleanup_action();
	/** Undoes the top action on the undo stack. */
	void undo();
	/** Redoes the top action on the redo stack. */
	void redo();

	undo_action_container* get_current()
	{
		return current_.get();
	}

private:
	/** Adds an undo step to the current action. */
	void add(std::unique_ptr<undo_action>&& action)
	{
		if(current_) {
			current_->add(std::move(action));
		}
	}
	/** Applies the pending fog/shroud changes from the undo stack. */
	bool apply_shroud_changes() const;

private: // data
	action_ptr_t current_;
	action_list undos_;
	redos_list redos_;

	/** Tracks the current side. */
	int side_;
	/** Tracks if actions have been cleared from the stack since the turn began. */
	bool committed_actions_;
};


}//namespace actions
