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
#include "attack.hpp"
#include "move.hpp"
#include "recall.hpp"
#include "recruit.hpp"
#include "side_actions.hpp"
#include "suppose_dead.hpp"
#include "utility.hpp"

#include "foreach.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

namespace wb
{

mapbuilder_visitor::mapbuilder_visitor(unit_map& unit_map)
	: visitor()
	, unit_map_(unit_map)
	, applied_actions_()
	, mode_(BUILD_PLANNED_MAP)
	, resetters_()
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
		if(u.side() != current_side)
			resetters_.push_back(new unit_movement_resetter(u));
	}
}

void mapbuilder_visitor::build_map()
{
	//Temporarily reset all units' moves to full EXCEPT for the ones on current_team.
	reset_moves();

	mode_ = BUILD_PLANNED_MAP;
	visit_all();
}

bool mapbuilder_visitor::visit(size_t, team&, side_actions&, side_actions::iterator itor)
{
	if((*itor)->is_valid())
		(*itor)->accept(*this);
	return true;
}

void mapbuilder_visitor::visit_move(move_ptr move)
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

void mapbuilder_visitor::visit_attack(attack_ptr attack)
{
	if(mode_ == BUILD_PLANNED_MAP)
	{
		attack->apply_temp_modifier(unit_map_);
		//remember which actions we applied, so we can unapply them later
		applied_actions_.push_back(attack);
	}
	else if (mode_ == RESTORE_NORMAL_MAP)
	{
		attack->remove_temp_modifier(unit_map_);
	}
}

void mapbuilder_visitor::visit_recruit(recruit_ptr recruit)
{
	if(mode_ == BUILD_PLANNED_MAP)
	{
		recruit->apply_temp_modifier(unit_map_);
		//remember which actions we applied, so we can unapply them later
		applied_actions_.push_back(recruit);
	}
	else if (mode_ == RESTORE_NORMAL_MAP)
	{
		recruit->remove_temp_modifier(unit_map_);
	}
}

void mapbuilder_visitor::visit_recall(recall_ptr recall)
{
	if(mode_ == BUILD_PLANNED_MAP)
	{
		recall->apply_temp_modifier(unit_map_);
		//remember which actions we applied, so we can unapply them later
		applied_actions_.push_back(recall);
	}
	else if (mode_ == RESTORE_NORMAL_MAP)
	{
		recall->remove_temp_modifier(unit_map_);
	}
}

void mapbuilder_visitor::visit_suppose_dead(suppose_dead_ptr sup_d)
{
	if(mode_ == BUILD_PLANNED_MAP)
	{
		sup_d->apply_temp_modifier(unit_map_);
		//remember which actions we applied, so we can unapply them later
		applied_actions_.push_back(sup_d);
	}
	else if(mode_ == RESTORE_NORMAL_MAP)
	{
		sup_d->remove_temp_modifier(unit_map_);
	}
}

void mapbuilder_visitor::restore_normal_map()
{
	mode_ = RESTORE_NORMAL_MAP;
	action_queue::const_reverse_iterator rit;
	action_queue::const_reverse_iterator end = applied_actions_.rend();
	//applied_actions_ contain only the actions that we applied to the unit map
	for (rit = applied_actions_.rbegin(); rit != end; ++rit)
	{
		assert((*rit)->is_valid());
		(*rit)->accept(*this);
	}
}

} // end namespace wb
