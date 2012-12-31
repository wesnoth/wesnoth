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

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>


namespace actions {


/// Class to store the actions that a player can undo and redo.
class undo_list {
	/// Records information to be able to undo an action.
	struct undo_action {
		enum ACTION_TYPE { NONE, MOVE, RECRUIT, RECALL, DISMISS, AUTO_SHROUD,
		                   UPDATE_SHROUD };
		static ACTION_TYPE parse_type(const std::string & str);

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
				affected_unit(boost::make_shared<unit>(u)),
				countdown_time_bonus(timebonus),
				starting_dir(dir == map_location::NDIRECTIONS ? u.facing() : dir),
				active()
			{
			}

		/// Constructor for recruit and recall actions.
		/// These types of actions are guaranteed to have a non-empty route.
		undo_action(const unit& u, const map_location& loc, const map_location& from,
			        const ACTION_TYPE action_type) :
				route(1, loc),
				starting_moves(),
				original_village_owner(),
				recall_from(from),
				type(action_type),
				affected_unit(boost::make_shared<unit>(u)),
				countdown_time_bonus(1),
				starting_dir(u.facing()),
				active()
			{}

		// Constructor for dismissals.
		explicit undo_action(const unit& u) :
				route(),
				starting_moves(),
				original_village_owner(),
				recall_from(),
				type(DISMISS),
				affected_unit(boost::make_shared<unit>(u)),
				countdown_time_bonus(),
				starting_dir(map_location::NDIRECTIONS),
				active()
			{}

		// Constructor for shroud actions.
		explicit undo_action(const ACTION_TYPE action_type, bool turned_on=true) :
				route(),
				starting_moves(),
				original_village_owner(),
				recall_from(),
				type(action_type),
				affected_unit(),
				countdown_time_bonus(),
				starting_dir(map_location::NDIRECTIONS),
				active(turned_on)
			{}

		/// Constructor from a config.
		undo_action(const config & cfg, const std::string & tag);

		/// For converting to a config.
		void write(config & cfg) const;

		// Shortcuts for identifying the type of action:
		bool is_dismiss() const { return type == DISMISS; }
		bool is_recall()  const { return type == RECALL; }
		bool is_recruit() const { return type == RECRUIT; }
		bool is_move()    const { return type == MOVE; }
		bool is_auto_shroud()   const { return type == AUTO_SHROUD; }
		bool is_update_shroud() const { return type == UPDATE_SHROUD; }
		bool valid()      const { return type != NONE; }

		/// This identifies which types of actions must (and always will) have
		/// a unit.
		bool needs_unit() const { return type == DISMISS  ||  type == RECALL  ||
		                                 type == RECRUIT  ||  type == MOVE; }


		// Data:
		/// The hexes occupied by affected_unit during this action.
		std::vector<map_location> route;
		int starting_moves;
		int original_village_owner;
		map_location recall_from;
		ACTION_TYPE type;
		// Use a shared (not scoped) pointer because this will get copied.
		boost::shared_ptr<unit> affected_unit;
		int countdown_time_bonus;
		map_location::DIRECTION starting_dir;
		bool active;
	};
	typedef std::vector<undo_action> action_list;

public:
	/// The config may be invalid.
	explicit undo_list(const config & cfg) :
		undos_(), redos_(), side_(1), committed_actions_(false)
	{ if ( cfg ) read(cfg); }
	~undo_list() {}

	// Functions related to managing the undo stack:

	/// Adds an auto-shroud toggle to the undo stack.
	void add_auto_shroud(bool turned_on);
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
private:
	/// Adds a shroud update to the undo stack.
	void add_update_shroud();
public:
	/// Clears the stack of undoable (and redoable) actions.
	void clear();
	/// Updates fog/shroud based on the undo stack, then updates stack as needed.
	void commit_vision(bool is_replay=false);
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


}//namespace actions

#endif
