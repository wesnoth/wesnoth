/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
 * Undoing, redoing.
 */

#include "undo.hpp"

#include "create.hpp"
#include "move.hpp"
#include "vision.hpp"

#include "../game_display.hpp"
#include "../game_events.hpp"
#include "../log.hpp"
#include "../play_controller.hpp"
#include "../replay.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../unit_display.hpp"
#include "../unit_map.hpp"
#include "../whiteboard/manager.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


/**
 * Clears the stack of undoable actions.
 * (Also handles updating fog/shroud if needed.)
 * Call this if an action alters the game state, but add that action to the
 * stack before calling this.
 * This may fire events and change the game state.
 */
void undo_list::clear()
{
	// The fact that this function was called indicates that something was done.
	// (Some actions, such as attacks, are never put on the stack.)
	committed_actions_ = true;

	// No need to do anything if the stack is already clear.
	if ( undos_.empty() )
		return;

	// Update fog/shroud.
	apply_shroud_changes();

	// Clear the stack.
	undos_.clear();
}


/**
 * Performs some initializations and error checks when starting a new side-turn.
 * @param[in]  side  The side whose turn is about to start.
 */
void undo_list::new_side_turn(int side)
{
	// Error checks.
	if ( !undos_.empty() ) {
		ERR_NG << "Undo stack not empty in new_side_turn().\n";
		// At worst, someone missed some sighted events, so try to recover.
		undos_.clear();
		redos_.clear();
	}
	else if ( !redos_.empty() ) {
		ERR_NG << "Redo stack not empty in new_side_turn().\n";
		// Sloppy tracking somewhere, but not critically so.
		redos_.clear();
	}

	// Reset the side.
	side_ = side;
	committed_actions_ = false;
}


/**
 * Undoes the top action on the undo stack.
 */
void undo_list::undo()
{
	if ( undos_.empty() )
		return;

	const events::command_disabler disable_commands;
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side_ - 1];

	// Get the action to undo. (This will be placed on the redo stack, but
	// only if the undo is successful.)
	undo_action action = undos_.back();
	undos_.pop_back();

	if (action.is_dismiss()) {
		//undo a dismissal

		if(!current_team.persistent()) {
			ERR_NG << "trying to undo a dismissal for side " << side_
				<< ", which has no recall list!\n";
			return;
		}
		current_team.recall_list().push_back(action.affected_unit);
		resources::whiteboard->on_gamestate_change();
	} else if(action.is_recall()) {

		if(!current_team.persistent()) {
			ERR_NG << "trying to undo a recall for side " << side_
				<< ", which has no recall list!\n";
			return;
		}
		// Undo a recall action
		if ( units.count(action.recall_loc) == 0 ) {
			return;
		}

		const unit &un = *units.find(action.recall_loc);
		statistics::un_recall_unit(un);
		current_team.spend_gold(-current_team.recall_cost());

		current_team.recall_list().push_back(un);
		// invalidate before erasing allow us
		// to also do the overlapped hexes
		gui.invalidate(action.recall_loc);
		units.erase(action.recall_loc);
		resources::whiteboard->on_gamestate_change();
	} else if(action.is_recruit()) {
		// Undo a recruit action
		if( units.count(action.recall_loc) == 0 ) {
			return;
		}

		const unit &un = *units.find(action.recall_loc);
		statistics::un_recruit_unit(un);
		assert(un.type());
		current_team.spend_gold(-un.type()->cost());

		//MP_COUNTDOWN take away recruit bonus
		if(action.countdown_time_bonus)
		{
			current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);
		}

		// invalidate before erasing allow us
		// to also do the ovelerlapped hexes
		gui.invalidate(action.recall_loc);
		units.erase(action.recall_loc);
		resources::whiteboard->on_gamestate_change();
	} else {
		// Undo a move action
		const int starting_moves = action.starting_moves;
		std::vector<map_location> route = action.route;
		std::reverse(route.begin(),route.end());
		unit_map::iterator u = units.find(route.front());
		const unit_map::iterator u_end = units.find(route.back());
		if ( u == units.end()  ||  u_end != units.end() ) {
			//this can actually happen if the scenario designer has abused the [allow_undo] command
			ERR_NG << "Illegal 'undo' found. Possible abuse of [allow_undo]?\n";
			return;
		}

		if ( resources::game_map->is_village(route.front()) ) {
			get_village(route.front(), action.original_village_owner + 1);
			//MP_COUNTDOWN take away capture bonus
			if(action.countdown_time_bonus)
			{
				current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);
			}
		}

		action.starting_moves = u->movement_left();

		undo_action action_copy(action);

		unit_display::move_unit(route, *u, true, action_copy.starting_dir);

		units.move(u->get_location(), route.back());
		unit::clear_status_caches();

		u = units.find(route.back());
		u->set_goto(map_location());
		u->set_movement(starting_moves, true);
		u->set_standing();

		gui.invalidate_unit_after_move(route.front(), route.back());
		resources::whiteboard->on_gamestate_change();
	}
	recorder.undo();
	redos_.push_back(action);

	gui.invalidate_unit();
	gui.invalidate_game_status();
	gui.redraw_minimap();
	gui.draw();
}


