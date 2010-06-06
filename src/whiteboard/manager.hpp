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
 * @file manager.hpp
 */

#ifndef WB_MANAGER_HPP_
#define WB_MANAGER_HPP_

#include "map_location.hpp"

#include <deque>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class unit;
class arrow;

namespace wb {

class action;
class move;

typedef boost::shared_ptr<action> action_ptr;
typedef std::deque<action_ptr> action_set;

/**
 * This class holds and manages all of the whiteboard's planned actions.
 */
class manager : private boost::noncopyable // Singleton -> Non-copyable
{
public:

	virtual ~manager();

	/**
	 * Get the singleton instance.
	 */
	static manager& instance();

	const action_set& get_actions() const;

	/**
	 * If index = -1, the default value, then the move is appended at the end of the action_set.
	 * For any other value, the added move REPLACES the action at the specified position.
	 */
	void add_move(unit& subject, const map_location& target_hex, arrow& arrow, int index = -1);

	/**
	 * Moves an action toward the front of the action_set by the specified increment.
	 * If index = -1, the default value, then the last action is moved up the action_set.
	 */
	void push_up(int index = -1, size_t increment = 1);

	/**
	 * Moves an action toward the back of the action_set by the specified increment.
	 */
	void push_down(int index, size_t increment = 1);

	/**
	 * If index = -1, the default value, the the last action in the action_set is deleted.
	 * Otherwise the action at the specified index is deleted.
	 */
	void remove_action(int index = -1);

private:
	/// Singleton -> private constructor
	manager();

	static manager* instance_;

	action_set planned_actions_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
