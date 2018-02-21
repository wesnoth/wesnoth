/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Default AI (Testing)
 * @file
 */

#include "ai/default/ca.hpp"
#include "ai/actions.hpp"
#include "ai/manager.hpp"
#include "ai/composite/engine.hpp"
#include "ai/composite/rca.hpp"
#include "ai/composite/stage.hpp"
#include "game_board.hpp"
#include "game_classification.hpp"
#include "game_data.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "pathfind/pathfind.hpp"
#include "pathfind/teleport.hpp"

#include <numeric>
#include <boost/dynamic_bitset.hpp>

#include <SDL_timer.h>

static lg::log_domain log_ai_testing_ai_default("ai/ca/testing_ai_default");
#define DBG_AI_TESTING_AI_DEFAULT LOG_STREAM(debug, log_ai_testing_ai_default)
#define LOG_AI_TESTING_AI_DEFAULT LOG_STREAM(info, log_ai_testing_ai_default)
#define WRN_AI_TESTING_AI_DEFAULT LOG_STREAM(warn, log_ai_testing_ai_default)
#define ERR_AI_TESTING_AI_DEFAULT LOG_STREAM(err, log_ai_testing_ai_default)


namespace ai {

namespace ai_default_rca {

//==============================================================

goto_phase::goto_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg)
	, move_()
{
}

goto_phase::~goto_phase()
{
}

double goto_phase::evaluate()
{
	// Execute goto-movements - first collect gotos in a list
	std::vector<map_location> gotos;
	unit_map &units_ = resources::gameboard->units();
	const gamemap &map_ = resources::gameboard->map();

	for(unit_map::iterator ui = units_.begin(); ui != units_.end(); ++ui) {
		if (ui->get_goto() == ui->get_location()) {
			ui->set_goto(map_location());
		} else if (ui->side() == get_side() && map_.on_board(ui->get_goto())) {
			gotos.push_back(ui->get_location());
		}
	}

	for(std::vector<map_location>::const_iterator g = gotos.begin(); g != gotos.end(); ++g) {
		unit_map::const_iterator ui = units_.find(*g);
		// passive_leader: never moves or attacks
		if(ui->can_recruit() && get_passive_leader() && !get_passive_leader_shares_keep()){
			continue;//@todo: only bail out if goto is on keep
		}
		// end of passive_leader

		const pathfind::shortest_path_calculator calc(*ui, current_team(), resources::gameboard->teams(), resources::gameboard->map());

		const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*ui, current_team());

		pathfind::plain_route route;
		route = pathfind::a_star_search(ui->get_location(), ui->get_goto(), 10000.0, calc, map_.w(), map_.h(), &allowed_teleports);

		if (!route.steps.empty()){
			move_ = check_move_action(ui->get_location(), route.steps.back(), true, true);
		} else {
			// there is no direct path (yet)
			// go to the nearest hex instead.
			// maybe a door will open later or something

			int closest_distance = -1;
			std::pair<map_location,map_location> closest_move;
			for(move_map::const_iterator i = get_dstsrc().begin(); i != get_dstsrc().end(); ++i) {
				if(i->second != ui->get_location()) {
						continue;
				}
				int distance = distance_between(i->first,ui->get_goto());
				if(closest_distance == -1 || distance < closest_distance) {
					closest_distance = distance;
					closest_move = *i;
				}
			}
			if(closest_distance != -1) {
				move_ = check_move_action(ui->get_location(), closest_move.first);
			} else {
				continue;
			}
		}

		if (move_->is_ok()) {
			return get_score();
		}
	}

	return BAD_SCORE;
}

void goto_phase::execute()
{
	if (!move_) {
		return;
	}

	move_->execute();
	if (!move_->is_ok()){
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok" << std::endl;
	}

	// In some situations, a theoretically possible path is blocked by allies,
	// resulting in the unit not moving. In this case, we remove all remaining
	// movement from the unit in order to prevent blacklisting of the CA.
	if (!move_->is_gamestate_changed()){
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute did not move unit; removing moves instead" << std::endl;
		stopunit_result_ptr stopunit = check_stopunit_action(move_->get_unit_location(), true, false);
		stopunit->execute();
	}
}


//==============================================================

combat_phase::combat_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg),best_analysis_(),choice_rating_(-1000.0)
{
}

combat_phase::~combat_phase()
{
}

