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

#include "global.hpp"

#include "game.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "util.hpp"
#include "astarnode.hpp"
#include "wassert.hpp"

#include <cmath>
#include <iostream>

#define LOG_PF lg::info(lg::engine)

typedef std::vector<gamemap::location> vector_location;
typedef std::vector<a_star_node*> vector_a_star_node;
typedef std::set<gamemap::location>	set_location;	

bool compare_strict_sup_a_star_node(const a_star_node* node1, const a_star_node* node2) {
	return (node1->g + (1.3 * node1->h) > node2->g + (1.3 * node2->h));		
}

bool compare_sup_equal_a_star_node(const a_star_node* node1, const a_star_node* node2) {
	return (node1->g + (1.3 * node1->h) >= node2->g + (1.3 * node2->h));		
}

void a_star_init(gamemap::location const &src, gamemap::location const &dst,
								 vector_a_star_node& openList, a_star_world& aStarGameWorld,
								 const size_t parWidth, const size_t parHeight,
								 vector_location& vectLocation, std::set<gamemap::location> const *teleports,
								 size_t& parNbTeleport)
{
	a_star_node*		locStartNode = NULL;
	bool						locIsCreated = false;
	
	aStarGameWorld.resize_IFN(parWidth, parHeight);
	wassert(aStarGameWorld.empty());
	locStartNode = aStarGameWorld.getNodeFromLocation(src, locIsCreated);
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

	if (locValueH < 32)
		locAllocSize = 32;
	else if (locValueH > 256)
		locAllocSize = 256;
	else
		locAllocSize = locValueH;
	
	if (teleports != NULL) 
		parNbTeleport = teleports->size();
	else
		parNbTeleport = 0;

	vectLocation.reserve(parNbTeleport + 6);
	vectLocation.resize(parNbTeleport + 6);	

	if (parNbTeleport > 0) 
	{
		gamemap::location* locLocation;

		locLocation = &vectLocation[6];
		for (set_location::const_iterator it = teleports->begin(); it != teleports->end(); ++it) {
			locLocation->x = (*it).x;
			locLocation->y = (*it).y;
			++locLocation;
		}		
	}
}

void a_star_sort_and_merge(vector_a_star_node& openList, const size_t locNbAdded)
{
	std::stable_sort(openList.end() - locNbAdded, openList.end(), compare_strict_sup_a_star_node);

#ifdef _PARANO_ASTAR_
	for (vector_a_star_node::iterator iter = openList.begin(); iter != openList.end(); ++iter)
		wassert((*iter)->isInCloseList == false);		
	if (openList.size() > locNbAdded) {
		for (vector_a_star_node::iterator iter = openList.begin() + 1; iter != openList.end() - locNbAdded; ++iter)
		{
			vector_a_star_node::iterator iterPrev = iter - 1;			
			wassert(compare_sup_equal_a_star_node(*iterPrev, *iter));
		}
	}
	if (locNbAdded > 0) {
		for (vector_a_star_node::iterator iter = openList.end() - locNbAdded + 1; iter != openList.end(); ++iter)
		{
			vector_a_star_node::iterator iterPrev = iter - 1;			
			wassert(compare_sup_equal_a_star_node(*iterPrev, *iter));
		}
	}
#endif

	std::inplace_merge(openList.begin(), openList.end() - locNbAdded, openList.end(), compare_strict_sup_a_star_node);

#ifdef _PARANO_ASTAR_
	if (openList.size() > 0) {
		vector_a_star_node::iterator iter;
		for (iter = openList.begin() + 1; iter != openList.end(); ++iter)
		{
			vector_a_star_node::iterator iterPrev = iter - 1;			
			wassert(compare_sup_equal_a_star_node(*iterPrev, *iter));				
		}
		for (iter = openList.begin(); iter != openList.end(); ++iter)
			wassert((*iter)->isInCloseList == false);
	}
#endif
}

