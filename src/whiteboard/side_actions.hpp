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

#include "action.hpp"

#include "map_location.hpp"

#include <deque>

#include <boost/shared_ptr.hpp>

class unit;
class arrow;

namespace wb
{

class move;

typedef std::deque<action_ptr> action_set;

class side_actions
{
public:

	typedef action_set::iterator iterator;
	typedef action_set::const_iterator const_iterator;

	side_actions();
	virtual ~side_actions();

	const action_set& actions() const;

	/**
	 * Executes the first action in the queue, and then deletes it.
	 * @return An iterator to the action itself if not finished, or else to the new first in line.
	 *         Returns end() if no actions remain.
	 */
	iterator execute_next();

	/**
	 * Executes the specified action, if it exists in the queue.
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

	/**
	 * Inserts a move at the specified position. The begin() and end() functions might prove useful here.
	 * @return The inserted move's position.
	 */
	iterator insert_move(unit& subject, const map_location& source_hex, const map_location& target_hex, iterator position,
			boost::shared_ptr<arrow> arrow,	boost::shared_ptr<unit> fake_unit);

	/**
	 * Inserts a move to be executed last (i.e. at the back of the queue)
	 * @return The queued move's position
	 */
	iterator queue_move(unit& subject, const map_location& source_hex, const map_location& target_hex,
			boost::shared_ptr<arrow> arrow,	boost::shared_ptr<unit> fake_unit);

	/**
	 * Moves an action earlier in the execution order (i.e. at the front of the queue),
	 * by the specified increment.
	 * @return The action's new position.
	 */
	iterator move_earlier(iterator position, size_t increment);

	/**
	 * Moves an action later in the execution order (i.e. at the back of the queue),
	 * by the specified increment.
	 * @return The action's new position.
	 */
	iterator move_later(iterator position, size_t increment);

	/**
	 * Deletes the action at the specified position.
	 * @return The position of the element after the one deleted, or end() if the queue is empty.
	 */
	iterator remove_action(iterator position);

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

	/**
	 * future_view = true : units are shown at their future positions
	 * future_view = false: units' future positions are shown with ghosts
	 */
	void set_future_view(bool future_view);

	/**
	 * Determines which is the last action of each unit, and updates the actions' display
	 * accordingly.
	 */
	void update_last_action_display();

	void validate_actions();

private:
	/**
	 * Utility function to move an action around the queue.
	 * Positive increment = move toward back of the queue and later execution.
	 * Negative increment = move toward front of the queue and earlier execution.
	 * @return The action's new position.
	 */
	iterator move_in_queue(iterator position, int increment);

	bool validate_iterator(iterator position) { return position >= begin() && position < end(); }

	action_set actions_;
};

}

#endif /* WB_SIDE_ACTIONS_HPP_ */
