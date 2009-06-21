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

// default ai context impl
void default_ai_context_impl::add_recent_attack(map_location loc)
{
	attacks_.insert(loc);
}

const int max_positions = 10000;

std::vector<attack_analysis> default_ai_context_impl::analyze_targets(
	             const move_map& srcdst, const move_map& dstsrc,
	             const move_map& enemy_srcdst, const move_map& enemy_dstsrc
                )
{
	log_scope2(log_ai, "analyzing targets...");

	std::vector<attack_analysis> res;
	unit_map units_ = get_info().units;

	std::vector<map_location> unit_locs;
	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
		if(i->second.side() == get_side() && i->second.attacks_left()) {
			unit_locs.push_back(i->first);
		}
	}

	bool used_locations[6];
	std::fill(used_locations,used_locations+6,false);

	moves_map dummy_moves;
	move_map fullmove_srcdst, fullmove_dstsrc;
	calculate_possible_moves(dummy_moves,fullmove_srcdst,fullmove_dstsrc,false,true);

	unit_stats_cache().clear();

	for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {

		// Attack anyone who is on the enemy side,
		// and who is not invisible or petrified.
		if(current_team().is_enemy(j->second.side()) && !j->second.incapacitated() &&
		   j->second.invisible(j->first,units_,get_info().teams) == false) {
			map_location adjacent[6];
			get_adjacent_tiles(j->first,adjacent);
			attack_analysis analysis;
			analysis.target = j->first;
			analysis.vulnerability = 0.0;
			analysis.support = 0.0;

//			const int ticks = SDL_GetTicks();

			do_attack_analysis(j->first,srcdst,dstsrc,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc,
			            adjacent,used_locations,unit_locs,res,analysis);

//			const int time_taken = SDL_GetTicks() - ticks;
//			static int max_time = 0;
//			if(time_taken > max_time)
//				max_time = time_taken;
		}
	}

	return res;
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


