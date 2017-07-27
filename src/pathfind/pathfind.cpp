/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
 * Various pathfinding functions and utilities.
 */

#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"

#include "game_board.hpp"
#include "game_display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "wml_exception.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

static lg::log_domain log_engine("engine");
#define ERR_PF LOG_STREAM(err, log_engine)

namespace pathfind {


/**
 * Function that will find a location on the board that is as near
 * to @a loc as possible, but which is unoccupied by any units.
 * If no valid location can be found, it will return a null location.
 * If @a pass_check is provided, the found location must have a terrain
 * that this unit can enter.
 * If @a shroud_check is provided, only locations not covered by this
 * team's shroud will be considered.
 */
map_location find_vacant_tile(const map_location& loc, VACANT_TILE_TYPE vacancy,
                              const unit* pass_check, const team* shroud_check, const game_board* board)
{
	if (!board) {
		board = resources::gameboard;
		assert(board);
	}
	const gamemap & map = board->map();
	const unit_map & units = board->units();

	if (!map.on_board(loc)) return map_location();

	const bool do_shroud = shroud_check  &&  shroud_check->uses_shroud();
	std::set<map_location> pending_tiles_to_check, tiles_checked;
	pending_tiles_to_check.insert(loc);
	// Iterate out 50 hexes from loc
	for (int distance = 0; distance < 50; ++distance) {
		if (pending_tiles_to_check.empty())
			return map_location();
		//Copy over the hexes to check and clear the old set
		std::set<map_location> tiles_checking;
		tiles_checking.swap(pending_tiles_to_check);
		//Iterate over all the hexes we need to check
		for (const map_location &l : tiles_checking)
		{
			// Skip shrouded locations.
			if ( do_shroud  &&  shroud_check->shrouded(l) )
				continue;
			//If this area is not a castle but should, skip it.
			if ( vacancy == VACANT_CASTLE  &&  !map.is_castle(l) ) continue;
			const bool pass_check_and_unreachable = pass_check
				&& pass_check->movement_cost(map[l]) == movetype::UNREACHABLE;
			//If the unit can't reach the tile and we have searched
			//an area of at least radius 10 (arbitrary), skip the tile.
			//Neccessary for cases such as an unreachable
			//starting hex surrounded by 6 other unreachable hexes, in which case
			//the algorithm would not even search distance==1
			//even if there's a reachable hex for distance==2.
			if (pass_check_and_unreachable && distance > 10) continue;
			//If the hex is empty and we do either no pass check or the hex is reachable, return it.
			if (units.find(l) == units.end() && !pass_check_and_unreachable) return l;
			map_location adjs[6];
			get_adjacent_tiles(l,adjs);
			for (const map_location &l2 : adjs)
			{
				if (!map.on_board(l2)) continue;
				// Add the tile to be checked if it hasn't already been and
				// isn't being checked.
				if (tiles_checked.find(l2) == tiles_checked.end() &&
				    tiles_checking.find(l2) == tiles_checking.end())
				{
					pending_tiles_to_check.insert(l2);
				}
			}
		}
		tiles_checked.swap(tiles_checking);
	}
	return map_location();
}

/**
 * Wrapper for find_vacant_tile() when looking for a vacant castle tile
 * near a leader.
 * If no valid location can be found, it will return a null location.
 */
map_location find_vacant_castle(const unit & leader)
{
	return find_vacant_tile(leader.get_location(), VACANT_CASTLE,
	                        nullptr, &resources::gameboard->get_team(leader.side()));
}


/**
 * Determines if a given location is in an enemy zone of control.
 *
 * @param current_team  The moving team (only ZoC of enemies of this team are considered).
 * @param loc           The location to check.
 * @param viewing_team  Only units visible to this team are considered.
 * @param see_all       If true, all units are considered (and viewing_team is ignored).
 *
 * @return true iff a visible enemy exerts zone of control over loc.
 */
bool enemy_zoc(team const &current_team, map_location const &loc,
               team const &viewing_team, bool see_all)
{
	// Check the adjacent tiles.
	map_location locs[6];
	get_adjacent_tiles(loc,locs);
	for (int i = 0; i != 6; ++i)
	{
		const unit *u = resources::gameboard->get_visible_unit(locs[i], viewing_team, see_all);
		if ( u  &&  current_team.is_enemy(u->side())  &&  u->emits_zoc() )
			return true;
	}

	// No adjacent tiles had an enemy exerting ZoC over loc.
	return false;
}


namespace {
	/**
	 * Nodes used by find_routes().
	 * These store the information necessary for extending the path
	 * and for tracing the route back to the source.
	 */
	struct findroute_node {
		int moves_left, turns_left;
		map_location prev;
		// search_num is used to detect which nodes have been collected
		// in the current search. (More than just a boolean value so
		// that nodes can be stored between searches.)
		unsigned search_num;

