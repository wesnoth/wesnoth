/*
 Copyright (C) 2010 - 2017 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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

#include <algorithm>
#include <iterator>
#include <limits>

#include "whiteboard/utility.hpp"

#include "whiteboard/manager.hpp"
#include "whiteboard/side_actions.hpp"

#include "actions/create.hpp"
#include "game_display.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"
#include "utils/iterable_pair.hpp"

namespace wb {

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
			resources::gameboard->teams()[resources::screen->viewing_team()].get_side_actions();
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
	for (unit_map::const_iterator unit = resources::gameboard->units().begin(); unit != resources::gameboard->units().end(); unit++)
	{
		if (unit->can_recruit() && unit->id() != leader.id())
		{
			if (dynamic_cast<game_state&>(*resources::filter_con).can_recruit_on(*unit, leader.get_location()))
				return unit.get_shared_ptr();
		}
	}
	return unit_const_ptr();
}

unit* find_recruiter(size_t team_index, map_location const& hex)
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

unit* future_visible_unit(map_location hex, int viewer_side)
{
	future_map planned_unit_map;
	if(!resources::whiteboard->has_planned_unit_map())
	{
		ERR_WB << "future_visible_unit cannot find unit, future unit map failed to build." << std::endl;
		return nullptr;
	}
	//use global method get_visible_unit
	return resources::gameboard->get_visible_unit(hex, resources::gameboard->get_team(viewer_side), false);
}

unit* future_visible_unit(int on_side, map_location hex, int viewer_side)
{
	unit* unit = future_visible_unit(hex, viewer_side);
	if (unit && unit->side() == on_side)
		return unit;
	else
		return nullptr;
}

int path_cost(std::vector<map_location> const& path, unit const& u)
{
	if(path.size() < 2)
		return 0;

	team const& u_team = resources::gameboard->get_team(u.side());
	map_location const& dest = path.back();
	if ( (resources::gameboard->map().is_village(dest) && !u_team.owns_village(dest))
	     || pathfind::enemy_zoc(u_team, dest, u_team) )
		return u.total_movement();

	int result = 0;
	gamemap const& map = resources::gameboard->map();
	for(map_location const& loc : std::make_pair(path.begin()+1,path.end())) {
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
	resources::screen->invalidate(unit->get_location());
}

void unghost_owner_unit(unit* unit)
{
	unit->anim_comp().set_standing(true);
	resources::screen->invalidate(unit->get_location());
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

void for_each_action(std::function<void(action*)> function, team_filter team_filter)
{
	bool end = false;
	for(size_t turn=0; !end; ++turn) {
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

action_ptr find_action_at(map_location hex, team_filter team_filter)
{
	action_ptr result;
	size_t result_turn = std::numeric_limits<size_t>::max();

	for(team &side : resources::gameboard->teams()) {
		side_actions &actions = *side.get_side_actions();
		if(team_filter(side)) {
			side_actions::iterator chall = actions.find_first_action_at(hex);
			if(chall == actions.end()) {
				continue;
			}

			size_t chall_turn = actions.get_turn(chall);
			if(chall_turn < result_turn) {
				result = *chall;
				result_turn = chall_turn;
			}
		}
	}

	return result;
}

std::deque<action_ptr> find_actions_of(unit const &target)
{
	return resources::gameboard->get_team(target.side()).get_side_actions()->actions_of(target);
}

} //end namespace wb

