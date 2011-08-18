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

#include "mapbuilder_visitor.hpp"

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

mapbuilder_visitor::mapbuilder_visitor(unit_map& unit_map)
	: unit_map_(unit_map)
	, applied_actions_()
	, resetters_()
	, acted_this_turn_()
{
}

mapbuilder_visitor::~mapbuilder_visitor()
{
	restore_normal_map();
	//Remember that the member variable resetters_ is destructed here
}

void mapbuilder_visitor::reset_moves()
{
	int current_side = resources::controller->current_side();
	foreach(unit& u, *resources::units)
	{
		resetters_.push_back(new unit_movement_resetter(u,false));
		//make sure current team's units are not reset to full moves on first turn
		if(u.side() == current_side)
			acted_this_turn_.insert(&u);
	}
}

void mapbuilder_visitor::build_map()
{
	reset_moves();
	visit_all();
}

bool mapbuilder_visitor::visit(size_t, team&, side_actions&, side_actions::iterator itor)
{
	action_ptr act = *itor;
	unit* u = act->get_unit();

	if(acted_this_turn_.find(u) == acted_this_turn_.end())
	{
		u->set_movement(u->total_movement());
		acted_this_turn_.insert(u);
	}
	validate(itor);
	if(act->is_valid())
	{
		act->apply_temp_modifier(unit_map_);
		applied_actions_.push_back(act);
	}
	return true;
}

bool mapbuilder_visitor::post_visit_team(size_t, team&, side_actions&)
	{acted_this_turn_.clear();   return true;}

void mapbuilder_visitor::restore_normal_map()
{
	//applied_actions_ contain only the actions that we applied to the unit map
	BOOST_REVERSE_FOREACH(action_ptr act, applied_actions_)
	{
		assert(act->is_valid());
		act->remove_temp_modifier(unit_map_);
	}
}

} // end namespace wb
