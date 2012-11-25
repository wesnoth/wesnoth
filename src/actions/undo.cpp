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

#include "vision.hpp"

#include "../game_display.hpp"
#include "../game_events.hpp"
#include "../log.hpp"
#include "../play_controller.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../unit_map.hpp"

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)


/**
 * Clears the stack of undoable actions.
 * Call this if an action alters the game state, but add that action to the
 * stack before calling this.
 * This may fire events and change the game state.
 */
void undo_list::clear()
{
	// No need to do anything if the stack is already clear.
	if ( undos_.empty() )
		return;

	// Update fog/shroud.
	apply_shroud_changes();

	// Clear the stack.
	undos_.clear();
}


/**
 * Applies the pending fog/shroud changes from the undo stack.
 * Does nothing if the the current side does not use fog or shroud.
 */
void undo_list::apply_shroud_changes() const
{
	int side = resources::controller->current_side();
	team &tm = (*resources::teams)[side - 1];
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
		clear_shroud(side);
		disp.invalidate_unit();
		disp.draw();
	}
}

