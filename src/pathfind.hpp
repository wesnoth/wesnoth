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

void get_adjacent_tiles(const gamemap::location& a, gamemap::location* res);
bool tiles_adjacent(const gamemap::location& a, const gamemap::location& b);

gamemap::location find_vacant_tile(const gamemap& map,
                                   const std::map<gamemap::location,unit>& un,
                                   const gamemap::location& loc,
                                   gamemap::TERRAIN terrain=0);

bool enemy_zoc(const gamemap& map,const std::map<gamemap::location,unit>& units,
               const gamemap::location& loc,const team& current_team,int side);

struct paths
{
	paths() {}
	paths(const gamemap& map, const game_data& gamedata,
	      const std::map<gamemap::location,unit>& units,
	      const gamemap::location& loc, std::vector<team>& teams,
		  bool ignore_zocs, bool allow_teleport);

	struct route
	{
		route() : move_left(0) {}
		std::vector<gamemap::location> steps;
		int move_left;
	};

	typedef std::map<gamemap::location,route> routes_map;
	routes_map routes;
};

struct shortest_path_calculator
{
	shortest_path_calculator(const unit& u, const team& t,
	                         const unit_map& units, const gamemap& map);
	double cost(const gamemap::location& loc, double so_far) const;

private:
	const unit& unit_;
	const team& team_;
	const unit_map& units_;
	const gamemap& map_;
};

namespace detail {
struct node {
	static double heuristic(const gamemap::location& src,
	                        const gamemap::location& dst) {
		return sqrt(pow(abs(dst.x-src.x),2) + pow(abs(dst.y-src.y),2));
	}


	node(const gamemap::location& pos, const gamemap::location& dst,
	     double cost, node* parent,
	     const std::set<gamemap::location>* teleports)
	    : parent(parent), loc(pos), g(cost), h(heuristic(pos,dst))
	{

		//if there are teleport locations, correct the heuristic to
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

	node* parent;
	gamemap::location loc;
	double g, h, f;
};

}

template<typename T>
paths::route a_star_search(const gamemap::location& src,
                           const gamemap::location& dst, double stop_at, T obj,
                           const std::set<gamemap::location>* teleports=NULL)
{
	std::cout << "a* search: " << src.x << ", " << src.y << " - " << dst.x << ", " << dst.y << "\n";
	using namespace detail;
	typedef gamemap::location location;
	std::list<node> open_list, closed_list;

	open_list.push_back(node(src,dst,0.0,NULL,teleports));

	while(!open_list.empty()) {

		//find the lowest estimated cost node on the open list
		std::list<node>::iterator lowest = open_list.end(), i;
		for(i = open_list.begin(); i != open_list.end(); ++i) {
			if(lowest == open_list.end() || i->f < lowest->f) {
				lowest = i;
			}
		}

		if(lowest->f > stop_at) {
			break;
		}

		//move the lowest element from the open list to the closed list
		closed_list.splice(closed_list.begin(),open_list,lowest);

		//find nodes we can go to from this node
		static std::vector<location> locs;
		locs.resize(6);
		get_adjacent_tiles(lowest->loc,&locs[0]);
		if(teleports != NULL && teleports->count(lowest->loc) != 0) {
			std::copy(teleports->begin(),teleports->end(),
			          std::back_inserter(locs));
		}

		for(size_t j = 0; j != locs.size(); ++j) {

			//if we have found a solution
			if(locs[j] == dst) {
				paths::route rt;
				for(node* n = &*lowest; n != NULL; n = n->parent) {
					rt.steps.push_back(n->loc);
				}

				std::reverse(rt.steps.begin(),rt.steps.end());
				rt.steps.push_back(dst);
				rt.move_left = int(lowest->f);

				assert(rt.steps.front() == src);

				std::cout << "exiting a* search (solved)\n";

				return rt;
			}

			const node nd(locs[j],dst,lowest->g+obj.cost(locs[j],lowest->g),
			              &*lowest,teleports);

			for(i = open_list.begin(); i != open_list.end(); ++i) {
				if(i->loc == nd.loc && i->f <= nd.f) {
					break;
				}
			}

			if(i != open_list.end()) {
				continue;
			}

			for(i = closed_list.begin(); i != closed_list.end(); ++i) {
				if(i != lowest && i->loc == nd.loc && i->f <= nd.f) {
					break;
				}
			}

			if(i != closed_list.end()) {
				continue;
			}

			open_list.push_back(nd);
		}
	}

	std::cout << "aborted a* search\n";
	paths::route val;
	val.move_left = 100000;
	return val;
}

#endif
