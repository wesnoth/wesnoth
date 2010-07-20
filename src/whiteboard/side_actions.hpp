/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file side_actions.hpp
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

class side_actions: public boost::enable_shared_from_this<side_actions>
{
public:

	typedef action_queue::iterator iterator;
	typedef action_queue::const_iterator const_iterator;

	side_actions();
	virtual ~side_actions();

	const action_queue& actions() const { return actions_; }

	/** Gets called when display is drawing a hex, to allow drawing symbols to the screen */
	void draw_hex(const map_location& hex);

	/**
	 * Executes the first action in the queue, and then deletes it.
	 * @return An iterator to the action itself if not finished, or else to the new first in line.
	 *         Returns end() if no actions remain.
	 */
	iterator execute_next();

	/**
	 * Executes the specified action, if it exists in the queue.
	 * If the action is not finished, it's moved at the end of the queue.
	 * @return An iterator to the action itself if not finished, or else the next action in the queue.
	 *         Returns end() if no actions remain.
	 */
	iterator execute(iterator position);

	/**
	 * Returns the iterator for the first (executed earlier) action within the actions set.
	 */
	iterator begin() {return actions_.begin(); }

	/**
	 * Returns the iterator for the position *after* the last executed action within the actions set.
	 */
	iterator end() { return actions_.end(); }

	/**
	 * Indicates whether the action queue is empty.
	 */
	bool empty() { return actions_.empty(); }

//	/**
//	 * Inserts a move at the specified position. The begin() and end() functions might prove useful here.
//	 * @return The inserted move's position.
//	 */
//	iterator insert_move(const map_location& source_hex, const map_location& target_hex,
//			iterator position, arrow_ptr arrow, fake_unit_ptr fake_unit);

	/**
	 * Queues a move to be executed last
	 * @return The queued move's position
	 */
	iterator queue_move(const map_location& source_hex, const map_location& target_hex,
			arrow_ptr arrow, fake_unit_ptr fake_unit);

	/**
	 * Queues an attack or attack-move to be executed last
	 * @return The queued attack's position
	 */
	iterator queue_attack(const map_location& target_hex, int weapon_choice, const map_location& source_hex, const map_location& dest_hex,
			arrow_ptr arrow, fake_unit_ptr fake_unit);

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
	iterator find_first_action_of(const unit& unit, iterator start_position = iterator());

	/**
	 * Finds the last action that belongs to this unit, starting the search backwards from the specified position.
	 * @return The position, or end() if not found.
	 */
	iterator find_last_action_of(const unit& unit, iterator start_position = iterator());

	void validate_actions();

private:

	bool validate_iterator(iterator position) { return position >= begin() && position < end(); }

	action_queue actions_;
};

/** Dumps side_actions on a stream, for debug purposes. */
std::ostream &operator<<(std::ostream &s, wb::side_actions const& side_actions);

}

#endif /* WB_SIDE_ACTIONS_HPP_ */