int default_ai_context_impl::attack_depth()
{
	if(attack_depth_ > 0) {
		return attack_depth_;
	}

	const config& parms = current_team().ai_parameters();
	attack_depth_ = std::max<int>(1,lexical_cast_default<int>(parms["attack_depth"],5));
	return attack_depth_;
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

void default_ai_context_impl::do_attack_analysis(
	                 const map_location& loc,
	                 const move_map& srcdst, const move_map& dstsrc,
					 const move_map& fullmove_srcdst, const move_map& fullmove_dstsrc,
	                 const move_map& enemy_srcdst, const move_map& enemy_dstsrc,
					 const map_location* tiles, bool* used_locations,
	                 std::vector<map_location>& units,
	                 std::vector<attack_analysis>& result,
					 attack_analysis& cur_analysis
	                )
{
	// This function is called fairly frequently, so interact with the user here.
	raise_user_interact();

	if(cur_analysis.movements.size() >= size_t(attack_depth())) {
		//std::cerr << "ANALYSIS " << cur_analysis.movements.size() << " >= " << attack_depth() << "\n";
		return;
	}
	gamemap &map_ = get_info().map;
	unit_map &units_ = get_info().units;
	std::vector<team> &teams_ = get_info().teams;

	static double best_results[6];
	if(result.empty()) {
		for(int i = 0; i != 6; ++i) {
			best_results[i] = 0.0;
		}
	}

	const size_t max_positions = 1000;
	if(result.size() > max_positions && !cur_analysis.movements.empty()) {
		LOG_AI << "cut analysis short with number of positions\n";
		return;
	}

	const double cur_rating = cur_analysis.movements.empty() ? -1.0 :
	                          cur_analysis.rating(current_team().aggression(),*this);

	double rating_to_beat = cur_rating;

	if(!cur_analysis.movements.empty()) {
		assert(cur_analysis.movements.size() < 6);
		double& best_res = best_results[cur_analysis.movements.size()-1];
		rating_to_beat = best_res = std::max(best_res,cur_rating);
	}

	for(size_t i = 0; i != units.size(); ++i) {
		const map_location current_unit = units[i];

		unit_map::iterator unit_itor = units_.find(current_unit);
		assert(unit_itor != units_.end());

		// See if the unit has the backstab ability.
		// Units with backstab will want to try to have a
		// friendly unit opposite the position they move to.
		//
		// See if the unit has the slow ability -- units with slow only attack first.
		bool backstab = false, slow = false;
		std::vector<attack_type>& attacks = unit_itor->second.attacks();
		for(std::vector<attack_type>::iterator a = attacks.begin(); a != attacks.end(); ++a) {
			a->set_specials_context(map_location(),map_location(),
															&units_,&map_,&get_info().state,&get_info().tod_manager_,&teams_,true,NULL);
			if(a->get_special_bool("backstab")) {
				backstab = true;
			}

			if(a->get_special_bool("slow")) {
				slow = true;
			}
		}

		if(slow && cur_analysis.movements.empty() == false) {
			continue;
		}

               // Check if the friendly unit is surrounded,
			   // A unit is surrounded if it is flanked by enemy units
			   // and at least one other enemy unit is nearby
			   // or if the unit is totaly surrounded by enemies
			   // with max. one tile to escape.
               bool is_surrounded = false;
               bool is_flanked = false;
               int enemy_units_around = 0;
               int accessible_tiles = 0;
               map_location adj[6];
               get_adjacent_tiles(current_unit, adj);

               size_t tile;
               for(tile = 0; tile != 3; ++tile) {

                       const unit_map::const_iterator tmp_unit = units_.find(adj[tile]);
                       bool possible_flanked = false;

                       if(map_.on_board(adj[tile]))
                       {
                               accessible_tiles++;
                               if(tmp_unit != units_.end() && get_side() != tmp_unit->second.side())
                               {
                                       enemy_units_around++;
                                       possible_flanked = true;
                               }
                       }

                       const unit_map::const_iterator tmp_opposite_unit = units_.find(adj[tile + 3]);
                        if(map_.on_board(adj[tile + 3]))
                       {
                               accessible_tiles++;
                               if(tmp_opposite_unit != units_.end() && get_side() != tmp_opposite_unit->second.side())
                               {
                                       enemy_units_around++;
                                       if(possible_flanked)
                                       {
                                               is_flanked = true;
                                       }
                               }
                       }
               }

               if((is_flanked && enemy_units_around > 2) || enemy_units_around >= accessible_tiles - 1)
                       is_surrounded = true;



		double best_vulnerability = 0.0, best_support = 0.0;
		int best_rating = 0;
		int cur_position = -1;

		// Iterate over positions adjacent to the unit, finding the best rated one.
		for(int j = 0; j != 6; ++j) {

			// If in this planned attack, a unit is already in this location.
			if(used_locations[j]) {
				continue;
			}

			// See if the current unit can reach that position.
			if (tiles[j] != current_unit) {
				typedef std::multimap<map_location,map_location>::const_iterator Itor;
				std::pair<Itor,Itor> its = dstsrc.equal_range(tiles[j]);
				while(its.first != its.second) {
					if(its.first->second == current_unit)
						break;
					++its.first;
				}

				// If the unit can't move to this location.
				if(its.first == its.second || units_.find(tiles[j]) != units_.end()) {
					continue;
				}
			}

			unit_ability_list abil = unit_itor->second.get_abilities("leadership",tiles[j]);
			int best_leadership_bonus = abil.highest("value").first;
			double leadership_bonus = static_cast<double>(best_leadership_bonus+100)/100.0;
			if (leadership_bonus > 1.1) {
				LOG_AI << unit_itor->second.name() << " is getting leadership " << leadership_bonus << "\n";
			}

			// Check to see whether this move would be a backstab.
			int backstab_bonus = 1;
			double surround_bonus = 1.0;

			if(tiles[(j+3)%6] != current_unit) {
				const unit_map::const_iterator itor = units_.find(tiles[(j+3)%6]);

				// Note that we *could* also check if a unit plans to move there
				// before we're at this stage, but we don't because, since the
				// attack calculations don't actually take backstab into account (too complicated),
				// this could actually make our analysis look *worse* instead of better.
				// So we only check for 'concrete' backstab opportunities.
				// That would also break backstab_check, since it assumes
				// the defender is in place.
				if(itor != units_.end() &&
					backstab_check(tiles[j], loc, units_, teams_)) {
					if(backstab) {
						backstab_bonus = 2;
					}

					// No surround bonus if target is skirmisher
					if (!itor->second.get_ability_bool("skirmisker"))
						surround_bonus = 1.2;
				}


			}

			// See if this position is the best rated we've seen so far.
			const int rating = static_cast<int>(rate_terrain(unit_itor->second,tiles[j]) * backstab_bonus * leadership_bonus);
			if(cur_position >= 0 && rating < best_rating) {
				continue;
			}

			// Find out how vulnerable we are to attack from enemy units in this hex.
			//FIXME: suokko's r29531 multiplied this by a constant 1.5. ?
			const double vulnerability = power_projection(tiles[j],enemy_dstsrc);

			// Calculate how much support we have on this hex from allies.
			const double support = power_projection(tiles[j], fullmove_dstsrc);

			// If this is a position with equal defense to another position,
			// but more vulnerability then we don't want to use it.
#ifdef SUOKKO
			//FIXME: this code was in sukko's r29531  Correct?
			// scale vulnerability to 60 hp unit
			if(cur_position >= 0 && rating < best_rating
					&& (vulnerability/surround_bonus*30.0)/unit_itor->second.hitpoints() -
						(support*surround_bonus*30.0)/unit_itor->second.max_hitpoints()
						> best_vulnerability - best_support) {
				continue;
			}
#else
			if(cur_position >= 0 && rating == best_rating && vulnerability/surround_bonus - support*surround_bonus >= best_vulnerability - best_support) {
				continue;
			}
#endif
			cur_position = j;
			best_rating = rating;
#ifdef SUOKKO
			//FIXME: this code was in sukko's r29531  Correct?
			best_vulnerability = (vulnerability/surround_bonus*30.0)/unit_itor->second.hitpoints();
			best_support = (support*surround_bonus*30.0)/unit_itor->second.max_hitpoints();
#else
			best_vulnerability = vulnerability/surround_bonus;
			best_support = support*surround_bonus;
#endif
		}

		if(cur_position != -1) {
			units.erase(units.begin() + i);

			cur_analysis.movements.push_back(std::pair<map_location,map_location>(current_unit,tiles[cur_position]));

			cur_analysis.vulnerability += best_vulnerability;

			cur_analysis.support += best_support;

			cur_analysis.is_surrounded = is_surrounded;

			cur_analysis.analyze(map_, units_, teams_, get_info().state, get_info().tod_manager_, *this, dstsrc, srcdst, enemy_dstsrc, current_team().aggression());

			//This logic to sometimes not add the attack because it doesn't
			//rate high enough seems to remove attacks from consideration
			//that should not be removed, so it has been removed.
			//  -- David.
//			if(cur_analysis.rating(current_team().aggression(),*this) > rating_to_beat) {

				result.push_back(cur_analysis);
				used_locations[cur_position] = true;
				do_attack_analysis(loc,srcdst,dstsrc,fullmove_srcdst,fullmove_dstsrc,enemy_srcdst,enemy_dstsrc,
				                   tiles,used_locations,
				                   units,result,cur_analysis);
				used_locations[cur_position] = false;
//			}

			cur_analysis.vulnerability -= best_vulnerability;
			cur_analysis.support -= best_support;

			cur_analysis.movements.pop_back();

			units.insert(units.begin() + i, current_unit);
		}
	}
}


default_ai_context& default_ai_context_impl::get_default_ai_context(){
	return *this;
}


void default_ai_context_impl::invalidate_attack_depth_cache(){
	attack_depth_ = 0;
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
