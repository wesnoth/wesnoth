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
#include "map.hpp"
#include "unit.hpp"
#include "unit_types.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <list>
#include <vector>

//this module contains various pathfinding functions and utilities.

//function which, given a location, will place all adjacent locations in
//res. res must point to an array of 6 location objects.
void get_adjacent_tiles(const gamemap::location& a, gamemap::location* res);

//a convenient type for storing a list of tiles adjacent to a certain tile
typedef util::array<gamemap::location,6> adjacent_tiles_array;

//function which, given a location, will find all tiles within 'radius' of that tile
void get_tiles_radius(const gamemap::location& a, size_t radius, std::set<gamemap::location>& res);

//function which tells if two locations are adjacent.
bool tiles_adjacent(const gamemap::location& a, const gamemap::location& b);

//function which gives the number of hexes between two tiles (i.e. the minimum
//number of hexes that have to be traversed to get from one hex to the other)
size_t distance_between(const gamemap::location& a, const gamemap::location& b);

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

//function which, given a unit and a route the unit can move on, will
//return the number of turns it will take the unit to traverse the route.
int route_turns_to_complete(const unit& u, const gamemap& map,
                            const paths::route& rt);

struct shortest_path_calculator
{
	shortest_path_calculator(const unit& u, const team& t,
	                         const unit_map& units, 
									 const std::vector<team>& teams,
									 const gamemap& map,
									 const gamestatus& status);
	double cost(const gamemap::location& loc, double so_far) const;

private:
	const unit& unit_;
	const team& team_;
	const unit_map& units_;
	const std::vector<team>& teams_;
	const gamemap& map_;
	const gamestatus& status_;
};

namespace detail {
struct node {
	static double heuristic(const gamemap::location& src,
	                        const gamemap::location& dst) {
		return distance_between(src,dst);
	}

	node(const gamemap::location& pos, const gamemap::location& dst,
		double cost, const gamemap::location& parent,
	     const std::set<gamemap::location>* teleports)
	    : loc(pos), parent(parent), g(cost), h(heuristic(pos,dst))
	{

		//if there are teleport locations, correct the heuristic to
		//take them into account
		if(teleports != NULL) {
			double srch = h, dsth = h;
			std::set<gamemap::location>::const_iterator i;
			for(i = teleports->begin(); i != teleports->end(); ++i) {
				const double new_srch = heuristic(pos,*i);
				const double new_dsth = heuristic(*i,dst);
				if(new_srch < srch) {
					srch = new_srch;
				}

				if(new_dsth < dsth) {
					dsth = new_dsth;
				}
			}

			if(srch + dsth + 1.0 < h) {
				h = srch + dsth + 1.0;
			}
		}

		f = g + h;
	}

	gamemap::location loc, parent;
	double g, h, f;
};

}

template<typename T>
paths::route a_star_search(const gamemap::location& src,
                           const gamemap::location& dst, double stop_at, T obj,
                           const std::set<gamemap::location>* teleports=NULL)
{
	std::cerr  << "a* search: " << src.x << ", " << src.y << " -> " << dst.x << ", " << dst.y << "\n";
	using namespace detail;
	typedef gamemap::location location;

	typedef std::map<location,node> list_map;
	typedef std::pair<location,node> list_type;

	std::multimap<double,location> open_list_ordered;
	list_map open_list, closed_list;

	open_list.insert(list_type(src,node(src,dst,0.0,location(),teleports)));
	open_list_ordered.insert(std::pair<double,location>(0.0,src));

	std::vector<location> locs;
	while(!open_list.empty()) {

		assert(open_list.size() == open_list_ordered.size());

		const list_map::iterator lowest_in_open = open_list.find(open_list_ordered.begin()->second);
		assert(lowest_in_open != open_list.end());

		//move the lowest element from the open list to the closed list
		closed_list.erase(lowest_in_open->first);
		const list_map::iterator lowest = closed_list.insert(*lowest_in_open).first;

		open_list.erase(lowest_in_open);
		open_list_ordered.erase(open_list_ordered.begin());

		//find nodes we can go to from this node
		locs.resize(6);
		get_adjacent_tiles(lowest->second.loc,&locs[0]);
		if(teleports != NULL && teleports->count(lowest->second.loc) != 0) {
			std::copy(teleports->begin(),teleports->end(),std::back_inserter(locs));
		}

		for(size_t j = 0; j != locs.size(); ++j) {

			//if we have found a solution
			if(locs[j] == dst) {
				std::cerr << "found solution; calculating it...\n";
				paths::route rt;

				for(location loc = lowest->second.loc; loc.valid(); ) {
					rt.steps.push_back(loc);
					list_map::const_iterator itor = open_list.find(loc);
					if(itor == open_list.end()) {
						itor = closed_list.find(loc);
						assert(itor != closed_list.end());
					}

					loc = itor->second.parent;
				}
				
				std::reverse(rt.steps.begin(),rt.steps.end());
				rt.steps.push_back(dst);
				rt.move_left = int(lowest->second.g + obj.cost(dst,lowest->second.g));

				assert(rt.steps.front() == src);

				std::cerr  << "exiting a* search (solved)\n";

				return rt;
			}

			list_map::iterator current_best = open_list.find(locs[j]);
			const bool in_open = current_best != open_list.end();
			if(!in_open) {
				current_best = closed_list.find(locs[j]);
			}

			if(current_best != closed_list.end() && current_best->second.g <= lowest->second.g+1.0) {
				continue;
			}

			const double new_cost = obj.cost(locs[j],lowest->second.g);

			const node nd(locs[j],dst,lowest->second.g+new_cost,
			              lowest->second.loc,teleports);

			if(current_best != closed_list.end()) {
				if(current_best->second.g <= nd.g) {
					continue;
				} else if(in_open) {
					typedef std::multimap<double,location>::iterator Itor;
					std::pair<Itor,Itor> itors = open_list_ordered.equal_range(current_best->second.f);
					while(itors.first != itors.second) {
						if(itors.first->second == current_best->second.loc) {
							open_list_ordered.erase(itors.first);
							break;
						}
						++itors.first;
					}

					open_list.erase(current_best);
				} else {
					closed_list.erase(current_best);
				}
			}

			if(nd.f < stop_at) {
				open_list.insert(list_type(nd.loc,nd));
				open_list_ordered.insert(std::pair<double,location>(nd.f,nd.loc));
			} else {
				closed_list.insert(list_type(nd.loc,nd));
			}
		}
	}

	std::cerr  << "aborted a* search\n";
	paths::route val;
	val.move_left = 100000;
	return val;
}

#endif
