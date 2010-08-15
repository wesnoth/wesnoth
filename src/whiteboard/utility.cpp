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
 * @file
 */

#include "utility.hpp"

#include "actions.hpp"
#include "foreach.hpp"
#include "map.hpp"
#include "resources.hpp"
#include "unit.hpp"

namespace wb {

unit const* find_backup_leader(unit const& leader)
{
	assert(leader.can_recruit());
	assert(resources::game_map->is_keep(leader.get_location()));
	foreach(unit const& unit, *resources::units)
	{
		if (unit.can_recruit() &&
				resources::game_map->is_keep(unit.get_location()) &&
				unit.id() != leader.id())
		{
			if (can_recruit_on(*resources::game_map, unit.get_location(), leader.get_location()))
				return &unit;
		}
	}
	return NULL;
}

} //end namespace wb

