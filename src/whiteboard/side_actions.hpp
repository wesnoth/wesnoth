/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 */

#ifndef WB_SIDE_ACTIONS_HPP_
#define WB_SIDE_ACTIONS_HPP_

#include "typedefs.hpp"

#include "map_location.hpp"

#include <deque>

#include <boost/enable_shared_from_this.hpp>

namespace wb
{

class move;

/**
 * This internal whiteboard class holds the planned action queue for a team, and offers many
 * utility methods to create and manipulate them. It maintains an internal data structure
 * but mostly hides it by providing its own iterators, begin() and end() methods, etc.
 */
class side_actions: public boost::enable_shared_from_this<side_actions>
{
public:

	typedef action_queue::iterator iterator;
	typedef action_queue::const_iterator const_iterator;
	typedef action_queue::reverse_iterator reverse_iterator;
	typedef action_queue::const_reverse_iterator const_reverse_iterator;

	side_actions();
	virtual ~side_actions();

	///Must be called only once, right after the team that owns this side_actions is added to the teams vector
	void set_team_index(size_t team_index);

	///Returns the team index this action queue belongs to
	size_t team_index() { assert(team_index_defined_); return team_index_; }

	/// Get the underlying action container
	const action_queue& actions() const { return actions_; }

	/** Gets called when display is drawing a hex, to allow drawing symbols to the screen */
	void draw_hex(const map_location& hex);

	/**
	 * Executes the first action in the queue, and then deletes it.
	 * @return true - if the action was completed successfully
	 */
	bool execute_next();

	/**
	 * Execute all actions in sequence until the fist attack, or until an action fails to execute.
	 */
	void execute_all();

	/**
	 * Executes the specified action, if it exists in the queue.
	 * If the action is not finished, it's moved at the end of the queue.
	 * @return true - if the action was completed successfully
	 */
	bool execute(iterator position);

	/**
	 * Returns the iterator for the first (executed earlier) action within the actions queue.
	 */
	iterator begin() { return actions_.begin(); }
	/// reverse version of the above
	reverse_iterator rbegin() { return actions_.rbegin(); }

	/**
	 * Returns the iterator for the position *after* the last executed action within the actions queue.
	 */
	iterator end() { return actions_.end(); }
	/// reverse version of the above
	reverse_iterator rend() { return actions_.rend(); }

	/**
	 * Indicates whether the action queue is empty.
	 */
	bool empty() const { return actions_.empty(); }

	/**
	 * Returns the number of actions in the action queue.
	 */
	size_t size() const { return actions_.size(); }

	/**
	 * Empties the action queue.
	 */
	void clear() { actions_.clear(); }

	/**
	 * Queues a move to be executed last
	 * @return The queued move's position
	 */
	iterator queue_move(const pathfind::marked_route& route,
			arrow_ptr arrow, fake_unit_ptr fake_unit);

	/**
	 * Queues an attack or attack-move to be executed last
	 * @return The queued attack's position
	 */
	iterator queue_attack(const map_location& target_hex, int weapon_choice, const pathfind::marked_route& route,
			arrow_ptr arrow, fake_unit_ptr fake_unit);

	/**
	 * Queues a recruit to be executed last
	 * @return The queued recruit's position
	 */
	iterator queue_recruit(const std::string& unit_name, const map_location& recruit_hex);

	/**
	 * Queues a recall to be executed last
	 * @return The queued recall's position
	 */
	iterator queue_recall(const unit& unit, const map_location& recall_hex);

	/**
	 * Inserts an action at the specified position. The begin() and end() functions might prove useful here.
	 * @return The inserted action's position.
	 */
	iterator insert_action(iterator position, action_ptr action);

	/**
	 * Queues an action to be executed last
	 * @return The queued action's position
	 */
	iterator queue_action(action_ptr action);

	/**
	 * Moves an action earlier in the execution order (i.e. at the front of the queue),
	 * by one position.
	 * @return The action's new position.
	 */
	iterator bump_earlier(iterator position);

	/**
	 * Moves an action later in the execution order (i.e. at the back of the queue),
	 * by one position.
	 * @return The action's new position.
	 */
	iterator bump_later(iterator position);

	/**
	 * Deletes the action at the specified position.
	 * @return The position of the element after the one deleted, or end() if the queue is empty.
	 */
	iterator remove_action(iterator position, bool validate_after_delete = true);

	/**
	 * @param action The action whose position you're looking for
	 * @return The action's position within the queue, or end() if action wasn't found.
	 */
	iterator get_position_of(action_ptr action);

	/**
	 * Finds the first action that belongs to this unit, starting the search at the specified position.
	 * @return The position, or end() if not found.
	 */
	iterator find_first_action_of(unit const* unit, iterator start_position);
	///Variant of this method that always start searching at the beginning of the queue
	iterator find_first_action_of(unit const* unit);

	/**
	 * Finds the last action that belongs to this unit, starting the search backwards from the specified position.
	 * @return The position, or end() if not found.
	 */
	iterator find_last_action_of(unit const* unit, iterator start_position);
	///Variant of the previous method that always start searching at the end of the queue
	iterator find_last_action_of(unit const* unit);

	bool unit_has_actions(unit const* unit);
	size_t count_actions_of(unit const* unit);

	///Validates all planned actions in the queue
	void validate_actions();

	///Used to track gold spending by recruits/recalls when building the future unit map
	int get_gold_spent() const { return gold_spent_; }
	///Used to track gold spending by recruits/recalls when building the future unit map
	void change_gold_spent_by(int difference) { gold_spent_ += difference; assert(gold_spent_ >= 0);}

private:

	bool validate_iterator(iterator position) { return position >= begin() && position < end(); }

	action_queue actions_;
	size_t team_index_;
	bool team_index_defined_;

	/// Used to store gold "spent" in planned recruits/recalls when the future unit map is applied
	int gold_spent_;
};

/** Dumps side_actions on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, wb::side_actions const& side_actions);

}

#endif /* WB_SIDE_ACTIONS_HPP_ */
