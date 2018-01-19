/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace actions {


/// Class to store the actions that a player can undo and redo.
class undo_list {

	typedef boost::ptr_vector<undo_action_base> action_list;
	typedef boost::ptr_vector<config> redos_list;

public:
	undo_list(const undo_list&) = delete;
	undo_list& operator=(const undo_list&) = delete;

	explicit undo_list(const config & cfg);
	~undo_list();

	/// Creates an undo_action based on a config.
	/// Throws bad_lexical_cast or config::error if it cannot parse the config properly.
	static undo_action_base * create_action(const config & cfg);

	// Functions related to managing the undo stack:

	/// Adds an auto-shroud toggle to the undo stack.
	void add_auto_shroud(bool turned_on);
	/// Adds an auto-shroud toggle to the undo stack.
	void add_dummy();
	/// Adds a dismissal to the undo stack.
	void add_dismissal(const unit_const_ptr u);
	/// Adds a move to the undo stack.
	void add_move(const unit_const_ptr u,
	              const std::vector<map_location>::const_iterator & begin,
	              const std::vector<map_location>::const_iterator & end,
	              int start_moves, int timebonus=0, int village_owner=-1,
	              const map_location::DIRECTION dir=map_location::NDIRECTIONS);
	/// Adds a recall to the undo stack.
	void add_recall(const unit_const_ptr u, const map_location& loc,
	                const map_location& from, int orig_village_owner, bool time_bonus);
	/// Adds a recruit to the undo stack.
	void add_recruit(const unit_const_ptr u, const map_location& loc,
	                 const map_location& from, int orig_village_owner, bool time_bonus);
	/// Adds a shroud update to the undo stack.
	void add_update_shroud();
private:
public:
	/// Clears the stack of undoable (and redoable) actions.
	void clear();
	/// Updates fog/shroud based on the undo stack, then updates stack as needed.
	void commit_vision();
	/// Performs some initializations and error checks when starting a new
	/// side-turn.
	void new_side_turn(int side);
	/// Returns true if the player has performed any actions this turn.
	bool player_acted() const { return committed_actions_ || !undos_.empty(); }
	/// Read the undo_list from the provided config.
	void read(const config & cfg);
	/// Write the undo_list into the provided config.
	void write(config & cfg) const;

	// Functions related to using the undo stack:

	/// True if there are actions that can be undone.
	bool can_undo() const  { return !undos_.empty(); }
	/// True if there are actions that can be redone.
	bool can_redo() const  { return !redos_.empty(); }
	/// Undoes the top action on the undo stack.
	void undo();
	/// Redoes the top action on the redo stack.
	void redo();

private: // functions
	/// Adds an action to the undo stack.
	void add(undo_action_base * action)
	{ undos_.push_back(action);  redos_.clear(); }
	/// Applies the pending fog/shroud changes from the undo stack.
	bool apply_shroud_changes() const;

private: // data
	action_list undos_;
	redos_list redos_;

	/// Tracks the current side.
	int side_;
	/// Tracks if actions have been cleared from the stack since the turn began.
	bool committed_actions_;
};


}//namespace actions
