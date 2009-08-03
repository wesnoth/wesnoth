/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Helper functions for the object which operates in the context of AI for specific side
 * This is part of AI interface
 * @file ai/default/contexts.cpp
 */

#include "contexts.hpp"

#include "../../attack_prediction.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

// =======================================================================
namespace ai {


// default ai context
default_ai_context::default_ai_context()
{
}


default_ai_context::~default_ai_context()
{
}

// default ai context proxy

default_ai_context_proxy::~default_ai_context_proxy()
{
}

void default_ai_context_proxy::init_default_ai_context_proxy(default_ai_context &target)
{
	init_readwrite_context_proxy(target);
	target_= &target.get_default_ai_context();
}


const int max_positions = 10000;


default_ai_context_impl::~default_ai_context_impl()
{
}


int default_ai_context_impl::count_free_hexes_in_castle(const map_location &loc, std::set<map_location> &checked_hexes)
{
	int ret = 0;
	unit_map &units_ = get_info().units;
	map_location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if (checked_hexes.find(adj[n]) != checked_hexes.end())
			continue;
		checked_hexes.insert(adj[n]);
		if (get_info().map.is_castle(adj[n])) {
			const unit_map::const_iterator u = units_.find(adj[n]);
			ret += count_free_hexes_in_castle(adj[n], checked_hexes);
			if (u == units_.end()
				|| (current_team().is_enemy(u->second.side())
					&& u->second.invisible(adj[n], units_, get_info().teams))
				|| ((&get_info().teams[u->second.side()-1]) == &current_team()
					&& u->second.movement_left() > 0)) {
				ret += 1;
			}
		}
	}
	return ret;
}


default_ai_context& default_ai_context_impl::get_default_ai_context(){
	return *this;
}


bool default_ai_context_impl::multistep_move_possible(const map_location& from,
	const map_location& to, const map_location& via,
	const moves_map& possible_moves) const
{
	unit_map &units_ = get_info().units;
	const unit_map::const_iterator i = units_.find(from);
	if(i != units_.end()) {
		if(from != via && to != via && units_.count(via) == 0) {
			LOG_AI << "when seeing if leader can move from "
				<< from << " -> " << to
				<< " seeing if can detour to keep at " << via << '\n';
			const std::map<map_location,paths>::const_iterator moves = possible_moves.find(from);
			if(moves != possible_moves.end()) {

				LOG_AI << "found leader moves..\n";

				// See if the unit can make it to 'via', and if it can,
				// how much movement it will have left when it gets there.
				paths::dest_vect::const_iterator itor =
					moves->second.destinations.find(via);
				if (itor != moves->second.destinations.end())
				{
					LOG_AI << "Can make it to keep with " << itor->move_left << " movement left.\n";
					unit temp_unit(i->second);
					temp_unit.set_movement(itor->move_left);
					const temporary_unit_placer unit_placer(units_,via,temp_unit);
					const paths unit_paths(get_info().map,units_,via,get_info().teams,false,false,current_team());

					LOG_AI << "Found " << unit_paths.destinations.size() << " moves for temp leader.\n";

					// See if this leader could make it back to the keep.
					if (unit_paths.destinations.contains(to)) {
						LOG_AI << "can make it back to the keep\n";
						return true;
					}
				}
			}
		}
	}

	return false;
}


int default_ai_context_impl::rate_terrain(const unit& u, const map_location& loc) const
{
	gamemap &map_ = get_info().map;
	const t_translation::t_terrain terrain = map_.get_terrain(loc);
	const int defense = u.defense_modifier(terrain);
	int rating = 100 - defense;

	const int healing_value = 10;
	const int friendly_village_value = 5;
	const int neutral_village_value = 10;
	const int enemy_village_value = 15;

	if(map_.gives_healing(terrain) && u.get_ability_bool("regenerates",loc) == false) {
		rating += healing_value;
	}

	if(map_.is_village(terrain)) {
		int owner = village_owner(loc, get_info().teams) + 1;

		if(owner == get_side()) {
			rating += friendly_village_value;
		} else if(owner == 0) {
			rating += neutral_village_value;
		} else {
			rating += enemy_village_value;
		}
	}

	return rating;
}

const map_location& default_ai_context_impl::suitable_keep(const map_location& leader_location, const paths& leader_paths){
	if (get_info().map.is_keep(leader_location)) {
		return leader_location; //if leader already on keep, then return leader_location
	}

	map_location const* best_free_keep = &map_location::null_location;
	double cost_to_best_free_keep = 0.0;

	map_location const* best_occupied_keep = &map_location::null_location;
	double cost_to_best_occupied_keep = 0.0;

	foreach (const paths::step &dest, leader_paths.destinations)
	{
		const map_location &loc = dest.curr;
		if (keeps().find(loc)!=keeps().end()){
			//@todo 1.7 move_left for 1-turn-moves is really "cost_to_get_there", it is just not renamed there yet. see r34430 for more detais.
			const int cost_to_loc = dest.move_left;
			if (get_info().units.count(loc) == 0) {
				if ((*best_free_keep==map_location::null_location)||(cost_to_loc<cost_to_best_free_keep)){
					best_free_keep = &loc;
					cost_to_best_free_keep = cost_to_loc;
				}
			} else {
				if ((*best_occupied_keep==map_location::null_location)||(cost_to_loc<cost_to_best_occupied_keep)){
					best_occupied_keep = &loc;
					cost_to_best_occupied_keep = cost_to_loc;
				}
			}
		}
	}

	if (*best_free_keep != map_location::null_location){
		return *best_free_keep; // if there is a free keep reachable during current turn, return it
	}

	if (*best_occupied_keep != map_location::null_location){
		return *best_occupied_keep; // if there is an occupied keep reachable during current turn, return it
	}

	return nearest_keep(leader_location); // return nearest keep
}


} //of namespace ai
