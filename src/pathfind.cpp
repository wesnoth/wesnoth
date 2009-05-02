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

#include "pathfind.hpp"

#include "foreach.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "wml_exception.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

static lg::log_domain log_engine("engine");
#define ERR_PF LOG_STREAM(err, log_engine)

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

static unsigned search_counter;

namespace {

struct node {
	int movement_left, turns_left;
	map_location prev, curr;

	/**
	 * If equal to search_counter, the node is off the list.
	 * If equal to search_counter + 1, the node is on the list.
	 * Otherwise it is outdated.
	 */
	unsigned in;

	node(int moves, int turns, const map_location &p, const map_location &c)
		: movement_left(moves), turns_left(turns), prev(p), curr(c) { }
	node() : in(0) { }
	bool operator<(const node& o) const {
		return turns_left > o.turns_left || (turns_left == o.turns_left && movement_left > o.movement_left);
	}
};

struct indexer {
	int w, h;
	indexer(int a, int b) : w(a), h(b) { }
	int operator()(const map_location& loc) const {
		return loc.y * w + loc.x;
	}
};

struct comp {
	const std::vector<node>& nodes;
	comp(const std::vector<node>& n) : nodes(n) { }
	bool operator()(int l, int r) const {
		return nodes[r] < nodes[l];
	}
};
}

static void find_routes(const gamemap& map, const unit_map& units,
		const unit& u, const map_location& loc,
		int move_left, paths::routes_map& routes,
		std::vector<team> const &teams,
		bool force_ignore_zocs, bool allow_teleport, int turns_left,
		const team &viewing_team,
		bool see_all, bool ignore_units)
{
	const team& current_team = teams[u.side() - 1];
	const std::set<map_location>& teleports = allow_teleport ? current_team.villages() : std::set<map_location>();
	
	const int total_movement = u.total_movement();
	
	std::vector<map_location> locs(6 + teleports.size());
	std::copy(teleports.begin(), teleports.end(), locs.begin() + 6);

	search_counter += 2;
	if (search_counter == 0) search_counter = 2;

	static std::vector<node> nodes;
	nodes.resize(map.w() * map.h());
	
	indexer index(map.w(), map.h());
	comp node_comp(nodes);

	int xmin = map.w(), xmax = 0, ymin = map.h(), ymax = 0;

	nodes[index(loc)] = node(move_left, turns_left, map_location::null_location, loc);
	std::vector<int> pq;
	pq.push_back(index(loc));
	
	while (!pq.empty()) {
		node& n = nodes[pq.front()];
		std::pop_heap(pq.begin(), pq.end(), node_comp);
		pq.pop_back();
		n.in = search_counter;
		
		get_adjacent_tiles(n.curr, &locs[0]);
		for (int i = teleports.count(n.curr) ? locs.size() : 6; i-- > 0; ) {
			if (!locs[i].valid(map.w(), map.h())) continue;
			
			node& next = nodes[index(locs[i])];

			bool next_visited = next.in - search_counter <= 1u;

			// test if the current path to locs[i] is better than this one could possibly be.
			// we do this a couple more times below
			if (next_visited && !(n < next)) continue;

			const int move_cost = u.movement_cost(map[locs[i]]);
			
			node t = node(n.movement_left, n.turns_left, n.curr, locs[i]);
			if (t.movement_left < move_cost) {
				t.movement_left = total_movement;
				t.turns_left--;
			}
			
			if (t.movement_left < move_cost || t.turns_left < 0) continue;
			
			t.movement_left -= move_cost;

			if (next_visited && !(t < next)) continue;

			if (!ignore_units) {
				const unit_map::const_iterator unit_it =
					find_visible_unit(units, locs[i], map, teams, viewing_team, see_all);
				if (unit_it != units.end() && current_team.is_enemy(unit_it->second.side()))
					continue;
					
					
				if (!force_ignore_zocs && t.movement_left > 0
						&& enemy_zoc(map, units, teams, locs[i], viewing_team, u.side(), see_all)
						&& !u.get_ability_bool("skirmisher", locs[i])) {
					t.movement_left = 0;
				}

				if (next_visited && !(t < next)) continue;
			}

			int x = locs[i].x;
			if (x < xmin) xmin = x;
			if (xmax < x) xmax = x;
			int y = locs[i].y;
			if (y < ymin) ymin = y;
			if (ymax < y) ymax = y;

			bool in_list = next.in == search_counter + 1;
			t.in = search_counter + 1;
			next = t;

			// if already in the priority queue then we just update it, else push it.
			if (in_list) {
				std::push_heap(pq.begin(), std::find(pq.begin(), pq.end(), index(locs[i])) + 1, node_comp);
			} else {
				pq.push_back(index(locs[i]));
				std::push_heap(pq.begin(), pq.end(), node_comp);
			}		
		}	
	}
	
	// build the routes for every map_location that we reached 
	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x)
		{
			const node &n = nodes[index(map_location(x, y))];
			if (n.in - search_counter > 1u) continue;
			paths::route route;
			route.move_left = n.movement_left + n.turns_left * total_movement;

			// the ai expects that the destination map_location not actually be in the route...
			if (n.prev.valid())
			{
				for (const node *curr = &nodes[index(n.prev)];
				     curr->prev.valid(); curr = &nodes[index(curr->prev)])
				{
					assert(curr->curr.valid());
					route.steps.push_back(curr->curr);
				}
			}
			route.steps.push_back(loc);
			std::reverse(route.steps.begin(), route.steps.end());
			routes[n.curr] = route;
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
		ERR_PF << "paths::paths() -- unit not found\n";
		return;
	}

	if(i->second.side() < 1 || i->second.side() > teams.size()) {
		return;
	}

	routes[loc].move_left = i->second.movement_left();
	find_routes(map,units,i->second,loc,
		i->second.movement_left(),routes,teams,force_ignore_zoc,
		allow_teleport,additional_turns,viewing_team,
		see_all, ignore_units);
}

