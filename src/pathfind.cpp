/* $Id$ */
/*
Copyright (C) 2003 by David White <dave@whitevine.net>
Copyright (C) 2005 - 2009 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2
or at your option any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

/**
 * @file pathfind.cpp
 * Various pathfinding functions and utilities.
 */

#include "global.hpp"

#include "foreach.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "map.hpp"
#include "wml_exception.hpp"


#define LOG_PF LOG_STREAM(info, engine)

static map_location find_vacant(const gamemap& map,
		const unit_map& units,
		const map_location& loc, int depth,
		VACANT_TILE_TYPE vacancy,
		std::set<map_location>& touched)
	{
		if(touched.count(loc))
			return map_location();

		touched.insert(loc);

		if (map.on_board(loc) && units.find(loc) == units.end() &&
		    (vacancy == VACANT_ANY || map.is_castle(loc))) {
			return loc;
		} else if(depth == 0) {
			return map_location();
		} else {
			map_location adj[6];
			get_adjacent_tiles(loc,adj);
			for(int i = 0; i != 6; ++i) {
				if(!map.on_board(adj[i]) || (vacancy == VACANT_CASTLE && !map.is_castle(adj[i])))
					continue;

				const map_location res =
					find_vacant(map, units, adj[i], depth - 1, vacancy, touched);

				if (map.on_board(res))
					return res;
			}

			return map_location();
		}
	}

map_location find_vacant_tile(const gamemap& map,
								   const unit_map& units,
								   const map_location& loc,
								   VACANT_TILE_TYPE vacancy)
{
	for(int i = 1; i != 50; ++i) {
		std::set<map_location> touch;
		const map_location res = find_vacant(map,units,loc,i,vacancy,touch);
		if(map.on_board(res))
			return res;
	}

	return map_location();
}

