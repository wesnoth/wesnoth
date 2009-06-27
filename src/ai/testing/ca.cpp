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
 * Default AI (Testing)
 * @file ai/testing/ca.cpp
 */

#include "ca.hpp"
#include "../composite/engine.hpp"
#include "../composite/rca.hpp"
#include "../../log.hpp"
#include "../../foreach.hpp"

#include <numeric>

static lg::log_domain log_ai_testing_ai_default("ai/testing/ai_default");
#define DBG_AI_TESTING_AI_DEFAULT LOG_STREAM(debug, log_ai_testing_ai_default)
#define LOG_AI_TESTING_AI_DEFAULT LOG_STREAM(info, log_ai_testing_ai_default)
#define ERR_AI_TESTING_AI_DEFAULT LOG_STREAM(err, log_ai_testing_ai_default)


namespace ai {

namespace testing_ai_default {

//==============================================================

goto_phase::goto_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::goto_phase",cfg["type"])
{
}

goto_phase::~goto_phase()
{
}

double goto_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented" << std::endl;
	return BAD_SCORE;
}

bool goto_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented" << std::endl;
	return true;
}

//==============================================================

recruitment_phase::recruitment_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::recruitment_phase",cfg["type"])
{
}


recruitment_phase::~recruitment_phase()
{
}

double recruitment_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool recruitment_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

combat_phase::combat_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::combat_phase",cfg["type"]),best_analysis_(),choice_rating_(-1000.0)
{
}

combat_phase::~combat_phase()
{
}

double combat_phase::evaluate()
{
	choice_rating_ = -1000.0;
	int ticks = SDL_GetTicks();

	std::vector<attack_analysis> analysis = analyze_targets(get_srcdst(), get_dstsrc(), get_enemy_srcdst(), get_enemy_dstsrc());

	int time_taken = SDL_GetTicks() - ticks;
	LOG_AI_TESTING_AI_DEFAULT << "took " << time_taken << " ticks for " << analysis.size()
		<< " positions. Analyzing...\n";

	ticks = SDL_GetTicks();

	const int max_sims = 50000;
	int num_sims = analysis.empty() ? 0 : max_sims/analysis.size();
	if(num_sims < 20)
		num_sims = 20;
	if(num_sims > 40)
		num_sims = 40;

	LOG_AI_TESTING_AI_DEFAULT << "simulations: " << num_sims << "\n";

	const int max_positions = 30000;
	const int skip_num = analysis.size()/max_positions;

	std::vector<attack_analysis>::iterator choice_it = analysis.end();
	for(std::vector<attack_analysis>::iterator it = analysis.begin();
			it != analysis.end(); ++it) {

		if(skip_num > 0 && ((it - analysis.begin())%skip_num) && it->movements.size() > 1)
			continue;

		const double rating = it->rating(current_team().aggression(),*this);
		LOG_AI_TESTING_AI_DEFAULT << "attack option rated at " << rating << " ("
			<< current_team().aggression() << ")\n";

		if(rating > choice_rating_) {
			choice_it = it;
			choice_rating_ = rating;
		}
	}

	time_taken = SDL_GetTicks() - ticks;
	LOG_AI_TESTING_AI_DEFAULT << "analysis took " << time_taken << " ticks\n";


	// suokko tested the rating against current_team().caution()
	// Bad mistake -- the AI became extremely reluctant to attack anything.
	// Documenting this in case someone has this bright idea again...*don't*...
	if(choice_rating_ > 0.0) {
		best_analysis_ = *choice_it;
		return 40;//@todo: externalize
	} else {
		return BAD_SCORE;
	}
}

bool combat_phase::execute()
{
	assert(choice_rating_ > 0.0);
	bool gamestate_changed = false;
	map_location from   = best_analysis_.movements[0].first;
	map_location to     = best_analysis_.movements[0].second;
	map_location target_loc = best_analysis_.target;

	if (from!=to) {
		move_result_ptr move_res = execute_move_action(from,to,false);
		gamestate_changed |= move_res->is_gamestate_changed();
		if (!move_res->is_ok()) {
			return gamestate_changed;
		}
	}

	attack_result_ptr attack_res = execute_attack_action(to, target_loc, -1);
	gamestate_changed |= attack_res->is_gamestate_changed();
	if (!attack_res->is_ok()) {
		return gamestate_changed;
	}

	return gamestate_changed;
}

//==============================================================

move_leader_to_goals_phase::move_leader_to_goals_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::move_leader_to_goals_phase",cfg["type"])
{
}

move_leader_to_goals_phase::~move_leader_to_goals_phase()
{
}

double move_leader_to_goals_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool move_leader_to_goals_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