double combat_phase::evaluate()
{
	std::vector<std::string> options = get_recruitment_pattern();

	choice_rating_ = -1000.0;
	int ticks = SDL_GetTicks();

	const std::vector<attack_analysis> analysis = get_attacks(); //passive_leader: in aspect_attacks::analyze_targets()

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

	std::vector<attack_analysis>::const_iterator choice_it = analysis.end();
	for(std::vector<attack_analysis>::const_iterator it = analysis.begin();
			it != analysis.end(); ++it) {

		if(skip_num > 0 && ((it - analysis.begin())%skip_num) && it->movements.size() > 1)
			continue;

		const double rating = it->rating(get_aggression(),*this);
		LOG_AI_TESTING_AI_DEFAULT << "attack option rated at " << rating << " ("
					  << (it->uses_leader ? get_leader_aggression() : get_aggression()) << ")\n";

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
		return get_score();
	} else {
		return BAD_SCORE;
	}
}

void combat_phase::execute()
{
	assert(choice_rating_ > 0.0);
	map_location from   = best_analysis_.movements[0].first;
	map_location to     = best_analysis_.movements[0].second;
	map_location target_loc = best_analysis_.target;

	if (from!=to) {
		move_result_ptr move_res = execute_move_action(from,to,false);
		if (!move_res->is_ok()) {
			LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok, move failed" << std::endl;
			return;
		}
	}

	attack_result_ptr attack_res = check_attack_action(to, target_loc, -1);
	if (!attack_res->is_ok()) {
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok, attack cancelled" << std::endl;
	} else {
		attack_res->execute();
		if (!attack_res->is_ok()) {
			LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok, attack failed" << std::endl;
		}
	}

}

//==============================================================

move_leader_to_goals_phase::move_leader_to_goals_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg), auto_remove_(), dst_(), id_(), move_()
{
}

move_leader_to_goals_phase::~move_leader_to_goals_phase()
{
}

double move_leader_to_goals_phase::evaluate()
{

	const config &goal = get_leader_goal();
	//passive leader can reach a goal
	if (!goal) {
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "No goal found\n";
		return BAD_SCORE;
	}

	if (goal.empty()) {
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "Empty goal found\n";
		return BAD_SCORE;
	}

	double max_risk = goal["max_risk"].to_double(1 - get_caution());
	auto_remove_ = goal["auto_remove"].to_bool();

	dst_ = map_location(goal, resources::gamedata);
	if (!dst_.valid()) {
		ERR_AI_TESTING_AI_DEFAULT << "Invalid goal: "<<std::endl<<goal;
		return BAD_SCORE;
	}

	const unit_map::iterator leader = resources::gameboard->units().find_leader(get_side());
	if (!leader.valid() || leader->incapacitated()) {
		WRN_AI_TESTING_AI_DEFAULT << "Leader not found" << std::endl;
		return BAD_SCORE;
	}

	id_ = goal["id"].str();
	if (leader->get_location() == dst_) {
		//goal already reached
		if (auto_remove_ && !id_.empty()) {
			remove_goal(id_);
		} else {
			move_ = check_move_action(leader->get_location(), leader->get_location(), !auto_remove_);//we do full moves if we don't want to remove goal
			if (move_->is_ok()) {
				return get_score();
			} else {
				return BAD_SCORE;
			}
		}
	}

	pathfind::shortest_path_calculator calc(*leader, current_team(), resources::gameboard->teams(), resources::gameboard->map());
	pathfind::plain_route route = a_star_search(leader->get_location(), dst_, 1000.0, calc,
			resources::gameboard->map().w(), resources::gameboard->map().h());
	if(route.steps.empty()) {
		LOG_AI_TESTING_AI_DEFAULT << "route empty";
		return BAD_SCORE;
	}

	const pathfind::paths leader_paths(*leader, false, true, current_team());

	std::map<map_location,pathfind::paths> possible_moves;
	possible_moves.emplace(leader->get_location(), leader_paths);

	map_location loc;
	for (const map_location &l : route.steps)
	{
		if (leader_paths.destinations.contains(l) &&
		    power_projection(l, get_enemy_dstsrc()) < leader->hitpoints() * max_risk)
		{
			loc = l;
		}
	}

	if(loc.valid()) {
		move_ = check_move_action(leader->get_location(), loc, false);
		if (move_->is_ok()) {
			return get_score();
		}
	}
	return BAD_SCORE;

}

void move_leader_to_goals_phase::execute()
{
	move_->execute();
	if (!move_->is_ok()){
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok" << std::endl;
	}
	if (move_->get_unit_location()==dst_) {
		//goal already reached
		if (auto_remove_ && !id_.empty()) {
			remove_goal(id_);
		}
	}
}

