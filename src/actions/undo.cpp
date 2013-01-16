/* $Id$ */
/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


namespace actions {


/**
 * Converts a string (as read from a config) to an ACTION_TYPE.
 */
undo_list::undo_action::ACTION_TYPE undo_list::undo_action::parse_type(const std::string & str)
{
	if ( str == "move" )
		return MOVE;

	if ( str == "recruit" )
		return RECRUIT;

	if ( str == "recall" )
		return RECALL;

	if ( str == "dismiss" )
		return DISMISS;

	if ( str == "auto_shroud" )
		return AUTO_SHROUD;

	if ( str == "update_shroud" )
		return UPDATE_SHROUD;

	// Unrecognized type.
	ERR_NG << "Unrecognized undo action type: " << str << ".\n";
	return NONE;
}


/**
 * Constructs an undo_action from the provided config.
 * @param  tag  is the tag of this config, which is used for error reporting.
 *              It should be enclosed in square brackets.
 */
undo_list::undo_action::undo_action(const config & cfg, const std::string & tag) :
	route(),
	starting_moves( cfg["starting_moves"] ),
	original_village_owner( cfg["village_owner"] ),
	recall_from( cfg.child_or_empty("leader"), NULL ),
	type( parse_type(cfg["type"]) ),
	affected_unit(),
	countdown_time_bonus( cfg["time_bonus"] ),
	starting_dir( map_location::parse_direction(cfg["starting_direction"]) ),
	active( cfg["active"].to_bool() )
{
	// Only some action types have a unit.
	if ( needs_unit() )
		affected_unit = boost::make_shared<unit>(cfg.child("unit", tag));

	// Now read the route.
	if ( type == MOVE )
		read_locations(cfg, route);
	else if ( type == RECALL  ||  type == RECRUIT )
		// This guarantees an element in the route, even if the config is malformed.
		route.push_back(map_location(cfg, NULL));
}


/**
 * Write an undo_action into the provided config.
 */
void undo_list::undo_action::write(config & cfg) const
{
	if ( is_dismiss() )
	{
		cfg["type"] = "dismiss";
		affected_unit->write(cfg.add_child("unit"));
	}

	else if ( is_recall() )
	{
		cfg["type"] = "recall";
		route.front().write(cfg);
		recall_from.write(cfg.add_child("leader"));
		affected_unit->write(cfg.add_child("unit"));
	}

	else if ( is_recruit() )
	{
		cfg["type"] = "recruit";
		route.front().write(cfg);
		recall_from.write(cfg.add_child("leader"));
		affected_unit->write(cfg.add_child("unit"));
	}

	else if ( is_move() )
	{
		cfg["type"] = "move";
		cfg["starting_direction"] = map_location::write_direction(starting_dir);
		cfg["starting_moves"] = starting_moves;
		cfg["time_bonus"] = countdown_time_bonus;
		cfg["village_owner"] = original_village_owner;
		write_locations(route, cfg);
		affected_unit->write(cfg.add_child("unit"));
	}

	else if ( is_auto_shroud() )
	{
		cfg["type"] = "auto_shroud";
		cfg["active"] = active;
	}

	else if ( is_update_shroud() )
	{
		cfg["type"] = "update_shroud";
	}
}



/**
 * Adds an auto-shroud toggle to the undo stack.
 */
void undo_list::add_auto_shroud(bool turned_on)
{
	/// @todo: Consecutive shroud actions can be collapsed into one.

	// Do not call add(), as this should not clear the redo stack.
	undos_.push_back(undo_action(undo_action::AUTO_SHROUD, turned_on));
}


/**
 * Adds a shroud update to the undo stack.
 * This is called from within commit_vision(), so there should be no need
 * for this to be publically visible.
 */
void undo_list::add_update_shroud()
{
	/// @todo: Consecutive shroud actions can be collapsed into one.

	// Do not call add(), as this should not clear the redo stack.
	undos_.push_back(undo_action(undo_action::UPDATE_SHROUD));
}


/**
 * Clears the stack of undoable (and redoable) actions.
 * (Also handles updating fog/shroud if needed.)
 * Call this if an action alters the game state, but add that action to the
 * stack before calling this (if the action is a kind that can be undone).
 * This may fire events and change the game state.
 */
void undo_list::clear()
{
	// The fact that this function was called indicates that something was done.
	// (Some actions, such as attacks, are never put on the stack.)
	committed_actions_ = true;

	// We can save some overhead by not calling apply_shroud_changes() for an
	// empty stack.
	if ( !undos_.empty() ) {
		apply_shroud_changes();
		undos_.clear();
	}
	// No special handling for redos, so just clear that stack.
	redos_.clear();
}


/**
 * Updates fog/shroud based on the undo stack, then updates stack as needed.
 * Call this when "updating shroud now".
 * This may fire events and change the game state.
 * @param[in]  is_replay  Set to true when this is called during a replay.
 */
void undo_list::commit_vision(bool is_replay)
{
	if ( !is_replay )
		// Record this.
		recorder.update_shroud();

	// Update fog/shroud.
	size_t erase_to = apply_shroud_changes();

	if ( erase_to != 0 ) {
		// The actions that led to information being revealed can no longer
		// be undone.
		undos_.erase(undos_.begin(), undos_.begin() + erase_to);
		committed_actions_ = true;
	}

	// Record that vision was updated.
	add_update_shroud();
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
 * Read the undo_list from the provided config.
 * Currently, this is only used when the undo_list is empty, but in theory
 * it could be used to append the config to the current data.
 */
void undo_list::read(const config & cfg)
{
	// Merge header data.
	side_ = cfg["side"].to_int(side_);
	committed_actions_ = committed_actions_ || cfg["committed"].to_bool();

	// Build the undo stack.
	BOOST_FOREACH( const config & child, cfg.child_range("undo") ) {
		undo_action action(child, "[undo]");
		if ( action.valid() )
			undos_.push_back(action);
	}

	// Build the redo stack.
	BOOST_FOREACH( const config & child, cfg.child_range("redo") ) {
		undo_action action(child, "[redo]");
		if ( action.valid() )
			redos_.push_back(action);
	}
}


/**
 * Write the undo_list into the provided config.
 */
void undo_list::write(config & cfg) const
{
	cfg["side"] = side_;
	cfg["committed"] = committed_actions_;

	for ( action_list::const_iterator it = undos_.begin(); it != undos_.end(); ++it )
		it->write(cfg.add_child("undo"));

	for ( action_list::const_iterator it = redos_.begin(); it != redos_.end(); ++it )
		it->write(cfg.add_child("redo"));
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
		current_team.recall_list().push_back(*action.affected_unit);
	}
	else if ( action.is_recall() ) {

		if(!current_team.persistent()) {
			ERR_NG << "trying to undo a recall for side " << side_
				<< ", which has no recall list!\n";
			return;
		}
		// Undo a recall action
		const map_location & recall_loc = action.route.front();
		unit_map::iterator un_it = units.find(recall_loc);
		if ( un_it == units.end() ) {
			return;
		}

		const unit &un = *un_it;
		statistics::un_recall_unit(un);
		current_team.spend_gold(-current_team.recall_cost());

		current_team.recall_list().push_back(un);
		// invalidate before erasing allow us
		// to also do the overlapped hexes
		gui.invalidate(recall_loc);
		units.erase(recall_loc);
	}
	else if ( action.is_recruit() ) {
		// Undo a recruit action
		const map_location & recruit_loc = action.route.front();
		unit_map::iterator un_it = units.find(recruit_loc);
		if ( un_it == units.end() ) {
			return;
		}

		const unit &un = *un_it;
		statistics::un_recruit_unit(un);
		current_team.spend_gold(-un.type().cost());

		//MP_COUNTDOWN take away recruit bonus
		if(action.countdown_time_bonus)
		{
			current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);
		}

		// invalidate before erasing allow us
		// to also do the ovelerlapped hexes
		gui.invalidate(recruit_loc);
		units.erase(recruit_loc);
	}
	else if ( action.is_move() ) {
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

		// Record the unit's current state so it can be redone.
		action.starting_moves = u->movement_left();
		action.affected_unit->set_goto(u->get_goto());

		unit_display::move_unit(route, *u, true, action.starting_dir);

		units.move(u->get_location(), route.back());
		unit::clear_status_caches();

		u = units.find(route.back());
		u->set_goto(map_location());
		u->set_movement(starting_moves, true);
		u->set_standing();

		gui.invalidate_unit_after_move(route.front(), route.back());
	}
	else if ( action.is_auto_shroud() ) {
		// This does not count as an undoable action, so undo the next
		// action instead.
		recorder.undo();
		undo();
		// Now keep the auto-shroud toggle at the top of the undo stack.
		recorder.add_auto_shroud(action.active);
		add_auto_shroud(action.active);
		return;
	}
	else if ( action.is_update_shroud() ) {
		// This does not count as an undoable action, so undo the next
		// action instead.
		recorder.undo();
		undo();
		// Now keep the shroud update at the top of the undo stack.
		recorder.update_shroud();
		add_update_shroud();
		return;
	}
	else {
		// Invalid/unknown action type.
		return;
	}
	recorder.undo();
	redos_.push_back(action);
	resources::whiteboard->on_gamestate_change();

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
		recorder.add_disband(action.affected_unit->id());
		std::vector<unit>::iterator unit_it =
			find_if_matches_id(current_team.recall_list(), action.affected_unit->id());
		current_team.recall_list().erase(unit_it);
	}
	else if ( action.is_recall() ) {
		if(!current_team.persistent()) {
			ERR_NG << "trying to redo a recall for side " << side_
				<< ", which has no recall list!\n";
			return;
		}
		// Redo recall

		map_location loc = action.route.front();
		map_location from = action.recall_from;

		const std::string &msg = find_recall_location(side_, loc, from, *action.affected_unit);
		if(msg.empty()) {
			recall_unit(action.affected_unit->id(), current_team, loc, from, true, false);

			// Quick error check. (Abuse of [allow_undo]?)
			if ( loc != action.route.front() ) {
				ERR_NG << "When redoing a recall at " << action.route.front()
				       << ", the location was moved to " << loc << ".\n";
				// Not really fatal, I suppose. Just update the action so
				// undoing this works.
				action.route.front() = loc;
			}
		} else {
			gui::dialog(gui, "", msg,gui::OK_ONLY).show();
			return;
		}
	}
	else if ( action.is_recruit() ) {
		// Redo recruit action
		map_location loc = action.route.front();
		map_location from = action.recall_from;
		const std::string name = action.affected_unit->type_id();

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
		const std::string &msg = find_recruit_location(side_, loc, from, action.affected_unit->type_id());
		if(msg.empty()) {
			const unit new_unit = *action.affected_unit;
			//unit new_unit(action.affected_unit->type(),team_num_,true);
			place_recruit(new_unit, loc, from, new_unit.type().cost(), false, true);
			statistics::recruit_unit(new_unit);
			gui.invalidate(loc);

			//MP_COUNTDOWN: restore recruitment bonus
			current_team.set_action_bonus_count(1 + current_team.action_bonus_count());

			recorder.add_checksum_check(loc);
			// Quick error check. (Abuse of [allow_undo]?)
			if ( loc != action.route.front() ) {
				ERR_NG << "When redoing a recruit at " << action.route.front()
				       << ", the location was moved to " << loc << ".\n";
				// Not really fatal, I suppose. Just update the action so
				// undoing this works.
				action.route.front() = loc;
			}
		} else {
			recorder.undo();
			gui::dialog(gui, "", msg,gui::OK_ONLY).show();
			return;
		}
	}
	else if ( action.is_move() ) {
		// Redo movement action
		const int starting_moves = action.starting_moves;
		const std::vector<map_location> & route = action.route;
		unit_map::iterator u = units.find(route.front());
		if ( u == units.end() ) {
			ERR_NG << "Illegal movement 'redo'.\n";
			assert(false);
			return;
		}

		action.starting_moves = u->movement_left();

		unit_display::move_unit(route, *u);

		units.move(u->get_location(), route.back());
		u = units.find(route.back());

		unit::clear_status_caches();
		u->set_goto(action.affected_unit->get_goto());
		u->set_movement(starting_moves, true);
		u->set_standing();

		if ( resources::game_map->is_village(route.back()) ) {
			get_village(route.back(), u->side());
			//MP_COUNTDOWN restore capture bonus
			if(action.countdown_time_bonus)
			{
				current_team.set_action_bonus_count(1 + current_team.action_bonus_count());
			}
		}

		gui.invalidate_unit_after_move(route.front(), route.back());

		recorder.add_movement(route);
	}
	// Auto-shroud toggles should never be in the redo stack.
	// Updating the shroud should never be in the redo stack.
	else {
		// Invalid/unknown action type.
		return;
	}
	undos_.push_back(action);
	resources::whiteboard->on_gamestate_change();

	gui.invalidate_unit();
	gui.invalidate_game_status();
	gui.redraw_minimap();
	gui.draw();
}


