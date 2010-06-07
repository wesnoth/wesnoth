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

#include "side_actions.hpp"

#include <vector>

#include <boost/noncopyable.hpp>

namespace wb {

/**
 * This class holds and manages all of the whiteboard's planned actions.
 */
class manager : private boost::noncopyable // Singleton -> Non-copyable
{
public:

	/**
	 * Get the singleton instance.
	 */
	static manager& instance();

	/**
	 * Determine whether the whiteboard is activated.
	 */
	bool active(){ return active_; }

	void set_active(bool active){ active_ = active; }

	side_actions& get_side_actions(size_t side);

private:
	/// Singleton -> private constructor
	manager();

	static manager* instance_;

	/**
	 * Tracks whether the whiteboard is active.
	 */
	bool active_;

	/**
	 * Data structure holding one side_actions object per side.
	 */
	std::vector<side_actions> actions_;

};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
