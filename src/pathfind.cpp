/* $Id$ */
/*
Copyright (C) 2003 by David White <davidnwhite@verizon.net>
Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#include "global.hpp"

#include "astarnode.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "util.hpp"
#include "wassert.hpp"

class gamestatus;

#include <cmath>
#include <iostream>

#define LOG_PF LOG_STREAM(info, engine)

typedef std::vector<gamemap::location> vector_location;
typedef std::vector<a_star_node*> vector_a_star_node;
typedef std::set<gamemap::location> set_location;

// heaps give the biggest element for free, so we want the biggest element to
// have the smallest cost
static bool compare_lt_a_star_node(const a_star_node* node1, const a_star_node* node2) {
	return node1->g + node1->h > node2->g + node2->h;
}

static void a_star_init(gamemap::location const &src, gamemap::location const &dst,
                        vector_a_star_node &openList, a_star_world &aStarGameWorld,
                        const size_t parWidth, const size_t parHeight,
                        vector_location &vectLocation, std::set<gamemap::location> const *teleports,
                        size_t &parNbTeleport)
{
	bool locIsCreated;

	aStarGameWorld.resize_IFN(parWidth, parHeight);
	a_star_node *locStartNode = aStarGameWorld.getNodeFromLocation(src, locIsCreated);
	wassert(locIsCreated);
	locStartNode->initNode(src, dst, 0.0, NULL, teleports);

	const size_t locValueH = size_t(locStartNode->h);
	size_t locAllocSize;

	if (locValueH < 16)
		locAllocSize = 16;
	else if (locValueH > 128)
		locAllocSize = 128;
	else
		locAllocSize = locValueH;
	openList.reserve(locAllocSize);
	openList.push_back(locStartNode);

	if (teleports != NULL)
		parNbTeleport = teleports->size();
	else
		parNbTeleport = 0;

	vectLocation.reserve(parNbTeleport + 6);
	vectLocation.resize(parNbTeleport + 6);

	if (parNbTeleport > 0)
		std::copy(teleports->begin(), teleports->end(), &vectLocation[6]);
}

static void a_star_explore_neighbours(gamemap::location const &dst, const double stop_at,
                                      cost_calculator const *costCalculator,
                                      const size_t parWidth, const size_t parHeight,
                                      std::set<gamemap::location> const *teleports,
                                      vector_location &vectLocation, vector_a_star_node &openList,
                                      a_star_world &aStarGameWorld,
                                      a_star_node *parCurNode, const size_t parNbTeleport)
{
	typedef std::pair<vector_a_star_node::iterator, vector_a_star_node::iterator> pair_node_iter;

	a_star_node *locNextNode;
	double locCost;
	pair_node_iter locPlace;
	size_t locSize;
	bool locIsCreated;
	const double locCostFather = parCurNode->g;

	get_adjacent_tiles(parCurNode->loc, &vectLocation[0]);

	if (parNbTeleport > 0 && teleports->count(parCurNode->loc) > 0)
		locSize = parNbTeleport + 6;
	else
		locSize = 6;

	bool broken_heap = false;
	int locNbAdded = 0;

	for (size_t i = 0; i != locSize; ++i)
	{
		const gamemap::location&  locLocation = vectLocation[i];

		if (locLocation.valid(int(parWidth), int(parHeight)) == false)
			continue;
		locNextNode = aStarGameWorld.getNodeFromLocation(locLocation, locIsCreated);
		locCost = locCostFather + costCalculator->cost(locLocation, locCostFather, locLocation == dst);
		if (locIsCreated) {
			locNextNode->initNode(locLocation, dst, locCost, parCurNode, teleports);
			if (locNextNode->g + locNextNode->h < stop_at) {
				openList.push_back(locNextNode);
				++locNbAdded;
			} else
				locNextNode->isInCloseList = true;

		} else if (locCost < locNextNode->g) {

			if (locNextNode->isInCloseList) {
				locNextNode->isInCloseList = false;
				openList.push_back(locNextNode);
				++locNbAdded;
			} else
				broken_heap = true;

			locNextNode->g = locCost;
			locNextNode->nodeParent = parCurNode;
		}
	}

	vector_a_star_node::iterator openList_begin = openList.begin(),
	                             openList_end = openList.end();
	if (broken_heap)
		std::make_heap(openList_begin, openList_end, compare_lt_a_star_node);
	else
		for(; locNbAdded > 0; --locNbAdded)
			std::push_heap(openList_begin, openList_end - (locNbAdded - 1), compare_lt_a_star_node);
}

paths::route a_star_search(gamemap::location const &src, gamemap::location const &dst,
                           double stop_at, cost_calculator const *costCalculator, const size_t parWidth,
                           const size_t parHeight, std::set<gamemap::location> const *teleports)
{
	//----------------- PRE_CONDITIONS ------------------
	wassert(src.valid(parWidth, parHeight));
	wassert(dst.valid(parWidth, parHeight));
	wassert(costCalculator != NULL);
	wassert(stop_at <= costCalculator->getNoPathValue());
	//---------------------------------------------------
	static a_star_world aStarGameWorld;

	vector_a_star_node openList;
	vector_location vectLocation;
	paths::route locRoute;
	size_t locNbTeleport;
	a_star_node *locDestNode = NULL;
	a_star_node *locCurNode = NULL;

	LOG_PF << "A* search: " << src << " -> " << dst << '\n';

	if (costCalculator->cost(dst, 0, true) >= stop_at) {
		LOG_PF << "aborted A* search because Start or Dest is invalid\n";
		locRoute.move_left = int(costCalculator->getNoPathValue());
		return locRoute;
	}

	a_star_init(src, dst, openList, aStarGameWorld, parWidth, parHeight, vectLocation, teleports, locNbTeleport);

	while (!openList.empty())
	{
		locCurNode = openList.front();
		wassert(locCurNode != NULL);

		//if we have found a solution
		if (locCurNode->loc == dst)
		{
			locDestNode = locCurNode;

			LOG_PF << "found solution; calculating it...\n";
			while (locCurNode != NULL)
			{
				locRoute.steps.push_back(locCurNode->loc);
				locCurNode = locCurNode->nodeParent;
			}
			std::reverse(locRoute.steps.begin(), locRoute.steps.end());
			locRoute.move_left = int(locDestNode->g);

			wassert(locRoute.steps.front() == src);
			wassert(locRoute.steps.back() == dst);

			LOG_PF << "exiting a* search (solved)\n";
			goto label_AStarSearch_end;
		}

		std::pop_heap(openList.begin(), openList.end(), compare_lt_a_star_node);
		openList.pop_back();

		wassert(locCurNode->isInCloseList == false);
		locCurNode->isInCloseList = true;

		a_star_explore_neighbours(dst, stop_at, costCalculator, parWidth, parHeight, teleports, vectLocation,
		                          openList, aStarGameWorld, locCurNode, locNbTeleport);

	}

	LOG_PF << "aborted a* search\n";
	locRoute.move_left = int(costCalculator->getNoPathValue());

label_AStarSearch_end:
	openList.clear();
	aStarGameWorld.clear();
	return locRoute;
}

namespace {
	gamemap::location find_vacant(const gamemap& map,
		const std::map<gamemap::location,unit>& units,
		const gamemap::location& loc, int depth,
		VACANT_TILE_TYPE vacancy,
		std::set<gamemap::location>& touched)
	{
		if(touched.count(loc))
			return gamemap::location();

		touched.insert(loc);

		if (map.on_board(loc) && units.find(loc) == units.end() &&
		    (vacancy == VACANT_ANY || map.is_castle(loc))) {
			return loc;
		} else if(depth == 0) {
			return gamemap::location();
		} else {
			gamemap::location adj[6];
			get_adjacent_tiles(loc,adj);
			for(int i = 0; i != 6; ++i) {
				if(!map.on_board(adj[i]) || vacancy == VACANT_CASTLE && !map.is_castle(adj[i]))
					continue;

				const gamemap::location res =
					find_vacant(map, units, adj[i], depth - 1, vacancy, touched);

				if (map.on_board(res))
					return res;
			}

			return gamemap::location();
		}
	}

}

gamemap::location find_vacant_tile(const gamemap& map,
																	 const std::map<gamemap::location,unit>& units,
																	 const gamemap::location& loc,
																	 VACANT_TILE_TYPE vacancy)
{
	for(int i = 1; i != 50; ++i) {
		std::set<gamemap::location> touch;
		const gamemap::location res = find_vacant(map,units,loc,i,vacancy,touch);
		if(map.on_board(res))
			return res;
	}

	return gamemap::location();
}

namespace {

	void get_tiles_radius_internal(const gamemap::location& a, size_t radius, std::set<gamemap::location>& res, std::map<gamemap::location,int>& visited)
	{
		visited[a] = radius;
		res.insert(a);

		if(radius == 0) {
			return;
		}

		gamemap::location adj[6];
		get_adjacent_tiles(a,adj);
		for(size_t i = 0; i != 6; ++i) {
			if(visited.count(adj[i]) == 0 || visited[adj[i]] < int(radius)-1) {
				get_tiles_radius_internal(adj[i],radius-1,res,visited);
			}
		}
	}

}

void get_tiles_radius(const gamemap::location& a, size_t radius, std::set<gamemap::location>& res)
{
	std::map<gamemap::location,int> visited;
	get_tiles_radius_internal(a,radius,res,visited);
}

void get_tiles_radius(gamemap const &map, std::vector<gamemap::location> const &locs,
                      size_t radius, std::set<gamemap::location> &res)
{
	typedef std::set<gamemap::location> location_set;
	location_set not_visited(locs.begin(), locs.end()), must_visit;
	++radius;

	for(;;) {
		location_set::const_iterator it = not_visited.begin(), it_end = not_visited.end();
		std::copy(it,it_end,std::inserter(res,res.end()));
		for(; it != it_end; ++it) {
			gamemap::location adj[6];
			get_adjacent_tiles(*it, adj);
			for(size_t i = 0; i != 6; ++i) {
				gamemap::location const &loc = adj[i];
				if(map.on_board(loc) && res.count(loc) == 0) {
					must_visit.insert(loc);
				}
			}
		}

		if(--radius == 0 || must_visit.empty()) {
			break;
		}

		not_visited.swap(must_visit);
		must_visit.clear();
	}
}

bool enemy_zoc(gamemap const &map, gamestatus const &status,
               std::map<gamemap::location, unit> const &units,
               std::vector<team> const &teams,
               gamemap::location const &loc, team const &current_team, int side)
{
	gamemap::location locs[6];
	get_adjacent_tiles(loc,locs);
	for(int i = 0; i != 6; ++i) {
		const std::map<gamemap::location,unit>::const_iterator it
			= find_visible_unit(units,locs[i],
			map,
			status.get_time_of_day().lawful_bonus,
			teams,current_team);
		if (it != units.end() && it->second.side() != side &&
		    current_team.is_enemy(it->second.side()) && it->second.emits_zoc()) {
			return true;
		}
	}

	return false;
}

namespace {

	void find_routes(const gamemap& map, const gamestatus& status,
		const game_data& gamedata,
		const std::map<gamemap::location,unit>& units,
		const unit& u,
		const gamemap::location& loc,
		int move_left,
		std::map<gamemap::location,paths::route>& routes,
		std::vector<team> const &teams,
		bool ignore_zocs, bool allow_teleport, int turns_left, bool starting_pos)
	{
		if(size_t(u.side()-1) >= teams.size()) {
			return;
		}

		team const &current_team = teams[u.side()-1];

		//find adjacent tiles
		std::vector<gamemap::location> locs(6);
		get_adjacent_tiles(loc,&locs[0]);

		//check for teleporting units -- we must be on a vacant (or occupied by this unit)
		//village, that is controlled by our team to be able to teleport.
		if (allow_teleport && map.is_village(loc) &&
		    current_team.owns_village(loc) && (starting_pos || units.count(loc) == 0)) {
			const std::vector<gamemap::location>& villages = map.villages();

			//if we are on a village, see all friendly villages that we can
			//teleport to
			for(std::vector<gamemap::location>::const_iterator t = villages.begin();
			    t != villages.end(); ++t) {
				if (!current_team.owns_village(*t) || units.count(*t))
					continue;

				locs.push_back(*t);
			}
		}

		//iterate over all adjacent tiles
		for(size_t i = 0; i != locs.size(); ++i) {
			const gamemap::location& currentloc = locs[i];

			//check if the adjacent location is off the board
			if (currentloc.x < 0 || currentloc.y < 0 ||
			    currentloc.x >= map.x() || currentloc.y >= map.y())
				continue;

			//see if the tile is on top of an enemy unit
			const std::map<gamemap::location,unit>::const_iterator unit_it =
				find_visible_unit(units, locs[i], map,
				                  status.get_time_of_day().lawful_bonus,
				                  teams, current_team);

			if (unit_it != units.end() &&
			    current_team.is_enemy(unit_it->second.side()))
				continue;

			//find the terrain of the adjacent location
			const gamemap::TERRAIN terrain = map[currentloc.x][currentloc.y];

			//find the movement cost of this type onto the terrain
			const int move_cost = u.movement_cost(map,terrain);
			if (move_cost <= move_left ||
			    turns_left > 0 && move_cost <= u.total_movement()) {
				int new_move_left = move_left - move_cost;
				int new_turns_left = turns_left;
				if (new_move_left < 0) {
					--new_turns_left;
					new_move_left = u.total_movement() - move_cost;
				}

				const int total_movement = new_turns_left * u.total_movement() + new_move_left;

				const std::map<gamemap::location,paths::route>::const_iterator
					rtit = routes.find(currentloc);

				//if a better route to that tile has already been found
				if(rtit != routes.end() && rtit->second.move_left >= total_movement)
					continue;

				const bool zoc = !ignore_zocs && enemy_zoc(map,status,units,teams,currentloc,
							current_team,u.side());
				paths::route new_route = routes[loc];
				new_route.steps.push_back(loc);

				const int zoc_move_left = zoc ? 0 : new_move_left;
				new_route.move_left = u.total_movement() * new_turns_left + zoc_move_left;
				routes[currentloc] = new_route;

				if (new_route.move_left > 0) {
					find_routes(map, status, gamedata, units, u, currentloc,
					            zoc_move_left, routes, teams, ignore_zocs,
					            allow_teleport, new_turns_left, false);
				}
			}
		}
	}

} //end anon namespace

paths::paths(gamemap const &map, gamestatus const &status,
             game_data const &gamedata,
             std::map<gamemap::location, unit> const &units,
             gamemap::location const &loc,
             std::vector<team> const &teams,
             bool ignore_zocs, bool allow_teleport, int additional_turns)
{
	const std::map<gamemap::location,unit>::const_iterator i = units.find(loc);
	if(i == units.end()) {
		std::cerr << "unit not found\n";
		return;
	}

	routes[loc].move_left = i->second.movement_left();
	find_routes(map,status,gamedata,units,i->second,loc,
		i->second.movement_left(),routes,teams,
		ignore_zocs,allow_teleport,additional_turns,true);
}

int route_turns_to_complete(unit const &u, gamemap const &map, paths::route const &rt)
{
	if(rt.steps.empty())
		return 0;

	int turns = 0, movement = u.movement_left();
	for(std::vector<gamemap::location>::const_iterator i = rt.steps.begin()+1;
	    i != rt.steps.end(); ++i) {
		wassert(map.on_board(*i));
		const int move_cost = u.movement_cost(map, map[i->x][i->y]);
		movement -= move_cost;
		if (movement < 0) {
			++turns;
			movement = u.total_movement() - move_cost;
			if(movement < 0) {
				return -1;
			}
		}
	}

	return turns;
}


shortest_path_calculator::shortest_path_calculator(unit const &u, team const &t, unit_map const &units,
                                                   std::vector<team> const &teams, gamemap const &map,
                                                   gamestatus const &status)
	: unit_(u), team_(t), units_(units), teams_(teams), map_(map),
	  lawful_bonus_(status.get_time_of_day().lawful_bonus),
	  unit_is_skirmisher_(unit_.type().is_skirmisher()),
	  movement_left_(unit_.movement_left()),
	  total_movement_(unit_.total_movement())
{
}

double shortest_path_calculator::cost(const gamemap::location& loc, const double so_far, const bool isDst) const
{
	wassert(map_.on_board(loc));

	//the location is not valid
	//1. if the loc is shrouded, or
	//2. if moving in it costs more than the total movement of the unit, or
	//3. if there is a visible enemy on the hex, or
	//4. if the unit is not a skirmisher and there is a visible enemy with
	//   a ZoC on an adjacent hex in the middle of the route

	if (team_.shrouded(loc.x, loc.y))
		return getNoPathValue();

	int const base_cost = unit_.movement_cost(map_, map_[loc.x][loc.y]);
	wassert(base_cost >= 1); // pathfinding heuristic: the cost must be at least 1
	if (total_movement_ < base_cost)
		return getNoPathValue();

	unit_map::const_iterator
		enemy_unit = find_visible_unit(units_, loc, map_, lawful_bonus_, teams_, team_),
		units_end = units_.end();

	if (enemy_unit != units_end && team_.is_enemy(enemy_unit->second.side()))
		return getNoPathValue();

	if (!isDst && !unit_is_skirmisher_) {
		gamemap::location adj[6];
		get_adjacent_tiles(loc, adj);

		for (size_t i = 0; i != 6; ++i) {
			enemy_unit = find_visible_unit(units_, adj[i], map_, lawful_bonus_, teams_, team_);

			if (enemy_unit != units_end && team_.is_enemy(enemy_unit->second.side()) &&
			    !team_.fogged(adj[i].x, adj[i].y) && enemy_unit->second.emits_zoc())
				return getNoPathValue();
		}
	}

	//compute how many movement points are left in the game turn needed to
	//reach the previous hex
	//total_movement_ is not zero, thanks to the pathfinding heuristic
	int remaining_movement = movement_left_ - static_cast<int>(so_far);
	if (remaining_movement < 0)
		remaining_movement = total_movement_ - (-remaining_movement) % total_movement_;

	//supposing we had 2 movement left, and wanted to move onto a hex which
	//takes 3 movement, it's going to cost us 5 movement in total, since we
	//sacrifice this turn's movement. Take that into account here.
	int additional_cost = base_cost > remaining_movement ? remaining_movement : 0;
	return base_cost + additional_cost;
}
