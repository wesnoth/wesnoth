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

#include "validate_visitor.hpp"
#include "attack.hpp"
#include "move.hpp"
#include "recruit.hpp"
#include "side_actions.hpp"

#include "arrow.hpp"
#include "foreach.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

namespace wb
{

validate_visitor::validate_visitor(unit_map& unit_map, side_actions_ptr side_actions)
	: mapbuilder_visitor(unit_map, side_actions, true)
	, actions_to_erase_()
{
}

validate_visitor::~validate_visitor()
{
}

//FIXME: move this some place it can be accessible from the whole whiteboard
static team& get_current_team()
{
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];
	return current_team;
}

bool validate_visitor::validate_actions()
{
	foreach(action_ptr action, *side_actions_)
	{
		action->accept(*this);
	}

	if (!actions_to_erase_.empty())
	{
		foreach(action_ptr action, actions_to_erase_)
		{
			side_actions_->remove_action(side_actions_->get_position_of(action), false);
		}
		actions_to_erase_.clear();
		return false;
	}
	else
	{
		return true;
	}
}

void validate_visitor::visit_move(move_ptr move)
{
	//invalidate start and end hexes so number display is updated properly
	resources::screen->invalidate(move->get_source_hex());
	resources::screen->invalidate(move->get_dest_hex());

	if (!(move->get_source_hex().valid() && move->get_dest_hex().valid()))
		move->set_valid(false);

	//TODO: need to check if the unit in the source hex has the same underlying unit id as before,
	//i.e. that it's the same unit
	if (move->valid_ && resources::units->find(move->get_source_hex()) == resources::units->end())
		move->set_valid(false);

	if (move->get_source_hex() != move->get_dest_hex()) //allow for zero-hex, move, in which case we skip pathfinding
	{
		//verify that the destination hex is free
		if (move->valid_ && (resources::units->find(move->get_dest_hex()) != resources::units->end()))
			move->set_valid(false);

		pathfind::plain_route new_plain_route;
		if (move->valid_)
		{
			pathfind::shortest_path_calculator path_calc(*move->get_unit(), get_current_team(), *resources::units,
					*resources::teams, *resources::game_map);
			new_plain_route = pathfind::a_star_search(move->get_source_hex(),
					move->get_dest_hex(), 10000, &path_calc, resources::game_map->w(), resources::game_map->h());
			if (new_plain_route.move_cost >= path_calc.getNoPathValue())
			{
				move->set_valid(false);
			}
		}

		if (move->valid_)
		{
			if ((!std::equal(new_plain_route.steps.begin(), new_plain_route.steps.end(), move->get_route().steps.begin()))
				|| new_plain_route.move_cost != move->get_route().move_cost)
			{
				//new valid path differs from the previous one, replace
				//TODO: use something else than empty vector for waypoints?
				pathfind::marked_route new_marked_route =
						pathfind::mark_route(new_plain_route, std::vector<map_location>());

				move->set_route(new_marked_route);

				//TODO: Since this might lengthen the path, we probably need a special conflict state
				// to warn the player that the initial path is no longer possible.

			}
			// Now call the superclass to apply the result of this move to the unit map,
			// so that further pathfinding takes it into account.
			mapbuilder_visitor::visit_move(move);
		}

		//FIXME: temporary until invalid arrow styles are in: delete invalid moves
		if (!move->valid_)
		{
			actions_to_erase_.insert(move);
		}
	}
}

void validate_visitor::visit_attack(attack_ptr attack)
{
	//invalidate target hex to make sure attack indicators are updated
	resources::screen->invalidate(attack->get_dest_hex());
	resources::screen->invalidate(attack->target_hex_);

	if (attack->target_hex_.valid())
	{
		//TODO: verify that the target hex contains the same unit that before,
		// comparing for example the underlying unit ID
		if (resources::units->find(attack->target_hex_) == resources::units->end())
		{
			attack->set_valid(false);
		}
		else
		{
			visit_move(boost::static_pointer_cast<move>(attack));
		}
	}
	else
	{
		attack->set_valid(false);
	}

	if (!attack->is_valid())
	{
		actions_to_erase_.insert(attack);
	}
}

void validate_visitor::visit_recruit(recruit_ptr recruit)
{
	log_scope("validate_visitor::visit_recruit");
	//invalidate recruit hex so number display is updated properly
	resources::screen->invalidate(recruit->recruit_hex_);

	int team_index = resources::screen->viewing_team();

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
}

}//end namespace wb
