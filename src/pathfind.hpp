/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file pathfind.hpp
//!

#ifndef PATHFIND_H_INCLUDED
#define PATHFIND_H_INCLUDED

class game_data;
class gamestatus;
class unit;
class unit_map;

#include "array.hpp"
#include "map.hpp"
#include "pathutils.hpp"
#include "team.hpp"

#include <map>
#include <list>
#include <set>
#include <vector>
#include <functional>

// This module contains various pathfinding functions and utilities.

//! A convenient type for storing a list of tiles adjacent to a certain tile.
typedef util::array<gamemap::location,6> adjacent_tiles_array;

class xy_pred : public std::unary_function<gamemap::location const&, bool>
{
public:
	virtual bool operator()(gamemap::location const&) = 0;
protected:
	virtual ~xy_pred() {}
};

//! Function which, given a location, will find all tiles within 'radius' of that tile
void get_tiles_radius(const gamemap::location& a, size_t radius,
					  std::set<gamemap::location>& res);

//! Function which, given a set of locations, will find all tiles within 'radius' of those tiles
void get_tiles_radius(const gamemap& map, const std::vector<gamemap::location>& locs, size_t radius,
					  std::set<gamemap::location>& res, xy_pred *pred=NULL);

enum VACANT_TILE_TYPE { VACANT_CASTLE, VACANT_ANY };

//! Function which will find a location on the board that is
//! as near to loc as possible, but which is unoccupied by any units.
//! If terrain is not 0, then the location found must be of the given terrain type,
//! and must have a path of that terrain type to loc.
//! If no valid location can be found, it will return a null location.
gamemap::location find_vacant_tile(const gamemap& map,
                                   const unit_map& un,
                                   const gamemap::location& loc,
                                   VACANT_TILE_TYPE vacancy=VACANT_ANY);

//! Function which determines if a given location is in an enemy zone of control.
bool enemy_zoc(gamemap const &map,
               unit_map const &units,
               std::vector<team> const &teams, gamemap::location const &loc,
               team const &viewing_team, unsigned int side, bool see_all=false);

struct cost_calculator
{
	virtual double cost(const gamemap::location& src, const gamemap::location& loc, const double so_far, const bool isDst) const = 0;
	virtual ~cost_calculator() {}
	inline double getNoPathValue(void) const { return (42424242.0); }
};

//! Object which contains all the possible locations a unit can move to,
//! with associated best routes to those locations.
struct paths
{
	paths() : routes() {}

	// Construct a list of paths for the unit at loc.
	// - force_ignore_zocs: find the path ignoring ZOC entirely,
	//                     if false, will use the unit on the loc's ability
	// - allow_teleport: indicates whether unit teleports between villages
	// - additional_turns: if 0, paths for how far the unit can move this turn will be calculated.
	//                     If 1, paths for how far the unit can move by the end of next turn
	//                     will be calculated, and so forth.
	// viewing_team is usually current team, except for Show Enemy Moves etc.
	paths(gamemap const &map,
	      unit_map const &units,
	      gamemap::location const &loc, std::vector<team> const &teams,
	      bool force_ignore_zocs,bool allow_teleport,
		 const team &viewing_team,int additional_turns = 0,
		 bool see_all = false, bool ignore_units = false);

	//! Structure which holds a single route between one location and another.
	struct route
	{
		route() : steps(), move_left(0), waypoints() {}
		std::vector<gamemap::location> steps;
		int move_left; // movement unit will have left at end of the route.
		struct waypoint
		{
			waypoint(int turns_number = 0, bool in_zoc = false,
					bool do_capture = false, bool is_invisible = false)
				: turns(turns_number), zoc(in_zoc),
					capture(do_capture), invisible(is_invisible) {}
			int turns;
			bool zoc;
			bool capture;
			bool invisible;
		};
		std::map<gamemap::location, waypoint> waypoints;
	};

	typedef std::map<gamemap::location,route> routes_map;
	routes_map routes;
};

paths::route a_star_search(gamemap::location const &src, gamemap::location const &dst,
                           double stop_at, cost_calculator const *costCalculator,
                           const size_t parWidth, const size_t parHeight,
                           std::set<gamemap::location> const *teleports = NULL);

//! Function which, given a unit and a route the unit can move on,
//! will return the number of turns it will take the unit to traverse the route.
//! adds "turn waypoints" to rt.turn_waypoints.
//! Note that "end of path" is also added.
int route_turns_to_complete(const unit& u, paths::route& rt, const team &viewing_team,
							const unit_map& units, const std::vector<team>& teams, const gamemap& map);

struct shortest_path_calculator : cost_calculator
{
	shortest_path_calculator(const unit& u, const team& t,
	                         const unit_map& units, const std::vector<team>& teams,
	                         const gamemap& map);
	virtual double cost(const gamemap::location& src, const gamemap::location& loc, const double so_far, const bool isDst) const;

private:
	unit const &unit_;
	team const &viewing_team_;
	unit_map const &units_;
	std::vector<team> const &teams_;
	gamemap const &map_;
	int const movement_left_;
	int const total_movement_;
};

//! Function which only uses terrain, ignoring shroud, enemies, etc.
//! Required by move_unit_fake if the normal path fails.
struct emergency_path_calculator : cost_calculator
{
	emergency_path_calculator(const unit& u, const gamemap& map);
	virtual double cost(const gamemap::location& src, const gamemap::location& loc, const double so_far, const bool isDst) const;

private:
	unit const &unit_;
	gamemap const &map_;
};

#endif
