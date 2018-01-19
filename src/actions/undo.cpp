/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#include "actions/undo.hpp"

#include "game_board.hpp"               // for game_board
#include "game_display.hpp"          // for game_display
#include "log.hpp"                   // for LOG_STREAM, logger, etc
#include "map/map.hpp"                      // for gamemap
#include "map/location.hpp"  // for map_location, operator<<, etc
#include "mouse_handler_base.hpp"       // for command_disabler
#include "preferences/general.hpp"
#include "recall_list_manager.hpp"   // for recall_list_manager
#include "replay.hpp"                // for recorder, replay
#include "replay_helper.hpp"         // for replay_helper
#include "resources.hpp"             // for screen, teams, units, etc
#include "synced_context.hpp"        // for set_scontext_synced
#include "team.hpp"                  // for team
#include "units/unit.hpp"                  // for unit
#include "units/animation_component.hpp"
#include "units/id.hpp"
#include "units/map.hpp"              // for unit_map, etc
#include "units/ptr.hpp"      // for unit_const_ptr, unit_ptr
#include "units/types.hpp"               // for unit_type, unit_type_data, etc
#include "whiteboard/manager.hpp"    // for manager

#include "actions/create.hpp"                   // for find_recall_location, etc
#include "actions/move.hpp"                   // for get_village
#include "actions/vision.hpp"           // for clearer_info, etc
#include "actions/shroud_clearing_action.hpp"
#include "actions/undo_dismiss_action.hpp"
#include "actions/undo_move_action.hpp"
#include "actions/undo_recall_action.hpp"
#include "actions/undo_recruit_action.hpp"
#include "actions/undo_update_shroud_action.hpp"

#include <algorithm>                    // for reverse
#include <cassert>                      // for assert
#include <ostream>                      // for operator<<, basic_ostream, etc
#include <set>                          // for set

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


