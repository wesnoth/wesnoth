/* $Id$ */
/*
Copyright (C) 2003 by David White <dave@whitevine.net>
Copyright (C) 2005 - 2007 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2
or at your option any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

//! @file pathfind.cpp
//! Various pathfinding functions and utilities.

#include "global.hpp"

#include "astarnode.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "util.hpp"
#include "wassert.hpp"

class gamestatus;

#include <cmath>
#include <iostream>

#define LOG_PF LOG_STREAM(info, engine)

static gamemap::location find_vacant(const gamemap& map,
		const unit_map& units,
		const gamemap::location& loc, int depth,
		VACANT_TILE_TYPE vacancy,
		std::set<gamemap::location>& touched)
	{
		if(touched.count(loc))
			return gamemap::location();

		touched.insert(loc);

		if (map.on_board(loc) && units.find(loc) == units.end() &&
		    (vacancy == VACANT_ANY || map.is_castle(loc))) {
			return loc;
		} else if(depth == 0) {
			return gamemap::location();
		} else {
			gamemap::location adj[6];
			get_adjacent_tiles(loc,adj);
			for(int i = 0; i != 6; ++i) {
				if(!map.on_board(adj[i]) || vacancy == VACANT_CASTLE && !map.is_castle(adj[i]))
					continue;

				const gamemap::location res =
					find_vacant(map, units, adj[i], depth - 1, vacancy, touched);

				if (map.on_board(res))
					return res;
			}

			return gamemap::location();
		}
	}

gamemap::location find_vacant_tile(const gamemap& map,
								   const unit_map& units,
								   const gamemap::location& loc,
								   VACANT_TILE_TYPE vacancy)
{
	for(int i = 1; i != 50; ++i) {
		std::set<gamemap::location> touch;
		const gamemap::location res = find_vacant(map,units,loc,i,vacancy,touch);
		if(map.on_board(res))
			return res;
	}

	return gamemap::location();
}

bool enemy_zoc(gamemap const &map,
               unit_map const &units,
               std::vector<team> const &teams,
               gamemap::location const &loc, team const &viewing_team, unsigned int side, bool see_all)
{
	gamemap::location locs[6];
	const team &current_team = teams[side-1];
	get_adjacent_tiles(loc,locs);
	for(int i = 0; i != 6; ++i) {
	  unit_map::const_iterator it;
	  it = find_visible_unit(units, locs[i], map, teams, viewing_team,see_all);

	  if (it != units.end() && it->second.side() != side &&
		 current_team.is_enemy(it->second.side()) && it->second.emits_zoc()) {
	    return true;
	  }
	}

	return false;
}

static void find_routes(const gamemap& map, const gamestatus& status,
		const game_data& gamedata,
		const unit_map& units,
		const unit& u,
		const gamemap::location& loc,
		int move_left,
		std::map<gamemap::location,paths::route>& routes,
		std::vector<team> const &teams,
		bool force_ignore_zocs, bool allow_teleport, int turns_left,
		  bool starting_pos, const team &viewing_team, bool see_all)
	{
		if(size_t(u.side()-1) >= teams.size()) {
			return;
		}

		team const &current_team = teams[u.side()-1];

		// Find adjacent tiles
		std::vector<gamemap::location> locs(6);
		get_adjacent_tiles(loc,&locs[0]);

		// Check for teleporting units -- we must be on a vacant village
		// (or occupied by this unit), that is controlled by our team
		// to be able to teleport.
		if (allow_teleport && map.is_village(loc) &&
		    current_team.owns_village(loc) &&
			(starting_pos || find_visible_unit(units, loc, map,
										teams, viewing_team,see_all) == units.end())) {
			

			// If we are on a village, search all known empty friendly villages
			// that we can teleport to
			const std::set<gamemap::location>& villages = current_team.villages();
			for(std::set<gamemap::location>::const_iterator t = villages.begin(); t != villages.end(); ++t) {
				if ((see_all || !viewing_team.is_enemy(u.side()) || !viewing_team.fogged(*t))
						&& find_visible_unit(units, *t, map, teams, viewing_team, see_all) == units.end()) {
					locs.push_back(*t);
				}
			}
		}

		// Iterate over all adjacent tiles
		for(size_t i = 0; i != locs.size(); ++i) {
			const gamemap::location& currentloc = locs[i];

			// Check if the adjacent location is off the board
			if (currentloc.x < 0 || currentloc.y < 0 ||
			    currentloc.x >= map.w() || currentloc.y >= map.h())
				continue;

			// See if the tile is on top of an enemy unit
			const unit_map::const_iterator unit_it =
			  find_visible_unit(units, locs[i], map, teams, viewing_team,see_all);

			if (unit_it != units.end() &&
			    current_team.is_enemy(unit_it->second.side()))
				continue;

			// Find the terrain of the adjacent location
			const t_translation::t_letter terrain = map[currentloc];

			// Find the movement cost of this type onto the terrain
			const int move_cost = u.movement_cost(terrain);
			if (move_cost <= move_left ||
			    turns_left > 0 && move_cost <= u.total_movement()) {
				int new_move_left = move_left - move_cost;
				int new_turns_left = turns_left;
				if (new_move_left < 0) {
					--new_turns_left;
					new_move_left = u.total_movement() - move_cost;
				}

				// We will check if a better route to that tile has already been found
				const std::map<gamemap::location,paths::route>::const_iterator
					old_rt = routes.find(currentloc);
				int old_move_left = -1;
				if (old_rt != routes.end())
					old_move_left = old_rt->second.move_left;

				// Only check ZoC if asked and if there is move to remove
				if (!force_ignore_zocs && new_move_left > 0) {
					// Test if, even with no ZoC, we already have a better route,
					// so no need to try with ZoC (and thus no need to search ZoC)
					const int total_move_no_zoc = new_turns_left * u.total_movement() + new_move_left;
					if(old_move_left >= total_move_no_zoc)
						continue;

					bool zoc = enemy_zoc(map,units,teams, currentloc, viewing_team,u.side(),see_all);

					// Check skirmisher only on ZoC (expensive and supposed to be rare)
					if (zoc && !u.get_ability_bool("skirmisher",currentloc)) {
						new_move_left = 0;
					}
				}

				const int new_total_move = new_turns_left * u.total_movement() + new_move_left;

				// Check if we already have a better route (now also use the possible ZoC)
				if(old_move_left >= new_total_move)
					continue;

				paths::route& src_route = routes[loc];
				paths::route& new_route = routes[currentloc];
				new_route.steps = src_route.steps;
				new_route.steps.push_back(loc);
				new_route.move_left = new_total_move;

				if (new_route.move_left > 0) {
					find_routes(map, status, gamedata, units, u, currentloc,
								new_move_left, routes, teams, force_ignore_zocs,
					            allow_teleport, new_turns_left, false, viewing_team, see_all);
				}
			}
		}
	}

paths::paths(gamemap const &map, gamestatus const &status,
             game_data const &gamedata,
             unit_map const &units,
             gamemap::location const &loc,
             std::vector<team> const &teams,
	     bool force_ignore_zoc,
             bool allow_teleport, const team &viewing_team,
		   int additional_turns, bool see_all)
{
	const unit_map::const_iterator i = units.find(loc);
	if(i == units.end()) {
		std::cerr << "unit not found\n";
		return;
	}

	routes[loc].move_left = i->second.movement_left();
	find_routes(map,status,gamedata,units,i->second,loc,
		i->second.movement_left(),routes,teams,force_ignore_zoc,
		allow_teleport,additional_turns,true,viewing_team, see_all);
}

int route_turns_to_complete(const unit& u, paths::route& rt, const team &viewing_team,
							const unit_map& units, const std::vector<team>& teams, const gamemap& map)
{
	if(rt.steps.empty())
		return 0;

	int turns = 0, movement = u.movement_left();
	const team& unit_team = teams[u.side()-1];

	for(std::vector<gamemap::location>::const_iterator i = rt.steps.begin()+1;
	    i != rt.steps.end(); ++i) {
		wassert(map.on_board(*i));
		const int move_cost = u.movement_cost(map[*i]);
		movement -= move_cost;

		if (movement < 0) {
			++turns;
			rt.turn_waypoints.insert(std::make_pair(*(i-1), turns));
			movement = u.total_movement() - move_cost;
			if(movement < 0) {
				return -1;
			}
		}

		if (!u.get_ability_bool("skirmisher",*i)) {
			gamemap::location adj[6];
			get_adjacent_tiles(*i, adj);

			for (size_t j = 0; j != 6; ++j) {
				unit_map::const_iterator enemy_unit = find_visible_unit(units, adj[j], map, teams, viewing_team);
				if (enemy_unit != units.end() && unit_team.is_enemy(enemy_unit->second.side())
					&& enemy_unit->second.emits_zoc()) {
					 movement = 0;
				}
			}
		}
	}

	// Add "end-of-path" to waypoints.
	const gamemap::location& new_turn_step = *(rt.steps.end()-1);
	int turn_number = 0;
	if (turns > 0) {
		turn_number = turns+1;
	} else if (movement==0) {
		turn_number = 1;
	} else if (map.is_village(new_turn_step)) {
		// if it's an enemy unit and a fogged village, we assume a capture 
		// (if he already owns it, we can't know that)
		// if it's not an enemy, we can always know if he owns the village
		if ( (viewing_team.is_enemy(u.side()) && viewing_team.fogged(new_turn_step))
				|| !unit_team.owns_village(new_turn_step) ) {
			turn_number = 1;
		}
	}

	rt.turn_waypoints.insert(std::make_pair(new_turn_step, turn_number));

	return turns;
}


shortest_path_calculator::shortest_path_calculator(unit const &u, team const &t, unit_map const &units,
                                                   std::vector<team> const &teams, gamemap const &map)
	: unit_(u), viewing_team_(t), units_(units), teams_(teams), map_(map),
	  movement_left_(unit_.movement_left()),
	  total_movement_(unit_.total_movement())
{
}

double shortest_path_calculator::cost(const gamemap::location& /*src*/,const gamemap::location& loc, const double so_far, const bool isDst) const
{
	wassert(map_.on_board(loc));

	// The location is not valid
	// 1. if the loc is shrouded, or
	// 2. if moving in it costs more than the total movement of the unit, or
	// 3. if there is a visible enemy on the hex, or
	// 4. if the unit is not a skirmisher and there is a visible enemy
	//    with a ZoC on an adjacent hex in the middle of the route
	// #4 is a bad criteria!  It should be that moving into a ZOC
	// uses up the rest of your moves

	if (viewing_team_.shrouded(loc))
		return getNoPathValue();

	int const base_cost = unit_.movement_cost(map_[loc]);
	wassert(base_cost >= 1); // Pathfinding heuristic: the cost must be at least 1
	if (total_movement_ < base_cost)
		return getNoPathValue();

	unit_map::const_iterator
		enemy_unit = find_visible_unit(units_, loc, map_, teams_, viewing_team_),
		units_end = units_.end();

	if (enemy_unit != units_end && teams_[unit_.side()-1].is_enemy(enemy_unit->second.side()))
		return getNoPathValue();

	// Compute how many movement points are left in the game turn
	// needed to reach the previous hex.
	// total_movement_ is not zero, thanks to the pathfinding heuristic
	int remaining_movement = movement_left_ - static_cast<int>(so_far);
	if (remaining_movement < 0)
		remaining_movement = total_movement_ - (-remaining_movement) % total_movement_;

	// Supposing we had 2 movement left, and wanted to move onto a hex
	// which takes 3 movement, it's going to cost us 5 movement in total,
	// since we sacrifice this turn's movement. Take that into account here.
	int additional_cost = base_cost > remaining_movement ? remaining_movement : 0;

	if (!isDst && !unit_.get_ability_bool("skirmisher",loc)) {
	  gamemap::location adj[6];
	  get_adjacent_tiles(loc, adj);

	  for (size_t i = 0; i != 6; ++i) {
	    enemy_unit = find_visible_unit(units_, adj[i], map_, teams_, viewing_team_);
	    if (enemy_unit != units_end && teams_[unit_.side()-1].is_enemy(enemy_unit->second.side()) &&
		   enemy_unit->second.emits_zoc()){
		 // Should cost us remaining movement.
		 //		 return getNoPathValue();
		 return total_movement_ + additional_cost;
	    }
	  }
	}

	return base_cost + additional_cost;
}

emergency_path_calculator::emergency_path_calculator(const unit& u, const gamemap& map)
	: unit_(u), map_(map)
{
}

double emergency_path_calculator::cost(const gamemap::location&,const gamemap::location& loc, const double, const bool) const
{
	wassert(map_.on_board(loc));

	return unit_.movement_cost(map_[loc]);
}
