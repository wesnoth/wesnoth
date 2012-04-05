/* $Id$ */
/*
 Copyright (C) 2010 - 2012 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include "validate_visitor.hpp"
#include "attack.hpp"
#include "manager.hpp"
#include "move.hpp"
#include "recall.hpp"
#include "recruit.hpp"
#include "side_actions.hpp"
#include "suppose_dead.hpp"
#include "utility.hpp"

#include "arrow.hpp"
#include "foreach.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

namespace wb
{

validate_visitor::validate_visitor(unit_map& unit_map)
	: builder_(unit_map,*this)
	, viewer_actions_(*viewer_actions())
	, actions_to_erase_()
	, arg_itor_()
{
	assert(!resources::whiteboard->has_planned_unit_map());
}

validate_visitor::~validate_visitor()
{
}

bool validate_visitor::validate_actions()
{
	builder_.build_map();

	//FIXME: by reverse iterating this can be done in a more efficiant way
	// by using the iterator returned by remove_action it could even be done in visit_all above
	if (!actions_to_erase_.empty())
	{
		int side_actions_size_before = viewer_actions_.size();
		LOG_WB << "Erasing " << actions_to_erase_.size() << " invalid actions.\n";
		foreach(action_ptr action, actions_to_erase_)
		{
			viewer_actions_.remove_action(viewer_actions_.get_position_of(action), false);
		}
		assert(side_actions_size_before - viewer_actions_.size() == actions_to_erase_.size());
		actions_to_erase_.clear();
		return false;
	}
	else
	{
		return true;
	}
}

/* private */
validate_visitor::VALIDITY validate_visitor::evaluate_move_validity(move_ptr m_ptr)
{
	move& m = *m_ptr;

	if (!(m.get_source_hex().valid() && m.get_dest_hex().valid()))
		return WORTHLESS;

	//Check that the unit still exists in the source hex
	unit_map::iterator unit_it;
	unit_it = resources::units->find(m.get_source_hex());
	if (unit_it == resources::units->end())
		return WORTHLESS;

	//check if the unit in the source hex has the same unit id as before,
	//i.e. that it's the same unit
	if (m.unit_id_ != unit_it->id() || m.unit_underlying_id_ != unit_it->underlying_id())
		return WORTHLESS;

	//If the path has at least two hexes (it can have less with the attack subclass), ensure destination hex is free
	if (m.get_route().steps.size() >= 2 && get_visible_unit(m.get_dest_hex(),resources::teams->at(viewer_team())) != NULL) {
		return WORTHLESS;
	}

	//check that the path is good
	if (m.get_source_hex() != m.get_dest_hex()) //skip zero-hex move used by attack subclass
	{
		// Mark the plain route to see if the move can still be done in one turn,
		// which is always the case for planned moves
		pathfind::marked_route checked_route = pathfind::mark_route(m.get_route().route);

		if (checked_route.marks[checked_route.steps.back()].turns != 1)
			return OBSTRUCTED;
	}

	return VALID;
}

// This helper function determines whether there are any invalid actions planned for (*itor)->get_unit()
// that occur earlier in viewer_actions_ than itor.
/* private */
bool validate_visitor::no_previous_invalids(side_actions::iterator const& itor)
{
	if(itor == viewer_actions_.begin())
		return true;
	side_actions::iterator prev_action_of_unit = viewer_actions_.find_last_action_of((*itor)->get_unit(),itor-1);
	if(prev_action_of_unit == viewer_actions_.end())
		return true;
	return (*prev_action_of_unit)->is_valid();
}

void validate_visitor::visit(move_ptr move)
{
	DBG_WB <<"visiting move from " << move->get_source_hex()
			<< " to " << move->get_dest_hex() << "\n";
	//invalidate start and end hexes so number display is updated properly
	resources::screen->invalidate(move->get_source_hex());
	resources::screen->invalidate(move->get_dest_hex());

	switch(evaluate_move_validity(move)) //< private helper fcn
	{
	case VALID:
		// Now call the superclass to apply the result of this move to the unit map,
		// so that further pathfinding takes it into account.
		move->set_valid(true);
		break;
	case OBSTRUCTED:
		move->set_valid(false);
		break;
	case WORTHLESS:
		move->set_valid(false);
		// Erase only if no previous invalid actions are planned for this unit -- otherwise, just mark it invalid.
		// Otherwise, we wouldn't be able to keep invalid actions that depend on previous invalid actions.
		if(viewer_team() == move->team_index() //< Don't mess with any other team's queue -- only our own
				&& no_previous_invalids(arg_itor_)) //< private helper fcn
		{
			LOG_WB << "Worthless invalid move detected, adding to actions_to_erase_.\n";
			actions_to_erase_.insert(move);
		}
		break;
	}
}

void validate_visitor::visit(attack_ptr attack)
{
	DBG_WB <<"visiting attack from " << attack->get_dest_hex()
			<< " to " << attack->target_hex_ << "\n";
	//invalidate target hex to make sure attack indicators are updated
	resources::screen->invalidate(attack->get_dest_hex());
	resources::screen->invalidate(attack->target_hex_);

	if  (
			// Verify that the unit that planned this attack exists
			attack->get_unit()
			// Verify that the target hex is still valid
			&& attack->target_hex_.valid()
			// Verify that the target hex isn't empty
			&& resources::units->find(attack->target_hex_) != resources::units->end()
			// Verify that the attacking unit has attacks left
			&& attack->get_unit()->attacks_left() > 0
			// Verify that the attacker and target are enemies
			&& (*resources::teams)[attack->get_unit()->side() - 1].is_enemy(resources::units->find(attack->target_hex_)->side())

			//@todo: (maybe) verify that the target hex contains the same unit that before,
			// comparing for example the unit ID
		)
	{
		//All checks pass, so call the visitor on the superclass
		visit(boost::static_pointer_cast<move>(attack));
	}
	else
	{
		attack->set_valid(false);

		if (viewer_team() == attack->team_index()) //< Don't mess with any other team's queue -- only our own
		{
			LOG_WB << "Worthless invalid attack detected, adding to actions_to_erase_.\n";
			actions_to_erase_.insert(attack);
		}
	}
}