marked_route mark_route(const plain_route &rt, const unit &u,
	const team &viewing_team, const unit_map &units,
	const std::vector<team> &teams, const gamemap &map)
{
	marked_route res;

	if (rt.steps.empty()) return res;
	res.steps = rt.steps;

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
		bool capture = false;

		if (last_step || zoc || move_cost > movement) {
			// check if we stop an a village and so maybe capture it
			// if it's an enemy unit and a fogged village, we assume a capture
			// (if he already owns it, we can't know that)
			// if it's not an enemy, we can always know if he owns the village
			bool capture = map.is_village(*i) && ( !unit_team.owns_village(*i)
				 || (viewing_team.is_enemy(u.side()) && viewing_team.fogged(*i)) );

			++turns;

			bool invisible = u.invisible(*i,units,teams,false);

			res.waypoints[*i] = marked_route::waypoint(turns, zoc, capture, invisible);

			if (last_step) break; // finished and we used dummy move_cost

			movement = u.total_movement();
			if(move_cost > movement) {
				return res; //we can't reach destination
			}
		}

		zoc = enemy_zoc(map,units,teams, *(i+1), viewing_team,u.side())
					&& !u.get_ability_bool("skirmisher", *(i+1));

		if (zoc || capture) {
			movement = 0;
		} else {
			movement -= move_cost;
		}
	}

	return res;
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
	int const terrain_cost = unit_.movement_cost(terrain);
	// Pathfinding heuristic: the cost must be at least 1
	VALIDATE(terrain_cost >= 1, _("Terrain with a movement cost less than 1 encountered."));

	// total MP is not enough to move on this terrain: impassable
	if (total_movement_ < terrain_cost)
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

	// this will sum all different costs of this move
	int move_cost = 0;

	// Suppose that we have only 2 remaining MP and want to move onto a hex
	// costing 3 MP. We don't have enough MP now, so we must end our turn here,
	// thus spend our remaining MP by waiting (next turn, with full MP, we will
	// be able to move on that hex)
	if (remaining_movement < terrain_cost) {
		move_cost += remaining_movement;
		remaining_movement = total_movement_; // we consider having full MP now
	}

	// check ZoC
	if (!ignore_unit_ && enemy_zoc(map_,units_,teams_, loc, viewing_team_, unit_.side())
			&& !unit_.get_ability_bool("skirmisher", loc)) {
		// entering ZoC cost all remaining MP
		move_cost += remaining_movement;
	} else {
		// empty hex, pay only the terrain cost
		move_cost += terrain_cost;
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

dummy_path_calculator::dummy_path_calculator(const unit&, const gamemap&)
{}

double dummy_path_calculator::cost(const map_location&, const map_location&, const double) const
{
	return 0.0;
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
	outstream << "\"\n\tmove_left=\"" << rt.move_left << "\"\n[/route]";
	return outstream;
}
