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
 * Sighting.
 */

#include "vision.hpp"

#include "../game_display.hpp"
#include "../game_events.hpp"
#include "../map.hpp"
#include "../map_label.hpp"
#include "../map_location.hpp"
#include "../pathfind/pathfind.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../unit.hpp"

#include <boost/foreach.hpp>


namespace {

	/**
	 * Clears shroud from a single location.
	 *
	 * In a few cases, this will also clear corner hexes that otherwise would
	 * not normally get cleared.
	 * @param tm               The team whose fog/shroud is affected.
	 * @param loc              The location to clear.
	 * @param viewer           The unit doing the viewing.
	 * @param seen_units       If the location was cleared and contained a visible,
	 *                         non-petrified unit, it gets added to this set.
	 * @param petrified_units  If the location was cleared and contained a visible,
	 *                         petrified unit, it gets added to this set.
	 * @param known_units      These locations are excluded from being added to
	 *                         seen_units and petrified_units.
	 *
	 * @return whether or not information was uncovered (i.e. returns true if
	 *         the specified location was fogged/ shrouded under shared vision/maps).
	 */
	bool clear_shroud_loc(team &tm, const map_location& loc, const unit & viewer,
	                      std::set<map_location>* seen_units = NULL,
	                      std::set<map_location>* petrified_units = NULL,
	                      const std::set<map_location>* known_units = NULL)
	{
		gamemap &map = *resources::game_map;
		// This counts as clearing a tile for the return value if it is on the
		// board and currently fogged under shared vision. (No need to explicitly
		// check for shrouded since shrouded implies fogged.)
		bool was_fogged = tm.fogged(loc);
		bool result = was_fogged && map.on_board(loc);

		// Clear the border as well as the board, so that the half-hexes
		// at the edge can also be cleared of fog/shroud.
		if ( map.on_board_with_border(loc)) {
			// Both functions should be executed so don't use || which
			// uses short-cut evaluation.
			// (This is different than the return value because shared vision does
			// not apply here.)
			if ( tm.clear_shroud(loc) | tm.clear_fog(loc) ) {
				// If we are near a corner, the corner might also need to be cleared.
				// This happens at the lower-left corner and at either the upper- or
				// lower- right corner (depending on the width).

				// Lower-left corner:
				if ( loc.x == 0  &&  loc.y == map.h()-1 ) {
					const map_location corner(-1, map.h());
					tm.clear_shroud(corner);
					tm.clear_fog(corner);
				}
				// Lower-right corner, odd width:
				else if ( is_odd(map.w())  &&  loc.x == map.w()-1  &&  loc.y == map.h()-1 ) {
					const map_location corner(map.w(), map.h());
					tm.clear_shroud(corner);
					tm.clear_fog(corner);
				}
				// Upper-right corner, even width:
				else if ( is_even(map.w())  &&  loc.x == map.w()-1  &&  loc.y == 0) {
					const map_location corner(map.w(), -1);
					tm.clear_shroud(corner);
					tm.clear_fog(corner);
				}
			}
		}

		// Possible screen invalidation.
		if ( was_fogged ) {
			resources::screen->invalidate(loc);
			// Need to also invalidate adjacent hexes to get rid of the
			// "fog edge" graphics.
			map_location adjacent[6];
			get_adjacent_tiles(loc, adjacent);
			for ( int i = 0; i != 6; ++i )
				resources::screen->invalidate(adjacent[i]);
		}

		// Does the caller want a list of discovered units?
		if ( result  &&  (seen_units || petrified_units) ) {
			// Allow known_units to override fogged().
			if ( loc != viewer.get_location()  &&
			     (known_units == NULL  ||  known_units->count(loc) == 0) )
			{
				// Is there a visible unit here?
				const unit_map::const_iterator sighted = resources::units->find(loc);
				if ( sighted.valid() ) {
					if ( !tm.is_enemy(sighted->side()) ||
					     !sighted->invisible(loc) )
					{
						// Add this unit to the appropriate list.
						if ( !sighted->get_state(unit::STATE_PETRIFIED) )
						{
							if ( seen_units != NULL )
								seen_units->insert(loc);
						}
						else if ( petrified_units != NULL )
							petrified_units->insert(loc);
					}
				}
			}
		}

		return result;
	}

}


/**
 * Clears shroud (and fog) around the provided location for @a view_team as
 * if a unit with @a viewer's sight range was standing there.
 * (This uses a team parameter instead of a side since it is assumed that
 * the caller already checked for fog or shroud being in use. Hence the
 * caller has the team readily available.)
 *
 * @a seen_units will return new units that have been seen by this unit.
 *
 * @return whether or not information was uncovered (i.e. returns true if any
 *         locations in visual range were fogged/shrouded under shared vision/maps).
 */
