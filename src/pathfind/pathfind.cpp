/* $Id$ */
/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2013 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "global.hpp"

#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"

#include "game_display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "wml_exception.hpp"

#include <boost/foreach.hpp>

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
                              const unit* pass_check, const team* shroud_check)
{
	const gamemap & map = *resources::game_map;
	const unit_map & units = *resources::units;

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
		BOOST_FOREACH(const map_location &loc, tiles_checking)
		{
			// Skip shrouded locations.
			if ( do_shroud  &&  shroud_check->shrouded(loc) )
				continue;
			//If this area is not a castle but should, skip it.
			if ( vacancy == VACANT_CASTLE  &&  !map.is_castle(loc) ) continue;
			const bool pass_check_and_unreachable = pass_check
				&& pass_check->movement_cost(map[loc]) == movetype::UNREACHABLE;
			//If the unit can't reach the tile and we have searched
			//an area of at least radius 10 (arbitrary), skip the tile.
			//Neccessary for cases such as an unreachable
			//starting hex surrounded by 6 other unreachable hexes, in which case
			//the algorithm would not even search distance==1
			//even if there's a reachable hex for distance==2.
			if (pass_check_and_unreachable && distance > 10) continue;
			//If the hex is empty and we do either no pass check or the hex is reachable, return it.
			if (units.find(loc) == units.end() && !pass_check_and_unreachable) return loc;
			map_location adjs[6];
			get_adjacent_tiles(loc,adjs);
			BOOST_FOREACH(const map_location &loc, adjs)
			{
				if (!map.on_board(loc)) continue;
				// Add the tile to be checked if it hasn't already been and
				// isn't being checked.
				if (tiles_checked.find(loc) == tiles_checked.end() &&
				    tiles_checking.find(loc) == tiles_checking.end())
				{
					pending_tiles_to_check.insert(loc);
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
	                        NULL, &(*resources::teams)[leader.side()-1]);
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
		const unit *u = get_visible_unit(locs[i], viewing_team, see_all);
		if ( u  &&  current_team.is_enemy(u->side())  &&  u->emits_zoc() )
			return true;
	}

	// No adjacent tiles had an enemy exerting ZoC over loc.
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
		: movement_left(moves)
		, turns_left(turns)
		, prev(p)
		, curr(c)
		, in(0)
	{
	}

	node()
		: movement_left(0)
		, turns_left(0)
		, prev()
		, curr()
		, in(0)
	{
	}

	bool operator<(const node& o) const {
		return turns_left > o.turns_left
				|| (turns_left == o.turns_left
						&& movement_left > o.movement_left);
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
 * @param[in]  teleporter    If not NULL, teleportaion will be considered, using
 *                           this unit's abilities.
 * @param[in]  current_team  If not NULL, enemies of this team can obstruct routes
 *                           both by occupying hexes and by exerting zones of control.
 *                           In addition, the presence of units can affect
 *                           teleportation options.
 * @param[in]  skirmisher    If not NULL, use this to determine where ZoC can and
 *                           cannot be ignored (due to this unit having or not
 *                           having the skirmisher ability).
 *                           If NULL, then ignore all zones of control.
 *                           (No effect if current_team is NULL).
 * @param[in]  viewing_team  If not NULL, use this team's vision when detecting
 *                           enemy units and teleport destinations.
 *                           If NULL, then "see all".
 *                           (No effect if teleporter and current_team are both NULL.)
 * @param[in]  jamming_map   The relevant "jamming" of the costs being used
 *                           (currently only used with vision costs).
 */
static void find_routes(
		const map_location & origin, const movetype::terrain_costs & costs,
		bool slowed, int moves_left, int max_moves, int turns_left,
		paths::dest_vect & destinations, std::set<map_location> * edges,
		const unit * teleporter, const team * current_team,
		const unit * skirmisher, const team * viewing_team,
		const std::map<map_location, int> * jamming_map=NULL)
{
	const gamemap& map = *resources::game_map;

	const bool see_all =  viewing_team == NULL;
	// When see_all is true, the viewing team never matters, but we still
	// need to supply one to some functions.
	if ( viewing_team == NULL )
		viewing_team = &resources::teams->front();

	teleport_map teleports;
	if ( teleporter ) {
		teleports = get_teleport_locations(*teleporter, *viewing_team, see_all,
		                                   current_team == NULL);
	}

	search_counter += 2;
	if (search_counter == 0) search_counter = 2;

	static std::vector<node> nodes;
	nodes.resize(map.w() * map.h());

	indexer index(map.w(), map.h());
	comp node_comp(nodes);

	int xmin = origin.x, xmax = origin.x, ymin = origin.y, ymax = origin.y;
	int nb_dest = 1;

	nodes[index(origin)] = node(moves_left, turns_left, map_location::null_location, origin);
	std::vector<int> hexes_to_process;
	hexes_to_process.push_back(index(origin));

	while ( !hexes_to_process.empty() ) {
		node& n = nodes[hexes_to_process.front()];
		std::pop_heap(hexes_to_process.begin(), hexes_to_process.end(), node_comp);
		hexes_to_process.pop_back();
		n.in = search_counter;

		std::set<map_location> allowed_teleports;
		teleports.get_adjacents(allowed_teleports, n.curr);
		std::vector<map_location> adj_locs(6 + allowed_teleports.size());
		std::copy(allowed_teleports.begin(), allowed_teleports.end(), adj_locs.begin() + 6);
		get_adjacent_tiles(n.curr, &adj_locs[0]);
		for ( int i = adj_locs.size(); i-- > 0; ) {
			// Get the node associated with this location.
			const map_location & next_hex = adj_locs[i];
			if ( !next_hex.valid(map.w(), map.h()) ) {
				if ( edges != NULL )
					edges->insert(next_hex);
				continue;
			}

			if ( next_hex == n.curr ) continue;

			node& next = nodes[index(next_hex)];
			bool next_visited = next.in - search_counter <= 1u;

			// Classic Dijkstra allow to skip chosen nodes (with next.in==search_counter)
			// But the cost function and hex grid allow to also skip visited nodes:
			// if next was visited, then we already have a path 'src-..-n2-next'
			// - n2 was chosen before n, meaning that it is nearer to src.
			// - the cost of 'n-next' can't be smaller than 'n2-next' because
			//   cost is independent of direction and we don't have more MP at n
			//   (important because more MP may allow to avoid waiting next turn)
			// Thus, 'src-..-n-next' can't be shorter.
			if (next_visited) continue;

			int cost = costs.cost(map[next_hex], slowed);
			if ( jamming_map ) {
				const std::map<map_location, int>::const_iterator jam_it =
					jamming_map->find(next_hex);
				if ( jam_it != jamming_map->end() )
					cost += jam_it->second;
			}

			node t = node(n.movement_left, n.turns_left, n.curr, next_hex);
			if (t.movement_left < cost) {
				t.movement_left = max_moves;
				t.turns_left--;
			}

			if (t.movement_left < cost || t.turns_left < 0) {
				if ( edges != NULL )
					edges->insert(t.curr);
				continue;
			}

			t.movement_left -= cost;

			if ( current_team ) {
				// Account for enemy units.
				const unit *v = get_visible_unit(next_hex, *viewing_team, see_all);
				if ( v && current_team->is_enemy(v->side()) ) {
					if ( edges != NULL )
						edges->insert(t.curr);
					continue;
				}

				if ( skirmisher  &&  t.movement_left > 0  &&
				     enemy_zoc(*current_team, next_hex, *viewing_team, see_all)  &&
				     !skirmisher->get_ability_bool("skirmisher", next_hex) ) {
					t.movement_left = 0;
				}
			}

			++nb_dest;
			int x = next_hex.x;
			if (x < xmin) xmin = x;
			if (xmax < x) xmax = x;
			int y = next_hex.y;
			if (y < ymin) ymin = y;
			if (ymax < y) ymax = y;

			bool in_list = next.in == search_counter + 1;
			t.in = search_counter + 1;
			next = t;

			// if already in the priority queue then we just update it, else push it.
			if (in_list) { // never happen see next_visited above
				std::push_heap(hexes_to_process.begin(), std::find(hexes_to_process.begin(), hexes_to_process.end(), index(next_hex)) + 1, node_comp);
			} else {
				hexes_to_process.push_back(index(next_hex));
				std::push_heap(hexes_to_process.begin(), hexes_to_process.end(), node_comp);
			}
		}
	}

	// Build the routes for every map_location that we reached.
	// The ordering must be compatible with map_location::operator<.
	destinations.reserve(nb_dest);
	for (int x = xmin; x <= xmax; ++x) {
		for (int y = ymin; y <= ymax; ++y)
		{
			const node &n = nodes[index(map_location(x, y))];
			if (n.in - search_counter > 1u) continue;
			paths::step s =
				{ n.curr, n.prev, n.movement_left + n.turns_left * max_moves };
			destinations.push_back(s);
		}
	}
}

static paths::dest_vect::iterator lower_bound(paths::dest_vect &v, const map_location &loc)
{
	size_t sz = v.size(), pos = 0;
	while (sz)
	{
		if (v[pos + sz / 2].curr < loc) {
			pos = pos + sz / 2 + 1;
			sz = sz - sz / 2 - 1;
		} else sz = sz / 2;
	}
	return v.begin() + pos;
}

paths::dest_vect::const_iterator paths::dest_vect::find(const map_location &loc) const
{
	const_iterator i = lower_bound(const_cast<dest_vect &>(*this), loc), i_end = end();
	if (i != i_end && i->curr != loc) i = i_end;
	return i;
}

void paths::dest_vect::insert(const map_location &loc)
{
	iterator i = lower_bound(*this, loc), i_end = end();
	if (i != i_end && i->curr == loc) return;
	paths::step s = { loc, map_location(), 0 };
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
	std::vector<team> const &teams = *resources::teams;
	if (u.side() < 1 || u.side() > int(teams.size())) {
		return;
	}

	find_routes(u.get_location(), u.movement_type().get_movement(),
	            u.get_state(unit::STATE_SLOWED), u.movement_left(),
	            u.total_movement(), additional_turns, destinations, NULL,
	            allow_teleport ? &u : NULL,
	            ignore_units ? NULL : &teams[u.side()-1],
	            force_ignore_zoc ? NULL : &u,
	            see_all ? NULL : &viewing_team);
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

	// The four NULL parameters indicate (in order): no teleports,
	// ignore units, ignore ZoC (no effect), and see all (no effect).
	find_routes(loc, viewer.movement_type().get_vision(),
	            viewer.get_state(unit::STATE_SLOWED), sight_range, sight_range,
	            0, destinations, &edges, NULL, NULL, NULL, NULL, &jamming_map);
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

	// The five NULL parameters indicate (in order): no edges, no teleports,
	// ignore units, ignore ZoC (no effect), and see all (no effect).
	find_routes(loc, jammer.movement_type().get_jamming(),
	            jammer.get_state(unit::STATE_SLOWED), jamming_range, jamming_range,
	            0, destinations, NULL, NULL, NULL, NULL, NULL);
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

	unit_map::const_iterator it = resources::units->find(rt.steps.front());
	if (it == resources::units->end()) return marked_route();
	unit const& u = *it;

	int turns = 0;
	int movement = u.movement_left();
	const team& unit_team = (*resources::teams)[u.side()-1];
	bool zoc = false;

	std::vector<map_location>::const_iterator i = rt.steps.begin();

	for (; i !=rt.steps.end(); ++i) {
		bool last_step = (i+1 == rt.steps.end());

		// move_cost of the next step is irrelevant for the last step
		assert(last_step || resources::game_map->on_board(*(i+1)));
		const int move_cost = last_step ? 0 : u.movement_cost((*resources::game_map)[*(i+1)]);

		team const& viewing_team = (*resources::teams)[resources::screen->viewing_team()];

		if (last_step || zoc || move_cost > movement) {
			// check if we stop an a village and so maybe capture it
			// if it's an enemy unit and a fogged village, we assume a capture
			// (if he already owns it, we can't know that)
			// if it's not an enemy, we can always know if he owns the village
			bool capture = resources::game_map->is_village(*i) && ( !unit_team.owns_village(*i)
				 || (viewing_team.is_enemy(u.side()) && viewing_team.fogged(*i)) );

			++turns;

			bool invisible = u.invisible(*i,false);

			res.marks[*i] = marked_route::mark(turns, zoc, capture, invisible);

			if (last_step) break; // finished and we used dummy move_cost

			movement = u.total_movement();
			if(move_cost > movement) {
				return res; //we can't reach destination
			}
		}

		zoc = enemy_zoc(unit_team, *(i + 1), viewing_team)
					&& !u.get_ability_bool("skirmisher", *(i+1));

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

	const t_translation::t_terrain terrain = map_[loc];
	int const terrain_cost = unit_.movement_cost(terrain);
	// Pathfinding heuristic: the cost must be at least 1
	VALIDATE(terrain_cost >= 1, _("Terrain with a movement cost less than 1 encountered."));

	// total MP is not enough to move on this terrain: impassable
	if (total_movement_ < terrain_cost)
		return getNoPathValue();

	int other_unit_subcost = 0;
	if (!ignore_unit_) {
		const unit *other_unit =
			get_visible_unit(loc, viewing_team_, see_all_);

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
	if (!ignore_unit_ && remaining_movement != terrain_cost
	    && enemy_zoc(teams_[unit_.side()-1], loc, viewing_team_, see_all_)
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

	const t_translation::t_terrain terrain = map_[loc];
	int const terrain_cost = movement_type_.movement_cost(terrain);

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


}//namespace pathfind
