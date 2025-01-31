/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

const team& display_context::get_team(int side) const
{
	return teams().at(side - 1);
}

bool display_context::has_team(int side) const
{
	return side > 0 && side <= static_cast<int>(teams().size());
}

bool display_context::would_be_discovered(const map_location & loc, int side_num, bool see_all)
{
	for(const map_location& u_loc : get_adjacent_tiles(loc)) {
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
			&& !u.invisible(u_loc, true)) {
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
	if (!u.valid() || !u->is_visible_to_team(current_team, see_all)) {
		return nullptr;
	}
	return &*u;
}

unit_const_ptr display_context::get_visible_unit_shared_ptr(const map_location & loc, const team &current_team, bool see_all) const
{
	if (!map().on_board(loc)) return nullptr;
	const unit_map::const_iterator u = units().find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, see_all)) {
		return unit_const_ptr();
	}
	return u.get_shared_ptr();
}

display_context::can_move_result display_context::unit_can_move(const unit& u) const
{
	if(!u.attacks_left() && u.movement_left() == 0)
		return {false, false};

	// Units with goto commands that have already done their gotos this turn
	// (i.e. don't have full movement left) should have red globes.
	if(u.has_moved() && u.has_goto()) {
		return {false, false};
	}

	const team& current_team = get_team(u.side());

	can_move_result result = {false, false};
	if(u.attacks_left() > 0 && !u.attacks().empty()) {
		const auto& attacks = u.attacks();

		std::set<int> attackable_distances;
	    for (const auto& attack : attacks) {
	        for (int i = attack.min_range(); i <= attack.max_range(); ++i) {
	            attackable_distances.insert(i);
	        }
	    }

		if(!attackable_distances.empty()) {
			int max_distance = *std::prev(attackable_distances.end());

			for (int dx = -max_distance; dx <= max_distance; ++dx) {
				for (int dy = -max_distance; dy <= max_distance && !result.attack_here; ++dy) {
					// Adjust for hex grid
					int adjusted_dy = dy + floor(dx / 2.0);

					map_location locs(u.get_location().x + dx, u.get_location().y + adjusted_dy);
					int distance = distance_between(u.get_location(), locs);

					if (attackable_distances.find(distance) == attackable_distances.end()) {
						continue;
					}
					if (map().on_board(locs)) {
						const unit_map::const_iterator i = units().find(locs);
						if (i.valid() && !i->incapacitated() && current_team.is_enemy(i->side()) && i->is_visible_to_team(get_team(u.side()), false)) {
							result.attack_here = true;
						}
					}
				}
			}
		}
	}
	for(const map_location& adj : get_adjacent_tiles(u.get_location())) {
		if (map().on_board(adj)) {
			if (!result.move && u.movement_cost(map()[adj]) <= u.movement_left()) {
				result.move = true;
			}
		}
	}
	// This should probably check if the unit can teleport too

	return result;
}

orb_status display_context::unit_orb_status(const unit& u) const
{
	if(u.user_end_turn())
		return orb_status::moved;
	if(u.movement_left() == u.total_movement() && u.attacks_left() == u.max_attacks())
		return orb_status::unmoved;
	auto can_move = unit_can_move(u);
	if(!can_move)
		return orb_status::moved;
	if(can_move.move && u.attacks_left() == 0)
		return orb_status::disengaged;
	return orb_status::partial;
}

int display_context::village_owner(const map_location& loc) const
{
	for(const team& t : teams()) {
		if(t.owns_village(loc)) {
			return t.side();
		}
	}
	return 0;
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

// Static info getters previously declared at global scope in unit.?pp

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

team_data::team_data(const display_context& dc, const team& tm)
	: side(tm.side())
	, units(dc.side_units(side))
	, upkeep(dc.side_upkeep(side))
	, expenses(std::max<int>(0, upkeep - tm.support()))
	, net_income(tm.total_income() - expenses)
{
}
