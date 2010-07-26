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

#include "mapbuilder_visitor.hpp"
#include "action.hpp"
#include "attack.hpp"
#include "move.hpp"
#include "side_actions.hpp"

#include "foreach.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

namespace wb
{

mapbuilder_visitor::mapbuilder_visitor(unit_map& unit_map, side_actions_ptr side_actions)
	: unit_map_(unit_map)
    , excluded_units_()
	, side_actions_(side_actions)
	, applied_actions_()
	, mode_(BUILD_PLANNED_MAP)
{
}

mapbuilder_visitor::~mapbuilder_visitor()
{
	restore_normal_map();
}

void mapbuilder_visitor::build_map()
{
	mode_ = BUILD_PLANNED_MAP;
	foreach(action_ptr action, *side_actions_)
	{
		if (action->is_valid())
		{
			action->accept(*this);
		}
	}
}

void mapbuilder_visitor::visit_move(move_ptr move)
{
	if (excluded_units_.find(move->get_unit()) == excluded_units_.end())
	{
		if(mode_ == BUILD_PLANNED_MAP)
		{
			move->apply_temp_modifier(unit_map_);
			//remember which actions we applied, so we can unapply them later
			applied_actions_.push_back(move);
		}
		else if (mode_ == RESTORE_NORMAL_MAP)
		{
			move->remove_temp_modifier(unit_map_);
		}
	}
}

void mapbuilder_visitor::visit_attack(attack_ptr attack)
{
	visit_move(boost::static_pointer_cast<move>(attack));
}


void mapbuilder_visitor::restore_normal_map()
{
	mode_ = RESTORE_NORMAL_MAP;
	action_queue::const_reverse_iterator rit;
	//applied_actions_ contain only the actions that we applied to the unit map
	for (rit = applied_actions_.rbegin(); rit != applied_actions_.rend(); ++rit)
	{
		if ((*rit)->is_valid())
		{
			(*rit)->accept(*this);
		}
	}
}

} // end namespace wb
