/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
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

#ifndef ACTIONS_UNDO_H_INCLUDED
#define ACTIONS_UNDO_H_INCLUDED

#include "../map_location.hpp"
#include "../unit.hpp"

#include <vector>


/// Class to store the actions that a player can undo and redo.
class undo_list {
	/// Records information to be able to undo an action.
	struct undo_action {
		enum ACTION_TYPE { MOVE, RECRUIT, RECALL, DISMISS };

		/// Constructor for move actions.
		undo_action(const unit& u,
			        const std::vector<map_location>::const_iterator & begin,
			        const std::vector<map_location>::const_iterator & end,
			        int sm, int timebonus=0, int orig=-1,
			        const map_location::DIRECTION dir=map_location::NDIRECTIONS) :
				route(begin, end),
				starting_moves(sm),
				original_village_owner(orig),
				recall_from(),
				type(MOVE),
				affected_unit(u),
				countdown_time_bonus(timebonus),
				starting_dir(dir == map_location::NDIRECTIONS ? u.facing() : dir)
			{
			}

		/// Constructor for recruit and recall actions.
		undo_action(const unit& u, const map_location& loc, const map_location& from,
			        const ACTION_TYPE action_type) :
				route(1, loc),
				starting_moves(),
				original_village_owner(),
				recall_from(from),
				type(action_type),
				affected_unit(u),
				countdown_time_bonus(1),
				starting_dir(u.facing())
			{}

		// Constructor for dismissals.
		explicit undo_action(const unit& u) :
				route(),
				starting_moves(),
				original_village_owner(),
				recall_from(),
				type(DISMISS),
				affected_unit(u),
				countdown_time_bonus(),
				starting_dir(map_location::NDIRECTIONS)
			{}

		/// The hexes occupied by affected_unit during this action.
		std::vector<map_location> route;
		int starting_moves;
		int original_village_owner;
		map_location recall_from;
		ACTION_TYPE type;
		unit affected_unit;
		int countdown_time_bonus;
		map_location::DIRECTION starting_dir;

		bool is_dismiss() const { return type == DISMISS; }
		bool is_recall() const { return type == RECALL; }
		bool is_recruit() const { return type == RECRUIT; }
	};
	typedef std::vector<undo_action> action_list;

public:
	undo_list() :
		undos_(), redos_(), side_(1), committed_actions_(false)
	{}
	~undo_list() {}

	// Functions related to managing the undo stack:

	/// Adds a dismissal to the undo stack.
	void add_dissmissal(const unit & u)
	{ add(undo_action(u)); }
	/// Adds a move to the undo stack.
	void add_move(const unit& u,
	              const std::vector<map_location>::const_iterator & begin,
	              const std::vector<map_location>::const_iterator & end,
	              int start_moves, int timebonus=0, int village_owner=-1,
	              const map_location::DIRECTION dir=map_location::NDIRECTIONS)
	{ add(undo_action(u, begin, end, start_moves, timebonus, village_owner, dir)); }
	/// Adds a recall to the undo stack.
	void add_recall(const unit& u, const map_location& loc,
	                const map_location& from)
	{ add(undo_action(u, loc, from, undo_action::RECALL)); }
	/// Adds a recruit to the undo stack.
	void add_recruit(const unit& u, const map_location& loc,
	                 const map_location& from)
	{ add(undo_action(u, loc, from, undo_action::RECRUIT)); }
	/// Clears the stack of undoable (and redoable) actions.
	void clear();
	/// Updates fog/shroud based on the undo stack, then updates stack as needed.
	void commit_vision();
	/// Performs some initializations and error checks when starting a new
	/// side-turn.
	void new_side_turn(int side);
	/// Returns true if the player has performed any actions this turn.
	bool player_acted() const { return committed_actions_ || !undos_.empty(); }

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
	/// Copying the undo list is probably an error, so it is not implemented.
	undo_list(const undo_list &);
	/// Assigning the undo list is probably an error, so it is not implemented.
	undo_list & operator=(const undo_list &);

	/// Adds an action to the undo stack.
	void add(const undo_action & action)
	{ undos_.push_back(action);  redos_.clear(); }
	/// Applies the pending fog/shroud changes from the undo stack.
	size_t apply_shroud_changes() const;

private: // data
	action_list undos_;
	action_list redos_;

	/// Tracks the current side.
	int side_;
	/// Tracks if actions have been cleared from the stack since the turn began.
	bool committed_actions_;
};

#endif
