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
#ifndef PATHFIND_H_INCLUDED
#define PATHFIND_H_INCLUDED

#include "array.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathutils.hpp"
#include "unit.hpp"

#include <map>
#include <list>
#include <set>
#include <vector>

//this module contains various pathfinding functions and utilities.

//a convenient type for storing a list of tiles adjacent to a certain tile
typedef util::array<gamemap::location,6> adjacent_tiles_array;

//function which, given a location, will find all tiles within 'radius' of that tile
void get_tiles_radius(const gamemap::location& a, size_t radius, std::set<gamemap::location>& res);

//function which, given a set of locations, will find all tiles within 'radius' of those tiles
void get_tiles_radius(const gamemap& map, const std::vector<gamemap::location>& locs, size_t radius,
	std::set<gamemap::location>& res);

enum VACANT_TILE_TYPE { VACANT_CASTLE, VACANT_ANY };

//function which will find a location on the board that is as near to loc as
//possible, but which is unoccupied by any units. If terrain is not 0, then
//the location found must be of the given terrain type, and must have a path
//of that terrain type to loc.
//
//if no valid location can be found, it will return a null location.
gamemap::location find_vacant_tile(const gamemap& map,
                                   const std::map<gamemap::location,unit>& un,
                                   const gamemap::location& loc,
                                   VACANT_TILE_TYPE vacancy=VACANT_ANY);

//function which determines if a given location is an enemy zone of control
bool enemy_zoc(const gamemap& map,const gamestatus& status, 
		         const std::map<gamemap::location,unit>& units,
		         const std::vector<team>& teams,
               const gamemap::location& loc,const team& current_team,int side);

struct cost_calculator
{
	virtual double cost(const gamemap::location& loc, const double so_far, const bool isDst) const = 0;
	virtual ~cost_calculator() {}
	inline double getNoPathValue(void) const { return (42424242.0); }
};

//object which contains all the possible locations a unit can move to, with
//associated best routes to those locations.
struct paths
{	
	paths() {}

	//construct a list of paths for the unit at loc.
	//ignore_zocs: determines whether unit has to stop upon entering an ezoc
	//allow_teleport: indicates whether unit teleports between villages
	//additional_turns: if 0, paths for how far the unit can move this turn
	//will be calculated. If 1, paths for how far the unit can move by the
	//end of next turn will be calculated, and so forth.
	paths(const gamemap& map, const gamestatus& status, 
			const game_data& gamedata,
	      const std::map<gamemap::location,unit>& units,
	      const gamemap::location& loc, std::vector<team>& teams,
		  bool ignore_zocs, bool allow_teleport, int additional_turns=0);

	//structure which holds a single route between one location and another.
	struct route
	{
		route() : move_left(0) {}
		std::vector<gamemap::location> steps;
		int move_left; //movement unit will have left at end of the route.
	};

	typedef std::map<gamemap::location,route> routes_map;
	routes_map routes;		
};

paths::route a_star_search(gamemap::location const &src, gamemap::location const &dst,
													 double stop_at, cost_calculator const *costCalculator,
													 const size_t parWidth, const size_t parHeight,
													 std::set<gamemap::location> const *teleports = NULL);

//function which, given a unit and a route the unit can move on, will
//return the number of turns it will take the unit to traverse the route.
int route_turns_to_complete(const unit& u, const gamemap& map,
                            const paths::route& rt);

struct shortest_path_calculator : cost_calculator
{
	shortest_path_calculator(const unit& u, const team& t,
	                         const unit_map& units, const std::vector<team>& teams,
	                         const gamemap& map, const gamestatus& status);
	virtual double cost(const gamemap::location& loc, const double so_far, const bool isDst) const;

private:
	const unit& unit_;
	const team& team_;
	const unit_map& units_;
	const std::vector<team>& teams_;
	const gamemap& map_;
	const gamestatus& status_;
};

#endif
