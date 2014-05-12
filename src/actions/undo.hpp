/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "vision.hpp"
#include "../map_location.hpp"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

class unit;


namespace actions {


/// Class to store the actions that a player can undo and redo.
class undo_list : boost::noncopyable {
	/// Records information to be able to undo an action.
	/// Each type of action gets its own derived type.
	struct undo_action : boost::noncopyable {
		/// Constructor for move actions.
		undo_action(const unit& u,
			        const std::vector<map_location>::const_iterator & begin,
			        const std::vector<map_location>::const_iterator & end) :
				route(begin, end),
				view_info(new clearer_info(u))
			{
			}
		/// Constructor for recruit and recall actions.
		/// These types of actions are guaranteed to have a non-empty route.
		undo_action(const unit& u, const map_location& loc) :
				route(1, loc),
				view_info(new clearer_info(u))
			{}
		/// Constructor from a config storing the view info.
		/// Does not set @a route.
		explicit undo_action(const config & cfg) :
				route(),
				view_info(new clearer_info(cfg))
			{}
		/// Constructor from a config storing the view info and a location.
		/// Guarantees a non-empty route.
		explicit undo_action(const config & cfg, const map_location & loc) :
				route(1, loc),
				view_info(new clearer_info(cfg))
			{}
		/// Default constructor.
		/// This is the only way to get NULL view_info.
		undo_action() :
				route(),
				view_info(NULL)
			{}
		// Virtual destructor to support derived classes.
		virtual ~undo_action();


		/// Creates an undo_action based on a config.
		static undo_action * create(const config & cfg, const std::string & tag);
		/// Writes this into the provided config.
		virtual void write(config & cfg) const = 0;

		/// Undoes this action.
		/// @return true on success; false on an error.
		virtual bool undo(int side, undo_list & undos) = 0;
		/// Redoes this action.
		/// @return true on success; false on an error.
		virtual bool redo(int side) = 0;

		config& get_replay_data() { return replay_data; }

		// Data:
		/// the replay data to do this action, this is only !empty() when this action is on the redo stack
		/// we need this because we dont recalculate the redos like they would be in real game, 
		/// but even undoable comands can have "dependent" (= user_input) commands, which we save here.
		config replay_data;

		/// The hexes occupied by the affected unit during this action.
		std::vector<map_location> route;
		/// A record of the affected unit's ability to see.
		/// For derived classes that use this, it must be never NULL.
		clearer_info * const view_info;
		// This pointer is the reason for deriving from noncopyable (an
		// alternative would be to implement deep copies, but we have no
		// need for copying, so noncopyable is simpler).
	};
	// The structs derived from undo_action.
	struct dismiss_action;
	struct move_action;
	struct recall_action;
	struct recruit_action;
	struct auto_shroud_action;
	struct update_shroud_action;
	// The update_shroud_action needs to be able to call add_update_shroud().
	friend struct update_shroud_action;

	typedef boost::ptr_vector<undo_action> action_list;

public:
	explicit undo_list(const config & cfg);
	~undo_list();

	// Functions related to managing the undo stack:

	/// Adds an auto-shroud toggle to the undo stack.
	void add_auto_shroud(bool turned_on);
	/// Adds a dismissal to the undo stack.
	void add_dismissal(const unit & u);
	/// Adds a move to the undo stack.
	void add_move(const unit& u,
	              const std::vector<map_location>::const_iterator & begin,
	              const std::vector<map_location>::const_iterator & end,
	              int start_moves, int timebonus=0, int village_owner=-1,
	              const map_location::DIRECTION dir=map_location::NDIRECTIONS);
	/// Adds a recall to the undo stack.
	void add_recall(const unit& u, const map_location& loc,
	                const map_location& from);
	/// Adds a recruit to the undo stack.
	void add_recruit(const unit& u, const map_location& loc,
	                 const map_location& from);
private:
	/// Adds a shroud update to the undo stack.
	void add_update_shroud();
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
	void add(undo_action * action)
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


}//namespace actions

#endif
