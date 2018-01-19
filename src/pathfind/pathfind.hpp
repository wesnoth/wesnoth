/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * This module contains various pathfinding functions and utilities.
 */

#pragma once

class gamemap;
class team;
class unit;
class unit_type;
class game_board;
#include "map/location.hpp"
#include "movetype.hpp"

#include <vector>
#include <map>
#include <set>

namespace pathfind {

class teleport_map;

enum VACANT_TILE_TYPE { VACANT_CASTLE, VACANT_ANY };

/// Function that will find a location on the board that is as near
/// to @a loc as possible, but which is unoccupied by any units.
map_location find_vacant_tile(const map_location& loc,
                              VACANT_TILE_TYPE vacancy=VACANT_ANY,
                              const unit* pass_check=nullptr,
                              const team* shroud_check=nullptr,
                              const game_board* board=nullptr);
/// Wrapper for find_vacant_tile() when looking for a vacant castle tile
/// near a leader.
map_location find_vacant_castle(const unit & leader);

/** Determines if a given location is in an enemy zone of control. */
bool enemy_zoc(team const &current_team, map_location const &loc,
               team const &viewing_team, bool see_all=false);


struct cost_calculator
{
	cost_calculator() {}

	virtual double cost(const map_location& loc, const double so_far) const = 0;
	virtual ~cost_calculator() {}

	static double getNoPathValue() { return (42424242.0); }
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

	/// Construct a list of paths for the specified unit.
	paths(const unit& u,
	      bool force_ignore_zocs, bool allow_teleport,
	      const team &viewing_team, int additional_turns = 0,
	      bool see_all = false, bool ignore_units = false);
	/// Virtual destructor (default processing).
	virtual ~paths();

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

/**
 * A refinement of paths for use when calculating vision.
 */
struct vision_path : public paths
{
	/// Construct a list of seen hexes for a unit.
	vision_path(const unit& viewer, map_location const &loc,
	            const std::map<map_location, int>& jamming_map);
	vision_path(const movetype::terrain_costs & view_costs, bool slowed,
	            int sight_range, const map_location & loc,
	            const std::map<map_location, int>& jamming_map);
	virtual ~vision_path();

	/// The edges are the non-destination hexes bordering the destinations.
	std::set<map_location> edges;
};

/**
 * A refinement of paths for use when calculating jamming.
 */
struct jamming_path : public paths
{
	/// Construct a list of jammed hexes for a unit.
	jamming_path(const unit& jammer, map_location const &loc);
	virtual ~jamming_path();
};


/** Structure which holds a single route between one location and another. */
struct plain_route
{
	plain_route() : steps(), move_cost(0) {}
	std::vector<map_location> steps;
	/** Movement cost for reaching the end of the route. */
	int move_cost;
};

/** Structure which holds a single route and marks for special events. */
struct marked_route
{
	marked_route()
		: route()
		, steps(route.steps)
		, move_cost(route.move_cost)
		, marks()
	{
	}

	marked_route(const marked_route& rhs)
		: route(rhs.route)
		, steps(route.steps)
		, move_cost(route.move_cost)
		, marks(rhs.marks)
	{
	}

	marked_route& operator=(const marked_route& rhs)
	{
		this->route = rhs.route;
		this->steps = this->route.steps;
		this->move_cost = this->route.move_cost;
		this->marks = rhs.marks;
		return *this;
	}

	struct mark
	{
		mark(int turns_number = 0, bool in_zoc = false,
				bool do_capture = false, bool is_invisible = false)
			: turns(turns_number), zoc(in_zoc),
			  capture(do_capture), invisible(is_invisible) {}
		int turns;
		bool zoc;
		bool capture;
		bool invisible;

		bool operator==(const mark& m) const {
			return turns == m.turns && zoc == m.zoc && capture == m.capture && invisible == m.invisible;
		}
	};
	typedef std::map<map_location, mark> mark_map;
	plain_route route;

	//make steps and move_cost of the underlying plain_route directly accessible
	std::vector<map_location>& steps;
	int& move_cost;

	mark_map marks;
};

plain_route a_star_search(map_location const &src, map_location const &dst,
		double stop_at, const cost_calculator& costCalculator,
		const size_t parWidth, const size_t parHeight,
		const teleport_map* teleports = nullptr, bool border = false);

/**
 * Add marks on a route @a rt assuming that the unit located at the first hex of
 * rt travels along it.
 */
marked_route mark_route(const plain_route &rt);

struct shortest_path_calculator : cost_calculator
{
	shortest_path_calculator(const unit& u, const team& t,
		const std::vector<team> &teams, const gamemap &map,
		bool ignore_unit = false, bool ignore_defense_ = false,
		bool see_all = false);
	virtual double cost(const map_location& loc, const double so_far) const;

private:
	unit const &unit_;
	team const &viewing_team_;
	std::vector<team> const &teams_;
	gamemap const &map_;
	const int movement_left_;
	const int total_movement_;
	bool const ignore_unit_;
	bool const ignore_defense_;
	bool see_all_;
};

struct move_type_path_calculator : cost_calculator
{
	move_type_path_calculator(const movetype& mt, int movement_left, int total_movement, const team& t, const gamemap& map);
	virtual double cost(const map_location& loc, const double so_far) const;

private:
	const movetype &movement_type_;
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

/**
 * Structure which uses find_routes() to build a cost map
 * This maps each hex to a the movements a unit will need to reach
 * this hex.
 * Can be used commutative by calling add_unit() multiple times.
 */
struct full_cost_map
{
	// "normal" constructor
	full_cost_map(const unit& u, bool force_ignore_zoc,
			bool allow_teleport, const team &viewing_team,
			bool see_all=true, bool ignore_units=true);

	// constructor for work with add_unit()
	// same as "normal" constructor. Just without unit
	full_cost_map(bool force_ignore_zoc,
			bool allow_teleport, const team &viewing_team,
			bool see_all=true, bool ignore_units=true);

	void add_unit(const unit& u, bool use_max_moves=true);
	void add_unit(const map_location& origin, const unit_type* const unit_type, int side);
	int get_cost_at(int x, int y) const;
	std::pair<int, int> get_pair_at(int x, int y) const;
	double get_average_cost_at(int x, int y) const;
	virtual ~full_cost_map()
	{
	}

	// This is a vector of pairs
	// Every hex has an entry.
	// The first int is the accumulated cost for one or multiple units
	// It is -1 when no unit can reach this hex.
	// The second int is how many units can reach this hex.
	// (For some units some hexes or even a whole regions are unreachable)
	// To calculate a *average cost map* it is recommended to divide first/second.
	std::vector<std::pair<int, int> > cost_map;

private:
	const bool force_ignore_zoc_;
	const bool allow_teleport_;
	const team &viewing_team_;
	const bool see_all_;
	const bool ignore_units_;
};
}
