/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "game.hpp"
#include "pathfind.hpp"
#include "util.hpp"

#include <cmath>
#include <iostream>
#include <set>

namespace {
gamemap::location find_vacant(const gamemap& map,
                              const std::map<gamemap::location,unit>& units,
							  const gamemap::location& loc, int depth,
                              gamemap::TERRAIN terrain,
                              std::set<gamemap::location>& touched)
{
	if(touched.count(loc))
		return gamemap::location();

	touched.insert(loc);

	if(map.on_board(loc) && units.find(loc) == units.end() &&
	   map.underlying_terrain(map[loc.x][loc.y]) != gamemap::TOWER &&
	   (terrain == 0 || terrain == map[loc.x][loc.y])) {
		return loc;
	} else if(depth == 0) {
		return gamemap::location();
	} else {
		gamemap::location adj[6];
		get_adjacent_tiles(loc,adj);
		for(int i = 0; i != 6; ++i) {
			if(!map.on_board(adj[i]) ||
			   terrain != 0 && terrain != map[adj[i].x][adj[i].y])
				continue;

			const gamemap::location res =
			       find_vacant(map,units,adj[i],depth-1,terrain,touched);

			if(map.on_board(res))
				return res;
		}

		return gamemap::location();
	}
}

}

gamemap::location find_vacant_tile(const gamemap& map,
                                const std::map<gamemap::location,unit>& units,
                                const gamemap::location& loc,
                                gamemap::TERRAIN terrain)
{
	for(int i = 1; i != 50; ++i) {
		std::set<gamemap::location> touch;
		const gamemap::location res = find_vacant(map,units,loc,i,terrain,touch);
		if(map.on_board(res))
			return res;
	}

	return gamemap::location();
}

void get_adjacent_tiles(const gamemap::location& a, gamemap::location* res)
{
	res->x = a.x;
	res->y = a.y-1;
	++res;
	res->x = a.x+1;
	res->y = a.y - is_even(a.x);
	++res;
	res->x = a.x+1;
	res->y = a.y + is_odd(a.x);
	++res;
	res->x = a.x;
	res->y = a.y+1;
	++res;
	res->x = a.x-1;
	res->y = a.y + is_odd(a.x);
	++res;
	res->x = a.x-1;
	res->y = a.y - is_even(a.x);
}

bool tiles_adjacent(const gamemap::location& a, const gamemap::location& b)
{
	//two tiles are adjacent if y is different by 1, and x by 0, or if
	//x is different by 1 and y by 0, or if x and y are each different by 1,
	//and the x value of the hex with the greater y value is odd

	const int xdiff = abs(a.x - b.x);
	const int ydiff = abs(a.y - b.y);
	return ydiff == 1 && a.x == b.x || xdiff == 1 && a.y == b.y ||
	   xdiff == 1 && ydiff == 1 && (a.y > b.y ? (a.x%2) == 1 : (b.x%2) == 1);
}

size_t distance_between(const gamemap::location& a, const gamemap::location& b)
{
	const size_t hdistance = abs(a.x - b.x);

	const size_t vpenalty = (is_even(a.x) && is_odd(b.x) && a.y < b.y ||
	                         is_even(b.x) && is_odd(a.x) && b.y < a.y) ? 1:0;
	const size_t vdistance = abs(a.y - b.y) + vpenalty;
	const size_t vsavings = minimum<int>(vdistance,hdistance/2 + hdistance%2);

	return hdistance + vdistance - vsavings;
}

bool enemy_zoc(const gamemap& map,const std::map<gamemap::location,unit>& units,
               const gamemap::location& loc, const team& current_team, int side)
{
	gamemap::location locs[6];
	get_adjacent_tiles(loc,locs);
	for(int i = 0; i != 6; ++i) {
		const std::map<gamemap::location,unit>::const_iterator it
				= units.find(locs[i]);
		if(it != units.end() && it->second.side() != side &&
		   current_team.is_enemy(it->second.side()) &&
		   !it->second.invisible(map.underlying_terrain(
		                             map[it->first.x][it->first.y]))) {
			return true;
		}
	}

	return false;
}

