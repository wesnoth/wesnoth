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
	   map[loc.x][loc.y] != gamemap::TOWER &&
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

namespace {

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

void find_routes(const gamemap& map, const game_data& gamedata,
				 const std::map<gamemap::location,unit>& units,
				 const unit& u,
				 const gamemap::location& loc,
				 int move_left,
				 std::map<gamemap::location,paths::route>& routes,
				 std::vector<team>& teams,
				 bool ignore_zocs, bool allow_teleport)
{
	//find adjacent tiles
	std::vector<gamemap::location> locs(6);
	get_adjacent_tiles(loc,&locs[0]);

	//check for teleporting units
	if(allow_teleport && map[loc.x][loc.y] == gamemap::TOWER) {
		const std::vector<gamemap::location>& towers = map.towers();

		//if we are on a tower, see all friendly towers that we can
		//teleport to
		for(std::vector<gamemap::location>::const_iterator t = towers.begin();
		    t != towers.end(); ++t) {
			if(!teams[u.side()-1].owns_tower(*t) ||
			   units.find(*t) != units.end())
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
		if(unit_it != units.end() && unit_it->second.side() != u.side())
			continue;

		//find the terrain of the adjacent location
		const gamemap::TERRAIN terrain = map[currentloc.x][currentloc.y];

		//find the movement cost of this type onto the terrain
		const int move_cost = u.movement_cost(map,terrain);
		if(move_cost <= move_left) {
			const std::map<gamemap::location,paths::route>::const_iterator
					rtit = routes.find(currentloc);

			//if a better route to that tile has already been found
			if(rtit != routes.end() &&
			   rtit->second.move_left >= move_left - move_cost)
				continue;

			const bool zoc = enemy_zoc(map,units,currentloc,
			                           teams[u.side()-1],u.side()) &&
			                 !ignore_zocs;
			paths::route new_route = routes[loc];
			new_route.steps.push_back(loc);
			new_route.move_left = zoc ? 0 : move_left - move_cost;
			routes[currentloc] = new_route;

			if(new_route.move_left > 0) {
				find_routes(map,gamedata,units,u,currentloc,
				            new_route.move_left,routes,teams,ignore_zocs,
							allow_teleport);
			}
		}
	}
}

} //end anon namespace

paths::paths(const gamemap& map, const game_data& gamedata,
             const std::map<gamemap::location,unit>& units,
             const gamemap::location& loc,
			 std::vector<team>& teams,
			 bool ignore_zocs, bool allow_teleport)
{
	const std::map<gamemap::location,unit>::const_iterator i = units.find(loc);
	if(i == units.end()) {
		std::cerr << "unit not found\n";
		return;
	}

	routes[loc].move_left = i->second.movement_left();
	find_routes(map,gamedata,units,i->second,loc,
	            i->second.movement_left(),routes,teams,
				ignore_zocs,allow_teleport);

	if(i->second.can_attack()) {
		gamemap::location adjacent[6];
		get_adjacent_tiles(loc,adjacent);
		for(int j = 0; j != 6; ++j) {
			const std::map<gamemap::location,unit>::const_iterator enemy =
					units.find(adjacent[j]);
			if(enemy != units.end() &&
			   enemy->second.side() != i->second.side() &&
			   teams[i->second.side()-1].is_enemy(enemy->second.side())) {
				route new_route;
				new_route.move_left = -1;
				routes.insert(std::pair<gamemap::location,route>(
										          adjacent[j],new_route));
			}
		}
	}
}