void a_star_explore_neighbours(gamemap::location const &dst, const double stop_at,
															 cost_calculator const *costCalculator,
															 const size_t parWidth, const size_t parHeight,
															 std::set<gamemap::location> const *teleports,
															 vector_location& vectLocation, vector_a_star_node& openList,
															 a_star_world& aStarGameWorld, size_t& locNbAdded,
															 a_star_node*	parCurNode, const size_t parNbTeleport)
{
	//----------------- PRE_CONDITIONS ------------------
	wassert(parCurNode != NULL);	
	//---------------------------------------------------
	
	typedef std::pair<vector_a_star_node::iterator, vector_a_star_node::iterator> pair_node_iter;

	a_star_node*	 locNextNode;
	double				 locCost;
	pair_node_iter locPlace;
	size_t				 locSize;
	bool					 locIsCreated = false;
	const double	 locCostFather = parCurNode->g;

	get_adjacent_tiles(parCurNode->loc, &vectLocation[0]);		

	if ((parNbTeleport > 0) && (teleports->count(parCurNode->loc) > 0))
	{
		wassert(teleports != NULL);
		wassert(teleports->size() == parNbTeleport);
		wassert(vectLocation.size() == parNbTeleport + 6);
		locSize = parNbTeleport + 6;
	}
	else
	{
		wassert(vectLocation.size() >= 6);
		locSize = 6;
	}	

	for (size_t i = 0; i != locSize; ++i)
	{
		const gamemap::location&  locLocation = vectLocation[i];

		if (locLocation.valid(int(parWidth), int(parHeight)) == false)
			continue;
		locNextNode = aStarGameWorld.getNodeFromLocation(locLocation, locIsCreated);
		locCost = locCostFather + costCalculator->cost(locLocation, locCostFather, locLocation == dst);
		if (locIsCreated)
		{
			locNextNode->initNode(locLocation, dst, locCost, parCurNode, teleports);
			if (locNextNode->g + locNextNode->h < stop_at) {					
				openList.push_back(locNextNode);
				++locNbAdded;					
			}
			else
			{
				wassert(locNextNode->isInCloseList == false);
				locNextNode->isInCloseList = true;
			}
		}
		else
		{
			// If this path is better that the previous for this node
			if (locCost < locNextNode->g)
			{										
				if (locNextNode->isInCloseList) {
					locNextNode->isInCloseList = false;
				}
				else
				{					
					locPlace = std::equal_range(openList.begin(), openList.end() - locNbAdded, locNextNode, compare_strict_sup_a_star_node);
					assertParanoAstar(*(std::find(locPlace.first, locPlace.second, locNextNode)) == locNextNode);
					openList.erase(std::find(locPlace.first, locPlace.second, locNextNode));
				}
				openList.push_back(locNextNode);
				++locNbAdded;
				locNextNode->g = locCost;
				locNextNode->nodeParent = parCurNode;					
			}
		}												
	}
}

