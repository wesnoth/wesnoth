/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "foreach.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

namespace wb
{

mapbuilder_visitor::mapbuilder_visitor(unit_map& unit_map, side_actions_ptr side_actions, bool for_pathfinding)
	: visitor(side_actions)
	, unit_map_(unit_map)
	, for_pathfinding_(for_pathfinding)
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
		if (for_pathfinding_)
		{
			recruit->apply_temp_modifier(unit_map_);
			//remember which actions we applied, so we can unapply them later
			applied_actions_.push_back(recruit);
		}
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
		if (for_pathfinding_)
		{
			recall->apply_temp_modifier(unit_map_);
			//remember which actions we applied, so we can unapply them later
			applied_actions_.push_back(recall);
		}
	}
	else if (mode_ == RESTORE_NORMAL_MAP)
	{
		recall->remove_temp_modifier(unit_map_);
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
		if ((*rit)->is_valid())
		{
			(*rit)->accept(*this);
		}
	}
}

} // end namespace wb