void move_leader_to_goals_phase::remove_goal(const std::string &id)
{
	config mod_ai;
	mod_ai["side"] = get_side();
	mod_ai["path"] = "aspect[leader_goal].facet["+id+"]";
	mod_ai["action"] = "delete";
	manager::get_singleton().modify_active_ai_for_side(get_side(), mod_ai);
}

//==============================================================

move_leader_to_keep_phase::move_leader_to_keep_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg),move_()
{

}

move_leader_to_keep_phase::~move_leader_to_keep_phase()
{

}

double move_leader_to_keep_phase::evaluate()
{
	if (get_leader_ignores_keep()) {
		return BAD_SCORE;
	}
	if (get_passive_leader() && !get_passive_leader_shares_keep()) {
		return BAD_SCORE;
	}

	// 1. Collect all leaders in a list
	// 2. Get the suitable_keep for each leader
	// 3. Choose the leader with the nearest suitable_keep (and which still have moves)
	// 4. If leader can reach this keep in 1 turn -> set move_ to there
	// 5. If not -> Calculate the best move_ (use a-star search)
	// 6. Save move_ for execution

	// 1.
	const unit_map &units_ = resources::gameboard->units();
	const std::vector<unit_map::const_iterator> leaders = units_.find_leaders(get_side());
	if (leaders.empty()) {
		return BAD_SCORE;
	}

	// 2. + 3.
	const unit* best_leader = nullptr;
	map_location best_keep;
	int shortest_distance = 99999;

	for (const unit_map::const_iterator& leader : leaders) {
		if (leader->incapacitated() || leader->movement_left() == 0) {
			continue;
		}

		// Find where the leader can move
		const ai::moves_map &possible_moves = get_possible_moves();
		const ai::moves_map::const_iterator& p_it = possible_moves.find(leader->get_location());
		if (p_it == possible_moves.end()) {
			return BAD_SCORE;
		}
		const pathfind::paths leader_paths = p_it->second;

		const map_location& keep = suitable_keep(leader->get_location(), leader_paths);
		if (keep == map_location::null_location() || keep == leader->get_location()) {
			continue;
		}

		const pathfind::shortest_path_calculator calc(*leader, current_team(), resources::gameboard->teams(), resources::gameboard->map());

		const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*leader, current_team());

		pathfind::plain_route route;
		route = pathfind::a_star_search(leader->get_location(), keep, 10000.0, calc, resources::gameboard->map().w(), resources::gameboard->map().h(), &allowed_teleports);

		if (!route.steps.empty() || route.move_cost < shortest_distance) {
			best_leader = &(*leader);
			best_keep = keep;
			shortest_distance = route.move_cost;
		}
	}

	if (best_leader == nullptr) {
		return BAD_SCORE;
	}

	// 4.
	const unit* leader = best_leader;
	const map_location keep = best_keep;
	const pathfind::paths leader_paths(*leader, false, true, current_team());
	const pathfind::shortest_path_calculator calc(*leader, current_team(), resources::gameboard->teams(), resources::gameboard->map());
	const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*leader, current_team());

	if (leader_paths.destinations.contains(keep) && units_.count(keep) == 0) {
		move_ = check_move_action(leader->get_location(), keep, false);
		if (move_->is_ok()) {
			return get_score();
		}
	}

	// 5.
	// The leader can't move to his keep, try to move to the closest location
	// to the keep where there are no enemies in range.
	// Make a map of the possible locations the leader can move to,
	// ordered by the distance from the keep.
	typedef std::multimap<int, map_location> ordered_locations;
	ordered_locations moves_toward_keep;

	pathfind::plain_route route;
	route = pathfind::a_star_search(leader->get_location(), keep, 10000.0, calc, resources::gameboard->map().w(), resources::gameboard->map().h(), &allowed_teleports);

	// find next hop
	map_location next_hop = map_location::null_location();
	int next_hop_cost = 0;
	for (const map_location& step : route.steps) {
		if (leader_paths.destinations.contains(step)) {
			next_hop = step;
			next_hop_cost += leader->movement_cost(resources::gameboard->map().get_terrain(step));
		} else {
			break;
		}
	}
	if (next_hop == map_location::null_location()) {
		return BAD_SCORE;
	}
	//define the next hop to have the lowest cost (0)
	moves_toward_keep.emplace(0, next_hop);

	for (const pathfind::paths::step &dest : leader_paths.destinations) {
		if (!units_.find(dest.curr).valid()) {
			route = pathfind::a_star_search(dest.curr, next_hop, 10000.0, calc,
					resources::gameboard->map().w(), resources::gameboard->map().h(), &allowed_teleports);
			if (route.move_cost < next_hop_cost) {
				moves_toward_keep.emplace(route.move_cost, dest.curr);
			}
		}
	}

	// Find the first location which we can move to,
	// without the threat of enemies.
	for (const ordered_locations::value_type& pair : moves_toward_keep) {
		const map_location& loc = pair.second;
		if (get_enemy_dstsrc().count(loc) == 0) {
			move_ = check_move_action(leader->get_location(), loc, true);
			if (move_->is_ok()) {
				return get_score();
			}
		}
	}
	return BAD_SCORE;
}