namespace {

void find_routes(const gamemap& map, const game_data& gamedata,
				 const std::map<gamemap::location,unit>& units,
				 const unit& u,
				 const gamemap::location& loc,
				 int move_left,
				 std::map<gamemap::location,paths::route>& routes,
				 std::vector<team>& teams,
				 bool ignore_zocs, bool allow_teleport, int turns_left)
{
	team& current_team = teams[u.side()-1];

	//find adjacent tiles
	std::vector<gamemap::location> locs(6);
	get_adjacent_tiles(loc,&locs[0]);

	//check for teleporting units
	if(allow_teleport && map.underlying_terrain(map[loc.x][loc.y]) == gamemap::TOWER) {
		const std::vector<gamemap::location>& towers = map.towers();

		//if we are on a tower, see all friendly towers that we can
		//teleport to
		for(std::vector<gamemap::location>::const_iterator t = towers.begin();
		    t != towers.end(); ++t) {
			if(!current_team.owns_tower(*t))
				continue;

			locs.push_back(*t);
		}
	}

	//iterate over all adjacent tiles
	for(size_t i = 0; i != locs.size(); ++i) {
		const gamemap::location& currentloc = locs[i];

		//check if the adjacent location is off the board
		if(currentloc.x < 0 || currentloc.y < 0 ||
		   currentloc.x >= map.x() || currentloc.y >= map.y())
			continue;

		//see if the tile is on top of an enemy unit
		const std::map<gamemap::location,unit>::const_iterator unit_it =
				units.find(locs[i]);
		if(unit_it != units.end() &&
		   current_team.is_enemy(unit_it->second.side()))
			continue;

		//find the terrain of the adjacent location
		const gamemap::TERRAIN terrain = map[currentloc.x][currentloc.y];

		//find the movement cost of this type onto the terrain
		const int move_cost = u.movement_cost(map,terrain);
		if(move_cost <= move_left ||
		   turns_left > 0 && move_cost <= u.total_movement()) {
			int new_move_left = move_left - move_cost;
			int new_turns_left = turns_left;
			if(new_move_left < 0) {
				--new_turns_left;
				new_move_left = u.total_movement() - move_cost;
			}

			const int total_movement = new_turns_left*u.total_movement() +
			                           new_move_left;

			const std::map<gamemap::location,paths::route>::const_iterator
					rtit = routes.find(currentloc);

			//if a better route to that tile has already been found
			if(rtit != routes.end() &&
			   rtit->second.move_left >= total_movement)
				continue;

			const bool zoc = enemy_zoc(map,units,currentloc,
			                           current_team,u.side()) &&
			                 !ignore_zocs;
			paths::route new_route = routes[loc];
			new_route.steps.push_back(loc);

			const int zoc_move_left = zoc ? 0 : new_move_left;
			new_route.move_left = u.total_movement() * new_turns_left +
			                      zoc_move_left;
			routes[currentloc] = new_route;

			if(new_route.move_left > 0) {
				find_routes(map,gamedata,units,u,currentloc,
				            zoc_move_left,routes,teams,ignore_zocs,
							allow_teleport,new_turns_left);
			}
		}
	}
}

} //end anon namespace

paths::paths(const gamemap& map, const game_data& gamedata,
             const std::map<gamemap::location,unit>& units,
             const gamemap::location& loc,
			 std::vector<team>& teams,
			 bool ignore_zocs, bool allow_teleport, int additional_turns)
{
	const std::map<gamemap::location,unit>::const_iterator i = units.find(loc);
	if(i == units.end()) {
		std::cerr << "unit not found\n";
		return;
	}

	routes[loc].move_left = i->second.movement_left();
	find_routes(map,gamedata,units,i->second,loc,
	            i->second.movement_left(),routes,teams,
				ignore_zocs,allow_teleport,additional_turns);
}

int route_turns_to_complete(const unit& u, const gamemap& map,
                            const paths::route& rt)
{
	if(rt.steps.empty())
		return 0;

	int turns = 0, movement = u.movement_left();
	for(std::vector<gamemap::location>::const_iterator i = rt.steps.begin()+1;
	    i != rt.steps.end(); ++i) {
		assert(map.on_board(*i));
		const int move_cost = u.movement_cost(map,map[i->x][i->y]);
		movement -= move_cost;
		if(movement < 0) {
			++turns;
			movement = u.total_movement() - move_cost;
			if(movement < 0) {
				return -1;
			}
		}
	}

	return turns;
}


shortest_path_calculator::shortest_path_calculator(const unit& u, const team& t,
                                                   const unit_map& units,
                                                   const gamemap& map)
      : unit_(u), team_(t), units_(units), map_(map)
{
}

double shortest_path_calculator::cost(const gamemap::location& loc,
                                      double so_far) const
{
	if(!map_.on_board(loc) || team_.shrouded(loc.x,loc.y))
		return 100000.0;

	const unit_map::const_iterator enemy_unit = units_.find(loc);
	if(enemy_unit != units_.end() && team_.is_enemy(enemy_unit->second.side()))
		return 100000.0;

	if(unit_.type().is_skirmisher() == false) {
		gamemap::location adj[6];
		get_adjacent_tiles(loc,adj);

		for(size_t i = 0; i != 6; ++i) {
			const unit_map::const_iterator u = units_.find(adj[i]);
			if(u != units_.end() && team_.is_enemy(u->second.side()) && !team_.fogged(adj[i].x,adj[i].y)) {
				return 100000.0;
			}
		}
	}

	const double base_cost(
	     unit_.movement_cost(map_,map_[loc.x][loc.y]));

	//supposing we had 2 movement left, and wanted to move onto a hex which
	//takes 3 movement, it's going to cost us 5 movement in total, since we
	//sacrifice this turn's movement. Take that into account here.
	assert(so_far == double(int(so_far)));

	const int current_cost(static_cast<int>(so_far));

	const int starting_movement = unit_.movement_left();
	const int remaining_movement = current_cost <= starting_movement ?
	             starting_movement - current_cost :
	             (current_cost-starting_movement)%unit_.total_movement();

	const double additional_cost = int(base_cost) > remaining_movement ?
	                               double(remaining_movement) : double(0);

	return base_cost + additional_cost;
}