void validate_visitor::visit(recruit_ptr recruit)
{
	DBG_WB << "visiting recruit on hex " << recruit->recruit_hex_ << "\n";
	//invalidate recruit hex so number display is updated properly
	resources::screen->invalidate(recruit->recruit_hex_);

	size_t team_index = recruit->team_index();

	//Check that destination hex is still free
	if(resources::units->find(recruit->recruit_hex_) != resources::units->end())
	{
		LOG_WB << "Recruit set as invalid because target hex is occupied.\n";
		recruit->set_valid(false);
	}
	//Check that unit to recruit is still in side's recruit list
	if (recruit->is_valid())
	{
		const std::set<std::string>& recruits = (*resources::teams)[team_index].recruits();
		if (recruits.find(recruit->unit_name_) == recruits.end())
		{
			recruit->set_valid(false);
			LOG_WB << " Validate visitor: Planned recruit invalid since unit is not in recruit list anymore.\n";
		}
	}
	//Check that there is still enough gold to recruit this unit
	if (recruit->is_valid() && recruit->temp_unit_->cost() > (*resources::teams)[team_index].gold())
	{
		LOG_WB << "Recruit set as invalid, team doesn't have enough gold.\n";
		recruit->set_valid(false);
	}
	//Check that there is a leader available to recruit this unit
	if(recruit->is_valid() && !find_recruiter(recruit->team_index(),recruit->get_recruit_hex()))
	{
		LOG_WB << "Recruit set as invalid, no leader can recruit this unit.\n";
		recruit->set_valid(false);
	}

	if(!recruit->is_valid())
	{
		if(viewer_team() == recruit->team_index()) //< Don't mess with any other team's queue -- only our own
		{
			LOG_WB << "Invalid recruit detected, adding to actions_to_erase_.\n";
			actions_to_erase_.insert(recruit);
		}
	}
}

void validate_visitor::visit(recall_ptr recall)
{
	DBG_WB << "visiting recall on hex " << recall->recall_hex_ << "\n";
	//invalidate recall hex so number display is updated properly
	resources::screen->invalidate(recall->recall_hex_);

	size_t team_index = recall->team_index();

	//Check that destination hex is still free
	if(resources::units->find(recall->recall_hex_) != resources::units->end())
	{
		LOG_WB << "Recall set as invalid because target hex is occupied.\n";
		recall->set_valid(false);
	}
	//Check that unit to recall is still in side's recall list
	if (recall->is_valid())
	{
		const std::vector<unit>& recalls = (*resources::teams)[team_index].recall_list();
		if (std::find_if(recalls.begin(), recalls.end(),
				unit_comparator_predicate(*recall->temp_unit_)) == recalls.end())
		{
			recall->set_valid(false);
			LOG_WB << " Validate visitor: Planned recall invalid since unit is not in recall list anymore.\n";
		}
	}
	//Check that there is still enough gold to recall this unit
	if (recall->is_valid()
			&& (*resources::teams)[team_index].recall_cost() > (*resources::teams)[team_index].gold())
	{
		LOG_WB << "Recall set as invalid, team doesn't have enough gold.\n";
		recall->set_valid(false);
	}
	//Check that there is a leader available to recall this unit
	if(recall->is_valid() && !find_recruiter(recall->team_index(),recall->get_recall_hex()))
	{
		LOG_WB << "Recall set as invalid, no leader can recall this unit.\n";
		recall->set_valid(false);
	}


	if(!recall->is_valid())
	{
		if(viewer_team() == recall->team_index()) //< Don't mess with any other team's queue -- only our own
		{
			LOG_WB << "Invalid recall detected, adding to actions_to_erase_.\n";
			actions_to_erase_.insert(recall);
		}
	}
}

void validate_visitor::visit(suppose_dead_ptr sup_d)
{
	DBG_WB << "visiting suppose_dead on hex " << sup_d->loc_ << "\n";
	//invalidate suppose-dead hex so number display is updated properly
	resources::screen->invalidate(sup_d->loc_);

	if(!sup_d->get_source_hex().valid())
		sup_d->set_valid(false);

	unit_map::const_iterator unit_it;
	//Check that the unit still exists in the source hex
	if(sup_d->valid_)
	{
		unit_it = resources::units->find(sup_d->get_source_hex());

		if(unit_it == resources::units->end())
		{
			sup_d->set_valid(false);
		}
	}

	//check if the unit in the source hex has the same unit id as before,
	//i.e. that it's the same unit
	if(sup_d->valid_ && sup_d->unit_id_ != unit_it->id())
	{
		sup_d->set_valid(false);
	}

	if(!sup_d->valid_)
	{
		if(viewer_team() == sup_d->team_index()) //< Don't mess with any other team's queue -- only our own
		{
			LOG_WB << "Invalid suppose_dead detected, adding to actions_to_erase_.\n";
			actions_to_erase_.insert(sup_d);
		}
	}
}

void validate_visitor::helper::validate(side_actions::iterator const& itor)
{
	parent_.arg_itor_ = itor;
	(*itor)->accept(parent_);
}

}//end namespace wb
