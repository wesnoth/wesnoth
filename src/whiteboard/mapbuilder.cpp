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

#include "whiteboard/mapbuilder.hpp"

#include "whiteboard/action.hpp"
#include "whiteboard/move.hpp"
#include "whiteboard/side_actions.hpp"
#include "whiteboard/utility.hpp"

#include "game_board.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "utils/ranges.hpp"

namespace wb
{

mapbuilder::mapbuilder(unit_map& unit_map)
	: unit_map_(unit_map)
	, applied_actions_()
	, applied_actions_this_turn_()
	, resetters_()
	, removers_()
	, acted_this_turn_()
	, has_invalid_actions_()
	, invalid_actions_()
{
}

mapbuilder::~mapbuilder()
{
	try {
	restore_normal_map();
	//Remember that the member variable resetters_ is destructed here
	} catch (...) {}
}

void mapbuilder::pre_build()
{
	for (team& t : resources::gameboard->teams()) {
		//Reset spent gold to zero, it'll be recalculated during the map building
		t.get_side_actions()->reset_gold_spent();
	}

	int current_side = resources::controller->current_side();
	for (unit& u : resources::gameboard->units()) {
		bool on_current_side = (u.side() == current_side);

		//Remove any unit the current side cannot see to avoid their detection by planning
		//Units will be restored to the unit map by destruction of removers_

		if(!on_current_side && !u.is_visible_to_team(display::get_singleton()->viewing_team(), false)) {
			removers_.emplace_back(new temporary_unit_remover(resources::gameboard->units(), u.get_location()));

			//Don't do anything else to the removed unit!
			continue;
		}

		//Reset movement points, to be restored by destruction of resetters_

		//restore movement points only to units not on the current side
		resetters_.emplace_back(new unit_movement_resetter(u,!on_current_side));
		//make sure current side's units are not reset to full moves on first turn
		if(on_current_side) {
			acted_this_turn_.insert(&u);
		}
	}
}

void mapbuilder::build_map()
{
	pre_build();
	if(!wb::has_actions()) {
		return;
	}

	bool stop = false;
	for(std::size_t turn=0; !stop; ++turn) {
		stop = true;
		for (team &side : resources::gameboard->teams()) {
			side_actions &actions = *side.get_side_actions();
			if(turn < actions.num_turns() && team_has_visible_plan(side)) {
				stop = false;
				side_actions::iterator it = actions.turn_begin(turn), next = it, end = actions.turn_end(turn);
				while(it != end) {
					std::advance(next, 1);
					process(actions, it, side.is_local());
					it = next;
				}

				post_visit_team(turn);
			}
		}
	}
}

void mapbuilder::process(side_actions &sa, side_actions::iterator action_it, bool is_local_side)
{
	action_ptr action = *action_it;
	bool acted=false;
	unit_ptr unit = action->get_unit();
	if(!unit) {
		return;
	}


	if(acted_this_turn_.find(unit.get()) == acted_this_turn_.end() && !action->places_new_unit()) {
		//reset MP
		unit->set_movement(unit->total_movement());
		acted=true;
	}

	// Validity check
	action::error erval = action->check_validity();
	action->redraw();

	if(erval != action::OK) {
		// We do not delete obstructed moves, nor invalid actions caused by obstructed moves.
		if(has_invalid_actions_.find(unit.get()) == has_invalid_actions_.end()) {
			if(!is_local_side || erval == action::TOO_FAR || (erval == action::LOCATION_OCCUPIED && std::dynamic_pointer_cast<move>(action))) {
				has_invalid_actions_.insert(unit.get());
				invalid_actions_.push_back(action_it);
			} else {
				sa.remove_action(action_it, false);
				return;
			}
		} else {
			invalid_actions_.push_back(action_it);
		}
		return;
	}

	// We do not keep invalid actions replaced by a valid one.
	if(is_local_side) {
		std::set<class unit const*>::iterator invalid_it = has_invalid_actions_.find(unit.get());
		if(invalid_it != has_invalid_actions_.end()) {
			for(std::list<side_actions::iterator>::iterator it = invalid_actions_.begin(); it != invalid_actions_.end();) {
				if((**it)->get_unit().get() == unit.get()) {
					sa.remove_action(*it, false);
					it = invalid_actions_.erase(it);
				} else {
					++it;
				}
			}
			has_invalid_actions_.erase(invalid_it);
		}
	}

	if(acted || action->places_new_unit()) {
		acted_this_turn_.insert(unit.get());
	}

	action->apply_temp_modifier(unit_map_);
	applied_actions_.push_back(action);
	applied_actions_this_turn_.push_back(action);
}

void mapbuilder::post_visit_team(std::size_t turn)
{
	std::set<unit const*> seen;

	// Go backwards through the actions of this turn to identify
	// which ones are moves that end a turn.
	for(action_ptr action : applied_actions_this_turn_ | utils::views::reverse) {
		move_ptr move = std::dynamic_pointer_cast<class move>(action);
		if(move) {
			move->set_turn_number(0);
			if(move->get_route().steps.size() > 1 && seen.count(move->get_unit().get()) == 0) {
				seen.insert(move->get_unit().get());
				move->set_turn_number(turn + 1);
			}
		}
	}

	// Clear list of planned actions applied this turn
	applied_actions_this_turn_.clear();
	// Clear the list of units of this team that have acted this turn
	acted_this_turn_.clear();
}

void mapbuilder::restore_normal_map()
{
	//applied_actions_ contain only the actions that we applied to the unit map
	for(action_ptr act : applied_actions_ | utils::views::reverse) {
		act->remove_temp_modifier(unit_map_);
	}
}

} // end namespace wb