bool clear_shroud_unit(const map_location &view_loc, const unit &viewer,
                       team &view_team, const std::map<map_location, int>& jamming_map,
                       const std::set<map_location>* known_units,
                       std::set<map_location>* seen_units,
                       std::set<map_location>* petrified_units)
{
	bool cleared_something = false;

	// Clear the fog.
	pathfind::vision_path sight(*resources::game_map, viewer, view_loc, jamming_map);
	BOOST_FOREACH(const pathfind::paths::step &dest, sight.destinations) {
		if ( clear_shroud_loc(view_team, dest.curr, viewer, seen_units,
		                      petrified_units, known_units) )
			cleared_something = true;
	}
	//TODO guard with game_config option
	BOOST_FOREACH(const map_location &dest, sight.edges) {
		if ( clear_shroud_loc(view_team, dest, viewer, seen_units,
		                      petrified_units, known_units) )
			cleared_something = true;
	}

	return cleared_something;
}


/**
 * Wrapper for the invalidations that should occur after fog or
 * shroud is cleared. (Needed in multiple places, so this makes
 * sure the same things are called each time.) This would be
 * called after one is done calling clear_shroud_unit().
 */
void invalidate_after_clearing_shroud()
{
	resources::screen->invalidate_game_status();
	resources::screen->recalculate_minimap();
	resources::screen->labels().recalculate_shroud();
	// The tiles are invalidated as they are cleared, so no need
	// to invalidate them here.
}


void calculate_jamming(int side, std::map<map_location, int>& jamming_map)
{
	team& viewer_tm = (*resources::teams)[side - 1];

	BOOST_FOREACH(const unit &u, *resources::units)
	{
		if (!viewer_tm.is_enemy(u.side())) continue;
		if (u.jamming() < 1) continue;

		int current = jamming_map[u.get_location()];
		if (current < u.jamming()) jamming_map[u.get_location()] = u.jamming();

		pathfind::jamming_path jamming(*resources::game_map, u, u.get_location());
		BOOST_FOREACH(const pathfind::paths::step& st, jamming.destinations) {
			current = jamming_map[st.curr];
			if (current < st.move_left)
				jamming_map[st.curr] = st.move_left;
		}
	}
}


/**
 * Function that recalculates the fog of war.
 *
 * This is used at the end of a turn and for the defender at the end of
 * combat. As a back-up, it is also called when clearing shroud at the
 * beginning of a turn.
 * This function does nothing if the indicated side does not use fog.
 * The display is invalidated as needed.
 *
 * @param[in] side The side whose fog will be recalculated.
 */
void recalculate_fog(int side)
{
	team &tm = (*resources::teams)[side - 1];

	if (!tm.uses_fog())
		return;

	// The following lines will be useful at some point, but not yet.
	// So they are commented out for now.
	//std::set<map_location> visible_locs;
	//// Loop through all units, looking for those that are visible.
	//BOOST_FOREACH(const unit &u, *resources::units) {
	//	const map_location & u_location = u.get_location();
	//
	//	if ( !tm.fogged(u_location) )
	//		visible_locs.insert(u_location);
	//}

	tm.refog();
	// Invalidate the screen before clearing the shroud.
	// This speeds up the invalidations within clear_shroud_unit().
	resources::screen->invalidate_all();

	std::map<map_location, int> jamming_map;
	calculate_jamming(side, jamming_map);
	BOOST_FOREACH(const unit &u, *resources::units)
	{
		if (u.side() == side) {
			clear_shroud_unit(u.get_location(), u, tm, jamming_map);
		}
	}

	//FIXME: This pump don't catch any sighted events (they are not fired by
	// clear_shroud_unit) and if it caches another old event, maybe the caller
	// don't want to pump it here
	game_events::pump();

	// Update the screen.
	invalidate_after_clearing_shroud();
}

/**
 * Function that will clear shroud (and fog) based on current unit positions.
 *
 * This will not re-fog hexes unless reset_fog is set to true.
 * This function will do nothing if the side uses neither shroud nor fog.
 * The display is invalidated as needed.
 *
 * @param[in] side      The side whose shroud (and fog) will be cleared.
 * @param[in] reset_fog If set to true, the fog will also be recalculated
 *                      (refogging hexes that can no longer be seen).
 * @returns true if some shroud/fog is actually cleared away.
 */
bool clear_shroud(int side, bool reset_fog)
{
	team &tm = (*resources::teams)[side - 1];
	if (!tm.uses_shroud() && !tm.uses_fog())
		return false;

	bool result = false;

	std::map<map_location, int> jamming_map;
	calculate_jamming(side, jamming_map);
	BOOST_FOREACH(const unit &u, *resources::units)
	{
		if (u.side() == side) {
			result |= clear_shroud_unit(u.get_location(), u, tm, jamming_map);
		}
	}

	//FIXME: This pump don't catch any sighted events (they are not fired by
	// clear_shroud_unit) and if it caches another old event, maybe the caller
	// don't want to pump it here
	game_events::pump();

	if ( reset_fog ) {
		// Note: This will not reveal any new tiles, so result is not affected.
		// Note: This will call invalidate_after_clearing_shroud().
		recalculate_fog(side);
	}
	else if ( result ) {
		// Update the screen.
		invalidate_after_clearing_shroud();
	}

	return result;
}