move_leader_to_keep_phase::move_leader_to_keep_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::move_leader_to_keep_phase",cfg["type"]),move_()
{

}

move_leader_to_keep_phase::~move_leader_to_keep_phase()
{

}

double move_leader_to_keep_phase::evaluate()
{
	unit_map &units_ = get_info().units;
	const unit_map::iterator leader = units_.find_leader(get_side());

	if(leader == units_.end() || leader->second.incapacitated() || leader->second.movement_left()==0 ) {
		return BAD_SCORE;
	}

	// Find where the leader can move
	const paths leader_paths(get_info().map, units_, leader->first,
		 get_info().teams, false, false, current_team());
	const map_location& keep = suitable_keep(leader->first,leader_paths);

	std::map<map_location,paths> possible_moves;
	possible_moves.insert(std::pair<map_location,paths>(leader->first,leader_paths));

	// If the leader is not on keep, move him there.
	if(leader->first != keep) {
		if (leader_paths.destinations.contains(keep) && units_.count(keep) == 0) {
			move_ = check_move_action(leader->first,keep,false);
			if (move_->is_ok()){
				return 70;
			}
			//move_unit(leader->first,keep,get_possible_moves());
		}
		// Make a map of the possible locations the leader can move to,
		// ordered by the distance from the keep.
		std::multimap<int,map_location> moves_toward_keep;

		// The leader can't move to his keep, try to move to the closest location
		// to the keep where there are no enemies in range.
		const int current_distance = distance_between(leader->first,keep);
		foreach (const paths::step &dest, leader_paths.destinations)
		{
			const int new_distance = distance_between(dest.curr,keep);
			if(new_distance < current_distance) {
				moves_toward_keep.insert(std::make_pair(new_distance, dest.curr));
			}
		}

		// Find the first location which we can move to,
		// without the threat of enemies.
		for(std::multimap<int,map_location>::const_iterator j = moves_toward_keep.begin();
			j != moves_toward_keep.end(); ++j) {

			if(get_enemy_dstsrc().count(j->second) == 0) {
				move_ = check_move_action(leader->first,j->second,true);
				if (move_->is_ok()){
					return 70;
				}
			}
		}
	}
	return BAD_SCORE;
}

bool move_leader_to_keep_phase::execute()
{
	bool gamestate_changed = false;
	move_->execute();
	gamestate_changed |= move_->is_ok();
	return gamestate_changed;
}

//==============================================================

get_villages_phase::get_villages_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::get_villages_phase",cfg["type"]), keep_loc_(),
		leader_loc_(),best_leader_loc_(),debug_(false)
{
}

get_villages_phase::~get_villages_phase()
{
}

double get_villages_phase::evaluate()
{
	moves_.clear();
	unit_map::const_iterator leader = get_info().units.find_leader(get_side());
	get_villages(get_possible_moves(),get_dstsrc(),get_enemy_dstsrc(),leader);
	if (moves_.size()>0) {
		return 25;
	}
	return BAD_SCORE;
}

bool get_villages_phase::execute()
{
	bool gamestate_changed = false;
	unit_map &units_ = get_info().units;
	unit_map::const_iterator leader = units_.find_leader(get_side());
	// Move all the units to get villages, however move the leader last,
	// so that the castle will be cleared if it wants to stop to recruit along the way.
	std::pair<map_location,map_location> leader_move;

	for(tmoves::const_iterator i = moves_.begin(); i != moves_.end(); ++i) {

		if(leader != units_.end() && leader->first == i->second) {
			leader_move = *i;
		} else {
			if(units_.count(i->first) == 0) {
				move_result_ptr move_res = execute_move_action(i->second,i->first,true);
				gamestate_changed |= move_res->is_gamestate_changed();
				if (!move_res->is_ok()) {
					return gamestate_changed;
				}

				const map_location loc = move_res->get_unit_location();
				leader = units_.find_leader(get_side());
				const unit_map::const_iterator new_unit = units_.find(loc);

				if(new_unit != units_.end() &&
						power_projection(i->first,get_enemy_dstsrc()) >= new_unit->second.hitpoints()/4) {
					LOG_AI_TESTING_AI_DEFAULT << "found support target... " << new_unit->first << "\n";
					//FIXME: suokko tweaked the constant 1.0 to the formula:
					//25.0* current_team().caution() * power_projection(loc,enemy_dstsrc) / new_unit->second.hitpoints()
					//Is this an improvement?

					//@todo 1.7 fix this to account for RCA semantics
					//add_target(target(new_unit->first,1.0,target::SUPPORT));
				}
			}
		}
	}

	if(leader_move.second.valid()) {
		if(units_.count(leader_move.first) == 0 && get_info().map.is_village(leader_move.first)) {
			move_result_ptr move_res = execute_move_action(leader_move.second,leader_move.first,true);
			gamestate_changed |= move_res->is_gamestate_changed();
			if (!move_res->is_ok()) {
				return gamestate_changed;
			}
		}
	}

	return gamestate_changed;
}