void move_leader_to_keep_phase::execute()
{
	move_->execute();
	if (!move_->is_ok()) {
		LOG_AI_TESTING_AI_DEFAULT <<  get_name() <<"::execute not ok" << std::endl;
	}
}

//==============================================================

get_villages_phase::get_villages_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg)
	, keep_loc_()
	, leader_loc_()
	, best_leader_loc_()
	, debug_(false)
	, moves_()
{
}

get_villages_phase::~get_villages_phase()
{
}

double get_villages_phase::evaluate()
{
	moves_.clear();
	unit_map::const_iterator leader = resources::gameboard->units().find_leader(get_side());
	get_villages(get_dstsrc(),get_enemy_dstsrc(),leader);
	if (!moves_.empty()) {
		return get_score();
	}
	return BAD_SCORE;
}


void get_villages_phase::execute()
{
	unit_map &units_ = resources::gameboard->units();
	unit_map::const_iterator leader = units_.find_leader(get_side());
	// Move all the units to get villages, however move the leader last,
	// so that the castle will be cleared if it wants to stop to recruit along the way.
	std::pair<map_location,map_location> leader_move;

	for(tmoves::const_iterator i = moves_.begin(); i != moves_.end(); ++i) {

		if(leader != units_.end() && leader->get_location() == i->second) {
			leader_move = *i;
		} else {
			if (resources::gameboard->find_visible_unit(i->first, current_team()) == units_.end()) {
				move_result_ptr move_res = execute_move_action(i->second,i->first,true);
				if (!move_res->is_ok()) {
					return;
				}

				const map_location loc = move_res->get_unit_location();
				leader = units_.find_leader(get_side());
				const unit_map::const_iterator new_unit = units_.find(loc);

				if (new_unit != units_.end() &&
				    power_projection(i->first, get_enemy_dstsrc()) >= new_unit->hitpoints() / 4.0)
				{
					LOG_AI_TESTING_AI_DEFAULT << "found support target... " << new_unit->get_location() << '\n';
					//FIXME: suokko tweaked the constant 1.0 to the formula:
					//25.0* current_team().caution() * power_projection(loc,enemy_dstsrc) / new_unit->second.hitpoints()
					//Is this an improvement?

					///@todo 1.7 check if this an improvement
					//add_target(target(new_unit->first,1.0,target::SUPPORT));
				}
			}
		}
	}

	if(leader_move.second.valid()) {
		if((resources::gameboard->find_visible_unit(leader_move.first , current_team()) == units_.end())
		   && resources::gameboard->map().is_village(leader_move.first)) {
			move_result_ptr move_res = execute_move_action(leader_move.second,leader_move.first,true);
			if (!move_res->is_ok()) {
				return;
			}
		}
	}

	return;
}

void get_villages_phase::get_villages(
		const move_map& dstsrc, const move_map& enemy_dstsrc,
		unit_map::const_iterator &leader)
{
	DBG_AI_TESTING_AI_DEFAULT << "deciding which villages we want...\n";
	unit_map &units_ = resources::gameboard->units();
	const int ticks = SDL_GetTicks();
	best_leader_loc_ = map_location::null_location();
	if(leader != units_.end()) {
		keep_loc_ = nearest_keep(leader->get_location());
		leader_loc_ = leader->get_location();
	} else {
		keep_loc_ = map_location::null_location();
		leader_loc_ = map_location::null_location();
	}

	debug_ = !lg::debug().dont_log(log_ai_testing_ai_default);

	// Find our units who can move.
	treachmap reachmap;
	for(unit_map::const_iterator u_itor = units_.begin();
			u_itor != units_.end(); ++u_itor) {
		if(u_itor->can_recruit() && get_passive_leader()){
			continue;
		}
		if(u_itor->side() == get_side() && u_itor->movement_left()) {
			reachmap.emplace(u_itor->get_location(),	std::vector<map_location>());
		}
	}


	DBG_AI_TESTING_AI_DEFAULT << reachmap.size() << " units found who can try to capture a village.\n";

	find_villages(reachmap, moves_, dstsrc, enemy_dstsrc);

	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		if(itor->second.empty()) {
			itor = remove_unit(reachmap, moves_, itor);
		} else {
			++itor;
		}
	}

	if(!reachmap.empty()) {
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
	const std::multimap<map_location,map_location>& enemy_dstsrc)

