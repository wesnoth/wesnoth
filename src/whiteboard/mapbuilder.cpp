/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file
 */

#include "mapbuilder.hpp"

#include "action.hpp"
#include "side_actions.hpp"
#include "utility.hpp"

#include "foreach.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

namespace wb
{

mapbuilder::mapbuilder(unit_map& unit_map)
	: unit_map_(unit_map)
	, applied_actions_()
	, resetters_()
	, removers_()
	, acted_this_turn_()
{
}

mapbuilder::~mapbuilder()
{
	restore_normal_map();
	//Remember that the member variable resetters_ is destructed here
}

void mapbuilder::pre_build()
{
	int current_side = resources::controller->current_side();
	foreach(unit& u, *resources::units)
	{
		bool on_current_side = (u.side() == current_side);

		//Remove any unit the current side cannot see to avoid their detection by planning
		//Units will be restored to the unit map by destruction of removers_

		if (!on_current_side && !u.is_visible_to_team((*resources::teams)[viewer_team()], false))
		{
			removers_.push_back(new temporary_unit_remover(*resources::units, u.get_location()));

			//Don't do anything else to the removed unit!
			continue;
		}

		//Reset movement points, to be restored by destruction of resetters_

		/**
		 * @todo Fix this code.
		 *
		 * It has been disabled since the destructor of this class restored the
		 * movement, also of deleted units causing bug #18534, as work-around
		 * this code is disabled.
		 */
		//restore movement points only to units not on the current side
		//resetters_.push_back(new unit_movement_resetter(u,!on_current_side));

		//make sure current side's units are not reset to full moves on first turn
		if(on_current_side)
			acted_this_turn_.insert(&u);
	}
}

void mapbuilder::build_map()
{
	pre_build();
	visit_all();
}

///@return whether act is invalid
bool mapbuilder::visit_helper(side_actions::iterator const& itor, action_ptr const& act)
{
	validate(itor);
	if(act->is_valid())
	{
		act->apply_temp_modifier(unit_map_);
		applied_actions_.push_back(act);
		return false;
	}
	else //invalid
		return true;
}

bool mapbuilder::visit(size_t, team&, side_actions&, side_actions::iterator itor)
{
	action_ptr act = *itor;
	unit* u = act->get_unit();

	if(acted_this_turn_.find(u) != acted_this_turn_.end())
		visit_helper(itor,act);
	else //gotta restore MP first
	{
		int original_moves = u->movement_left();

		//reset MP
		u->set_movement(u->total_movement());
		acted_this_turn_.insert(u);

		bool revert = visit_helper(itor,act);

		if(revert) //< the action was invalid
		{
			//didn't need to restore MP after all ... so let's change it back
			acted_this_turn_.erase(u);
			u->set_movement(original_moves);
		}
	}
	return true;
}

bool mapbuilder::post_visit_team(size_t, team&, side_actions&)
{
	acted_this_turn_.clear();
	return true;
}

void mapbuilder::restore_normal_map()
{
	//applied_actions_ contain only the actions that we applied to the unit map
	BOOST_REVERSE_FOREACH(action_ptr act, applied_actions_)
	{
		assert(act->is_valid());
		act->remove_temp_modifier(unit_map_);
	}
}

} // end namespace wb
