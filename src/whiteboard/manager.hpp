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

#include "log.hpp"

static lg::log_domain log_whiteboard("whiteboard");
#define LOG_WB LOG_STREAM(info, log_whiteboard)
#define DBG_WB LOG_STREAM(debug, log_whiteboard)
#define ERR_WB LOG_STREAM(err, log_whiteboard)

namespace wb {

/**
 * This class holds and manages all of the whiteboard's planned actions.
 */
class manager
{
public:

	manager();

	/**
	 * Determine whether the whiteboard is activated.
	 */
	bool active(){ return active_; }

	void set_active(bool active){ active_ = active; }

private:
	/**
	 * Tracks whether the whiteboard is active.
	 */
	bool active_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
