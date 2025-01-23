/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "vision.hpp"
#include "map/location.hpp"
#include "units/ptr.hpp"

namespace actions
{
/** base class for classes that clear srhoud (move/recruit/recall) */
struct shroud_clearing_action
{

	shroud_clearing_action(const config& cfg)
		: route()
		, view_info(cfg.child_or_empty("unit"))
	{
		read_locations(cfg, route);
	}

	shroud_clearing_action(const unit_const_ptr u, const map_location& loc)
		: route(1, loc)
		, view_info(*u)
	{

	}

	typedef std::vector<map_location> route_t;

	shroud_clearing_action(const unit_const_ptr u, const route_t::const_iterator& begin, const route_t::const_iterator& end)
		: route(begin, end)
		, view_info(*u)
	{

	}

	/**
	 * The hexes occupied by the affected unit during this action.
	 * For recruits and recalls this only contains one hex.
	 */
	route_t route;
	/** A record of the affected unit's ability to see. */
	clearer_info view_info;

	void write(config & cfg) const
	{
		write_locations(route, cfg);
		view_info.write(cfg.add_child("unit"));
	}

	virtual ~shroud_clearing_action() {}
};
}
