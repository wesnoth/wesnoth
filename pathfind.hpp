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

	std::map<gamemap::location,route> routes;
};

namespace detail {
struct node {
	node(const gamemap::location& pos, const gamemap::location& dst,
	     double cost, node* parent)
	    : parent(parent), loc(pos), g(cost),
	      h(sqrt(pow(abs(dst.x-pos.x),2) + pow(abs(dst.y-pos.y),2)))
	{
		f = g + h;
	}

	node* parent;
	gamemap::location loc;
	double g, h, f;
};

}

template<typename T>
paths::route a_star_search(const gamemap::location& src,
                           const gamemap::location& dst, double stop_at, T obj)
{
	std::cout << "a* search: " << src.x << ", " << src.y << " - " << dst.x << ", " << dst.y << "\n";
	using namespace detail;
	typedef gamemap::location location;
	std::list<node> open_list, closed_list;

	open_list.push_back(node(src,dst,0.0,NULL));

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

		//std::cerr << "processing " << (lowest->loc.x+1) << "," << (lowest->loc.y+1) << " with cost = " << lowest->g << " (known) + " << lowest->h << " (estimated) = " << lowest->f << "\n";

		//move the lowest element from the open list to the closed list
		closed_list.splice(closed_list.begin(),open_list,lowest);

		//find nodes we can go to from this node
		location locs[6];
		get_adjacent_tiles(lowest->loc,locs);
		for(int j = 0; j != 6; ++j) {

			//if we have found a solution
			if(locs[j] == dst) {
				paths::route rt;
				for(node* n = &*lowest; n != NULL; n = n->parent) {
					rt.steps.push_back(n->loc);
				}

				std::reverse(rt.steps.begin(),rt.steps.end());
				rt.steps.push_back(dst);
				rt.move_left = int(lowest->f);

				std::cout << "exiting a* search (solved)\n";

				return rt;
			}

			const node nd(locs[j],dst,lowest->g+obj.cost(locs[j]),&*lowest);

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
