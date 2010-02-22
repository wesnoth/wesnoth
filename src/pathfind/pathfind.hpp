/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file pathfind.hpp
 * This module contains various pathfinding functions and utilities.
 */

#ifndef PATHFIND_H_INCLUDED
#define PATHFIND_H_INCLUDED

class unit;
class unit_map;
class unit_movement_type;

#include "map_location.hpp"
#include "team.hpp"

#include <map>
#include <list>
#include <set>
#include <vector>
#include <functional>

namespace pathfind {

typedef std::set<map_location> teleport_map;

enum VACANT_TILE_TYPE { VACANT_CASTLE, VACANT_ANY };

/**
 * Function which will find a location on the board that is
 * as near to loc as possible, but which is unoccupied by any units.
 * If no valid location can be found, it will return a null location.
 */
map_location find_vacant_tile(const gamemap& map,
                                   const unit_map& un,
                                   const map_location& loc,
			     	   VACANT_TILE_TYPE vacancy=VACANT_ANY,
                                   const unit* pass_check=NULL);

/** Function which determines if a given location is in an enemy zone of control. */
bool enemy_zoc(unit_map const &units,
               std::vector<team> const &teams, map_location const &loc,
               team const &viewing_team, int side, bool see_all=false);

std::set<map_location> get_teleport_locations(const unit &u,
	const unit_map &units, const team &viewing_team,
	bool see_all = false, bool ignore_units = false);

struct cost_calculator
{
	cost_calculator() {}

	virtual double cost(const map_location& loc, const double so_far) const = 0;
	virtual ~cost_calculator() {}

	inline double getNoPathValue() const { return (42424242.0); }
};

/**
 * Object which contains all the possible locations a unit can move to,
 * with associated best routes to those locations.
 */
struct paths
{
	paths()
		: destinations()
	{
	}

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
	      map_location const &loc, std::vector<team> const &teams,
	      bool force_ignore_zocs,bool allow_teleport,
		 const team &viewing_team,int additional_turns = 0,
		 bool see_all = false, bool ignore_units = false);

	struct step
	{
		map_location curr, prev;
		int move_left;
	};

	/** Ordered vector of possible destinations. */
	struct dest_vect : std::vector<step>
	{
		const_iterator find(const map_location &) const;
		bool contains(const map_location &) const;
		void insert(const map_location &);
		std::vector<map_location> get_path(const const_iterator &) const;
	};
	dest_vect destinations;
};

/** Structure which holds a single route and marks for special events. */
struct marked_route
{
	marked_route()
		: steps()
		, marks()
	{
	}

	struct mark
	{
		mark(int turns_number = 0, bool pass = false,  bool in_zoc = false,
		         bool do_capture = false, bool is_invisible = false)
			: turns(turns_number), pass_here(pass), zoc(in_zoc),
			  capture(do_capture), invisible(is_invisible) {}
		int turns;
		bool pass_here;
		bool zoc;
		bool capture;
		bool invisible;
	};
	typedef std::map<map_location, mark> mark_map;
	std::vector<map_location> steps;
	mark_map marks;
};

/** Structure which holds a single route between one location and another. */
struct plain_route
{
	plain_route() : steps(), move_cost(0) {}
	std::vector<map_location> steps;
	/** Movement cost for reaching the end of the route. */
	int move_cost;
};

plain_route a_star_search(map_location const &src, map_location const &dst,
                           double stop_at, const cost_calculator* costCalculator,
                           const size_t parWidth, const size_t parHeight,
                           std::set<map_location> const *teleports = NULL);

/**
 * Add marks on a route @a rt assuming that a @unit u travels along it.
 */
marked_route mark_route(const plain_route &rt,
	const std::vector<map_location>& waypoints, const unit &u,
	const team &viewing_team, const unit_map &units,
	const std::vector<team> &teams, const gamemap &map);

struct shortest_path_calculator : cost_calculator
{
	shortest_path_calculator(const unit& u, const team& t, const unit_map& units,
		const std::vector<team> &teams, const gamemap &map,
		bool ignore_unit = false, bool ignore_defense_ = false,
		bool see_all = false);
	virtual double cost(const map_location& loc, const double so_far) const;

private:
	unit const &unit_;
	team const &viewing_team_;
	unit_map const &units_;
	std::vector<team> const &teams_;
	gamemap const &map_;
	int const movement_left_;
	int const total_movement_;
	bool const ignore_unit_;
	bool const ignore_defense_;
	bool see_all_;
};

struct move_type_path_calculator : cost_calculator
{
	move_type_path_calculator(const unit_movement_type& mt, int movement_left, int total_movement, const team& t, const gamemap& map);
	virtual double cost(const map_location& loc, const double so_far) const;

private:
	const unit_movement_type &movement_type_;
	const int movement_left_;
	const int total_movement_;
	team const &viewing_team_;
	gamemap const &map_;
};

/**
 * Function which only uses terrain, ignoring shroud, enemies, etc.
 * Required by move_unit_fake if the normal path fails.
 */
struct emergency_path_calculator : cost_calculator
{
	emergency_path_calculator(const unit& u, const gamemap& map);
	virtual double cost(const map_location& loc, const double so_far) const;

private:
	unit const &unit_;
	gamemap const &map_;
};

/**
 * Function which doesn't take anything into account. Used by
 * move_unit_fake for the last-chance case.
 */
struct dummy_path_calculator : cost_calculator
{
	dummy_path_calculator(const unit& u, const gamemap& map);
	virtual double cost(const map_location& loc, const double so_far) const;

};

}

#endif