		// Constructors.
		findroute_node(int moves, int turns, const map_location &prev_loc, unsigned search_count)
			: moves_left(moves)
			, turns_left(turns)
			, prev(prev_loc)
			, search_num(search_count)
		{ }
		findroute_node()
			: moves_left(0)
			, turns_left(0)
			, prev()
			, search_num(0)
		{ }

		// Compare these nodes based on movement consumed.
		bool operator<(const findroute_node& o) const {
			return turns_left > o.turns_left ||
				  (turns_left == o.turns_left && moves_left > o.moves_left);
		}
	};

	/**
	 * Converts map locations to and from integer indices.
	 */
	struct findroute_indexer {
		int w, h; // Width and height of the map.

		// Constructor:
		findroute_indexer(int a, int b) : w(a), h(b) { }
		// Convert to an index: (returns -1 on out of bounds)
		int operator()(int x, int y) const {
			if ( x < 0  || w <= x  ||  y < 0 || h <= y )
				return -1;
			else
				return x + y*w;
		}
		int operator()(const map_location& loc) const {
			return (*this)(loc.x, loc.y);
		}
		// Convert from an index:
		map_location operator()(int index) const {
			return map_location(index%w, index/w);
		}
	};

	/**
	 * A function object for comparing indices.
	 */
	struct findroute_comp {
		const std::vector<findroute_node>& nodes;

