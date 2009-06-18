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

// default ai context impl
void default_ai_context_impl::add_recent_attack(map_location loc)
{
	attacks_.insert(loc);
}


bool default_ai_context_impl::attack_close(const map_location& loc) const
{
	for(std::set<map_location>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		if(distance_between(*i,loc) < 4) {
			return true;
		}
	}

	return false;
}


default_ai_context_impl::~default_ai_context_impl()
{
}


std::map<std::pair<map_location,const unit_type *>,
	std::pair<battle_context::unit_stats,battle_context::unit_stats> >& default_ai_context_impl::unit_stats_cache()
{
	return unit_stats_cache_;
}

const defensive_position& default_ai_context_impl::best_defensive_position(const map_location& loc,
		const move_map& dstsrc, const move_map& srcdst, const move_map& enemy_dstsrc)
{
	const unit_map::const_iterator itor = get_info().units.find(loc);
	if(itor == get_info().units.end()) {
		static defensive_position pos;
		pos.chance_to_hit = 0;
		pos.vulnerability = pos.support = 0;
		return pos;
	}

	const std::map<map_location,defensive_position>::const_iterator position =
		defensive_position_cache_.find(loc);

	if(position != defensive_position_cache_.end()) {
		return position->second;
	}

	defensive_position pos;
	pos.chance_to_hit = 100;
	pos.vulnerability = 10000.0;
	pos.support = 0.0;

	typedef move_map::const_iterator Itor;
	const std::pair<Itor,Itor> itors = srcdst.equal_range(loc);
	for(Itor i = itors.first; i != itors.second; ++i) {
		const int defense = itor->second.defense_modifier(get_info().map.get_terrain(i->second));
		if(defense > pos.chance_to_hit) {
			continue;
		}

		const double vulnerability = power_projection(i->second,enemy_dstsrc);
		const double support = power_projection(i->second,dstsrc);

		if(defense < pos.chance_to_hit || support - vulnerability > pos.support - pos.vulnerability) {
			pos.loc = i->second;
			pos.chance_to_hit = defense;
			pos.vulnerability = vulnerability;
			pos.support = support;
		}
	}

	defensive_position_cache_.insert(std::pair<map_location,defensive_position>(loc,pos));
	return defensive_position_cache_[loc];
}


std::map<map_location,defensive_position>& default_ai_context_impl::defensive_position_cache()
{
	return defensive_position_cache_;
}


default_ai_context& default_ai_context_impl::get_default_ai_context(){
	return *this;
}


void default_ai_context_impl::invalidate_defensive_position_cache()
{
	defensive_position_cache_.clear();
}


void default_ai_context_impl::invalidate_keeps_cache()
{
	keeps_.clear();
}


void default_ai_context_impl::invalidate_recent_attacks_list()
{
	attacks_.clear();
}


const std::set<map_location>& default_ai_context_impl::keeps()
{
	gamemap &map_ = get_info().map;
	if(keeps_.empty()) {
		// Generate the list of keeps:
		// iterate over the entire map and find all keeps.
		for(size_t x = 0; x != size_t(map_.w()); ++x) {
			for(size_t y = 0; y != size_t(map_.h()); ++y) {
				const map_location loc(x,y);
				if(map_.is_keep(loc)) {
					map_location adj[6];
					get_adjacent_tiles(loc,adj);
					for(size_t n = 0; n != 6; ++n) {
						if(map_.is_castle(adj[n])) {
							keeps_.insert(loc);
							break;
						}
					}
				}
			}
		}
	}

	return keeps_;
}


bool default_ai_context_impl::leader_can_reach_keep()
{
	const unit_map::iterator leader = get_info().units.find_leader(get_side());
	if(leader == get_info().units.end() || leader->second.incapacitated()) {
		return false;
	}

	const map_location& start_pos = nearest_keep(leader->first);
	if(start_pos.valid() == false) {
		return false;
	}

	if(leader->first == start_pos) {
		return true;
	}

	// Find where the leader can move
	const paths leader_paths(get_info().map,get_info().units,leader->first,get_info().teams,false,false,current_team());


	return leader_paths.destinations.contains(start_pos);
}


