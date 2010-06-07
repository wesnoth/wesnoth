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

#include <vector>

#include <boost/noncopyable.hpp>

namespace wb {

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

	/**
	 * Determine whether the whiteboard is activated.
	 */
	bool active(){ return active_; }

	void set_active(bool active){ active_ = active; }

private:
	/// Singleton -> private constructor
	manager();

	static manager* instance_;

	/**
	 * Tracks whether the whiteboard is active.
	 */
	bool active_;

};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