/**
 * Redoes the top action on the redo stack.
 */
void undo_list::redo()
{
	if ( redos_.empty() )
		return;

	const events::command_disabler disable_commands;
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side_ - 1];

	// Get the action to redo. (This will be placed on the undo stack, but
	// only if the redo is successful.)
	undo_action action = redos_.back();
	redos_.pop_back();

	if (action.is_dismiss()) {
		if(!current_team.persistent()) {
			ERR_NG << "trying to redo a dismiss for side " << side_
				<< ", which has no recall list!\n";
			return;
		}
		//redo a dismissal
		recorder.add_disband(action.affected_unit.id());
		std::vector<unit>::iterator unit_it =
			find_if_matches_id(current_team.recall_list(), action.affected_unit.id());
		current_team.recall_list().erase(unit_it);
		resources::whiteboard->on_gamestate_change();
	} else if(action.is_recall()) {
		if(!current_team.persistent()) {
			ERR_NG << "trying to redo a recall for side " << side_
				<< ", which has no recall list!\n";
			return;
		}
		// Redo recall

		recorder.add_recall(action.affected_unit.id(), action.recall_loc, action.recall_from);
		map_location loc = action.recall_loc;
		map_location from = map_location::null_location;
		const events::command_disabler disable_commands;
		const std::string &msg = find_recall_location(side_, loc, from, action.affected_unit);
		if(msg.empty()) {
			unit un = action.affected_unit;
			//remove the unit from the recall list
			std::vector<unit>::iterator unit_it =
				find_if_matches_id(current_team.recall_list(), action.affected_unit.id());
			assert(unit_it != current_team.recall_list().end());
			current_team.recall_list().erase(unit_it);

			place_recruit(un, loc, from, current_team.recall_cost(), true, true);
			statistics::recall_unit(un);
			gui.invalidate(loc);
			recorder.add_checksum_check(loc);
		} else {
			recorder.undo();
			gui::dialog(gui, "", msg,gui::OK_ONLY).show();
			return;
		}
		resources::whiteboard->on_gamestate_change();
	} else if(action.is_recruit()) {
		// Redo recruit action
		map_location loc = action.recall_loc;
		map_location from = action.recall_from;
		const std::string name = action.affected_unit.type_id();

		//search for the unit to be recruited in recruits
		int recruit_num = 0;
		const std::set<std::string>& recruits = current_team.recruits();
		for(std::set<std::string>::const_iterator r = recruits.begin(); ; ++r) {
			if (r == recruits.end()) {
				ERR_NG << "trying to redo a recruit for side " << side_
					<< ", which does not recruit type \"" << name << "\"\n";
				assert(false);
				return;
			}
			if (name == *r) {
				break;
			}
			++recruit_num;
		}
		current_team.last_recruit(name);
		recorder.add_recruit(recruit_num,loc,from);
		const events::command_disabler disable_commands;
		const std::string &msg = find_recruit_location(side_, loc, from, action.affected_unit.type_id());
		if(msg.empty()) {
			const unit new_unit = action.affected_unit;
			//unit new_unit(action.affected_unit.type(),team_num_,true);
			place_recruit(new_unit, loc, from, new_unit.type()->cost(), false, true);
			statistics::recruit_unit(new_unit);
			gui.invalidate(loc);

			//MP_COUNTDOWN: restore recruitment bonus
			current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

			recorder.add_checksum_check(loc);
		} else {
			recorder.undo();
			gui::dialog(gui, "", msg,gui::OK_ONLY).show();
			return;
		}
		resources::whiteboard->on_gamestate_change();
	} else {
		// Redo movement action
		const int starting_moves = action.starting_moves;
		std::vector<map_location> route = action.route;
		unit_map::iterator u = units.find(route.front());
		if ( u == units.end() ) {
			ERR_NG << "Illegal movement 'redo'.\n";
			assert(false);
			return;
		}

		action.starting_moves = u->movement_left();

		undo_action action_copy(action);

		unit_display::move_unit(route, *u);

		units.move(u->get_location(), route.back());
		u = units.find(route.back());

		unit::clear_status_caches();
		u->set_goto(action_copy.affected_unit.get_goto());
		u->set_movement(starting_moves, true);
		u->set_standing();

		if ( resources::game_map->is_village(route.back()) ) {
			get_village(route.back(), u->side());
			//MP_COUNTDOWN restore capture bonus
			if(action_copy.countdown_time_bonus)
			{
				current_team.set_action_bonus_count(1 + current_team.action_bonus_count());
			}
		}

		gui.invalidate_unit_after_move(route.front(), route.back());
		resources::whiteboard->on_gamestate_change();

		recorder.add_movement(action_copy.route);
	}
	resources::undo_stack->push_back(action);

	gui.invalidate_unit();
	gui.invalidate_game_status();
	gui.redraw_minimap();
	gui.draw();
}


