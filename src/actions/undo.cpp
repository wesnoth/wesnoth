/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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
 * Undoing, redoing.
 */

#include "actions/undo.hpp"

#include "game_board.hpp"               // for game_board
#include "game_display.hpp"          // for game_display
#include "gui/dialogs/transient_message.hpp"
#include "log.hpp"                   // for LOG_STREAM, logger, etc
#include "map/location.hpp"  // for map_location, operator<<, etc
#include "mouse_handler_base.hpp"       // for command_disabler
#include "replay.hpp"                // for recorder, replay
#include "resources.hpp"             // for screen, teams, units, etc
#include "synced_context.hpp"        // for set_scontext_synced
#include "team.hpp"                  // for team
#include "units/id.hpp"
#include "units/ptr.hpp"      // for unit_const_ptr, unit_ptr
#include "units/types.hpp"               // for unit_type, unit_type_data, etc
#include "whiteboard/manager.hpp"    // for manager

#include "actions/vision.hpp"           // for clearer_info, etc
#include "actions/shroud_clearing_action.hpp"
#include "actions/undo_dismiss_action.hpp"
#include "actions/undo_move_action.hpp"
#include "actions/undo_recall_action.hpp"
#include "actions/undo_recruit_action.hpp"
#include "actions/undo_update_shroud_action.hpp"

#include <algorithm>                    // for reverse
#include <cassert>                      // for assert

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)