void get_villages_phase::get_villages(const moves_map& possible_moves,
		const move_map& dstsrc, const move_map& enemy_dstsrc,
		unit_map::const_iterator &leader)
{
	DBG_AI_TESTING_AI_DEFAULT << "deciding which villages we want...\n";
	unit_map &units_ = get_info().units;
	const int ticks = SDL_GetTicks();
	best_leader_loc_ = map_location::null_location;
	if(leader != units_.end()) {
		keep_loc_ = nearest_keep(leader->first);
		leader_loc_ = leader->first;
	} else {
		keep_loc_ = map_location::null_location;
		leader_loc_ = map_location::null_location;
	}

	debug_ = !lg::debug.dont_log(log_ai_testing_ai_default);

	// Find our units who can move.
	treachmap reachmap;
	for(unit_map::const_iterator u_itor = units_.begin();
			u_itor != units_.end(); ++u_itor) {

		if(u_itor->second.side() == get_side()
				&& u_itor->second.movement_left()) {
			reachmap.insert(std::make_pair(u_itor->first,	std::vector<map_location>()));
		}
	}


	DBG_AI_TESTING_AI_DEFAULT << reachmap.size() << " units found who can try to capture a village.\n";

	find_villages(reachmap, moves_, dstsrc, possible_moves, enemy_dstsrc);

	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		if(itor->second.size() == 0) {
			itor = remove_unit(reachmap, moves_, itor);
		} else {
			++itor;
		}
	}

	if(reachmap.size()) {
		DBG_AI_TESTING_AI_DEFAULT << reachmap.size() << " units left after removing the ones who "
			"can't reach a village, send the to the dispatcher.\n";

		dump_reachmap(reachmap);

		dispatch(reachmap, moves_);
	} else {
		DBG_AI_TESTING_AI_DEFAULT << "No more units left after removing the ones who can't reach a village.\n";
	}

	LOG_AI_TESTING_AI_DEFAULT << "Village assignment done: " << (SDL_GetTicks() - ticks)
		<< " ms, resulted in " << moves_.size() << " units being dispatched.\n";

}

void get_villages_phase::find_villages(
	treachmap& reachmap,
	tmoves& moves,
	const std::multimap<map_location,map_location>& dstsrc,
	const std::map<map_location,paths>& possible_moves,
	const std::multimap<map_location,map_location>& enemy_dstsrc)

