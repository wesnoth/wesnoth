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

#include "action.hpp"
#include "move.hpp"

#include <deque>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace wb {

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

	void add_move(unit& subject, const map_location& target_hex);

private:
	/// Singleton -> private constructor
	manager();

	static manager* instance_;

	action_set planned_actions_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
