/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "display_context.hpp"

#include "map.hpp"
#include "map_location.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_map.hpp"

const unit * display_context::get_visible_unit(const map_location & loc, const team &current_team, bool see_all) const
{
	if (!map().on_board(loc)) return NULL;
	const unit_map::const_iterator u = units().find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, map(), see_all)) {
		return NULL;
	}
	return &*u;
}

/**
 * Will return true iff the unit @a u has any possible moves
 * it can do (including attacking etc).
 */

bool display_context::unit_can_move(const unit &u) const
{
	if(!u.attacks_left() && u.movement_left()==0)
		return false;

	// Units with goto commands that have already done their gotos this turn
	// (i.e. don't have full movement left) should have red globes.
	if(u.has_moved() && u.has_goto()) {
		return false;
	}

	const team &current_team = teams()[u.side() - 1];

	map_location locs[6];
	get_adjacent_tiles(u.get_location(), locs);
	for(int n = 0; n != 6; ++n) {
		if (map().on_board(locs[n])) {
			const unit_map::const_iterator i = units().find(locs[n]);
			if (i.valid() && !i->incapacitated() &&
			    current_team.is_enemy(i->side())) {
				return true;
			}

			if (u.movement_cost(map()[locs[n]]) <= u.movement_left()) {
				return true;
			}
		}
	}

	return false;
}

