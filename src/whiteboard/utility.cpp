/*
	Copyright (C) 2010 - 2024
	by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <limits>

#include "whiteboard/utility.hpp"

#include "whiteboard/manager.hpp"
#include "whiteboard/side_actions.hpp"

#include "display.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "utils/iterable_pair.hpp"

namespace wb {
int viewer_side()
{
	return display::get_singleton()->viewing_team().side();
}

side_actions_ptr viewer_actions()
{
	side_actions_ptr side_actions = display::get_singleton()->viewing_team().get_side_actions();
	return side_actions;
}

side_actions_ptr current_side_actions()
{
	side_actions_ptr side_actions =
			resources::gameboard->get_team(resources::controller->current_side()).get_side_actions();
	return side_actions;
}

unit_const_ptr find_backup_leader(const unit & leader)
{
	assert(leader.can_recruit());
	assert(resources::gameboard->map().is_keep(leader.get_location()));
	for (unit_map::const_iterator unit = resources::gameboard->units().begin(); unit != resources::gameboard->units().end(); ++unit)
	{
		if (unit->can_recruit() && unit->id() != leader.id())
		{
			if (dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(*unit, leader.get_location()))
				return unit.get_shared_ptr();
		}
	}
	return unit_const_ptr();
}

unit* find_recruiter(std::size_t team_index, const map_location& hex)
{
	if ( !resources::gameboard->map().is_castle(hex) )
		return nullptr;

	for(unit& u : resources::gameboard->units())
		if(u.can_recruit()
				&& u.side() == static_cast<int>(team_index+1)
				&& dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(u, hex))
			return &u;
	return nullptr;
}

bool any_recruiter(int team_num, const map_location& loc, const std::function<bool(unit&)>& func)
{
	if ( !resources::gameboard->map().is_castle(loc) ) {
		return false;
	}

	for(unit& u : resources::gameboard->units()) {
		if(u.can_recruit() && u.side() == team_num && dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(u, loc)) {
			if(func(u)) {
				return true;
			}
		}
	}
	return false;
}

const unit* future_visible_unit(map_location hex, int viewer_side)
{
	future_map planned_unit_map;
	if(!resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "future_visible_unit cannot find unit, future unit map failed to build.";
		return nullptr;
	}
	//use global method get_visible_unit
	return resources::gameboard->get_visible_unit(hex, resources::gameboard->get_team(viewer_side), false);
}

const unit* future_visible_unit(int on_side, map_location hex, int viewer_side)
{
	const unit* unit = future_visible_unit(hex, viewer_side);
	if (unit && unit->side() == on_side)
		return unit;
	else
		return nullptr;
}

int path_cost(const std::vector<map_location>& path, const unit& u)
{
	if(path.size() < 2)
		return 0;

	const team& u_team = resources::gameboard->get_team(u.side());
	const map_location& dest = path.back();
	if ( (resources::gameboard->map().is_village(dest) && !u_team.owns_village(dest))
	     || pathfind::enemy_zoc(u_team, dest, u_team) )
		return u.total_movement();

	int result = 0;
	const gamemap& map = resources::gameboard->map();
	for(const map_location& loc : std::pair(path.begin()+1,path.end())) {
		result += u.movement_cost(map[loc]);
	}
	return result;
}

temporary_unit_hider::temporary_unit_hider(unit& u)
		: unit_(&u)
	{unit_->set_hidden(true);}
temporary_unit_hider::~temporary_unit_hider()
{
	try {
		unit_->set_hidden(false);
	} catch (...) {}
}

void ghost_owner_unit(unit* unit)
{
	unit->anim_comp().set_disabled_ghosted(false);
	display::get_singleton()->invalidate(unit->get_location());
}

void unghost_owner_unit(unit* unit)
{
	unit->anim_comp().set_standing(true);
	display::get_singleton()->invalidate(unit->get_location());
}

bool has_actions()
{
	for (team& t : resources::gameboard->teams()) {
		if (!t.get_side_actions()->empty())
			return true;
	}

	return false;
}

bool team_has_visible_plan(team &t)
{
	return !t.get_side_actions()->hidden();
}

void for_each_action(const std::function<void(action*)>& function, const team_filter& team_filter)
{
	bool end = false;
	for(std::size_t turn=0; !end; ++turn) {
		end = true;
		for(team &side : resources::gameboard->teams()) {
			side_actions &actions = *side.get_side_actions();
			if(turn < actions.num_turns() && team_filter(side)) {
				for(auto iter = actions.turn_begin(turn); iter != actions.turn_end(turn); ++iter) {
					function(iter->get());
				}
				end = false;
			}
		}
	}
}

action_ptr find_action_at(map_location hex, const team_filter& team_filter)
{
	action_ptr result;
	std::size_t result_turn = std::numeric_limits<std::size_t>::max();

	for(team &side : resources::gameboard->teams()) {
		side_actions &actions = *side.get_side_actions();
		if(team_filter(side)) {
			side_actions::iterator chall = actions.find_first_action_at(hex);
			if(chall == actions.end()) {
				continue;
			}

			std::size_t chall_turn = actions.get_turn(chall);
			if(chall_turn < result_turn) {
				result = *chall;
				result_turn = chall_turn;
			}
		}
	}

	return result;
}

std::deque<action_ptr> find_actions_of(const unit& target)
{
	return resources::gameboard->get_team(target.side()).get_side_actions()->actions_of(target);
}

} //end namespace wb