paths::route a_star_search( gamemap::location const &src, gamemap::location const &dst,
													  double stop_at, cost_calculator const *costCalculator, const size_t parWidth, 
														const size_t parHeight, std::set<gamemap::location> const *teleports)
{
	//----------------- PRE_CONDITIONS ------------------
	wassert(parWidth > 0);
	wassert(parHeight > 0);
	wassert(src.valid());
	wassert(dst.valid());
	wassert(costCalculator != NULL);
	wassert(stop_at <= costCalculator->getNoPathValue());
	//---------------------------------------------------			
	static a_star_world				aStarGameWorld;	
	static poss_a_star_node	  POSS_AStarNode;
	
	vector_a_star_node			openList;	
	vector_location					vectLocation;
	paths::route						locRoute;
	size_t									locNbTeleport;	
	size_t									locNbAdded;	
	a_star_node*						locDestNode = NULL;
	a_star_node*						locCurNode = NULL;
	
	wassert(openList.empty());	
	wassert(aStarGameWorld.empty());
	assertParanoAstar(aStarGameWorld.reallyEmpty());	
	
	LOG_PF << "A* search: " << src.x << ", " << src.y << " -> " << dst.x << ", " << dst.y << "\n";

	if ( (src.valid(int(parWidth), int(parHeight)) == false) ||
			 (dst.valid(int(parWidth), int(parHeight)) == false) ||
			 (costCalculator->cost(dst, 0, true) >= stop_at))
	{
		LOG_PF << "aborted A* search because Start or Dest is invalid\n";
		locRoute.move_left = int(costCalculator->getNoPathValue());
		return (locRoute);
	}	

	a_star_init(src, dst, openList, aStarGameWorld, parWidth, parHeight, vectLocation, teleports, locNbTeleport);

	while (!openList.empty()) 
	{
		locCurNode = openList.back();
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

		openList.pop_back();

		wassert(locCurNode->isInCloseList == false);
		locCurNode->isInCloseList = true ;		
		
		locNbAdded = 0;
		a_star_explore_neighbours(dst, stop_at, costCalculator, parWidth, parHeight, teleports, vectLocation,
															openList, aStarGameWorld, locNbAdded, locCurNode, locNbTeleport);

		a_star_sort_and_merge(openList, locNbAdded);
	}
	
	LOG_PF << "aborted a* search\n";
	locRoute.move_left = int(costCalculator->getNoPathValue());
	
label_AStarSearch_end:
	openList.clear();
	POSS_AStarNode.reduce();
	aStarGameWorld.clear();

	//----------------- POST_CONDITIONS -----------------
	wassert(openList.empty());
	wassert(aStarGameWorld.empty());
	assertParanoAstar(aStarGameWorld.reallyEmpty());	
	//---------------------------------------------------
	return (locRoute);
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

		if(map.on_board(loc) && units.find(loc) == units.end() &&
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
						find_vacant(map,units,adj[i],depth-1,vacancy,touched);

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

void get_tiles_radius(const gamemap& map, const std::vector<gamemap::location>& locs, size_t radius,
											std::set<gamemap::location>& res)
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

bool enemy_zoc(const gamemap& map, const gamestatus& status, 
							 const std::map<gamemap::location,unit>& units,
							 const std::vector<team>& teams,
							 const gamemap::location& loc, const team& current_team, int side)
{
	gamemap::location locs[6];
	get_adjacent_tiles(loc,locs);
	for(int i = 0; i != 6; ++i) {
		const std::map<gamemap::location,unit>::const_iterator it
			= find_visible_unit(units,locs[i],
			map,
			status.get_time_of_day().lawful_bonus,
			teams,current_team);
		if(it != units.end() && it->second.side() != side &&
			current_team.is_enemy(it->second.side()) && !it->second.stone()) {
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
		std::vector<team>& teams,
		bool ignore_zocs, bool allow_teleport, int turns_left, bool starting_pos)
	{
		if(size_t(u.side()-1) >= teams.size()) {
			return;
		}

		team& current_team = teams[u.side()-1];

		//find adjacent tiles
		std::vector<gamemap::location> locs(6);
		get_adjacent_tiles(loc,&locs[0]);

		//check for teleporting units -- we must be on a vacant (or occupied by this unit)
		//village, that is controlled by our team to be able to teleport.
		if(allow_teleport && map.is_village(loc) &&
			current_team.owns_village(loc) && (starting_pos || units.count(loc) == 0)) {
				const std::vector<gamemap::location>& villages = map.villages();

				//if we are on a village, see all friendly villages that we can
				//teleport to
				for(std::vector<gamemap::location>::const_iterator t = villages.begin();
					t != villages.end(); ++t) {
						if(!current_team.owns_village(*t) || units.count(*t))
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
					find_visible_unit(units,locs[i],map,
					status.get_time_of_day().lawful_bonus,
					teams,current_team);

				if(unit_it != units.end() &&
					current_team.is_enemy(unit_it->second.side()))
					continue;

				//find the terrain of the adjacent location
				const gamemap::TERRAIN terrain = map[currentloc.x][currentloc.y];

				//find the movement cost of this type onto the terrain
				const int move_cost = u.movement_cost(map,terrain);
				if(move_cost <= move_left ||
					turns_left > 0 && move_cost <= u.total_movement()) {
						int new_move_left = move_left - move_cost;
						int new_turns_left = turns_left;
						if(new_move_left < 0) {
							--new_turns_left;
							new_move_left = u.total_movement() - move_cost;
						}

						const int total_movement = new_turns_left*u.total_movement() +
							new_move_left;

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
						new_route.move_left = u.total_movement() * new_turns_left +
							zoc_move_left;
						routes[currentloc] = new_route;

						if(new_route.move_left > 0) {
							find_routes(map,status,gamedata,units,u,currentloc,
								zoc_move_left,routes,teams,ignore_zocs,
								allow_teleport,new_turns_left,false);
						}
					}
			}
	}

} //end anon namespace

paths::paths(const gamemap& map, const gamestatus& status,
						 const game_data& gamedata,
						 const std::map<gamemap::location,unit>& units,
						 const gamemap::location& loc,
						 std::vector<team>& teams,
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

int route_turns_to_complete(const unit& u, const gamemap& map,
														const paths::route& rt)
{
	if(rt.steps.empty())
		return 0;

	int turns = 0, movement = u.movement_left();
	for(std::vector<gamemap::location>::const_iterator i = rt.steps.begin()+1;
		i != rt.steps.end(); ++i) {
			wassert(map.on_board(*i));
			const int move_cost = u.movement_cost(map,map[i->x][i->y]);
			movement -= move_cost;
			if(movement < 0) {
				++turns;
				movement = u.total_movement() - move_cost;
				if(movement < 0) {
					return -1;
				}
			}
		}

		return turns;
}


shortest_path_calculator::shortest_path_calculator(const unit& u, const team& t,
																									 const unit_map& units,
																									 const std::vector<team>& teams,
																									 const gamemap& map,
																									 const gamestatus& status)
																									 : unit_(u), team_(t), units_(units), teams_(teams), 
																									 map_(map), status_(status)
{
}

double shortest_path_calculator::cost(const gamemap::location& loc, const double so_far, const bool isDst) const
{
	wassert(map_.on_board(loc));

	if (team_.shrouded(loc.x, loc.y))
		return (getNoPathValue());

	const unit_map::const_iterator enemy_unit = find_visible_unit(units_, loc, map_, status_.get_time_of_day().lawful_bonus, teams_, team_);

	if (enemy_unit != units_.end() && team_.is_enemy(enemy_unit->second.side()))
		return (getNoPathValue());

	if ((isDst == false) && (unit_.type().is_skirmisher() == false)) 
	{
		gamemap::location adj[6];
		get_adjacent_tiles(loc,adj);

		for (size_t i = 0; i != 6; ++i) {
			const unit_map::const_iterator u = find_visible_unit(units_, adj[i],map_, status_.get_time_of_day().lawful_bonus, teams_, team_);

			if (u != units_.end() && team_.is_enemy(u->second.side()) && !team_.fogged(adj[i].x,adj[i].y) && u->second.stone() == false) {
				return (getNoPathValue());
			}
		}
	}

	int const base_cost = unit_.movement_cost(map_, map_[loc.x][loc.y]);

	//supposing we had 2 movement left, and wanted to move onto a hex which
	//takes 3 movement, it's going to cost us 5 movement in total, since we
	//sacrifice this turn's movement. Take that into account here.
	const int current_cost(static_cast<int>(so_far));

	const int starting_movement = unit_.movement_left();
	int remaining_movement = starting_movement - current_cost;
	int total = unit_.total_movement();
	if (total != 0) {
		if (remaining_movement < 0) {
		remaining_movement = total - (-remaining_movement) % total;
		}
	} else if (remaining_movement < 0) {
		remaining_movement = -remaining_movement;
	}

	int additional_cost = base_cost > remaining_movement ? remaining_movement : 0;

	return base_cost + additional_cost;
}