{
	std::map<map_location, double> vulnerability;

	const bool passive_leader = utils::string_bool(current_team().ai_parameters()["passive_leader"]);//@todo: make an aspect

	size_t min_distance = 100000;
	gamemap &map_ = get_info().map;
	std::vector<team> &teams_ = get_info().teams;

	// When a unit is dispatched we need to make sure we don't
	// dispatch this unit a second time, so store them here.
	std::vector<map_location> dispatched_units;
	for(std::multimap<map_location, map_location>::const_iterator
			j = dstsrc.begin();
			j != dstsrc.end(); ++j) {

		const map_location &current_loc = j->first;

		if(j->second == leader_loc_) {
			if(passive_leader) {
				continue;
			}

			const size_t distance = distance_between(keep_loc_, current_loc);
			if(distance < min_distance) {
				min_distance = distance;
				best_leader_loc_ = current_loc;
			}
		}

		if(std::find(dispatched_units.begin(), dispatched_units.end(),
				j->second) != dispatched_units.end()) {
			continue;
		}

		if(map_.is_village(current_loc) == false) {
			continue;
		}

		bool want_village = true, owned = false;
		for(size_t n = 0; n != teams_.size(); ++n) {
			owned = teams_[n].owns_village(current_loc);
			if(owned && !current_team().is_enemy(n+1)) {
				want_village = false;
			}

			if(owned) {
				break;
			}
		}

		if(want_village == false) {
			continue;
		}

		// If it is a neutral village, and we have no leader,
		// then the village is of no use to us, and we don't want it.
		if(!owned && leader_loc_ == map_location::null_location) {
			continue;
		}

		// If we have a decent amount of gold, and the leader can't access
		// the keep this turn if they get this village,
		// then don't get this village with them.
		if(want_village &&
				current_team().gold() > 20 &&
				leader_loc_ == current_loc &&
				leader_loc_ != keep_loc_ &&
				multistep_move_possible(j->second, current_loc, keep_loc_, possible_moves) == false) {
			continue;
		}

		double threat = 0.0;
		const std::map<map_location,double>::const_iterator vuln = vulnerability.find(current_loc);
		if(vuln != vulnerability.end()) {
			threat = vuln->second;
		} else {
			threat = power_projection(current_loc,enemy_dstsrc);
			vulnerability.insert(std::pair<map_location,double>(current_loc,threat));
		}

		const unit_map::const_iterator u = get_info().units.find(j->second);
		if(u == get_info().units.end() || utils::string_bool(u->second.get_state("guardian"))) {
			continue;
		}

		const unit& un = u->second;
		//FIXME: suokko turned this 2:1 to 1.5:1.0.
		//and dropped the second term of the multiplication.  Is that better?
		//const double threat_multipler = (current_loc == leader_loc?2:1) * current_team().caution() * 10;
		if(un.hitpoints() < (threat*2*un.defense_modifier(map_.get_terrain(current_loc)))/100) {
			continue;
		}

		// If the next and previous destination differs from our current destination,
		// we're the only one who can reach the village -> dispatch.
		std::multimap<map_location, map_location>::const_iterator next = j;
		++next; // j + 1 fails
		const bool at_begin = (j == dstsrc.begin());
		std::multimap<map_location, map_location>::const_iterator prev = j; //FIXME seems not to work
		if(!at_begin) {
			--prev;
		}
#if 1
		if((next == dstsrc.end() || next->first != current_loc)
				&& (at_begin || prev->first != current_loc)) {

			move_result_ptr move_check_res = check_move_action(j->second,j->first,true);
			if (move_check_res->is_ok()) {
				DBG_AI_TESTING_AI_DEFAULT << "Dispatched unit at " << j->second << " to village " << j->first << '\n';
				moves.push_back(std::make_pair(j->first, j->second));
			}
			reachmap.erase(j->second);
			dispatched_units.push_back(j->second);
			continue;
		}
#endif
		reachmap[j->second].push_back(current_loc);
	}

	DBG_AI_TESTING_AI_DEFAULT << moves.size() << " units already dispatched, "
		<< reachmap.size() << " left to evaluate.\n";
}

void get_villages_phase::dispatch(treachmap& reachmap, tmoves& moves)
{
	DBG_AI_TESTING_AI_DEFAULT << "Starting simple dispatch.\n";

	// we now have a list with units with the villages they can reach.
	// keep trying the following steps as long as one of them changes
	// the state.
	// 1. Dispatch units who can reach 1 village (if more units can reach that
	//    village only one can capture it, so use the first in the list.)
	// 2. Villages which can only be reached by one unit get that unit dispatched
	//    to them.
	size_t village_count = 0;
	bool dispatched = true;
	while(dispatched) {
		dispatched = false;

		if(dispatch_unit_simple(reachmap, moves)) {
			dispatched = true;
		} else {
			if(reachmap.empty()) {
				DBG_AI_TESTING_AI_DEFAULT << "dispatch_unit_simple() found a final solution.\n";
				break;
			} else {
				DBG_AI_TESTING_AI_DEFAULT << "dispatch_unit_simple() couldn't dispatch more units.\n";
			}
		}

		if(dispatch_village_simple(reachmap, moves, village_count)) {
			dispatched = true;
		} else {
			if(reachmap.empty()) {
				DBG_AI_TESTING_AI_DEFAULT << "dispatch_village_simple() found a final solution.\n";
				break;
			} else {
				DBG_AI_TESTING_AI_DEFAULT << "dispatch_village_simple() couldn't dispatch more units.\n";
			}
		}

		if(reachmap.size() != 0 && dispatched) {
			DBG_AI_TESTING_AI_DEFAULT << reachmap.size() << " unit(s) left restarting simple dispatching.\n";

			dump_reachmap(reachmap);
		}
	}

	if(reachmap.size() == 0) {
		DBG_AI_TESTING_AI_DEFAULT << "No units left after simple dispatcher.\n";
		return;
	}

	DBG_AI_TESTING_AI_DEFAULT << reachmap.size() << " units left for complex dispatch with "
		<< village_count << " villages left.\n";

	dump_reachmap(reachmap);

	dispatch_complex(reachmap, moves, village_count);
}

