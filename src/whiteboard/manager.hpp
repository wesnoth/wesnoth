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

#include "mapbuilder_visitor.hpp"

#include "log.hpp"
#include "map_location.hpp"

#include <boost/scoped_ptr.hpp>

#include <vector>

static lg::log_domain log_whiteboard("whiteboard");
#define LOG_WB LOG_STREAM(info, log_whiteboard)
#define DBG_WB LOG_STREAM(debug, log_whiteboard)
#define ERR_WB LOG_STREAM(err, log_whiteboard)

class arrow;

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

	/**
	 * Temporarily apply the effects of the current team's
	 * planned moves to the unit map.
	 */
	void apply_temp_modifiers();
	void remove_temp_modifiers();

	/** Set the route for move creation purposes */
	void set_route(const std::vector<map_location> &steps);

private:
	/**
	 * Tracks whether the whiteboard is active.
	 */
	bool active_;

	boost::scoped_ptr<mapbuilder_visitor> mapbuilder_;

	std::vector<map_location> route_;

	arrow* move_arrow_;
};

} // end namespace wb

#endif /* WB_MANAGER_HPP_ */