		// Constructor:
		findroute_comp(const std::vector<findroute_node>& n) : nodes(n) { }
		// Binary predicate evaluating the order of its arguments:
		bool operator()(int l, int r) const {
			return nodes[r] < nodes[l];
		}
	};
}

/**
 * Creates a list of routes that a unit can traverse from the provided location.
 * (This is called when creating pathfind::paths and descendant classes.)
 *
 * @param[in]  origin        The location at which to begin the routes.
 * @param[in]  costs         The costs to use for route finding.
 * @param[in]  slowed        Whether or not to use the slowed costs.
 * @param[in]  moves_left    The number of movement points left for the current turn.
 * @param[in]  max_moves     The number of movement points in each future turn.
 * @param[in]  turns_left    The number of future turns of movement to calculate.
 * @param[out] destinations  The traversable routes.
 * @param[out] edges         The hexes (possibly off-map) adjacent to those in
 *                           destinations. (It is permissible for this to contain
 *                           some hexes that are also in destinations.)
 *
 * @param[in]  teleporter    If not nullptr, teleportation will be considered, using
 *                           this unit's abilities.
 * @param[in]  current_team  If not nullptr, enemies of this team can obstruct routes
 *                           both by occupying hexes and by exerting zones of control.
 *                           In addition, the presence of units can affect
 *                           teleportation options.
 * @param[in]  skirmisher    If not nullptr, use this to determine where ZoC can and
 *                           cannot be ignored (due to this unit having or not
 *                           having the skirmisher ability).
 *                           If nullptr, then ignore all zones of control.
 *                           (No effect if current_team is nullptr).
 * @param[in]  viewing_team  If not nullptr, use this team's vision when detecting
 *                           enemy units and teleport destinations.
 *                           If nullptr, then "see all".
 *                           (No effect if teleporter and current_team are both nullptr.)
 * @param[in]  jamming_map   The relevant "jamming" of the costs being used
 *                           (currently only used with vision costs).
 * @param[out] full_cost_map If not nullptr, build a cost_map instead of destinations.
 *                           Destinations is ignored.
 *                           full_cost_map is a vector of pairs. The first entry is the
 *                           cost itself, the second how many units already visited this hex
 * @param[in]  check_vision  If true, use vision check for teleports, that is, ignore
 *                           units potentially blocking the teleport exit
 */
static void find_routes(
		const map_location & origin, const movetype::terrain_costs & costs,
		bool slowed, int moves_left, int max_moves, int turns_left,
		paths::dest_vect & destinations, std::set<map_location> * edges,
		const unit * teleporter, const team * current_team,
		const unit * skirmisher, const team * viewing_team,
		const std::map<map_location, int> * jamming_map=nullptr,
		std::vector<std::pair<int, int> > * full_cost_map=nullptr, bool check_vision=false)
{
	const gamemap& map = resources::gameboard->map();

	const bool see_all =  viewing_team == nullptr;
	// When see_all is true, the viewing team never matters, but we still
	// need to supply one to some functions.
	if ( viewing_team == nullptr )
		viewing_team = &resources::gameboard->teams().front();

	// Build a teleport map, if needed.
	const teleport_map teleports = teleporter ?
			get_teleport_locations(*teleporter, *viewing_team, see_all, current_team == nullptr, check_vision) :
			teleport_map();

	// Since this is called so often, keep memory reserved for the node list.
	static std::vector<findroute_node> nodes;
	static unsigned search_counter = 0;
	// Incrementing search_counter means we ignore results from earlier searches.
	++search_counter;
	// Whenever the counter cycles, trash the contents of nodes and restart at 1.
	if ( search_counter == 0 ) {
		nodes.resize(0);
		search_counter = 1;
	}
	// Initialize the nodes for this search.
	nodes.resize(map.w() * map.h());
	findroute_comp node_comp(nodes);
	findroute_indexer index(map.w(), map.h());

	// Check if full_cost_map has the correct size.
	// If not, ignore it. If yes, initialize the start position.
	if ( full_cost_map ) {
		if ( full_cost_map->size() != static_cast<unsigned>(map.w() * map.h()) )
			full_cost_map = nullptr;
		else {
			if ( (*full_cost_map)[index(origin)].second == 0 )
				(*full_cost_map)[index(origin)].first = 0;
			(*full_cost_map)[index(origin)].second += 1;
		}
	}

	// Used to optimize the final collection of routes.
	int xmin = origin.x, xmax = origin.x, ymin = origin.y, ymax = origin.y;
	int nb_dest = 1;

	// Record the starting location.
	assert(index(origin) >= 0);
	nodes[index(origin)] = findroute_node(moves_left, turns_left,
	                                      map_location::null_location(),
	                                      search_counter);
	// Begin the search at the starting location.
	std::vector<int> hexes_to_process(1, index(origin));  // Will be maintained as a heap.

	while ( !hexes_to_process.empty() ) {
		// Process the hex closest to the origin.
		const int cur_index = hexes_to_process.front();
		const map_location cur_hex = index(cur_index);
		const findroute_node& current = nodes[cur_index];
		// Remove from the heap.
		std::pop_heap(hexes_to_process.begin(), hexes_to_process.end(), node_comp);
		hexes_to_process.pop_back();

		// Get the locations adjacent to current.
		std::vector<map_location> adj_locs(6);
		get_adjacent_tiles(cur_hex, &adj_locs[0]);
		if ( teleporter ) {
			std::set<map_location> allowed_teleports;
			teleports.get_adjacents(allowed_teleports, cur_hex);
			adj_locs.insert(adj_locs.end(), allowed_teleports.begin(), allowed_teleports.end());
		}
		for ( int i = adj_locs.size()-1; i >= 0; --i ) {
			// Get the node associated with this location.
			const map_location & next_hex = adj_locs[i];
			const int next_index = index(next_hex);
			if ( next_index < 0 ) {
				// Off the map.
				if ( edges != nullptr )
					edges->insert(next_hex);
				continue;
			}
			findroute_node & next = nodes[next_index];

			// Skip nodes we have already collected.
			// (Since no previously checked routes were longer than
			// the current one, the current route cannot be shorter.)
			// (Significant difference from classic Dijkstra: we have
			// vertex weights, not edge weights.)
			if ( next.search_num == search_counter )
				continue;

			// If we go to next, it will be from current.
			next.prev = cur_hex;

			// Calculate the cost of entering next_hex.
			int cost = costs.cost(map[next_hex], slowed);
			if ( jamming_map ) {
				const std::map<map_location, int>::const_iterator jam_it =
					jamming_map->find(next_hex);
				if ( jam_it != jamming_map->end() )
					cost += jam_it->second;
			}

			// Calculate movement remaining after entering next_hex.
			next.moves_left = current.moves_left - cost;
			next.turns_left = current.turns_left;
			if ( next.moves_left < 0 ) {
				// Have to delay until the next turn.
				next.turns_left--;
				next.moves_left = max_moves - cost;
			}
			if ( next.moves_left < 0 || next.turns_left < 0 ) {
				// Either can never enter this hex or out of turns.
				if ( edges != nullptr )
					edges->insert(next_hex);
				continue;
			}

			if ( current_team ) {
				// Account for enemy units.
				const unit *v = resources::gameboard->get_visible_unit(next_hex, *viewing_team, see_all);
				if ( v && current_team->is_enemy(v->side()) ) {
					// Cannot enter enemy hexes.
					if ( edges != nullptr )
						edges->insert(next_hex);
					continue;
				}

				if ( skirmisher  &&  next.moves_left > 0  &&
				     enemy_zoc(*current_team, next_hex, *viewing_team, see_all)  &&
				     !skirmisher->get_ability_bool("skirmisher", next_hex, *resources::gameboard) ) {
					next.moves_left = 0;
				}
			}

			// Update full_cost_map
			if ( full_cost_map ) {
				if ( (*full_cost_map)[next_index].second == 0 )
					(*full_cost_map)[next_index].first = 0;
				int summed_cost = (turns_left - next.turns_left + 1) * max_moves - next.moves_left;
				(*full_cost_map)[next_index].first += summed_cost;
				(*full_cost_map)[next_index].second += 1;
			}

			// Mark next as being collected.
			next.search_num = search_counter;

			// Add this node to the heap.
			hexes_to_process.push_back(next_index);
			std::push_heap(hexes_to_process.begin(), hexes_to_process.end(), node_comp);

			// Bookkeeping (for later).
			++nb_dest;
			if ( next_hex.x < xmin )
				xmin = next_hex.x;
			else if ( xmax < next_hex.x )
				xmax = next_hex.x;
			if ( next_hex.y < ymin )
				ymin = next_hex.y;
			else if ( ymax < next_hex.y )
				ymax = next_hex.y;
		}//for (i)
	}//while (hexes_to_process)

	// Currently the only caller who uses full_cost_map doesn't need the
	// destinations. We can skip this part.
	if ( full_cost_map ) {
		return;
	}

	// Build the routes for every map_location that we reached.
	// The ordering must be compatible with map_location::operator<.
	destinations.reserve(nb_dest);
	for (int x = xmin; x <= xmax; ++x) {
		for (int y = ymin; y <= ymax; ++y)
		{
			const findroute_node &n = nodes[index(x,y)];
			if ( n.search_num == search_counter ) {
				paths::step s =
					{ map_location(x,y), n.prev, n.moves_left + n.turns_left*max_moves };
				destinations.push_back(s);
			}
		}
	}
}

static bool step_compare(const paths::step& a, const map_location& b) {
	return a.curr < b;
}

paths::dest_vect::const_iterator paths::dest_vect::find(const map_location &loc) const
{
	const_iterator i = std::lower_bound(begin(), end(), loc, step_compare);
	if (i != end() && i->curr != loc) return end();
	return i;
}

void paths::dest_vect::insert(const map_location &loc)
{
	iterator i = std::lower_bound(begin(), end(), loc, step_compare);
	if (i != end() && i->curr == loc) return;
	paths::step s { loc, map_location(), 0 };
	std::vector<step>::insert(i, s);
}

/**
 * Returns the path going from the source point (included) to the
 * destination point @a j (excluded).
 */
std::vector<map_location> paths::dest_vect::get_path(const const_iterator &j) const
{
	std::vector<map_location> path;
	if (!j->prev.valid()) {
		path.push_back(j->curr);
	} else {
		const_iterator i = j;
		do {
			i = find(i->prev);
			assert(i != end());
			path.push_back(i->curr);
		} while (i->prev.valid());
	}
	std::reverse(path.begin(), path.end());
	return path;
}

bool paths::dest_vect::contains(const map_location &loc) const
{
	return find(loc) != end();
}

/**
 * Construct a list of paths for the specified unit.
 *
 * This function is used for several purposes, including showing a unit's
 * potential moves and generating currently possible paths.
 * @param u                The unit whose moves and movement type will be used.
 * @param force_ignore_zoc Set to true to completely ignore zones of control.
 * @param allow_teleport   Set to true to consider teleportation abilities.
 * @param viewing_team     Usually the current team, except for "show enemy moves", etc.
 * @param additional_turns The number of turns to account for, in addition to the current.
 * @param see_all          Set to true to remove unit visibility from consideration.
 * @param ignore_units     Set to true if units should never obstruct paths (implies ignoring ZoC as well).
 */
paths::paths(const unit& u, bool force_ignore_zoc,
		bool allow_teleport, const team &viewing_team,
		int additional_turns, bool see_all, bool ignore_units)
	: destinations()
{
	std::vector<team> const &teams = resources::gameboard->teams();
	if (u.side() < 1 || u.side() > int(teams.size())) {
		return;
	}

	find_routes(u.get_location(), u.movement_type().get_movement(),
	            u.get_state(unit::STATE_SLOWED), u.movement_left(),
	            u.total_movement(), additional_turns, destinations, nullptr,
	            allow_teleport ? &u : nullptr,
	            ignore_units ? nullptr : &teams[u.side()-1],
	            force_ignore_zoc ? nullptr : &u,
	            see_all ? nullptr : &viewing_team);
}

/**
 * Virtual destructor to support child classes.
 */
paths::~paths()
{
}

/**
 * Constructs a list of vision paths for a unit.
 *
 * This is used to construct a list of hexes that the indicated unit can see.
 * It differs from pathfinding in that it will only ever go out one turn,
 * and that it will also collect a set of border hexes (the "one hex beyond"
 * movement to which vision extends).
 * @param viewer     The unit doing the viewing.
 * @param loc        The location from which the viewing occurs
 *                   (does not have to be the unit's location).
 */
vision_path::vision_path(const unit& viewer, map_location const &loc,
                         const std::map<map_location, int>& jamming_map)
	: paths(), edges()
{
	const int sight_range = viewer.vision();

	// The three nullptr parameters indicate (in order):
	// ignore units, ignore ZoC (no effect), and don't build a cost_map.
	team const& viewing_team = resources::gameboard->teams()[resources::screen->viewing_team()];
	find_routes(loc, viewer.movement_type().get_vision(),
	            viewer.get_state(unit::STATE_SLOWED), sight_range, sight_range,
	            0, destinations, &edges, &viewer, nullptr, nullptr, &viewing_team, &jamming_map, nullptr, true);
}

/**
 * Constructs a list of vision paths for a unit.
 *
 * This constructor is provided so that only the relevant portion of a unit's
 * data is required to construct the vision paths.
 * @param view_costs   The vision costs of the unit doing the viewing.
 * @param slowed       Whether or not the unit is slowed.
 * @param sight_range  The vision() of the unit.
 * @param loc          The location from which the viewing occurs
 *                     (does not have to be the unit's location).
 */
vision_path::vision_path(const movetype::terrain_costs & view_costs, bool slowed,
                         int sight_range, const map_location & loc,
                         const std::map<map_location, int>& jamming_map)
	: paths(), edges()
{
	// The three nullptr parameters indicate (in order):
	// ignore units, ignore ZoC (no effect), and don't build a cost_map.
	team const& viewing_team = resources::gameboard->teams()[resources::screen->viewing_team()];
	const unit_map::const_iterator u = resources::gameboard->units().find(loc);
	find_routes(loc, view_costs, slowed, sight_range, sight_range, 0,
	            destinations, &edges, u.valid() ? &*u : nullptr, nullptr, nullptr, &viewing_team, &jamming_map, nullptr, true);
}

/// Default destructor
vision_path::~vision_path()
{
}


/**
 * Constructs a list of jamming paths for a unit.
 *
 * This is used to construct a list of hexes that the indicated unit can jam.
 * It differs from pathfinding in that it will only ever go out one turn.
 * @param jammer     The unit doing the jamming.
 * @param loc        The location from which the jamming occurs
 *                   (does not have to be the unit's location).
 */
jamming_path::jamming_path(const unit& jammer, map_location const &loc)
	: paths()
{
	const int jamming_range = jammer.jamming();

	// The five nullptr parameters indicate (in order): no edges, no teleports,
	// ignore units, ignore ZoC (no effect), and see all (no effect).
	find_routes(loc, jammer.movement_type().get_jamming(),
	            jammer.get_state(unit::STATE_SLOWED), jamming_range, jamming_range,
	            0, destinations, nullptr, nullptr, nullptr, nullptr, nullptr);
}

/// Default destructor
jamming_path::~jamming_path()
{
}

marked_route mark_route(const plain_route &rt)
{
	marked_route res;

	if (rt.steps.empty()) return marked_route();
	res.route = rt;

	unit_map::const_iterator it = resources::gameboard->units().find(rt.steps.front());
	if (it == resources::gameboard->units().end()) return marked_route();
	unit const& u = *it;

	int turns = 0;
	int movement = u.movement_left();
	const team& unit_team = resources::gameboard->get_team(u.side());
	bool zoc = false;

	std::vector<map_location>::const_iterator i = rt.steps.begin();

	for (; i !=rt.steps.end(); ++i) {
		bool last_step = (i+1 == rt.steps.end());

		// move_cost of the next step is irrelevant for the last step
		assert(last_step || resources::gameboard->map().on_board(*(i+1)));
		const int move_cost = last_step ? 0 : u.movement_cost((resources::gameboard->map())[*(i+1)]);

		team const& viewing_team = resources::gameboard->teams()[resources::screen->viewing_team()];

		if (last_step || zoc || move_cost > movement) {
			// check if we stop an a village and so maybe capture it
			// if it's an enemy unit and a fogged village, we assume a capture
			// (if he already owns it, we can't know that)
			// if it's not an enemy, we can always know if he owns the village
			bool capture = resources::gameboard->map().is_village(*i) && ( !unit_team.owns_village(*i)
				 || (viewing_team.is_enemy(u.side()) && viewing_team.fogged(*i)) );

			++turns;

			bool invisible = u.invisible(*i, *resources::gameboard, false);

			res.marks[*i] = marked_route::mark(turns, zoc, capture, invisible);

			if (last_step) break; // finished and we used dummy move_cost

			movement = u.total_movement();
			if(move_cost > movement) {
				return res; //we can't reach destination
			}
		}

		zoc = enemy_zoc(unit_team, *(i + 1), viewing_team)
					&& !u.get_ability_bool("skirmisher", *(i+1), *resources::gameboard);

		if (zoc) {
			movement = 0;
		} else {
			movement -= move_cost;
		}
	}

	return res;
}

shortest_path_calculator::shortest_path_calculator(unit const &u, team const &t,
		std::vector<team> const &teams, gamemap const &map,
		bool ignore_unit, bool ignore_defense, bool see_all)
	: unit_(u), viewing_team_(t), teams_(teams), map_(map),
	  movement_left_(unit_.movement_left()),
	  total_movement_(unit_.total_movement()),
	  ignore_unit_(ignore_unit), ignore_defense_(ignore_defense),
	  see_all_(see_all)
{}

double shortest_path_calculator::cost(const map_location& loc, const double so_far) const
{
	assert(map_.on_board(loc));

	// loc is shrouded, consider it impassable
	// NOTE: This is why AI must avoid to use shroud
	if (!see_all_ && viewing_team_.shrouded(loc))
		return getNoPathValue();

	const t_translation::terrain_code terrain = map_[loc];
	const int terrain_cost = unit_.movement_cost(terrain);
	// Pathfinding heuristic: the cost must be at least 1
	VALIDATE(terrain_cost >= 1, _("Terrain with a movement cost less than 1 encountered."));

	// Compute how many movement points are left in the game turn
	// needed to reach the previous hex.
	// total_movement_ is not zero, thanks to the pathfinding heuristic
	int remaining_movement = movement_left_ - static_cast<int>(so_far);
	if (remaining_movement < 0) {
		remaining_movement = total_movement_ - (-remaining_movement) % total_movement_;
	}

	if (terrain_cost >= movetype::UNREACHABLE || (total_movement_ < terrain_cost && remaining_movement < terrain_cost)) {
		return getNoPathValue();
	}

	int other_unit_subcost = 0;
	if (!ignore_unit_) {
		const unit *other_unit =
			resources::gameboard->get_visible_unit(loc, viewing_team_, see_all_);

		// We can't traverse visible enemy and we also prefer empty hexes
		// (less blocking in multi-turn moves and better when exploring fog,
		// because we can't stop on a friend)

		if (other_unit)
		{
			if (teams_[unit_.side() - 1].is_enemy(other_unit->side()))
				return getNoPathValue();
			else
				// This value will be used with the defense_subcost (see below)
				// The 1 here means: consider occupied hex as a -1% defense
				// (less important than 10% defense because friends may move)
				other_unit_subcost = 1;
		}
	}

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
	if (!ignore_unit_ && remaining_movement != terrain_cost
	    && enemy_zoc(teams_[unit_.side()-1], loc, viewing_team_, see_all_)
			&& !unit_.get_ability_bool("skirmisher", loc, *resources::gameboard)) {
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

move_type_path_calculator::move_type_path_calculator(const movetype& mt, int movement_left, int total_movement, team const &t, gamemap const &map)
	: movement_type_(mt), movement_left_(movement_left),
	  total_movement_(total_movement), viewing_team_(t), map_(map)
{}

// This is an simplified version of shortest_path_calculator (see above for explanation)
double move_type_path_calculator::cost(const map_location& loc, const double so_far) const
{
	assert(map_.on_board(loc));
	if (viewing_team_.shrouded(loc))
		return getNoPathValue();

	const t_translation::terrain_code terrain = map_[loc];
	const int terrain_cost = movement_type_.movement_cost(terrain);

	if (total_movement_ < terrain_cost)
		return getNoPathValue();

	int remaining_movement = movement_left_ - static_cast<int>(so_far);
	if (remaining_movement < 0)
		remaining_movement = total_movement_ - (-remaining_movement) % total_movement_;

	int move_cost = 0;

	if (remaining_movement < terrain_cost) {
		move_cost += remaining_movement;
	}

	move_cost += terrain_cost;

	return move_cost;
}


emergency_path_calculator::emergency_path_calculator(const unit& u, const gamemap& map)
	: unit_(u), map_(map)
{}

double emergency_path_calculator::cost(const map_location& loc, const double) const
{
	assert(map_.on_board(loc));

	return unit_.movement_cost(map_[loc]);
}

dummy_path_calculator::dummy_path_calculator(const unit&, const gamemap&)
{}

double dummy_path_calculator::cost(const map_location&, const double) const
{
	return 1.0;
}

/**
 * Constructs a cost-map. For a unit each hex is mapped to the cost the
 * unit will need to reach this hex. Considers movement-loss caused by
 * turn changes.
 * Can also used with multiple units to accumulate their costs efficiently.
 * Will also count how many units could reach a hex for easy normalization.
 * @param u the unit
 * @param force_ignore_zoc Set to true to completely ignore zones of control.
 * @param allow_teleport   Set to true to consider teleportation abilities.
 * @param viewing_team     Usually the current team, except for "show enemy moves", etc.
 * @param see_all          Set to true to remove unit visibility from consideration.
 * @param ignore_units     Set to true if units should never obstruct paths (implies ignoring ZoC as well).
 */
full_cost_map::full_cost_map(const unit& u, bool force_ignore_zoc,
		bool allow_teleport, const team &viewing_team,
		bool see_all, bool ignore_units)
	:force_ignore_zoc_(force_ignore_zoc), allow_teleport_(allow_teleport),
	 viewing_team_(viewing_team), see_all_(see_all), ignore_units_(ignore_units)
{
	const gamemap& map = resources::gameboard->map();
	cost_map = std::vector<std::pair<int, int> >(map.w() * map.h(), std::make_pair(-1, 0));
	add_unit(u);
}

/**
 * Same as other constructor but without unit. Use this when working
 * with add_unit().
 */
full_cost_map::full_cost_map(bool force_ignore_zoc,
		bool allow_teleport, const team &viewing_team,
		bool see_all, bool ignore_units)
	:force_ignore_zoc_(force_ignore_zoc), allow_teleport_(allow_teleport),
	 viewing_team_(viewing_team), see_all_(see_all), ignore_units_(ignore_units)
{
	const gamemap& map = resources::gameboard->map();
	cost_map = std::vector<std::pair<int, int> >(map.w() * map.h(), std::make_pair(-1, 0));
}

/**
 * Adds a units cost map to cost_map (increments the elements in cost_map)
 * @param u a real existing unit on the map
 */
void full_cost_map::add_unit(const unit& u, bool use_max_moves)
{
	std::vector<team> const &teams = resources::gameboard->teams();
	if (u.side() < 1 || u.side() > int(teams.size())) {
		return;
	}

	// We don't need the destinations, but find_routes() wants to have this parameter
	paths::dest_vect dummy = paths::dest_vect();

		find_routes(u.get_location(), u.movement_type().get_movement(),
		            u.get_state(unit::STATE_SLOWED),
		            (use_max_moves) ? u.total_movement() : u.movement_left(),
		            u.total_movement(), 99, dummy, nullptr,
		            allow_teleport_ ? &u : nullptr,
		            ignore_units_ ? nullptr : &teams[u.side()-1],
		            force_ignore_zoc_ ? nullptr : &u,
		            see_all_ ? nullptr : &viewing_team_,
		            nullptr, &cost_map);
}

/**
 * Adds a units cost map to cost_map (increments the elements in cost_map)
 * This function can be used to generate a cost_map with a non existing unit.
 * @param origin the location on the map from where the calculations shall start
 * @param ut the unit type we are interested in
 * @param side the side of the unit. Important for zocs.
 */
void full_cost_map::add_unit(const map_location& origin, const unit_type* const ut, int side)
{
	if (!ut) {
		return;
	}
	unit u(*ut, side, false);
	u.set_location(origin);
	add_unit(u);
}

/**
 * Accessor for the cost/reach-amount pairs.
 * Read comment in pathfind.hpp to cost_map.
 *
 * @return the entry of the cost_map at (x, y)
 *         or (-1, 0) if value is not set or (x, y) is invalid.
 */
std::pair<int, int> full_cost_map::get_pair_at(int x, int y) const
{
	const gamemap& map = resources::gameboard->map();
	assert(cost_map.size() == static_cast<unsigned>(map.w() * map.h()));

	if (x < 0 || x >= map.w() || y < 0 || y >= map.h()) {
		return std::make_pair(-1, 0);  // invalid
	}

	return cost_map[x + (y * map.w())];
}

/**
 * Accessor for the costs.
 *
 * @return the value of the cost_map at (x, y)
 *         or -1 if value is not set or (x, y) is invalid.
 */
int full_cost_map::get_cost_at(int x, int y) const
{
	return get_pair_at(x, y).first;
}

/**
 * Accessor for the costs.
 *
 * @return The average cost of all added units for this hex
 *         or -1 if no unit can reach the hex.
 */
double full_cost_map::get_average_cost_at(int x, int y) const
{
	if (get_pair_at(x, y).second == 0) {
		return -1;
	} else {
		return static_cast<double>(get_pair_at(x, y).first) / get_pair_at(x, y).second;
	}
}
}//namespace pathfind
