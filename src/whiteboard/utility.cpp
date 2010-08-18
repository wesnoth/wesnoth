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

#include "manager.hpp"

#include "actions.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "map.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit.hpp"

namespace wb {


void fake_unit_deleter::operator() (unit*& fake_unit)
{
    if (fake_unit)
    {
        if(resources::screen)
        {
        	resources::screen->remove_temporary_unit(fake_unit);
        }
        DBG_WB << "Erasing temporary unit " << fake_unit->name() << " [" << fake_unit->id() << "]\n";
        delete fake_unit;
    }
}

size_t viewer_team()
{
	return resources::screen->viewing_team();
}

int viewer_side()
{
	return resources::screen->viewing_side();
}

side_actions_ptr viewer_actions()
{
	side_actions_ptr side_actions =
			(*resources::teams)[resources::screen->viewing_team()].get_side_actions();
	return side_actions;
}

side_actions_ptr current_side_actions()
{
	side_actions_ptr side_actions =
			(*resources::teams)[resources::controller->current_side() - 1].get_side_actions();
	return side_actions;
}

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

unit* future_visible_unit(map_location hex, int viewer_side)
{
	scoped_planned_unit_map planned_unit_map;
	assert(resources::whiteboard->has_planned_unit_map());
	//use global method get_visible_unit
	return get_visible_unit(hex, resources::teams->at(viewer_side - 1), false);
}

unit* future_visible_unit(int on_side, map_location hex, int viewer_side)
{
	unit* unit = future_visible_unit(hex, viewer_side);
	if (unit && unit->side() == on_side)
		return unit;
	else
		return NULL;
}

} //end namespace wb