namespace actions {



/**
 * Creates an undo_action based on a config.
 * @return a pointer that must be deleted, or nullptr if the @a cfg could not be parsed.
 */
undo_action_base * undo_list::create_action(const config & cfg)
{
	const std::string str = cfg["type"];
	undo_action_base * res = nullptr;
	// The general division of labor in this function is that the various
	// constructors will parse the "unit" child config, while this function
	// parses everything else.

	if ( str == "move" ) {
		res = new undo::move_action(cfg, cfg.child_or_empty("unit"),
		                       cfg["starting_moves"],
		                       map_location::parse_direction(cfg["starting_direction"]));
	}

	else if ( str == "recruit" ) {
		// Validate the unit type.
		const config & child = cfg.child("unit");
		const unit_type * u_type = unit_types.find(child["type"]);

		if ( !u_type ) {
			// Bad data.
			ERR_NG << "Invalid recruit found in [undo] or [redo]; unit type '"
			       << child["type"] << "' was not found.\n";
			return nullptr;
		}
		res = new undo::recruit_action(cfg, *u_type, map_location(cfg.child_or_empty("leader"), nullptr));
	}

	else if ( str == "recall" )
		res =  new undo::recall_action(cfg, map_location(cfg.child_or_empty("leader"), nullptr));

	else if ( str == "dismiss" )
		res =  new undo::dismiss_action(cfg, cfg.child("unit"));

	else if ( str == "auto_shroud" )
		res =  new undo::auto_shroud_action(cfg["active"].to_bool());

	else if ( str == "update_shroud" )
		res =  new undo::update_shroud_action();
	else
	{
		// Unrecognized type.
		ERR_NG << "Unrecognized undo action type: " << str << "." << std::endl;
		return nullptr;
	}
	return res;
}


/**
 * Constructor.
 * The config is allowed to be invalid.
 */
undo_list::undo_list(const config & cfg) :
	undos_(), redos_(), side_(1), committed_actions_(false)
{
	if ( cfg )
		read(cfg);
}

/**
 * Destructor.
 */
undo_list::~undo_list()
{
	// Default destructor, but defined out-of-line to localize the templating.
	// (Might make compiles faster.)
}


/**
 * Adds an auto-shroud toggle to the undo stack.
 */
void undo_list::add_auto_shroud(bool turned_on)
{
	/// @todo: Consecutive shroud actions can be collapsed into one.

	// Do not call add(), as this should not clear the redo stack.
	add(new undo::auto_shroud_action(turned_on));
}

void undo_list::add_dummy()
{
	/// @todo: Consecutive shroud actions can be collapsed into one.

	// Do not call add(), as this should not clear the redo stack.
	add(new undo_dummy_action());
}

/**
 * Adds a dismissal to the undo stack.
 */
void undo_list::add_dismissal(const unit_const_ptr u)
{
	add(new undo::dismiss_action(u));
}

/**
 * Adds a move to the undo stack.
 */
void undo_list::add_move(const unit_const_ptr u,
                         const std::vector<map_location>::const_iterator & begin,
                         const std::vector<map_location>::const_iterator & end,
                         int start_moves, int timebonus, int village_owner,
                         const map_location::DIRECTION dir)
{
	add(new undo::move_action(u, begin, end, start_moves, timebonus, village_owner, dir));
}

/**
 * Adds a recall to the undo stack.
 */
void undo_list::add_recall(const unit_const_ptr u, const map_location& loc,
                           const map_location& from, int orig_village_owner, bool time_bonus)
{
	add(new undo::recall_action(u, loc, from, orig_village_owner, time_bonus));
}

/**
 * Adds a recruit to the undo stack.
 */
void undo_list::add_recruit(const unit_const_ptr u, const map_location& loc,
                            const map_location& from, int orig_village_owner, bool time_bonus)
{
	add(new undo::recruit_action(u, loc, from, orig_village_owner, time_bonus));
}

/**
 * Adds a shroud update to the undo stack.
 * This is called from within commit_vision(), so there should be no need
 * for this to be publicly visible.
 */
void undo_list::add_update_shroud()
{
	/// @todo: Consecutive shroud actions can be collapsed into one.

	add(new undo::update_shroud_action());
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
void undo_list::commit_vision()
{
	// Update fog/shroud.
	bool cleared_something = apply_shroud_changes();

	if (cleared_something) {
		// The actions that led to information being revealed can no longer
		// be undone.
		undos_.clear();
		//undos_.erase(undos_.begin(), undos_.begin() + erase_to);
		committed_actions_ = true;
	}
}


/**
 * Performs some initializations and error checks when starting a new side-turn.
 * @param[in]  side  The side whose turn is about to start.
 */
void undo_list::new_side_turn(int side)
{
	// Error checks.
	if ( !undos_.empty() ) {
		ERR_NG << "Undo stack not empty in new_side_turn()." << std::endl;
		// At worst, someone missed some sighted events, so try to recover.
		undos_.clear();
		redos_.clear();
	}
	else if ( !redos_.empty() ) {
		ERR_NG << "Redo stack not empty in new_side_turn()." << std::endl;
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
	for (const config & child : cfg.child_range("undo")) {
		try {
			undo_action_base * action = create_action(child);
			if ( action ) {
				undos_.push_back(action);
			}
		} catch (bad_lexical_cast &) {
			ERR_NG << "Error when parsing undo list from config: bad lexical cast." << std::endl;
			ERR_NG << "config was: " << child.debug() << std::endl;
			ERR_NG << "Skipping this undo action..." << std::endl;
		} catch (config::error& e) {
			ERR_NG << "Error when parsing undo list from config: " << e.what() << std::endl;
			ERR_NG << "config was: " << child.debug() << std::endl;
			ERR_NG << "Skipping this undo action..." << std::endl;
		}
	}

	// Build the redo stack.
	for (const config & child : cfg.child_range("redo")) {
		try {
			redos_.push_back(new config(child));
		} catch (bad_lexical_cast &) {
			ERR_NG << "Error when parsing redo list from config: bad lexical cast." << std::endl;
			ERR_NG << "config was: " << child.debug() << std::endl;
			ERR_NG << "Skipping this redo action..." << std::endl;
		} catch (config::error& e) {
			ERR_NG << "Error when parsing redo list from config: " << e.what() << std::endl;
			ERR_NG << "config was: " << child.debug() << std::endl;
			ERR_NG << "Skipping this redo action..." << std::endl;
		}
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

	for ( redos_list::const_iterator it = redos_.begin(); it != redos_.end(); ++it )
		cfg.add_child("redo") = *it;
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

	// Get the action to undo. (This will be placed on the redo stack, but
	// only if the undo is successful.)
	action_list::auto_type action = undos_.pop_back();
	if (undo_action* undoable_action = dynamic_cast<undo_action*>(action.ptr()))
	{
		int last_unit_id = resources::gameboard->unit_id_manager().get_save_id();
		if ( !undoable_action->undo(side_) ) {
			return;
		}
		if(last_unit_id - undoable_action->unit_id_diff < 0) {
			ERR_NG << "Next unit id is below 0 after undoing" << std::endl;
		}
		resources::gameboard->unit_id_manager().set_save_id(last_unit_id - undoable_action->unit_id_diff);

		// Bookkeeping.
		redos_.push_back(new config());
		resources::recorder->undo_cut(redos_.back());

		resources::whiteboard->on_gamestate_change();

		// Screen updates.
		gui.invalidate_unit();
		gui.invalidate_game_status();
		gui.redraw_minimap();
	}
	else
	{
		//ignore this action, and undo the previous one.
		config replay_data;
		resources::recorder->undo_cut(replay_data);
		undo();
		resources::recorder->redo(replay_data);
		undos_.push_back(action.release());
	}
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

	// Get the action to redo. (This will be placed on the undo stack, but
	// only if the redo is successful.)
	redos_list::auto_type action = redos_.pop_back();

	const config& command_wml = action->child("command");
	std::string commandname = command_wml.all_children_range().front().key;
	const config& data = command_wml.all_children_range().front().cfg;

	resources::recorder->redo(const_cast<const config&>(*action));


	// synced_context::run readds the undo command with the normal undo_lis::add function whihc clears the
	// redo stack which makes redoign of more than one move impossible. to work around that we save redo stack here and set it later.
	redos_list temp;
	temp.swap(redos_);
	synced_context::run(commandname, data, /*use_undo*/ true, /*show*/ true);
	temp.swap(redos_);

	// Screen updates.
	gui.invalidate_unit();
	gui.invalidate_game_status();
	gui.redraw_minimap();
}





/**
 * Applies the pending fog/shroud changes from the undo stack.
 * Does nothing if the the current side does not use fog or shroud.
 * @returns  true if shroud  or fog was cleared.
 */
bool undo_list::apply_shroud_changes() const
{
	game_display &disp = *resources::screen;
	team &tm = resources::gameboard->get_team(side_);
	// No need to do clearing if fog/shroud has been kept up-to-date.
	if ( tm.auto_shroud_updates()  ||  !tm.fog_or_shroud() ) {
		return false;
	}
	shroud_clearer clearer;
	bool cleared_shroud = false;
	const size_t list_size = undos_.size();


	// Loop through the list of undo_actions.
	for( size_t i = 0; i != list_size; ++i ) {
		if (const shroud_clearing_action* action = dynamic_cast<const shroud_clearing_action*>(&undos_[i])) {
			LOG_NG << "Turning an undo...\n";

			// Clear the hexes this unit can see from each hex occupied during
			// the action.
			std::vector<map_location>::const_iterator step;
			for (step = action->route.begin(); step != action->route.end(); ++step) {
				// Clear the shroud, collecting new sighted events.
				// (This can be made gradual by changing "true" to "false".)
				if ( clearer.clear_unit(*step, tm, action->view_info, true) ) {
					cleared_shroud = true;
				}
			}
		}
	}


	if (!cleared_shroud) {
		return false;
	}
	// If we clear fog or shroud outside a synced context we get OOS
	// Note that it can happen that we call this function from ouside a synced context
	// when we reload  a game and want to prevent undoing. But in this case this is
	// preceeded by a manual update_shroud call so that cleared_shroud is false.
	assert(synced_context::is_synced());

	// The entire stack needs to be cleared in order to preserve replays.
	// (The events that fired might depend on current unit positions.)
	// (Also the events that did not fire might depend on unit positions (they whould have fired if the unit would have standed on different positions, for example this can happen if they have a [have_unit] in [filter_condition]))

	// Update the display before pumping events.
	clearer.invalidate_after_clear();

	// Fire sighted events
	if ( std::get<0>(clearer.fire_events() )) {
		// Fix up the display in case WML changed stuff.
		clear_shroud(side_);
		disp.invalidate_unit();
	}

	return true;
}


}//namespace actions