const map_location& default_ai_context_impl::nearest_keep(const map_location& loc)
{
	const std::set<map_location>& keeps = this->keeps();
	if(keeps.empty()) {
		static const map_location dummy;
		return dummy;
	}

	const map_location* res = NULL;
	int closest = -1;
	for(std::set<map_location>::const_iterator i = keeps.begin(); i != keeps.end(); ++i) {
		const int distance = distance_between(*i,loc);
		if(res == NULL || distance < closest) {
			closest = distance;
			res = &*i;
		}
	}

	return *res;
}


double default_ai_context_impl::power_projection(const map_location& loc, const move_map& dstsrc) const
{
	map_location used_locs[6];
	int ratings[6];
	int num_used_locs = 0;

	map_location locs[6];
	get_adjacent_tiles(loc,locs);

	const int lawful_bonus = get_info().tod_manager_.get_time_of_day().lawful_bonus;
	gamemap& map_ = get_info().map;
	unit_map& units_ = get_info().units;

	int res = 0;

	bool changed = false;
	for (int i = 0;; ++i) {
		if (i == 6) {
			if (!changed) break;
			// Loop once again, in case a unit found a better spot
			// and freed the place for another unit.
			changed = false;
			i = 0;
		}

		if (map_.on_board(locs[i]) == false) {
			continue;
		}

		const t_translation::t_terrain terrain = map_[locs[i]];

		typedef move_map::const_iterator Itor;
		typedef std::pair<Itor,Itor> Range;
		Range its = dstsrc.equal_range(locs[i]);

		map_location* const beg_used = used_locs;
		map_location* end_used = used_locs + num_used_locs;

		int best_rating = 0;
		map_location best_unit;

		for(Itor it = its.first; it != its.second; ++it) {
			const unit_map::const_iterator u = units_.find(it->second);

			// Unit might have been killed, and no longer exist
			if(u == units_.end()) {
				continue;
			}

			const unit& un = u->second;

			int tod_modifier = 0;
			if(un.alignment() == unit_type::LAWFUL) {
				tod_modifier = lawful_bonus;
			} else if(un.alignment() == unit_type::CHAOTIC) {
				tod_modifier = -lawful_bonus;
			}

			// The 0.5 power avoids underestimating too much the damage of a wounded unit.
			int hp = int(sqrt(double(un.hitpoints()) / un.max_hitpoints()) * 1000);
			int most_damage = 0;
			foreach (const attack_type &att, un.attacks())
			{
				int damage = att.damage() * att.num_attacks() * (100 + tod_modifier);
				if (damage > most_damage) {
					most_damage = damage;
				}
			}

			int village_bonus = map_.is_village(terrain) ? 3 : 2;
			int defense = 100 - un.defense_modifier(terrain);
			int rating = hp * defense * most_damage * village_bonus / 200;
			if(rating > best_rating) {
				map_location *pos = std::find(beg_used, end_used, it->second);
				// Check if the spot is the same or better than an older one.
				if (pos == end_used || rating >= ratings[pos - beg_used]) {
					best_rating = rating;
					best_unit = it->second;
				}
			}
		}

		if (!best_unit.valid()) continue;
		map_location *pos = std::find(beg_used, end_used, best_unit);
		int index = pos - beg_used;
		if (index == num_used_locs)
			++num_used_locs;
		else if (best_rating == ratings[index])
			continue;
		else {
			// The unit was in another spot already, so remove its older rating
			// from the final result, and require a new run to fill its old spot.
			res -= ratings[index];
			changed = true;
		}
		used_locs[index] = best_unit;
		ratings[index] = best_rating;
		res += best_rating;
	}

	return res / 100000.;
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
