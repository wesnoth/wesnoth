/*
   Copyright (C) 2009 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "ca.hpp"
#include "../actions.hpp"
#include "../manager.hpp"
#include "../composite/engine.hpp"
#include "../composite/rca.hpp"
#include "../composite/stage.hpp"
#include "../../gamestatus.hpp"
#include "../../log.hpp"
#include "../../map.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"
#include "../../pathfind/pathfind.hpp"
#include "../../pathfind/teleport.hpp"

#include <boost/foreach.hpp>

#include <numeric>

static lg::log_domain log_ai_testing_ai_default("ai/ca/testing_ai_default");
#define DBG_AI_TESTING_AI_DEFAULT LOG_STREAM(debug, log_ai_testing_ai_default)
#define LOG_AI_TESTING_AI_DEFAULT LOG_STREAM(info, log_ai_testing_ai_default)
#define WRN_AI_TESTING_AI_DEFAULT LOG_STREAM(warn, log_ai_testing_ai_default)
#define ERR_AI_TESTING_AI_DEFAULT LOG_STREAM(err, log_ai_testing_ai_default)


namespace ai {

namespace testing_ai_default {

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
	unit_map &units_ = *resources::units;
	gamemap &map_ = *resources::game_map;

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

		const pathfind::shortest_path_calculator calc(*ui, current_team(), *resources::teams, *resources::game_map);

		const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*ui, current_team());

		pathfind::plain_route route;
		route = pathfind::a_star_search(ui->get_location(), ui->get_goto(), 10000.0, &calc, map_.w(), map_.h(), &allowed_teleports);

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
				return BAD_SCORE;
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
}

//==============================================================

aspect_recruitment_phase::aspect_recruitment_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg)
{
}


aspect_recruitment_phase::~aspect_recruitment_phase()
{
}

double aspect_recruitment_phase::evaluate()
{
	const unit_map::const_iterator leader = resources::units->find_leader(get_side());
	if(leader == resources::units->end()) {
		return BAD_SCORE;
	}
	if (!resources::game_map->is_keep(leader->get_location())) {
		return BAD_SCORE;
	}

	map_location recruit_loc = pathfind::find_vacant_castle(*leader);
	if (!resources::game_map->on_board(recruit_loc)) {
		return BAD_SCORE;
	}

	//note that no gold check is done. This is intended, to speed up recruitment_phase::evaluate()
	//so, after 1st failed recruit, this candidate action will be blacklisted for 1 turn.

	return get_score();
}

void aspect_recruitment_phase::execute()
{
	raise_user_interact();
	stage_ptr r = get_recruitment(*this);
	if (r) {
		r->play_stage();
	} else {
		ERR_AI_TESTING_AI_DEFAULT << "no recruitment aspect - skipping recruitment" << std::endl;
	}
}

//==============================================================

recruitment_phase::recruitment_phase( rca_context &context, const config &cfg )
	: candidate_action(context,cfg)
	, unit_movement_scores_()
	, not_recommended_units_()
	, unit_combat_scores_()
{
}


recruitment_phase::~recruitment_phase()
{
}

double recruitment_phase::evaluate()
{
	const unit_map::const_iterator leader = resources::units->find_leader(get_side());
	if(leader == resources::units->end()) {
		return BAD_SCORE;
	}
	if (!resources::game_map->is_keep(leader->get_location())) {
		return BAD_SCORE;
	}

	std::set<map_location> checked_hexes;
	checked_hexes.insert(leader->get_location());
	if (count_free_hexes_in_castle(leader->get_location(), checked_hexes)==0) {
		return BAD_SCORE;
	}

	//note that no gold check is done. This is intended, to speed up recruitment_phase::evaluate()
	//so, after 1st failed recruit, this candidate action will be blacklisted for 1 turn.

	return get_score();
}

void recruitment_phase::execute()
{
	not_recommended_units_.clear();
	unit_combat_scores_.clear();
	unit_movement_scores_.clear();

	unit_map &units_ = *resources::units;
	gamemap &map_ = *resources::game_map;
	std::vector<team> &teams_ = *resources::teams;

	map_location start_pos = units_.find_leader(get_side())->get_location();

	raise_user_interact();
	//analyze_potential_recruit_movements();
	analyze_potential_recruit_combat();

	std::vector<std::string> options = get_recruitment_pattern();

	if (std::count(options.begin(), options.end(), "scout") > 0) {
		size_t neutral_villages = 0;

		// We recruit the initial allocation of scouts
		// based on how many neutral villages there are
		// that are closer to us than to other keeps.
		const std::vector<map_location>& villages = map_.villages();
		for(std::vector<map_location>::const_iterator v = villages.begin(); v != villages.end(); ++v) {
			const int owner = village_owner(*v);
			if(owner == -1) {
				const size_t distance = distance_between(start_pos,*v);

				bool closest = true;
				for(std::vector<team>::const_iterator i = teams_.begin(); i != teams_.end(); ++i) {
					const int index = i - teams_.begin() + 1;
					const map_location& loc = map_.starting_position(index);
					if(loc != start_pos && distance_between(loc,*v) < distance) {
						closest = false;
						break;
					}
				}

				if(closest) {
					++neutral_villages;
				}
			}
		}

		// The villages per scout is for a two-side battle,
		// accounting for all neutral villages on the map.
		// We only look at villages closer to us, so we halve it,
		// making us get twice as many scouts.
		const int villages_per_scout = get_villages_per_scout()/2;

		// Get scouts depending on how many neutral villages there are.
		int scouts_wanted = villages_per_scout > 0 ? neutral_villages/villages_per_scout : 0;

		LOG_AI_TESTING_AI_DEFAULT << "scouts_wanted: " << neutral_villages << "/"
			<< villages_per_scout << " = " << scouts_wanted << "\n";

		std::map<std::string,int> unit_types;

		for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
			if (u->side() == get_side()) {
				++unit_types[u->usage()];
			}
		}

		LOG_AI_TESTING_AI_DEFAULT << "we have " << unit_types["scout"] << " scouts already and we want "
			<< scouts_wanted << " in total\n";

		while(unit_types["scout"] < scouts_wanted) {
			if (!recruit_usage("scout")){
				break;
			}
			++unit_types["scout"];
		}
	}

	// If there is no recruitment_pattern use "" which makes us consider
	// any unit available.
	if (options.empty()) {
		options.push_back("");
	}
	// Buy units as long as we have room and can afford it.
	while (recruit_usage(options[rand()%options.size()])) {
		//refresh the recruitment pattern - it can be changed by recruit_usage
		options = get_recruitment_pattern();
		if (options.empty()) {
			options.push_back("");
		}
	}

}

bool recruitment_phase::recruit_usage(const std::string& usage)
{
	raise_user_interact();

	const int min_gold = 0;

	log_scope2(log_ai_testing_ai_default, "recruiting troops");
	LOG_AI_TESTING_AI_DEFAULT << "recruiting '" << usage << "'\n";

	//make sure id, usage and cost are known for the coming evaluation of unit types
	unit_types.build_all(unit_type::HELP_INDEXED);

	std::vector<std::string> options;
	bool found = false;
	// Find an available unit that can be recruited,
	// matches the desired usage type, and comes in under budget.
	BOOST_FOREACH(const std::string &name, current_team().recruits())
	{
		const unit_type *ut = unit_types.find(name);
		if (!ut) continue;
		// If usage is empty consider any unit.
		if (usage.empty() || ut->usage() == usage) {
			LOG_AI_TESTING_AI_DEFAULT << name << " considered for " << usage << " recruitment\n";
			found = true;

			if (current_team().gold() - ut->cost() < min_gold) {
				LOG_AI_TESTING_AI_DEFAULT << name << " rejected, cost too high (cost: " << ut->cost() << ", current gold: " << current_team().gold() <<", min_gold: " << min_gold << ")\n";
				continue;
			}

			if (not_recommended_units_.count(name))
			{
				LOG_AI_TESTING_AI_DEFAULT << name << " rejected, bad terrain or combat\n";
				continue;
			}

			LOG_AI_TESTING_AI_DEFAULT << "recommending '" << name << "'\n";
			options.push_back(name);
		}
	}

	// From the available options, choose one at random
	if(options.empty() == false) {
		const int option = rand()%options.size();
		recruit_result_ptr recruit_result = execute_recruit_action(options[option]);
		return recruit_result->is_ok();
	}
	if (found) {
		LOG_AI_TESTING_AI_DEFAULT << "No available units to recruit that come under the price.\n";
	} else if (usage != "")	{
		//FIXME: This message should be suppressed when WML author
		//chooses the default recruitment pattern.
		const std::string warning = "At difficulty level " +
			resources::gamedata->difficulty() + ", trying to recruit a:" +
			usage + " but no unit of that type (usage=) is"
			" available. Check the recruit and [ai]"
			" recruitment_pattern keys for team '" +
			current_team().name() + "' (" +
			lexical_cast<std::string>(get_side()) + ")"
			" against the usage key of the"
			" units in question! Removing invalid"
			" recruitment_pattern entry and continuing...\n";
		WRN_AI_TESTING_AI_DEFAULT << warning;
		// Uncommented until the recruitment limiting macro can be fixed to not trigger this warning.
		//lg::wml_error << warning;
		//@fixme
		//return current_team_w().remove_recruitment_pattern_entry(usage);
		return false;
	}
	return false;
}

int recruitment_phase::average_resistance_against(const unit_type& a, const unit_type& b) const
{
	int weighting_sum = 0, defense = 0;
	const std::map<t_translation::t_terrain, size_t>& terrain =
		resources::game_map->get_weighted_terrain_frequencies();

	for (std::map<t_translation::t_terrain, size_t>::const_iterator j = terrain.begin(),
	     j_end = terrain.end(); j != j_end; ++j)
	{
		// Use only reachable tiles when computing the average defense.
	  if (a.movement_type().movement_cost(j->first) < movetype::UNREACHABLE) {
			defense += a.movement_type().defense_modifier(j->first) * j->second;
			weighting_sum += j->second;
		}
	}

	if (weighting_sum == 0) {
		// This unit can't move on this map, so just get the average weighted
		// of all available terrains. This still is a kind of silly
		// since the opponent probably can't recruit this unit and it's a static unit.
		for (std::map<t_translation::t_terrain, size_t>::const_iterator jj = terrain.begin(),
				jj_end = terrain.end(); jj != jj_end; ++jj)
		{
			defense += a.movement_type().defense_modifier(jj->first) * jj->second;
			weighting_sum += jj->second;
		}
	}

	if(weighting_sum != 0) {
		defense /= weighting_sum;
	} else {
		ERR_AI_TESTING_AI_DEFAULT << "The weighting sum is 0 and is ignored.\n";
	}

	LOG_AI_TESTING_AI_DEFAULT << "average defense of '" << a.id() << "': " << defense << "\n";

	int sum = 0, weight_sum = 0;

	// calculation of the average damage taken
	bool steadfast = a.has_ability_by_id("steadfast");
	bool poisonable = !a.musthave_status("unpoisonable");
	const std::vector<attack_type>& attacks = b.attacks();
	for (std::vector<attack_type>::const_iterator i = attacks.begin(),
	     i_end = attacks.end(); i != i_end; ++i)
	{
		int resistance = a.movement_type().resistance_against(*i);
		// Apply steadfast resistance modifier.
		if (steadfast && resistance < 100)
			resistance = std::max<int>(resistance * 2 - 100, 50);
		// Do not look for filters or values, simply assume 70% if CTH is customized.
		int cth = i->get_special_bool("chance_to_hit", true) ? 70 : defense;
		int weight = i->damage() * i->num_attacks();
		// if cth == 0 the division will do 0/0 so don't execute this part
		if (poisonable && cth != 0 && i->get_special_bool("poison", true)) {
			// Compute the probability of not poisoning the unit.
			int prob = 100;
			for (int j = 0; j < i->num_attacks(); ++j)
				prob = prob * (100 - cth);
			// Assume poison works one turn.
			weight += game_config::poison_amount * (100 - prob) / 100;
		}
		sum += cth * resistance * weight * weight; // average damage * weight
		weight_sum += weight;
	}

	// normalize by HP
	sum /= std::max<int>(1,std::min<int>(a.hitpoints(),1000)); // avoid values really out of range

	// Catch division by zero here if the attacking unit
	// has zero attacks and/or zero damage.
	// If it has no attack at all, the ai shouldn't prefer
	// that unit anyway.
	if (weight_sum == 0) {
		return sum;
	}
	return sum/weight_sum;
}

int recruitment_phase::compare_unit_types(const unit_type& a, const unit_type& b) const
{
	const int a_effectiveness_vs_b = average_resistance_against(b,a);
	const int b_effectiveness_vs_a = average_resistance_against(a,b);

	LOG_AI_TESTING_AI_DEFAULT << "comparison of '" << a.id() << " vs " << b.id() << ": "
		<< a_effectiveness_vs_b << " - " << b_effectiveness_vs_a << " = "
		<< (a_effectiveness_vs_b - b_effectiveness_vs_a) << '\n';
	return a_effectiveness_vs_b - b_effectiveness_vs_a;
}

void recruitment_phase::analyze_potential_recruit_combat()
{
	unit_map &units_ = *resources::units;
	if(unit_combat_scores_.empty() == false || get_recruitment_ignore_bad_combat()) {
		return;
	}

	log_scope2(log_ai_testing_ai_default, "analyze_potential_recruit_combat()");

	// Records the best combat analysis for each usage type.
	std::map<std::string,int> best_usage;

	const std::set<std::string>& recruits = current_team().recruits();
	std::set<std::string>::const_iterator i;
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const unit_type *info = unit_types.find(*i);
		if (!info || not_recommended_units_.count(*i)) {
			continue;
		}

		int score = 0, weighting = 0;

		for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {
			if (j->can_recruit() || !current_team().is_enemy(j->side())) {
				continue;
			}

			unit const &un = *j;

			int weight = un.cost() * un.hitpoints() / un.max_hitpoints();
			weighting += weight;
			score += compare_unit_types(*info, un.type()) * weight;
		}

		if(weighting != 0) {
			score /= weighting;
		}

		LOG_AI_TESTING_AI_DEFAULT << "combat score of '" << *i << "': " << score << "\n";
		unit_combat_scores_[*i] = score;

		if (best_usage.count(info->usage()) == 0 ||
				score > best_usage[info->usage()]) {
			best_usage[info->usage()] = score;
		}
	}

	// Recommend not to use units of a certain usage type
	// if they have a score more than 600 below
	// the best unit of that usage type.
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const unit_type *info = unit_types.find(*i);
		if (!info || not_recommended_units_.count(*i)) {
			continue;
		}

		if (unit_combat_scores_[*i] + 600 < best_usage[info->usage()]) {
			LOG_AI_TESTING_AI_DEFAULT << "recommending not to use '" << *i << "' because of poor combat performance "
				      << unit_combat_scores_[*i] << "/" << best_usage[info->usage()] << "\n";
			not_recommended_units_.insert(*i);
		}
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

	const unit_map::iterator leader = resources::units->find_leader(get_side());
	if (!leader.valid() || leader->incapacitated()) {
		WRN_AI_TESTING_AI_DEFAULT << "Leader not found\n";
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

	pathfind::shortest_path_calculator calc(*leader, current_team(), *resources::teams, *resources::game_map);
	pathfind::plain_route route = a_star_search(leader->get_location(), dst_, 1000.0, &calc,
			resources::game_map->w(), resources::game_map->h());
	if(route.steps.empty()) {
		LOG_AI_TESTING_AI_DEFAULT << "route empty";
		return BAD_SCORE;
	}

	const pathfind::paths leader_paths(*leader, false, true, current_team());

	std::map<map_location,pathfind::paths> possible_moves;
	possible_moves.insert(std::pair<map_location,pathfind::paths>(leader->get_location(), leader_paths));

	map_location loc;
	BOOST_FOREACH(const map_location &l, route.steps)
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
	manager::modify_active_ai_for_side(get_side(),mod_ai);
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
	const unit_map &units_ = *resources::units;
	const std::vector<unit_map::const_iterator> leaders = units_.find_leaders(get_side());
	if (leaders.empty()) {
		return BAD_SCORE;
	}

	// 2. + 3.
	const unit* best_leader = NULL;
	map_location best_keep;
	int shortest_distance = 99999;

	BOOST_FOREACH(const unit_map::const_iterator& leader, leaders) {
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
		if (keep == map_location::null_location || keep == leader->get_location()) {
			continue;
		}

		const pathfind::shortest_path_calculator calc(*leader, current_team(), *resources::teams, *resources::game_map);

		const pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(*leader, current_team());

		pathfind::plain_route route;
		route = pathfind::a_star_search(leader->get_location(), keep, 10000.0, &calc, resources::game_map->w(), resources::game_map->h(), &allowed_teleports);

		if (!route.steps.empty() || route.move_cost < shortest_distance) {
			best_leader = &(*leader);
			best_keep = keep;
			shortest_distance = route.move_cost;
		}
	}

	if (best_leader == NULL) {
		return BAD_SCORE;
	}

	// 4.
	const unit* leader = best_leader;
	const map_location keep = best_keep;
	const pathfind::paths leader_paths(*leader, false, true, current_team());
	const pathfind::shortest_path_calculator calc(*leader, current_team(), *resources::teams, *resources::game_map);
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
	route = pathfind::a_star_search(leader->get_location(), keep, 10000.0, &calc, resources::game_map->w(), resources::game_map->h(), &allowed_teleports);

	// find next hop
	map_location next_hop = map_location::null_location;
	int next_hop_cost = 0;
	BOOST_FOREACH(const map_location& step, route.steps) {
		if (leader_paths.destinations.contains(step)) {
			next_hop = step;
			next_hop_cost += leader->movement_cost(resources::game_map->get_terrain(step));
		} else {
			break;
		}
	}
	if (next_hop == map_location::null_location) {
		return BAD_SCORE;
	}
	//define the next hop to have the lowest cost (0)
	moves_toward_keep.insert(std::make_pair(0, next_hop));

	BOOST_FOREACH(const pathfind::paths::step &dest, leader_paths.destinations) {
		if (!units_.find(dest.curr).valid()) {
			route = pathfind::a_star_search(dest.curr, next_hop, 10000.0, &calc,
					resources::game_map->w(), resources::game_map->h(), &allowed_teleports);
			if (route.move_cost < next_hop_cost) {
				moves_toward_keep.insert(std::make_pair(route.move_cost, dest.curr));
			}
		}
	}

	// Find the first location which we can move to,
	// without the threat of enemies.
	BOOST_FOREACH(const ordered_locations::value_type& pair, moves_toward_keep) {
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
	unit_map::const_iterator leader = resources::units->find_leader(get_side());
	get_villages(get_dstsrc(),get_enemy_dstsrc(),leader);
	if (!moves_.empty()) {
		return get_score();
	}
	return BAD_SCORE;
}


void get_villages_phase::execute()
{
	unit_map &units_ = *resources::units;
	unit_map::const_iterator leader = units_.find_leader(get_side());
	// Move all the units to get villages, however move the leader last,
	// so that the castle will be cleared if it wants to stop to recruit along the way.
	std::pair<map_location,map_location> leader_move;

	for(tmoves::const_iterator i = moves_.begin(); i != moves_.end(); ++i) {

		if(leader != units_.end() && leader->get_location() == i->second) {
			leader_move = *i;
		} else {
			if (find_visible_unit(i->first, current_team()) == units_.end()) {
				move_result_ptr move_res = execute_move_action(i->second,i->first,true);
				if (!move_res->is_ok()) {
					return;
				}

				const map_location loc = move_res->get_unit_location();
				leader = units_.find_leader(get_side());
				const unit_map::const_iterator new_unit = units_.find(loc);

				if (new_unit != units_.end() &&
				    power_projection(i->first, get_enemy_dstsrc()) >= new_unit->hitpoints() / 4)
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
		if((find_visible_unit(leader_move.first , current_team()) == units_.end())
		   && resources::game_map->is_village(leader_move.first)) {
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
	unit_map &units_ = *resources::units;
	const int ticks = SDL_GetTicks();
	best_leader_loc_ = map_location::null_location;
	if(leader != units_.end()) {
		keep_loc_ = nearest_keep(leader->get_location());
		leader_loc_ = leader->get_location();
	} else {
		keep_loc_ = map_location::null_location;
		leader_loc_ = map_location::null_location;
	}

	debug_ = !lg::debug.dont_log(log_ai_testing_ai_default);

	// Find our units who can move.
	treachmap reachmap;
	for(unit_map::const_iterator u_itor = units_.begin();
			u_itor != units_.end(); ++u_itor) {
		if(u_itor->can_recruit() && get_passive_leader()){
			continue;
		}
		if(u_itor->side() == get_side() && u_itor->movement_left()) {
			reachmap.insert(std::make_pair(u_itor->get_location(),	std::vector<map_location>()));
		}
	}


	DBG_AI_TESTING_AI_DEFAULT << reachmap.size() << " units found who can try to capture a village.\n";

	find_villages(reachmap, moves_, dstsrc, enemy_dstsrc);

	treachmap::iterator itor = reachmap.begin();
	while(itor != reachmap.end()) {
		if(itor->second.size() == 0) {
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
	gamemap &map_ = *resources::game_map;
	std::vector<team> &teams_ = *resources::teams;

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

		double threat = 0.0;
		const std::map<map_location,double>::const_iterator vuln = vulnerability.find(current_loc);
		if(vuln != vulnerability.end()) {
			threat = vuln->second;
		} else {
			threat = power_projection(current_loc,enemy_dstsrc);
			vulnerability.insert(std::pair<map_location,double>(current_loc,threat));
		}

		const unit_map::const_iterator u = resources::units->find(j->second);
		if (u == resources::units->end() || u->get_state("guardian")) {
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
		moves.insert(moves.end(), best_result.begin(), best_result.end());

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
	: candidate_action(context,cfg),move_()
{
}

get_healing_phase::~get_healing_phase()
{
}

double get_healing_phase::evaluate()
{
	// Find units in need of healing.
	unit_map &units_ = *resources::units;
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
		    !u.get_ability_bool("regenerate"))
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
				if (resources::game_map->gives_healing(dst) && (units_.find(dst) == units_.end() || dst == u_it->get_location())) {
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
	const unit_map& units_ = *resources::units;

	//unit_map::const_iterator leader = units_.find_leader(get_side());
	std::vector<unit_map::const_iterator> leaders = units_.find_leaders(get_side());
	std::map<map_location,pathfind::paths> dummy_possible_moves;

	move_map fullmove_srcdst;
	move_map fullmove_dstsrc;
	calculate_possible_moves(dummy_possible_moves, fullmove_srcdst, fullmove_dstsrc,
			false, true, &get_avoid());

	/*map_location leader_adj[6];
	if(leader != units_.end()) {
		get_adjacent_tiles(leader->get_location(), leader_adj);
	}*/
	//int leader_adj_count = 0;
	std::vector<map_location> leaders_adj_v;
	BOOST_FOREACH(unit_map::const_iterator leader, leaders){
		map_location tmp_leader_adj[6];
		get_adjacent_tiles(leader->get_location(), tmp_leader_adj);
		BOOST_FOREACH(map_location &loc, tmp_leader_adj){
			bool found = false;
			BOOST_FOREACH(map_location &new_loc, leaders_adj_v){
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
				int best_defensive_rating = i->defense_modifier(resources::game_map->get_terrain(i->get_location()))
					- (resources::game_map->is_village(i->get_location()) ? 10 : 0);
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
					const int defense = i->defense_modifier(resources::game_map->get_terrain(hex));
					const double our_power = power_projection(hex,get_dstsrc());
					const double their_power = power_projection(hex,get_enemy_dstsrc()) * double(defense)/100.0;
					const double rating = our_power - their_power;
					if(rating > best_rating) {
						best_pos = hex;
						best_rating = rating;
					}

					// Give a bonus for getting to a village.
					const int modified_defense = defense - (resources::game_map->is_village(hex) ? 10 : 0);

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
		un->defense_modifier(resources::game_map->get_terrain(loc)) / 100.0;

	// The 'exposure' is the additional % chance to hit
	// this unit receives from being on a sub-optimal defensive terrain.
	const double exposure = proposed_terrain - optimal_terrain;

	const double our_power = power_projection(loc,dstsrc);
	const double their_power = power_projection(loc,enemy_dstsrc);
	return caution*their_power*(1.0+exposure) > our_power;
}


//==============================================================

simple_move_and_targeting_phase::simple_move_and_targeting_phase(
		rca_context &context, const config &cfg)
	: candidate_action(context,cfg)
	, move_()
{
}

simple_move_and_targeting_phase::~simple_move_and_targeting_phase()
{
}

double simple_move_and_targeting_phase::evaluate()
{
    // Pick first enemy leader, move all units as close to 
    // enemy leader as possible.
    // Own leader should not be moved.
    // Code does not support multiple leaders per side.
    // Do it fast even if there is a lot of units present.
	unit_map &units_ = *resources::units;

	unit_map::const_iterator leader = units_.find_leader(get_side());
	map_location my_leader_loc = map_location::null_location;
	if (leader.valid()) {
		my_leader_loc = leader->get_location();
	}

	for(leader = units_.begin(); leader != units_.end(); ++leader) {
		if(leader->can_recruit() && current_team().is_enemy(leader->side())) {
			break;
		}
	}

	if(leader == units_.end()) {
		return BAD_SCORE;
	}

	int closest_distance = -1;
	std::pair<map_location,map_location> closest_move;

	for(move_map::const_iterator i = get_dstsrc().begin(); i != get_dstsrc().end(); ++i) {
		if(my_leader_loc == i->second){// Do not move leaders.
			continue;
		}
		const int distance = distance_between(i->first, leader->get_location());
		if(closest_distance == -1 || distance < closest_distance) {
			closest_distance = distance;
			closest_move = *i;
		}
	}

	if(closest_distance != -1) {
		move_ = check_move_action(closest_move.second,closest_move.first,true);
		if (move_->is_ok()){
			return get_score();
		}
	}

	return BAD_SCORE;
}

void simple_move_and_targeting_phase::execute()
{
	move_->execute();
	if (!move_->is_ok()){
		LOG_AI_TESTING_AI_DEFAULT << get_name() << "::execute not ok" << std::endl;
	}
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
	BOOST_FOREACH(team &tmp_team, *resources::teams){
		if(!current_team().is_enemy(tmp_team.side())){
			std::vector<unit_map::unit_iterator> allied_leaders = resources::units->find_leaders(get_side());
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
	std::vector<unit_map::unit_iterator> ai_leaders = resources::units->find_leaders(get_side());

	//calculate all possible moves (AI + allies)
	typedef std::map<map_location, pathfind::paths> path_map;
	path_map possible_moves;
	move_map friends_srcdst, friends_dstsrc;
	calculate_moves(*resources::units, possible_moves, friends_srcdst, friends_dstsrc, false, true);

	//check for each ai leader if he should move away from his keep
	BOOST_FOREACH(unit_map::unit_iterator &ai_leader, ai_leaders){
		//only if leader is on a keep
		const map_location &keep = ai_leader->get_location();
		if ( !resources::game_map->is_keep(keep) ) {
			continue;
		}
		map_location recruit_loc = pathfind::find_vacant_castle(*ai_leader);
		if(!resources::game_map->on_board(recruit_loc)){
			continue;
		}
		bool friend_can_reach_keep = false;

		//for each leader, check if he's allied and can reach our keep
		for(path_map::const_iterator i = possible_moves.begin(); i != possible_moves.end(); ++i){
			const unit_map::const_iterator itor = resources::units->find(i->first);
			team &leader_team = (*resources::teams)[itor->side() - 1];
			if(itor != resources::units->end() && itor->can_recruit() && itor->side() != get_side() && (leader_team.total_income() + leader_team.gold() > leader_team.minimum_recruit_price())){
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

					int tmp_def_mod = ai_leader->defense_modifier(resources::game_map->get_terrain(i->curr));
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
	BOOST_FOREACH(unit_map::unit_iterator &leader, ai_leaders){
		leader->remove_movement_ai();
	}
	//ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented" << std::endl;
}


//==============================================================


} //end of namespace testing_ai_default

} //end of namespace ai
