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
	side_actions();
	virtual ~side_actions();

	const action_set& actions() const;

	/**
	 * Returns the index for the first (executed earlier) action within the actions set.
	 */
	size_t begin() {return 0; }

	/**
	 * Returns the index for the position *after* the last executed action within the actions set.
	 */
	size_t end() { return actions_.size(); }

	/**
	 * Inserts a move at the specified index. The begin() and end() functions might prove useful here.
	 */
	void insert_move(unit& subject, const map_location& target_hex, size_t index, boost::shared_ptr<arrow> arrow,
			boost::shared_ptr<unit> fake_unit);

	/**
	 * Inserts a move to be executed last (i.e. at the back of the queue)
	 */
	void queue_move(unit& subject, const map_location& target_hex, boost::shared_ptr<arrow> arrow,
			boost::shared_ptr<unit> fake_unit);

	/**
	 * Moves an action earlier in the execution order (i.e. at the front of the queue),
	 * by the specified increment.
	 * If the increment is too large, the action will be simply moved at the earliest position.
	 */
	void move_earlier(size_t index, size_t increment);

	/**
	 * Moves an action later in the execution order (i.e. at the back of the queue),
	 * by the specified increment.
	 * If the increment is too large, the action will be simply moved at the latest position.
	 */
	void move_later(size_t index, size_t increment);

	/**
	 * Deletes the action at the specified index. The begin() and end() functions might prove useful here.
	 * If the index doesn't exist, the function does nothing.
	 */
	void remove_action(size_t index);

	/**
	 * Deletes the specified action. If the action doesn't exist, the function does nothing.
	 */
	void remove_action(action_ptr action);

private:
	/**
	 * Utility function to move actions around the queue.
	 * Positive increment = move toward back of the queue and later execution.
	 * Negative increment = move toward front of the queue and earlier execution.
	 */
	void move_in_queue(size_t index, int increment);

	action_set actions_;
};

}

#endif /* WB_SIDE_ACTIONS_HPP_ */
