/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
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

#include "map/map.hpp"
#include "map/location.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"

std::vector<std::string>& display_context::hidden_label_categories_ref() {
	return const_cast<std::vector<std::string>&>(this->hidden_label_categories());
}

const team& display_context::get_team(int side) const
{
	return teams().at(side - 1);
}

bool display_context::would_be_discovered(const map_location & loc, int side_num, bool see_all)
{
	map_location adjs[6];
	get_adjacent_tiles(loc,adjs);

	for (const map_location &u_loc : adjs)
	{
		unit_map::const_iterator u_it = units().find(u_loc);
		if (!u_it.valid()) {
			continue;
		}
		const unit & u = *u_it;
		if (get_team(side_num).is_enemy(u.side()) && !u.incapacitated()) {
			// Enemy spotted in adjacent tiles, check if we can see him.
			// Watch out to call invisible with see_all=true to avoid infinite recursive calls!
			if(see_all) {
				return true;
			} else if (!get_team(side_num).fogged(u_loc)
			&& !u.invisible(u_loc, *this, true)) {
				return true;
			}
		}
	}
	return false;
}

const unit * display_context::get_visible_unit(const map_location & loc, const team &current_team, bool see_all) const
{
	if (!map().on_board(loc)) return nullptr;
	const unit_map::const_iterator u = units().find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, *this, see_all)) {
		return nullptr;
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

	const team &current_team = get_team(u.side());

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


/**
 * Given the location of a village, will return the 0-based index
 * of the team that currently owns it, and -1 if it is unowned.
 */
int display_context::village_owner(const map_location& loc) const
{
	const std::vector<team> & t = teams();
	for(size_t i = 0; i != t.size(); ++i) {
		if(t[i].owns_village(loc))
			return i;
	}
	return -1;
}

/**
 * Determine if we are an observer, by checking if every team is not locally controlled
 */
bool display_context::is_observer() const
{
	for (const team &t : teams()) {
		if (t.is_local())
			return false;
	}

	return true;
}

/// Static info getters previously declared at global scope in unit.?pp

int display_context::side_units(int side) const
{
	int res = 0;
	for (const unit &u : units()) {
		if (u.side() == side) ++res;
	}
	return res;
}

int display_context::side_units_cost(int side) const
{
	int res = 0;
	for (const unit &u : units()) {
		if (u.side() == side) res += u.cost();
	}
	return res;
}

int display_context::side_upkeep(int side) const
{
	int res = 0;
	for (const unit &u : units()) {
		if (u.side() == side) res += u.upkeep();
	}
	return res;
}

team_data display_context::calculate_team_data(const team& tm) const
{
	team_data res;
	res.units = side_units(tm.side());
	res.upkeep = side_upkeep(tm.side());
	res.villages = tm.villages().size();
	res.expenses = std::max<int>(0,res.upkeep - tm.support());
	res.net_income = tm.total_income() - res.expenses;
	res.gold = tm.gold();
	res.teamname = tm.user_team_name();
	return res;
}