bool enemy_zoc(gamemap const &map,
               unit_map const &units,
               std::vector<team> const &teams,
               map_location const &loc, team const &viewing_team, unsigned int side, bool see_all)
{
	map_location locs[6];
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

static void find_routes(const gamemap& map, const unit_map& units,
		const unit& u, const map_location& loc,
		int move_left, paths::routes_map& routes,
		std::vector<team> const &teams,
		bool force_ignore_zocs, bool allow_teleport, int turns_left,
		bool starting_pos, const team &viewing_team,
		bool see_all, bool ignore_units)
{
	team const &current_team = teams[u.side()-1];

	// Find adjacent tiles
	std::vector<map_location> locs(6);
	get_adjacent_tiles(loc,&locs[0]);

	// Check for teleporting units -- we must be on a vacant village
	// (or occupied by this unit), that is controlled by our team
	// to be able to teleport.
	if (allow_teleport && map.is_village(loc) && current_team.owns_village(loc) &&
			(starting_pos || ignore_units ||
			 find_visible_unit(units, loc, map, teams, viewing_team,see_all) == units.end())) {

		// If we are on a village, search all known empty friendly villages
		// that we can teleport to
		const std::set<map_location>& villages = current_team.villages();
		for(std::set<map_location>::const_iterator t = villages.begin(); t != villages.end(); ++t) {
			if ((see_all || !viewing_team.is_enemy(u.side()) || !viewing_team.fogged(*t))
					&& (ignore_units || find_visible_unit(units, *t, map, teams, viewing_team, see_all) == units.end())) {
				locs.push_back(*t);
			}
		}
	}

	// Iterate over all adjacent tiles
	for(std::vector<map_location>::const_iterator i=locs.begin(); i != locs.end(); ++i) {
		const map_location& currentloc = *i;

		if (!map.on_board(currentloc))
			continue;

		// check if we can move on this terrain
		const int move_cost = u.movement_cost(map[currentloc]);
		if (move_cost > move_left && (turns_left < 1 || move_cost > u.total_movement()))
			continue;

		int new_move_left = move_left - move_cost;
		int new_turns_left = turns_left;
		if (new_move_left < 0) {
			--new_turns_left;
			new_move_left = u.total_movement() - move_cost;
		}
		const int new_turns_moves = new_turns_left * u.total_movement();

		// Search if we already have a route here, and how good it is
		int old_move_left = -1;
		const paths::routes_map::const_iterator old_rt = routes.find(currentloc);
		if (old_rt != routes.end())
			old_move_left = old_rt->second.move_left;

		// Test if, even with no ZoC, we already have a better route,
		// so no need to try with ZoC (and thus no need to search ZoC)
		if(old_move_left >= new_turns_moves + new_move_left)
			continue;

		if (!ignore_units) {
			// we can not traverse enemies
			const unit_map::const_iterator unit_it =
				find_visible_unit(units, currentloc, map, teams, viewing_team,see_all);
			if (unit_it != units.end() && current_team.is_enemy(unit_it->second.side()))
				continue;

			// Evaluation order is optimized (this is the main bottleneck)
			// Only check ZoC if asked and if there is move to remove.
			// Do skirmisher test only on ZoC (expensive and supposed to be rare)
			if (!force_ignore_zocs && new_move_left > 0
					&& enemy_zoc(map,units,teams, currentloc, viewing_team,u.side(),see_all)
					&& !u.get_ability_bool("skirmisher", currentloc)) {
				new_move_left = 0;
				// Recheck if we already have a better route, but now with the ZoC effect
				if(old_move_left >= new_turns_moves + 0)
					continue;
			}
		}

		paths::route& src_route = routes[loc];
		paths::route& new_route = routes[currentloc];
		new_route.steps = src_route.steps;
		new_route.steps.push_back(loc);
		new_route.move_left = new_turns_moves + new_move_left;

		if (new_route.move_left > 0) {
			find_routes(map, units, u, currentloc,
						new_move_left, routes, teams, force_ignore_zocs,
						allow_teleport, new_turns_left, false, viewing_team,
						see_all, ignore_units);
		}
	}
}

paths::paths(gamemap const &map, unit_map const &units,
		map_location const &loc, std::vector<team> const &teams,
		bool force_ignore_zoc, bool allow_teleport, const team &viewing_team,
		int additional_turns, bool see_all, bool ignore_units) :
	routes()
{
	const unit_map::const_iterator i = units.find(loc);
	if(i == units.end()) {
		std::cerr << "unit not found\n";
		return;
	}

	if(i->second.side() < 1 || i->second.side() > teams.size()) {
		return;
	}

	routes[loc].move_left = i->second.movement_left();
	find_routes(map,units,i->second,loc,
		i->second.movement_left(),routes,teams,force_ignore_zoc,
		allow_teleport,additional_turns,true,viewing_team,
		see_all, ignore_units);
}

int route_turns_to_complete(const unit& u, paths::route& rt, const team &viewing_team,
							const unit_map& units, const std::vector<team>& teams, const gamemap& map)
{
	if(rt.steps.empty())
		return 0;

	int turns = 0;
	int movement = u.movement_left();
	const team& unit_team = teams[u.side()-1];
	bool zoc = false;

	for (std::vector<map_location>::const_iterator i = rt.steps.begin();
		i !=rt.steps.end(); i++) {
		bool last_step = (i+1 == rt.steps.end());

		// move_cost of the next step is irrelevant for the last step
		assert(last_step || map.on_board(*(i+1)));
		const int move_cost = last_step ? 0 : u.movement_cost(map[*(i+1)]);

		if (last_step || zoc || move_cost > movement) {
			// check if we stop an a village and so maybe capture it
			// if it's an enemy unit and a fogged village, we assume a capture
			// (if he already owns it, we can't know that)
			// if it's not an enemy, we can always know if he owns the village
			bool capture = map.is_village(*i) && ( !unit_team.owns_village(*i)
				 || (viewing_team.is_enemy(u.side()) && viewing_team.fogged(*i)) );

			++turns;

			bool invisible = u.invisible(*i,units,teams,false);

			rt.waypoints[*i] = paths::route::waypoint(turns, zoc, capture, invisible);

			if (last_step) break; // finished and we used dummy move_cost

			movement = u.total_movement();
			if(move_cost > movement) {
				return -1; //we can't reach destination
			}
		}

		zoc = enemy_zoc(map,units,teams, *(i+1), viewing_team,u.side())
					&& !u.get_ability_bool("skirmisher", *(i+1));

		if (zoc) {
			movement = 0;
		} else {
			movement -= move_cost;
		}
	}
	if (turns == 1)
		rt.move_left = movement;
	else
		rt.move_left = 0;

	return turns;
}


shortest_path_calculator::shortest_path_calculator(unit const &u, team const &t,
		unit_map const &units, std::vector<team> const &teams, gamemap const &map,
		bool ignore_unit, bool ignore_defense)
	: unit_(u), viewing_team_(t), units_(units), teams_(teams), map_(map),
	  movement_left_(unit_.movement_left()),
	  total_movement_(unit_.total_movement()),
	  ignore_unit_(ignore_unit), ignore_defense_(ignore_defense)
{}

double shortest_path_calculator::cost(const map_location& /*src*/,const map_location& loc, const double so_far) const
{
	assert(map_.on_board(loc));

	// loc is shrouded, consider it impassable
	// NOTE: This is why AI must avoid to use shroud
	if (viewing_team_.shrouded(loc))
		return getNoPathValue();

	const t_translation::t_terrain terrain = map_[loc];
	int const base_cost = unit_.movement_cost(terrain);
	// Pathfinding heuristic: the cost must be at least 1
	VALIDATE(base_cost >= 1, _("Terrain with a movement cost less than 1 encountered."));

	// costs more than the total movement of the unit, impassbale
	if (total_movement_ < base_cost)
		return getNoPathValue();

	int other_unit_subcost = 0;
	if (!ignore_unit_) {
		unit_map::const_iterator
			other_unit = find_visible_unit(units_, loc, map_, teams_, viewing_team_);

		// We can't traverse visible enemy and we also prefer empty hexes
		// (less blocking in multi-turn moves and better when exploring fog,
		// because we can't stop on a friend)

		if (other_unit != units_.end()) {
			if (teams_[unit_.side()-1].is_enemy(other_unit->second.side()))
				return getNoPathValue();
			else
				// This value will be used with the defense_subcost (see below)
				// The 1 here means: consider occupied hex as a -1% defense
				// (less important than 10% defense because friends may move)
				other_unit_subcost = 1;
		}
	}

	// Compute how many movement points are left in the game turn
	// needed to reach the previous hex.
	// total_movement_ is not zero, thanks to the pathfinding heuristic
	int remaining_movement = movement_left_ - static_cast<int>(so_far);
	if (remaining_movement < 0)
		remaining_movement = total_movement_ - (-remaining_movement) % total_movement_;

	// we will always pay the terrain movement cost.
	int move_cost = base_cost;

	// Supposing we had 2 movement left, and wanted to move onto a hex
	// which takes 3 movement, it's going to cost us 5 movement in total,
	// since we sacrifice this turn's movement. So check that.
	bool need_new_turn = base_cost > remaining_movement;

	// and if it happens, all remaining movements will be lost waiting the turn's end
	if (need_new_turn)
		move_cost += remaining_movement;

	if (!ignore_unit_ && enemy_zoc(map_,units_,teams_, loc, viewing_team_, unit_.side())
			&& !unit_.get_ability_bool("skirmisher", loc)) {
		// the ZoC cost all remaining movements, but if we already use them
		// in the sacrified turn, we will spend all our fresh total movement
		move_cost += need_new_turn ? total_movement_ : remaining_movement;
	}

	// We will add a tiny cost based on terrain defense, so the pathfinding
	// will prefer good terrains between 2 with the same MP cost
	// Keep in mind that defense_modifier is inverted (= 100 - defense%)
	const int defense_subcost = ignore_defense_ ? 0 : unit_.defense_modifier(terrain);

	// We divide subcosts by 100 * 100, because defense is 100-based and
	// we don't want any impact on move cost for less then 100-steps path
	// (even ~200 since mean defense is around ~50%)
	return move_cost + (defense_subcost + other_unit_subcost) / 10000.0;
}

emergency_path_calculator::emergency_path_calculator(const unit& u, const gamemap& map)
	: unit_(u), map_(map)
{}

double emergency_path_calculator::cost(const map_location&,const map_location& loc, const double) const
{
	assert(map_.on_board(loc));

	return unit_.movement_cost(map_[loc]);
}

int emergency_path_calculator::get_max_cost() const
{
	return unit_.movement_left();
}

dummy_path_calculator::dummy_path_calculator(const unit& u, const gamemap&)
{}

double dummy_path_calculator::cost(const map_location&, const map_location&, const double) const
{
	return 0.0;
}

int dummy_path_calculator::get_max_cost() const
{
	return 0;
}

std::ostream& operator << (std::ostream& outstream, const paths::route& rt) {
	outstream << "\n[route]\n\tsteps=\"";
	bool first_loop = true;
	foreach(map_location const& loc, rt.steps) {
		if(first_loop) {
			first_loop = false;
		} else {
			outstream << "->";
		}
		outstream << '(' << loc << ')';
	}
	outstream << "\"\n\tmove_left=\"" << rt.move_left << "\"\n";
	typedef std::pair<map_location, paths::route::waypoint> loc_waypoint;
	foreach(loc_waypoint const& lw, rt.waypoints) {
		outstream << "\t[waypoint]\n\t\tx,y=\"" << lw.first
		<< "\"\n\t\tturns=\"" << lw.second.turns
		<< "\"\n\t\tzoc=\"" << (lw.second.zoc?"yes":"no")
		<< "\"\n\t\tcapture=\"" << (lw.second.capture?"yes":"no")
		<< "\"\n\t\tinvisible=\"" << (lw.second.invisible?"yes":"no")
		<< "\"\n\t[/waypoint]\n";
	}
	outstream << "[/route]";
	return outstream;
}