/**
 * Applies the pending fog/shroud changes from the undo stack.
 * Does nothing if the the current side does not use fog or shroud.
 */
void undo_list::apply_shroud_changes() const
{
	team &tm = (*resources::teams)[side_ - 1];
	// No need to do clearing if fog/shroud has been kept up-to-date.
	if ( tm.auto_shroud_updates()  ||  !tm.fog_or_shroud() )
		return;

	game_display &disp = *resources::screen;
	unit_map &units = *resources::units;

	/*
	   This function works thusly:
	   1. run through the list of undo_actions
	   2. for each one, play back the unit's move
	   3. for each location along the route, clear any "shrouded" hexes that the unit can see
	      and record sighted events
	   4. render shroud/fog cleared.
	   5. pump all events
	   6. call clear_shroud to update the fog of war for each unit
	   7. fix up associated display stuff (done in a similar way to turn_info::undo())
	*/

	actions::shroud_clearer clearer;
	bool cleared_shroud = false;  // for further optimization

	for( action_list::const_iterator un = undos_.begin(); un != undos_.end(); ++un ) {
		LOG_NG << "Turning an undo...\n";
		//NOTE: for the moment shroud cleared during recall seems never delayed
		//Shroud update during recall can be delayed, during recruit as well
		//if we have a non-random recruit (e.g. undead)
		//if(un->is_recall() || un->is_recruit()) continue;

		// Make a temporary unit move in map and hide the original
		const unit_map::const_unit_iterator unit_itor = units.find(un->affected_unit.underlying_id());
		// check if the unit is still existing (maybe killed by an event)
		// FIXME: A wml-killed unit will not update the shroud explored before its death
		if(unit_itor == units.end())
			continue;

		std::vector<map_location> route(un->route.begin(), un->route.end());
		if ( un->recall_loc.valid() )
			route.push_back(un->recall_loc);
		std::vector<map_location>::const_iterator step;
		for(step = route.begin(); step != route.end(); ++step) {
			// Clear the shroud, collecting new sighted events.
			cleared_shroud |= clearer.clear_unit(*step, *unit_itor, tm);
		}
	}

	// Optimization: if nothing was cleared, then there is nothing to redraw.
	if ( cleared_shroud ) {
		// Update the display before pumping events.
		clearer.invalidate_after_clear();
		disp.draw();
	}

	// Fire sighted events
	if ( clearer.fire_events() ) {
		// Updates in case WML changed stuff.
		clear_shroud(side_);
		disp.invalidate_unit();
		disp.draw();
	}
}