namespace actions {


/**
 * Constructor.
 * The config is allowed to be invalid.
 */
undo_list::undo_list() :
	undos_(), redos_(), side_(1), committed_actions_(false)
{
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
	add(std::make_unique<undo::auto_shroud_action>(turned_on));
}


/**
 * Adds a dismissal to the undo stack.
 */
void undo_list::add_dismissal(const unit_const_ptr& u)
{
	add(std::make_unique<undo::dismiss_action>(u));
}

/**
 * Adds a move to the undo stack.
 */
void undo_list::add_move(const unit_const_ptr& u,
                         const std::vector<map_location>::const_iterator & begin,
                         const std::vector<map_location>::const_iterator & end,
                         int start_moves,
                         const map_location::direction dir)
{
	add(std::make_unique<undo::move_action>(u, begin, end, start_moves, dir));
}

/**
 * Adds a recall to the undo stack.
 */
void undo_list::add_recall(const unit_const_ptr& u, const map_location& loc,
                           const map_location& from)
{
	add(std::make_unique<undo::recall_action>(u, loc, from));
}

/**
 * Adds a recruit to the undo stack.
 */
void undo_list::add_recruit(const unit_const_ptr& u, const map_location& loc,
                            const map_location& from)
{
	add(std::make_unique<undo::recruit_action>(u, loc, from));
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
 */
bool undo_list::commit_vision()
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
	return cleared_something;
}


/**
 * Performs some initializations and error checks when starting a new side-turn.
 * @param[in]  side  The side whose turn is about to start.
 */
void undo_list::new_side_turn(int side)
{
	// Error checks.
	if ( !undos_.empty() ) {
		ERR_NG << "Undo stack not empty in new_side_turn().";
		// At worst, someone missed some sighted events, so try to recover.
		undos_.clear();
		redos_.clear();
	}
	else if ( !redos_.empty() ) {
		ERR_NG << "Redo stack not empty in new_side_turn().";
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
void undo_list::read(const config& cfg, int current_side)
{
	side_ = current_side;
	committed_actions_ = committed_actions_ || cfg["committed"].to_bool();

	//If we have the side parameter this means that this was the old format pre 1.19.7, we ignore this since it's incompatible.
	if(cfg.has_attribute("side")) {
		return;
	}

	// Build the undo stack.
	try {
		for(const config& child : cfg.child_range("undo")) {
			undos_.push_back(std::make_unique<undo_action_container>());
			undos_.back()->read(child);
		}
	} catch(const bad_lexical_cast&) {
		//It ddoenst make sense to "skip" actions in the undo stakc since that would just result in errors later.
		ERR_NG << "Error when parsing undo list from config: bad lexical cast.";
		ERR_NG << "config was: " << cfg.debug();
		ERR_NG << "discardind undo stack...";
		undos_.clear();
	} catch(const config::error& e) {
		ERR_NG << "Error when parsing undo list from config: " << e.what();
		ERR_NG << "config was: " << cfg.debug();
		ERR_NG << "discardind undo stack...";
		undos_.clear();
	}


	// Build the redo stack.
	for (const config & child : cfg.child_range("redo")) {
		redos_.emplace_back(new config(child));
	}
}


/**
 * Write the undo_list into the provided config.
 */
void undo_list::write(config & cfg) const
{
	cfg["committed"] = committed_actions_;

	for ( const auto& action_ptr : undos_)
		action_ptr->write(cfg.add_child("undo"));

	for ( const auto& cfg_ptr : redos_)
		cfg.add_child("redo") = *cfg_ptr;
}


void undo_list::init_action()
{
	current_ = std::make_unique<undo_action_container>();
	redos_.clear();
}


void undo_list::finish_action(bool can_undo)
{
	if(current_) {
		current_->set_unit_id_diff(synced_context::get_unit_id_diff());
		undos_.emplace_back(std::move(current_));
		if(!can_undo) {
			clear();
		}
	}
}

void undo_list::cleanup_action()
{
	// This in particular makes sure no commands that do nothing stay on the undo stack but also on the recorder
	// in particular so that menu items that did nothing because the user aborted in a custom menu dont persist on the replay.
	if(!undos_.empty() && undos_.back()->empty()) {
		undo();
	}
}
/**
 * Undoes the top action on the undo stack.
 */
void undo_list::undo()
{
	if ( undos_.empty() )
		return;

	const events::command_disabler disable_commands;

	// Get the action to undo. (This will be placed on the redo stack, but
	// only if the undo is successful.)
	auto action = std::move(undos_.back());
	if(!action->undo(side_)) {
		return;
	}

	// Bookkeeping.
	undos_.pop_back();
	redos_.emplace_back(new config());
	resources::recorder->undo_cut(*redos_.back());

	resources::whiteboard->on_gamestate_change();

	// Screen updates.
	game_display& gui = *game_display::get_singleton();
	gui.invalidate_unit();
	gui.invalidate_game_status();
	gui.redraw_minimap();
}



/**
 * Redoes the top action on the redo stack.
 */
void undo_list::redo()
{
	if (redos_.empty()) {
		return;
	}
	// Get the action to redo.
	auto action = std::move(redos_.back());
	redos_.pop_back();

	auto [commandname, data] = action->mandatory_child("command").all_children_view().front();

	// Note that this might add more than one [command]
	resources::recorder->redo(*action);

	auto spectator = action_spectator([](const std::string& msg)
	{
		ERR_NG << "Out of sync when redoing: " << msg;
		gui2::show_transient_message(_("Redo Error"),
					_("The redo stack is out of sync. This is most commonly caused by a corrupt save file or by faulty WML code in the scenario or era. Details:") + msg);

	});
	// synced_context::run readds the undo command with the normal
	// undo_list::add function which clears the redo stack which would
	// make redoing of more than one move impossible. To work around
	// that we save redo stack here and set it later.
	redos_list temp;
	temp.swap(redos_);
	synced_context::run(commandname, data, spectator);
	temp.swap(redos_);

	// Screen updates.
	game_display & gui = *game_display::get_singleton();
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
	game_display &disp = *game_display::get_singleton();
	team &tm = resources::gameboard->get_team(side_);
	// No need to do clearing if fog/shroud has been kept up-to-date.
	if ( tm.auto_shroud_updates()  ||  !tm.fog_or_shroud() ) {
		return false;
	}
	shroud_clearer clearer;
	bool cleared_shroud = false;
	const std::size_t list_size = undos_.size();


	// Loop through the list of undo_actions.
	for( std::size_t i = 0; i != list_size; ++i ) {
		// Loop through the staps of the action.
		for(auto& step_ptr : undos_[i]->steps()) {
			if(const shroud_clearing_action* action = dynamic_cast<const shroud_clearing_action*>(step_ptr.get())) {
				LOG_NG << "Turning an undo...";

				// Clear the hexes this unit can see from each hex occupied during
				// the action.
				std::vector<map_location>::const_iterator step;
				for(step = action->route.begin(); step != action->route.end(); ++step) {
					// Clear the shroud, collecting new sighted events.
					// (This can be made gradual by changing "true" to "false".)
					if(clearer.clear_unit(*step, tm, action->view_info, true)) {
						cleared_shroud = true;
					}
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
	// preceded by a manual update_shroud call so that cleared_shroud is false.
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