// Returns		need further processing
// false		Nothing has been modified or no units left
bool get_villages_phase::dispatch_unit_simple(treachmap& reachmap, tmoves& moves)
{
	bool result = false;

	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		if(itor->second.size() == 1) {
			const map_location village = itor->second[0];
			result = true;

			DBG_AI_TESTING_AI_DEFAULT << "Dispatched unit at " << itor->first << " to village " << village << '\n';
			moves.push_back(std::make_pair(village, itor->first));
			reachmap.erase(itor++);

			if(remove_village(reachmap, moves, village)) {
				itor = reachmap.begin();
			}

		} else {
			++itor;
		}
	}

	// Test special cases.
	if(reachmap.empty()) {
		// We're done.
		return false;
	}

	if(reachmap.size() == 1) {
		// One unit left.
		DBG_AI_TESTING_AI_DEFAULT << "Dispatched _last_ unit at " << reachmap.begin()->first
			<< " to village " << reachmap.begin()->second[0] << '\n';

		moves.push_back(std::make_pair(
			reachmap.begin()->second[0], reachmap.begin()->first));

		reachmap.clear();
		// We're done.
		return false;
	}

	return result;
}

bool get_villages_phase::dispatch_village_simple(
	treachmap& reachmap, tmoves& moves, size_t& village_count)
{

	bool result = false;
	bool dispatched = true;
	while(dispatched) {
		dispatched = false;

		// build the reverse map
		std::map<map_location /*village location*/,
			std::vector<map_location /* units that can reach it*/> >reversemap;

		treachmap::const_iterator itor = reachmap.begin();
		for(;itor != reachmap.end(); ++itor) {

			for(std::vector<map_location>::const_iterator
					v_itor = itor->second.begin();
			 		v_itor != itor->second.end(); ++v_itor) {

				reversemap[*v_itor].push_back(itor->first);

			}
		}

		village_count = reversemap.size();

		itor = reversemap.begin();
		while(itor != reversemap.end()) {
			if(itor->second.size() == 1) {
				// One unit can reach this village.
				const map_location village = itor->first;
				dispatched = true;
				result = true;

				DBG_AI_TESTING_AI_DEFAULT << "Dispatched unit at " << itor->second[0] << " to village " << itor->first << '\n';
				moves.push_back(std::make_pair(itor->first, itor->second[0]));

				reachmap.erase(itor->second[0]);
				remove_village(reachmap, moves, village);
				// Get can go to some trouble to remove the unit from the other villages
				// instead we abort this loop end do a full rebuild on the map.
				break;
			} else {
				++itor;
			}
		}
	}

	return result;
}

bool get_villages_phase::remove_village(
	treachmap& reachmap, tmoves& moves, const map_location& village)
{
	bool result = false;
	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		itor->second.erase(std::remove(itor->second.begin(), itor->second.end(), village), itor->second.end());
		if(itor->second.empty()) {
			result = true;
			itor = remove_unit(reachmap, moves, itor);
		} else {
			++itor;
		}
	}
	return result;
}

get_villages_phase::treachmap::iterator get_villages_phase::remove_unit(
	treachmap& reachmap, tmoves& moves, treachmap::iterator unit)
{
	assert(unit->second.empty());

	if(unit->first == leader_loc_ && best_leader_loc_ != map_location::null_location) {
		DBG_AI_TESTING_AI_DEFAULT << "Dispatch leader at " << leader_loc_ << " closer to the keep at "
			<< best_leader_loc_ << '\n';

		moves.push_back(std::make_pair(best_leader_loc_, leader_loc_));
	}

	reachmap.erase(unit++);
	return unit;
}