/**
 * Applies the pending fog/shroud changes from the undo stack.
 * Does nothing if the the current side does not use fog or shroud.
 * @returns  an index (into undos_) pointing to the first undoable action
 *           that can be kept (or undos_.size() if none can be kept).
 */
size_t undo_list::apply_shroud_changes() const
{
	game_display &disp = *resources::screen;
	team &tm = (*resources::teams)[side_ - 1];
	// No need to do clearing if fog/shroud has been kept up-to-date.
	if ( tm.auto_shroud_updates()  ||  !tm.fog_or_shroud() )
		return 0;


	shroud_clearer clearer;
	bool cleared_shroud = false;  // for optimization
	size_t erase_to = 0;
	size_t list_size = undos_.size();


	// Loop through the list of undo_actions.
	for( size_t i = 0; i != list_size; ++i ) {
		const undo_action & action = undos_[i];
		// Only actions with a unit are relevant.
		if ( !action.affected_unit )
			continue;
		LOG_NG << "Turning an undo...\n";

		// Clear the hexes this unit can see from each hex occupied during
		// the action.
		std::vector<map_location>::const_iterator step;
		for (step = action.route.begin(); step != action.route.end(); ++step) {
			// Clear the shroud, collecting new sighted events.
			if ( clearer.clear_unit(*step, *action.affected_unit, tm) ) {
				cleared_shroud = true;
				erase_to = i + 1;
			}
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
		// Fix up the display in case WML changed stuff.
		clear_shroud(side_);
		disp.invalidate_unit();
		disp.draw();
		// The entire stack needs to be cleared in order to preserve replays.
		// (The events that fired might depend on current unit positions.)
		erase_to = list_size;
	}

	return erase_to;
}


}//namespace actions

