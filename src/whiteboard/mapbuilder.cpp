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
	foreach(team& t, *resources::teams)
	{
		//Reset spent gold to zero, it'll be recalculated during the map building
		t.get_side_actions()->reset_gold_spent();
	}

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

		//restore movement points only to units not on the current side
		resetters_.push_back(new unit_movement_resetter(u,!on_current_side));
		//make sure current side's units are not reset to full moves on first turn
		if(on_current_side)
			acted_this_turn_.insert(&u);
	}
}

void mapbuilder::build_map()
{
	pre_build();
	if (wb::has_actions()) {
		visit_all();
	}
}

///@return whether act is valid
bool mapbuilder::process_helper(side_actions::iterator const& itor, action_ptr const& act)
{
	validate(itor);
	if(act->is_valid())
	{
		act->apply_temp_modifier(unit_map_);
		applied_actions_.push_back(act);
		return true;
	}
	else //invalid
		return false;
}

bool mapbuilder::process(size_t, team&, side_actions&, side_actions::iterator action_it)
{
	action_ptr action = *action_it;

	unit* unit = action->get_unit();

	if(!unit || !action->is_valid() || acted_this_turn_.find(unit) != acted_this_turn_.end())
		process_helper(action_it,action);
	else //gotta restore MP first
	{
		int original_moves = unit->movement_left();

		//reset MP
		unit->set_movement(unit->total_movement());
		acted_this_turn_.insert(unit);

		bool revert = !process_helper(action_it,action);

		if(revert) //< the action was invalid
		{
			//didn't need to restore MP after all ... so let's change it back
			acted_this_turn_.erase(unit);
			unit->set_movement(original_moves);
		}
	}
	return true;
}

bool mapbuilder::pre_visit_team(size_t /*team_index*/, team&, side_actions& sa)
{
	return !sa.hidden();
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
