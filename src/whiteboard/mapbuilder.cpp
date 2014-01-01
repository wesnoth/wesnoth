/*
 Copyright (C) 2010 - 2014 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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
#include "move.hpp"
#include "side_actions.hpp"
#include "utility.hpp"

#include "play_controller.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

#include <boost/foreach.hpp>

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
	restore_normal_map();
	//Remember that the member variable resetters_ is destructed here
}

void mapbuilder::pre_build()
{
	BOOST_FOREACH(team& t, *resources::teams) {
		//Reset spent gold to zero, it'll be recalculated during the map building
		t.get_side_actions()->reset_gold_spent();
	}

	int current_side = resources::controller->current_side();
	BOOST_FOREACH(unit& u, *resources::units) {
		bool on_current_side = (u.side() == current_side);

		//Remove any unit the current side cannot see to avoid their detection by planning
		//Units will be restored to the unit map by destruction of removers_

		if(!on_current_side && !u.is_visible_to_team((*resources::teams)[viewer_team()], false)) {
			removers_.push_back(new temporary_unit_remover(*resources::units, u.get_location()));

			//Don't do anything else to the removed unit!
			continue;
		}

		//Reset movement points, to be restored by destruction of resetters_

		//restore movement points only to units not on the current side
		resetters_.push_back(new unit_movement_resetter(u,!on_current_side));
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

	bool end = false;
	for(size_t turn=0; !end; ++turn) {
		end = true;
		BOOST_FOREACH(team &side, *resources::teams) {
			side_actions &actions = *side.get_side_actions();
			if(turn < actions.num_turns() && team_has_visible_plan(side)) {
				end = false;
				side_actions::iterator it = actions.turn_begin(turn), next = it, end = actions.turn_end(turn);
				while(it != end) {
					std::advance(next, 1);
					process(actions, it);
					it = next;
				}

				post_visit_team(turn);
			}
		}
	}
}

void mapbuilder::process(side_actions &sa, side_actions::iterator action_it)
{
	action_ptr action = *action_it;
	bool acted=false;
	unit* unit = action->get_unit();
	if(!unit) {
		return;
	}

	if(acted_this_turn_.find(unit) == acted_this_turn_.end()) {
		//reset MP
		unit->set_movement(unit->total_movement());
		acted=true;
	}

	// Validity check
	action::error erval = action->check_validity();
	action->redraw();

	if(erval != action::OK) {
		// We do not delete obstructed moves, nor invalid actions caused by obstructed moves.
		if(has_invalid_actions_.find(unit) == has_invalid_actions_.end()) {
			if(erval == action::TOO_FAR || (erval == action::LOCATION_OCCUPIED && boost::dynamic_pointer_cast<move>(action))) {
				has_invalid_actions_.insert(unit);
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
	std::set<class unit const*>::iterator invalid_it = has_invalid_actions_.find(unit);
	if(invalid_it != has_invalid_actions_.end()) {
		for(std::list<side_actions::iterator>::iterator it = invalid_actions_.begin(); it != invalid_actions_.end();) {
			if((**it)->get_unit() == unit) {
				sa.remove_action(*it, false);
				it = invalid_actions_.erase(it);
			} else {
				++it;
			}
		}
		has_invalid_actions_.erase(invalid_it);
	}

	if(acted) {
		acted_this_turn_.insert(unit);
	}

	action->apply_temp_modifier(unit_map_);
	applied_actions_.push_back(action);
	applied_actions_this_turn_.push_back(action);
}

void mapbuilder::post_visit_team(size_t turn)
{
	std::set<unit const*> seen;

	// Go backwards through the actions of this turn to identify
	// which ones are moves that end a turn.
	BOOST_REVERSE_FOREACH(action_ptr action, applied_actions_this_turn_) {
		move_ptr move = boost::dynamic_pointer_cast<class move>(action);
		if(move) {
			move->set_turn_number(0);
			if(move->get_route().steps.size() > 1 && seen.count(move->get_unit()) == 0) {
				seen.insert(move->get_unit());
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
	BOOST_REVERSE_FOREACH(action_ptr act, applied_actions_) {
		act->remove_temp_modifier(unit_map_);
	}
}

} // end namespace wb
