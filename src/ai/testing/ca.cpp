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
#include "../../global.hpp"

#include "ca.hpp"
#include "../composite/engine.hpp"
#include "../composite/rca.hpp"
#include "../../log.hpp"

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
	map_location from   = best_analysis_.movements[0].first;
	map_location to     = best_analysis_.movements[0].second;
	map_location target_loc = best_analysis_.target;

	if (from!=to) {
		move_result_ptr move_res = execute_move_action(from,to,false);
		if (!move_res->is_ok()) {
			if (!move_res->is_gamestate_changed()) {
				return false;
			}
			return true;
		}
	}

	attack_result_ptr attack_res = execute_attack_action(to, target_loc, -1);
	if (!attack_res->is_ok()) {
		if (!attack_res->is_gamestate_changed()) {
			return false;
		}
		return true;
	}

	return true;
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

get_villages_phase::get_villages_phase( rca_context &context, const config &cfg )
	: candidate_action(context,"testing_ai_default::get_villages_phase",cfg["type"])
{
}

get_villages_phase::~get_villages_phase()
{
}

double get_villages_phase::evaluate()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": evaluate - not yet implemented!" << std::endl;
	return BAD_SCORE;
}

bool get_villages_phase::execute()
{
	ERR_AI_TESTING_AI_DEFAULT << get_name() << ": execute - not yet implemented!" << std::endl;
	return true;
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