void get_villages_phase::dispatch_complex(
	treachmap& reachmap, tmoves& moves, const size_t village_count)
{
	// ***** ***** Init and dispatch if every unit can reach every village.

	const size_t unit_count = reachmap.size();
	// The maximum number of villages we can capture with the available units.
	const size_t max_result = unit_count < village_count ? unit_count : village_count;

	assert(unit_count >= 2 && village_count >= 2);

	// Every unit can reach every village.
	if(unit_count == 2 && village_count == 2) {
		DBG_AI_TESTING_AI_DEFAULT << "Every unit can reach every village for 2 units, dispatch them.\n";
		full_dispatch(reachmap, moves);
		return;
	}

	std::vector<map_location> units(unit_count);
	std::vector<size_t> villages_per_unit(unit_count);
	std::vector<map_location> villages;
	std::vector<size_t> units_per_village(village_count);

	// We want to test the units, the ones who can reach the least
	// villages first so this is our lookup map.
	std::multimap<size_t /* villages_per_unit value*/,
		size_t /*villages_per_unit index*/> unit_lookup;

	std::vector</*unit*/std::vector</*village*/bool> >
		matrix(reachmap.size(), std::vector<bool>(village_count, false));

	treachmap::const_iterator itor = reachmap.begin();
	for(size_t u = 0; u < unit_count; ++u, ++itor) {
		units[u] = itor->first;
		villages_per_unit[u] = itor->second.size();
		unit_lookup.insert(std::make_pair(villages_per_unit[u], u));

		assert(itor->second.size() >= 2);

		for(size_t v = 0; v < itor->second.size(); ++v) {

			size_t v_index;
			// find the index of the v in the villages
			std::vector<map_location>::const_iterator v_itor =
				std::find(villages.begin(), villages.end(), itor->second[v]);
			if(v_itor == villages.end()) {
				v_index = villages.size(); // will be the last element after push_back.
				villages.push_back(itor->second[v]);
			} else {
				v_index = v_itor - villages.begin();
			}

			units_per_village[v_index]++;

			matrix[u][v_index] = true;
		}
	}
	for(std::vector<size_t>::const_iterator upv_it = units_per_village.begin();
			upv_it != units_per_village.end(); ++upv_it) {

		assert(*upv_it >=2);
	}

	if(debug_) {
		// Print header
		std::cerr << "Reach matrix:\n\nvillage";
		size_t u, v;
		for(v = 0; v < village_count; ++v) {
			std::cerr << '\t' << villages[v];
		}
		std::cerr << "\ttotal\nunit\n";

		// Print data
		for(u = 0; u < unit_count; ++u) {
			std::cerr << units[u];

			for(size_t v = 0; v < village_count; ++v) {
				std::cerr << '\t' << matrix[u][v];
			}
			std::cerr << "\t" << villages_per_unit[u] << '\n';
		}

		// Print footer
		std::cerr << "total";
		for(v = 0; v < village_count; ++v) {
			std::cerr << '\t' << units_per_village[v];
		}
		std::cerr << '\n';
	}

	// Test the special case, everybody can reach all villages
	const bool reach_all = ((village_count == unit_count)
		&& (std::accumulate(villages_per_unit.begin(), villages_per_unit.end(), size_t())
		== (village_count * unit_count)));

	if(reach_all) {
		DBG_AI_TESTING_AI_DEFAULT << "Every unit can reach every village, dispatch them\n";
		full_dispatch(reachmap, moves);
		reachmap.clear();
		return;
	}

	// ***** ***** Find a square
	std::multimap<size_t /* villages_per_unit value*/, size_t /*villages_per_unit index*/>
		::const_iterator src_itor =  unit_lookup.begin();

	while(src_itor != unit_lookup.end() && src_itor->first == 2) {

		for(std::multimap<size_t, size_t>::const_iterator
				dst_itor = unit_lookup.begin();
				dst_itor != unit_lookup.end(); ++ dst_itor) {

			// avoid comparing us with ourselves.
			if(src_itor == dst_itor) {
				continue;
			}

			std::vector<bool> result;
			std::transform(matrix[src_itor->second].begin(), matrix[src_itor->second].end(),
				matrix[dst_itor->second].begin(),
				std::back_inserter(result),
				std::logical_and<bool>()
				);

			size_t matched = std::count(result.begin(), result.end(), true);

			// we found a  solution, dispatch
			if(matched == 2) {
				// Collect data
				std::vector<bool>::iterator first = std::find(result.begin(), result.end(), true);
				std::vector<bool>::iterator second = std::find(first + 1, result.end(), true);

				const map_location village1 = villages[first - result.begin()];
				const map_location village2 = villages[second - result.begin()];

				const bool perfect = (src_itor->first == 2 &&
					dst_itor->first == 2 &&
					units_per_village[first - result.begin()] == 2 &&
					units_per_village[second - result.begin()] == 2);

				// Dispatch
				DBG_AI_TESTING_AI_DEFAULT << "Found a square.\nDispatched unit at " << units[src_itor->second]
						<< " to village " << village1 << '\n';
				moves.push_back(std::make_pair(village1, units[src_itor->second]));

				DBG_AI_TESTING_AI_DEFAULT << "Dispatched unit at " << units[dst_itor->second]
						<< " to village " << village2 << '\n';
				moves.push_back(std::make_pair(village2, units[dst_itor->second]));

				// Remove the units
				reachmap.erase(units[src_itor->second]);
				reachmap.erase(units[dst_itor->second]);

				// Evaluate and start correct function.
				if(perfect) {
					// We did a perfect dispatch 2 units who could visit 2 villages.
					// This means we didn't change the assertion for this funtions
					// so call ourselves recursively, and finish afterwards.
					DBG_AI_TESTING_AI_DEFAULT << "Perfect dispatch, do complex again.\n";
					dispatch_complex(reachmap, moves, village_count - 2);
					return;
				} else {
					// We did a not perfect dispatch but we did modify things
					// so restart dispatching.
					DBG_AI_TESTING_AI_DEFAULT << "NON Perfect dispatch, do dispatch again.\n";
					remove_village(reachmap, moves, village1);
					remove_village(reachmap, moves, village2);
					dispatch(reachmap, moves);
					return;
				}
			}
		}

		++src_itor;
	}

	// ***** ***** Do all permutations.
	// Now walk through all possible permutations
	// - test whether the suggestion is possible
	// - does it result in max_villages
	//   - dispatch and ready
	// - is it's result better as the last best
	//   - store
	std::vector<std::pair<map_location, map_location> > best_result;

	// Bruteforcing all possible permutations can result in a slow game.
	// So there needs to be a balance between the best possible result and
	// not too slow. From the test (at the end of the file) a good number is
	// picked. In general we shouldn't reach this point too often if we do
	// there are a lot of villages which are unclaimed and a lot of units
	// to claim them.
	const size_t max_options = 8;
	if(unit_count >= max_options && village_count >= max_options) {

		DBG_AI_TESTING_AI_DEFAULT << "Too many units " << unit_count << " and villages "
			<< village_count<<" found, evaluate only the first "
			<< max_options << " options;\n";

		std::vector<size_t> perm (max_options, 0);
		for(size_t i =0; i < max_options; ++i) {
			perm[i] = i;
		}
		while(std::next_permutation(perm.begin(), perm.end())) {

			// Get result for current permutation.
			std::vector<std::pair<map_location,map_location> > result;
			for(size_t u = 0; u < max_options; ++u) {
				if(matrix[u][perm[u]]) {
					result.push_back(std::make_pair(villages[perm[u]], units[u]));

				}
			}
			if(result.size() == max_result) {
				best_result.swap(result);
				break;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assign the best
		std::copy(best_result.begin(), best_result.end(), std::back_inserter(moves));

		// Clean up the reachmap for dispatched units.
		for(std::vector<std::pair<map_location, map_location> >::const_iterator
				itor = best_result.begin(); itor != best_result.end(); ++itor) {
			reachmap.erase(itor->second);
		}

		// Try to dispatch whatever is left
		dispatch(reachmap, moves);
		return;

	} else if(unit_count <= village_count) {

		DBG_AI_TESTING_AI_DEFAULT << "Unit major\n";

		std::vector<size_t> perm (unit_count, 0);
		for(size_t i =0; i < unit_count; ++i) {
			perm[i] = i;
		}
		while(std::next_permutation(perm.begin(), perm.end())) {
			// Get result for current permutation.
			std::vector<std::pair<map_location,map_location> > result;
			for(size_t u = 0; u < unit_count; ++u) {
				if(matrix[u][perm[u]]) {
					result.push_back(std::make_pair(villages[perm[u]], units[u]));

				}
			}
			if(result.size() == max_result) {
				std::copy(result.begin(), result.end(), std::back_inserter(moves));
				reachmap.clear();
				return;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assign the best
		std::copy(best_result.begin(), best_result.end(), std::back_inserter(moves));

		// clean up the reachmap we need to test whether the leader is still there
		// and if so remove him manually to get him dispatched.
		for(std::vector<std::pair<map_location, map_location> >::const_iterator
				itor = best_result.begin(); itor != best_result.end(); ++itor) {
			reachmap.erase(itor->second);
		}
		treachmap::iterator unit = reachmap.find(leader_loc_);
		if(unit != reachmap.end()) {
			unit->second.clear();
			remove_unit(reachmap, moves, unit);
		}
		reachmap.clear();

	} else {

		DBG_AI_TESTING_AI_DEFAULT << "Village major\n";

		std::vector<size_t> perm (village_count, 0);
		for(size_t i =0; i < village_count; ++i) {
			perm[i] = i;
		}
		while(std::next_permutation(perm.begin(), perm.end())) {
			// Get result for current permutation.
			std::vector<std::pair<map_location,map_location> > result;
			for(size_t v = 0; v < village_count; ++v) {
				if(matrix[perm[v]][v]) {
					result.push_back(std::make_pair(villages[v], units[perm[v]]));

				}
			}
			if(result.size() == max_result) {
				std::copy(result.begin(), result.end(), std::back_inserter(moves));
				reachmap.clear();
				return;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assigne the best
		std::copy(best_result.begin(), best_result.end(), std::back_inserter(moves));

		// clean up the reachmap we need to test whether the leader is still there
		// and if so remove him manually to get him dispatched.
		for(std::vector<std::pair<map_location, map_location> >::const_iterator
				itor = best_result.begin(); itor != best_result.end(); ++itor) {
			reachmap.erase(itor->second);
		}
		treachmap::iterator unit = reachmap.find(leader_loc_);
		if(unit != reachmap.end()) {
			unit->second.clear();
			remove_unit(reachmap, moves, unit);
		}
		reachmap.clear();
	}
}

void get_villages_phase::full_dispatch(treachmap& reachmap, tmoves& moves)
{
	treachmap::const_iterator itor = reachmap.begin();
	for(size_t i = 0; i < reachmap.size(); ++i, ++itor) {
		DBG_AI_TESTING_AI_DEFAULT << "Dispatched unit at " << itor->first
				<< " to village " << itor->second[i] << '\n';
		moves.push_back(std::make_pair(itor->second[i], itor->first));
	}
}

void get_villages_phase::dump_reachmap(treachmap& reachmap)
{
	if(!debug_) {
		return;
	}

	for(treachmap::const_iterator itor =
			reachmap.begin(); itor != reachmap.end(); ++itor) {

		std::cerr << "Reachlist for unit at " << itor->first;

		if(itor->second.empty()) {
			std::cerr << "\tNone";
		}

		for(std::vector<map_location>::const_iterator
				v_itor = itor->second.begin();
				v_itor != itor->second.end(); ++v_itor) {

			std::cerr << '\t' << *v_itor;
		}
		std::cerr << '\n';

	}
}

//==============================================================

get_healing_phase::get_healing_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::get_healing_phase",cfg["type"]),from_(),to_()
{
}

get_healing_phase::~get_healing_phase()
{
}

double get_healing_phase::evaluate()
{
	// Find units in need of healing.
	unit_map &units_ = get_info().units;
	unit_map::iterator u_it = units_.begin();
	for(; u_it != units_.end(); ++u_it) {
		unit& u = u_it->second;

		// If the unit is on our side, has lost as many or more than
		// 1/2 round worth of healing, and doesn't regenerate itself,
		// then try to find a vacant village for it to rest in.
		if(u.side() == get_side() &&
		   (u.max_hitpoints() - u.hitpoints() >= game_config::poison_amount/2
		   || u.get_state(unit::STATE_POISONED)) &&
		    !u.get_ability_bool("regenerate"))
		{
			// Look for the village which is the least vulnerable to enemy attack.
			typedef std::multimap<map_location,map_location>::const_iterator Itor;
			std::pair<Itor,Itor> it = get_srcdst().equal_range(u_it->first);
			double best_vulnerability = 100000.0;
			// Make leader units more unlikely to move to vulnerable villages
			const double leader_penalty = (u.can_recruit()?2.0:1.0);
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const map_location& dst = it.first->second;
				if(get_info().map.gives_healing(dst) && (units_.find(dst) == units_.end() || dst == u_it->first)) {
					const double vuln = power_projection(it.first->first, get_enemy_dstsrc());
					DBG_AI_TESTING_AI_DEFAULT << "found village with vulnerability: " << vuln << "\n";
					if(vuln < best_vulnerability) {
						best_vulnerability = vuln;
						best_loc = it.first;
						DBG_AI_TESTING_AI_DEFAULT << "chose village " << dst << '\n';
					}
				}

				++it.first;
			}

			// If we have found an eligible village,
			// and we can move there without expecting to get whacked next turn:
			if(best_loc != it.second && best_vulnerability*leader_penalty < u.hitpoints()) {
				from_ = best_loc->first;
				to_ = best_loc->second;
				return 30; //@todo: externalize
			}
		}
	}

	return BAD_SCORE;
}

bool get_healing_phase::execute()
{
	if (from_==to_){
		stopunit_result_ptr stop_res = execute_stopunit_action(from_,true,false);
		if (!stop_res->is_ok()) {
			if (!stop_res->is_gamestate_changed()) {
				return false;
			}
			return true;
		}

		return true;
	}

	LOG_AI_TESTING_AI_DEFAULT << "moving unit to village for healing...\n";
	move_result_ptr move_res = execute_move_action(from_,to_,true);
	if (!move_res->is_ok()) {
		if (!move_res->is_gamestate_changed()) {
			return false;
		}
		return true;
	}
	return true;
}

//==============================================================

retreat_phase::retreat_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::retreat_phase",cfg["type"])
{
}

retreat_phase::~retreat_phase()
{
}

double retreat_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool retreat_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

move_and_targeting_phase::move_and_targeting_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::move_and_targeting_phase",cfg["type"])
{
}

move_and_targeting_phase::~move_and_targeting_phase()
{
}

double move_and_targeting_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool move_and_targeting_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

leader_control_phase::leader_control_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::leader_control_phase",cfg["type"])
{
}

leader_control_phase::~leader_control_phase()
{
}

double leader_control_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool leader_control_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
}

//==============================================================

} //end of namespace testing_ai_default

} //end of namespace ai