{
	std::map<map_location, double> vulnerability;

	const bool passive_leader = get_passive_leader();

	size_t min_distance = 100000;
	const gamemap &map_ = resources::gameboard->map();
	std::vector<team> &teams_ = resources::gameboard->teams();

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
		if(!owned && leader_loc_ == map_location::null_location()) {
			continue;
		}

		double threat = 0.0;
		const std::map<map_location,double>::const_iterator vuln = vulnerability.find(current_loc);
		if(vuln != vulnerability.end()) {
			threat = vuln->second;
		} else {
			threat = power_projection(current_loc,enemy_dstsrc);
			vulnerability.emplace(current_loc, threat);
		}

		const unit_map::const_iterator u = resources::gameboard->units().find(j->second);
		if (u == resources::gameboard->units().end() || u->get_state("guardian")) {
			continue;
		}

		const unit  &un = *u;
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
				moves.emplace_back(j->first, j->second);
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

		if(!reachmap.empty() && dispatched) {
			DBG_AI_TESTING_AI_DEFAULT << reachmap.size() << " unit(s) left restarting simple dispatching.\n";

			dump_reachmap(reachmap);
		}
	}

	if(reachmap.empty()) {
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
			moves.emplace_back(village, itor->first);
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

		moves.emplace_back(reachmap.begin()->second[0], reachmap.begin()->first);

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
			std::vector<map_location /* units that can reach it*/>>reversemap;

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
				moves.emplace_back(itor->first, itor->second[0]);

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

	if(unit->first == leader_loc_ && best_leader_loc_ != map_location::null_location()) {
		DBG_AI_TESTING_AI_DEFAULT << "Dispatch leader at " << leader_loc_ << " closer to the keep at "
			<< best_leader_loc_ << '\n';

		moves.emplace_back(best_leader_loc_, leader_loc_);
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

	std::vector</*unit*/boost::dynamic_bitset</*village*/>> matrix(reachmap.size(), boost::dynamic_bitset<>(village_count));

	treachmap::const_iterator itor = reachmap.begin();
	for(size_t u = 0; u < unit_count; ++u, ++itor) {
		units[u] = itor->first;
		villages_per_unit[u] = itor->second.size();
		unit_lookup.emplace(villages_per_unit[u], u);

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

			for(v = 0; v < village_count; ++v) {
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

			boost::dynamic_bitset<> result = matrix[src_itor->second] & matrix[dst_itor->second];
			size_t matched = result.count();

			// we found a  solution, dispatch
			if(matched == 2) {
				// Collect data
				size_t first = result.find_first();
				size_t second = result.find_next(first);

				const map_location village1 = villages[first];
				const map_location village2 = villages[second];

				const bool perfect = (src_itor->first == 2 &&
					dst_itor->first == 2 &&
					units_per_village[first] == 2 &&
					units_per_village[second] == 2);

				// Dispatch
				DBG_AI_TESTING_AI_DEFAULT << "Found a square.\nDispatched unit at " << units[src_itor->second]
						<< " to village " << village1 << '\n';
				moves.emplace_back(village1, units[src_itor->second]);

				DBG_AI_TESTING_AI_DEFAULT << "Dispatched unit at " << units[dst_itor->second]
						<< " to village " << village2 << '\n';
				moves.emplace_back(village2, units[dst_itor->second]);

				// Remove the units
				reachmap.erase(units[src_itor->second]);
				reachmap.erase(units[dst_itor->second]);

				// Evaluate and start correct function.
				if(perfect) {
					// We did a perfect dispatch 2 units who could visit 2 villages.
					// This means we didn't change the assertion for this functions
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
	std::vector<std::pair<map_location, map_location>> best_result;

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
			std::vector<std::pair<map_location,map_location>> result;
			for(size_t u = 0; u < max_options; ++u) {
				if(matrix[u][perm[u]]) {
					result.emplace_back(villages[perm[u]], units[u]);

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
		moves.insert(moves.end(), best_result.begin(), best_result.end());

		// Clean up the reachmap for dispatched units.
		for(const auto& unit_village_pair : best_result) {
			reachmap.erase(unit_village_pair.second);
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
			std::vector<std::pair<map_location,map_location>> result;
			for(size_t u = 0; u < unit_count; ++u) {
				if(matrix[u][perm[u]]) {
					result.emplace_back(villages[perm[u]], units[u]);

				}
			}
			if(result.size() == max_result) {
				moves.insert(moves.end(), result.begin(), result.end());
				reachmap.clear();
				return;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assign the best
		moves.insert(moves.end(), best_result.begin(), best_result.end());

		// clean up the reachmap we need to test whether the leader is still there
		// and if so remove him manually to get him dispatched.
		for(const auto& unit_village_pair : best_result) {
			reachmap.erase(unit_village_pair.second);
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
			std::vector<std::pair<map_location,map_location>> result;
			for(size_t v = 0; v < village_count; ++v) {
				if(matrix[perm[v]][v]) {
					result.emplace_back(villages[v], units[perm[v]]);

				}
			}
			if(result.size() == max_result) {
				moves.insert(moves.end(), result.begin(), result.end());
				reachmap.clear();
				return;
			}

			if(result.size() > best_result.size()) {
				best_result.swap(result);
			}
		}
		// End of loop no optimal found, assigne the best
		moves.insert(moves.end(), best_result.begin(), best_result.end());

		// clean up the reachmap we need to test whether the leader is still there
		// and if so remove him manually to get him dispatched.
		for(const auto& unit_village_pair : best_result) {
			reachmap.erase(unit_village_pair.second);
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
		moves.emplace_back(itor->second[i], itor->first);
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
	: candidate_action(context,cfg),move_()
{
}

get_healing_phase::~get_healing_phase()
{
}

double get_healing_phase::evaluate()
{
	// Find units in need of healing.
	unit_map &units_ = resources::gameboard->units();
	unit_map::iterator u_it = units_.begin();
	for(; u_it != units_.end(); ++u_it) {
		unit &u = *u_it;

		if(u.can_recruit() && get_passive_leader()){
			continue;
		}

		// If the unit is on our side, has lost as many or more than
		// 1/2 round worth of healing, and doesn't regenerate itself,
		// then try to find a vacant village for it to rest in.
		if(u.side() == get_side() &&
		   (u.max_hitpoints() - u.hitpoints() >= game_config::poison_amount/2
		   || u.get_state(unit::STATE_POISONED)) &&
		    !u.get_ability_bool("regenerate", *resources::gameboard))
		{
			// Look for the village which is the least vulnerable to enemy attack.
			typedef std::multimap<map_location,map_location>::const_iterator Itor;
			std::pair<Itor,Itor> it = get_srcdst().equal_range(u_it->get_location());
			double best_vulnerability = 100000.0;
			// Make leader units more unlikely to move to vulnerable villages
			const double leader_penalty = (u.can_recruit()?2.0:1.0);
			Itor best_loc = it.second;
			while(it.first != it.second) {
				const map_location& dst = it.first->second;
				if (resources::gameboard->map().gives_healing(dst) && (units_.find(dst) == units_.end() || dst == u_it->get_location())) {
					const double vuln = power_projection(dst, get_enemy_dstsrc());
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
				move_ = check_move_action(best_loc->first,best_loc->second,true);
				if (move_->is_ok()) {
					return get_score();
				}
			}
		}
	}

	return BAD_SCORE;
}

void get_healing_phase::execute()
{
	LOG_AI_TESTING_AI_DEFAULT << "moving unit to village for healing...\n";
	move_->execute();
	if (!move_->is_ok()){
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok" << std::endl;
	}
}

//==============================================================

retreat_phase::retreat_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg), move_()
{
}

retreat_phase::~retreat_phase()
{
}

double retreat_phase::evaluate()
{


	// Get versions of the move map that assume that all units are at full movement
	const unit_map& units_ = resources::gameboard->units();

	//unit_map::const_iterator leader = units_.find_leader(get_side());
	std::vector<unit_map::const_iterator> leaders = units_.find_leaders(get_side());
	std::map<map_location,pathfind::paths> dummy_possible_moves;

	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves, fullmove_srcdst, fullmove_dstsrc,
			false, true, &get_avoid());

	/*adjacent_loc_array_t leader_adj;
	if(leader != units_.end()) {
		get_adjacent_tiles(leader->get_location(), leader_adj.data());
	}*/
	//int leader_adj_count = 0;
	std::vector<map_location> leaders_adj_v;
	for (unit_map::const_iterator leader : leaders) {
		adjacent_loc_array_t tmp_leader_adj;
		get_adjacent_tiles(leader->get_location(), tmp_leader_adj.data());
		for (map_location &loc : tmp_leader_adj) {
			bool found = false;
			for (map_location &new_loc : leaders_adj_v) {
				if(new_loc == loc){
					found = true;
					break;
				}
			}
			if(!found){
				leaders_adj_v.push_back(loc);
			}
		}
	}
	//leader_adj_count = leaders_adj_v.size();


	for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
		if (i->side() == get_side() &&
		    i->movement_left() == i->total_movement() &&
		    //leaders.find(*i) == leaders.end() && //unit_map::const_iterator(i) != leader &&
		    std::find(leaders.begin(), leaders.end(), i) == leaders.end() &&
		    !i->incapacitated())
		{
			// This unit still has movement left, and is a candidate to retreat.
			// We see the amount of power of each side on the situation,
			// and decide whether it should retreat.
			if(should_retreat(i->get_location(), i, fullmove_srcdst, fullmove_dstsrc, get_caution())) {

				bool can_reach_leader = false;

				// Time to retreat. Look for the place where the power balance
				// is most in our favor.
				// If we can't find anywhere where we like the power balance,
				// just try to get to the best defensive hex.
				typedef move_map::const_iterator Itor;
				std::pair<Itor,Itor> itors = get_srcdst().equal_range(i->get_location());
				map_location best_pos, best_defensive(i->get_location());

				double best_rating = -1000.0;
				int best_defensive_rating = i->defense_modifier(resources::gameboard->map().get_terrain(i->get_location()))
					- (resources::gameboard->map().is_village(i->get_location()) ? 10 : 0);
				while(itors.first != itors.second) {

					//if(leader != units_.end() && std::count(leader_adj,
					//			leader_adj + 6, itors.first->second)) {
					if(std::find(leaders_adj_v.begin(), leaders_adj_v.end(), itors.first->second) != leaders_adj_v.end()){

						can_reach_leader = true;
						break;
					}

					// We rate the power balance of a hex based on our power projection
					// compared to theirs, multiplying their power projection by their
					// chance to hit us on the hex we're planning to flee to.
					const map_location& hex = itors.first->second;
					const int defense = i->defense_modifier(resources::gameboard->map().get_terrain(hex));
					const double our_power = power_projection(hex,get_dstsrc());
					const double their_power = power_projection(hex,get_enemy_dstsrc()) * double(defense)/100.0;
					const double rating = our_power - their_power;
					if(rating > best_rating) {
						best_pos = hex;
						best_rating = rating;
					}

					// Give a bonus for getting to a village.
					const int modified_defense = defense - (resources::gameboard->map().is_village(hex) ? 10 : 0);

					if(modified_defense < best_defensive_rating) {
						best_defensive_rating = modified_defense;
						best_defensive = hex;
					}

					++itors.first;
				}

				// If the unit is in range of its leader, it should
				// never retreat -- it has to defend the leader instead.
				if(can_reach_leader) {
					continue;
				}

				if(!best_pos.valid()) {
					best_pos = best_defensive;
				}

				if(best_pos.valid()) {
					move_ = check_move_action(i->get_location(), best_pos, true);
					if (move_->is_ok()) {
						return get_score();
					}
				}
			}
		}
	}

	return BAD_SCORE;
}

void retreat_phase::execute()
{
	move_->execute();
	if (!move_->is_ok()){
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok" << std::endl;
	}
}



bool retreat_phase::should_retreat(const map_location& loc, const unit_map::const_iterator& un,  const move_map &srcdst, const move_map &dstsrc, double caution)
{
	const move_map &enemy_dstsrc = get_enemy_dstsrc();

	if(caution <= 0.0) {
		return false;
	}

	double optimal_terrain = best_defensive_position(un->get_location(), dstsrc,
			srcdst, enemy_dstsrc).chance_to_hit/100.0;
	const double proposed_terrain =
		un->defense_modifier(resources::gameboard->map().get_terrain(loc)) / 100.0;

	// The 'exposure' is the additional % chance to hit
	// this unit receives from being on a sub-optimal defensive terrain.
	const double exposure = proposed_terrain - optimal_terrain;

	const double our_power = power_projection(loc,dstsrc);
	const double their_power = power_projection(loc,enemy_dstsrc);
	return caution*their_power*(1.0+exposure) > our_power;
}


//==============================================================

leader_control_phase::leader_control_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg)
{
}


leader_control_phase::~leader_control_phase()
{
}

double leader_control_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented" << std::endl;
	return BAD_SCORE;
}



void leader_control_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented" << std::endl;
}

//==============================================================

leader_shares_keep_phase::leader_shares_keep_phase( rca_context &context, const config &cfg )
	:candidate_action(context, cfg)
{
}

leader_shares_keep_phase::~leader_shares_keep_phase()
{
}

double leader_shares_keep_phase::evaluate()
{
	if(get_passive_leader() && !get_passive_leader_shares_keep()){
		return BAD_SCORE;
	}
	bool allied_leaders_available = false;
	for(team &tmp_team : resources::gameboard->teams()) {
		if(!current_team().is_enemy(tmp_team.side())){
			std::vector<unit_map::unit_iterator> allied_leaders = resources::gameboard->units().find_leaders(get_side());
			if (!allied_leaders.empty()){
				allied_leaders_available = true;
				break;
			}
		}
	}
	if(allied_leaders_available){
		return get_score();
	}
	return BAD_SCORE;
}

void leader_shares_keep_phase::execute()
{
	//get all AI leaders
	std::vector<unit_map::unit_iterator> ai_leaders = resources::gameboard->units().find_leaders(get_side());

	//calculate all possible moves (AI + allies)
	typedef std::map<map_location, pathfind::paths> path_map;
	path_map possible_moves;
	move_map friends_srcdst, friends_dstsrc;
	calculate_moves(resources::gameboard->units(), possible_moves, friends_srcdst, friends_dstsrc, false, true);

	//check for each ai leader if he should move away from his keep
	for (unit_map::unit_iterator &ai_leader : ai_leaders) {
		//only if leader is on a keep
		const map_location &keep = ai_leader->get_location();
		if ( !resources::gameboard->map().is_keep(keep) ) {
			continue;
		}
		map_location recruit_loc = pathfind::find_vacant_castle(*ai_leader);
		if(!resources::gameboard->map().on_board(recruit_loc)){
			continue;
		}
		bool friend_can_reach_keep = false;

		//for each leader, check if he's allied and can reach our keep
		for(path_map::const_iterator i = possible_moves.begin(); i != possible_moves.end(); ++i){
			const unit_map::const_iterator itor = resources::gameboard->units().find(i->first);
			team &leader_team = resources::gameboard->get_team(itor->side());
			if(itor != resources::gameboard->units().end() && itor->can_recruit() && itor->side() != get_side() && (leader_team.total_income() + leader_team.gold() > leader_team.minimum_recruit_price())){
				pathfind::paths::dest_vect::const_iterator tokeep = i->second.destinations.find(keep);
				if(tokeep != i->second.destinations.end()){
					friend_can_reach_keep = true;
					break;
				}
			}
		}
		//if there's no allied leader who can reach the keep, check next ai leader
		if(friend_can_reach_keep){
			//determine the best place the ai leader can move to
			map_location best_move;
			int defense_modifier = 100;
			for(pathfind::paths::dest_vect::const_iterator i = possible_moves[keep].destinations.begin()
					; i != possible_moves[keep].destinations.end()
					; ++i){

				//calculate_moves() above uses max. moves -> need to check movement_left of leader here
				if(distance_between(i->curr, keep) <= 3
						&& static_cast<int>(distance_between(i->curr, keep)) <= ai_leader->movement_left()){

					int tmp_def_mod = ai_leader->defense_modifier(resources::gameboard->map().get_terrain(i->curr));
					if(tmp_def_mod < defense_modifier){
						defense_modifier = tmp_def_mod;
						best_move = i->curr;
					}
				}
			}
			//only move if there's a place with a good defense
			if(defense_modifier < 100){
				move_result_ptr move = check_move_action(keep, best_move, true);
				if(move->is_ok()){
					move->execute();
					if (!move->is_ok()){
						LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok" << std::endl;
					}else{
						ai_leader->set_goto(keep);
					}
				}else{
					LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok" << std::endl;
				}
			}
		}
		ai_leader->remove_movement_ai();
	}
	for(unit_map::unit_iterator &leader : ai_leaders) {
		leader->remove_movement_ai();
	}
	//ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented" << std::endl;
}


//==============================================================


} //end of namespace testing_ai_default

} //end of namespace ai
