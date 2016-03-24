/*
   Copyright (C) 2014 - 2016 by Guorui Xi <kevin.xgr@gmail.com>
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
 * Strategy formulation with candidate action evaluator
 * @file
 * See http://wiki.wesnoth.org/AI_sfrca
 */

#include "ai/testing/stage_sf_with_rca.hpp"

#include "ai/manager.hpp"
#include "ai/composite/ai.hpp"
#include "ai/composite/engine.hpp"
#include "ai/composite/property_handler.hpp"
#include "ai/gamestate_observer.hpp"
#include "actions/attack.hpp"
#include "actions/heal.hpp"
#include "config.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "tod_manager.hpp"
#include "team.hpp"
#include "units/unit.hpp"

#include <boost/bind.hpp>
#include <SDL.h>

namespace ai {

namespace testing_ai_default {

static lg::log_domain log_ai_testing_sf_with_rca("ai/stage/sf_with_rca");
#define DBG_AI_TESTING_SF_WITH_RCA LOG_STREAM(debug, log_ai_testing_sf_with_rca)
#define LOG_AI_TESTING_SF_WITH_RCA LOG_STREAM(info, log_ai_testing_sf_with_rca)
#define ERR_AI_TESTING_SF_WITH_RCA LOG_STREAM(err, log_ai_testing_sf_with_rca)

using ai_default_rca::candidate_action_evaluation_loop;

strategy_formulation_with_rca::strategy_formulation_with_rca(ai_context &context, const config &cfg)
	: stage(context,cfg)
	, rca_(boost::shared_ptr<candidate_action_evaluation_loop>(new candidate_action_evaluation_loop(context, cfg)))
{
}

void strategy_formulation_with_rca::on_create()
{
	const std::vector<team> teams = *resources::teams;
	const int own_side = this->get_side();
	const team own_team = teams[own_side-1];

	rca_->on_create();

	for(size_t i = 0; i != teams.size(); ++i){
		if(own_side+i <= teams.size()){
			enemy_this_turn_.push_back(own_team.is_enemy(own_side+i));
		} else {
			enemy_this_turn_.push_back(own_team.is_enemy((own_side+i)%teams.size()));
		}
	}

	enemy_this_turn_.push_back(false);
}

config strategy_formulation_with_rca::to_config() const
{
	config cfg = stage::to_config();
	cfg.append_children(rca_->to_config());
	return cfg;
}


bool strategy_formulation_with_rca::do_play_stage()
{
	// Make general "offense" or "defense" decision for current turn.
	LOG_AI_TESTING_SF_WITH_RCA << "------Analyze started------" << std::endl;

	int ticks = SDL_GetTicks();

	DBG_AI_TESTING_SF_WITH_RCA << "Clear strategy flag at the beginning of do_play_stage()" << std::endl;
	clear_strategy();

	// Find optimal strategy.
	// First calculate the state of current turn.
	boost::shared_ptr<turn_state> current_state(new turn_state(this->get_side(), 0, *resources::units, *resources::teams));
	states_.push(*current_state);

	// Second calculate decisions based on current state.
	simulate_states_ahead();

	// Finally figure out the optimal strategy for the total
	// three turns and set the flag for CA to use.
	set_optimal_strategy();

	// Clean the queue.
	while(!states_.empty())
		states_.pop();

	int time_taken = SDL_GetTicks() - ticks;
	LOG_AI_TESTING_SF_WITH_RCA <<"Took " << time_taken <<" ticks on decision making." << std::endl;

	LOG_AI_TESTING_SF_WITH_RCA << "------Analyze completed------\n" << std::endl;

	rca_->do_play_stage();

	clear_strategy();

	return false;
}

rca_context& strategy_formulation_with_rca::get_rca_context()
{
	return *this;
}

void strategy_formulation_with_rca::simulate_states_ahead()
{
	DBG_AI_TESTING_SF_WITH_RCA << "------simulate_states_ahead() begin------" << std::endl;
	// Simulate until next time own side take control. For example,
	// if 3 teams totally and we are side 2, the simulation sequence
	// is side 2->3->1->2.
	const int total_turns = resources::teams->size() + 1;
	int current_turn = 0;
	int last_turn = 0;

	// Store the current data structures that would be modified during simulation.
	const unit_map units_stored_ = *resources::units;
	const std::vector<team> teams_stored_ = *resources::teams;
	const tod_manager tod_stored_ = *resources::tod_manager;

	DBG_AI_TESTING_SF_WITH_RCA << "Set simulation_ flag" << std::endl;
	resources::simulation_ = true;

	// Pop one state from queue and calculate resulted states
	// of each decision, push them back to the queue.
	while(current_turn < total_turns){
		if(current_turn != last_turn){
			switch_side();
		}

		last_turn = current_turn;

		const turn_state current_state = states_.front();
		states_.pop();

		for(int i = 0; i != decision::total_decisions; ++i){
			states_.push(simulate_state(i, current_state));
		}

		current_turn = states_.front().get_turn_no();
	}

	// Restore the current data structures.
	*resources::units = units_stored_;
	*resources::teams = teams_stored_;
	*resources::tod_manager = tod_stored_;

	DBG_AI_TESTING_SF_WITH_RCA << "Clear simulation_ flag" << std::endl;
	resources::simulation_ = false;
	DBG_AI_TESTING_SF_WITH_RCA << "------simulate_states_ahead() end------" << std::endl;
}


void strategy_formulation_with_rca::set_optimal_strategy()
{
	DBG_AI_TESTING_SF_WITH_RCA << "------set_optimal_strategy() begin------" << std::endl;
	// Find the optimal final state with maxmin method.

	int turn_num = resources::teams->size()+1;

	while(turn_num != 0){
		DBG_AI_TESTING_SF_WITH_RCA << "In turn " << turn_num << std::endl;
		int group_num = 0;
		int total_group_num = states_.size()/2;
		while(group_num != total_group_num){
			DBG_AI_TESTING_SF_WITH_RCA << "\tIn group " << group_num << std::endl;

			turn_state attack_state = states_.front();
			states_.pop();
			DBG_AI_TESTING_SF_WITH_RCA << "\t\tAttack scoring" << std::endl;
			attack_state.scoring_state();
			double attack_score = attack_state.get_state_score();

			turn_state defend_state = states_.front();
			states_.pop();
			DBG_AI_TESTING_SF_WITH_RCA << "\t\tDefend scoring" << std::endl;
			defend_state.scoring_state();
			double defend_score = defend_state.get_state_score();

			DBG_AI_TESTING_SF_WITH_RCA << "\t\tattack vs defend: "
				<< attack_score << " vs " << defend_score << std::endl;
			if(!enemy_this_turn_[turn_num-1]){
				if(attack_score > defend_score){
					states_.push(attack_state);
					DBG_AI_TESTING_SF_WITH_RCA << "\t\tchoose attack for us" <<std::endl;
				} else {
					states_.push(defend_state);
					DBG_AI_TESTING_SF_WITH_RCA << "\t\tchoose defend for us" <<std::endl;
				}
			} else {
				if(attack_score < defend_score){
					states_.push(attack_state);
					DBG_AI_TESTING_SF_WITH_RCA << "\t\tchoose attack for enemy" <<std::endl;
				} else {
					states_.push(defend_state);
					DBG_AI_TESTING_SF_WITH_RCA << "\t\tchoose defend for enemy" <<std::endl;
				}
			}

			DBG_AI_TESTING_SF_WITH_RCA << "\tGroup " << group_num << " end" << std::endl;
			++group_num;
		}

		DBG_AI_TESTING_SF_WITH_RCA << "Turn " << turn_num << " end" << std::endl;
		--turn_num;
	}

	turn_state optimal_final_state = states_.front();
	decision optimal_decision = optimal_final_state.get_decision();
	LOG_AI_TESTING_SF_WITH_RCA << optimal_decision << std::endl;

	optimal_decision.get_decision_no() == 0 ? set_offense() : set_defense();
	DBG_AI_TESTING_SF_WITH_RCA << "------set_optimal_strategy() end------" << std::endl;
}

const turn_state strategy_formulation_with_rca::simulate_state(int decision_no_, const turn_state &state)
{
	DBG_AI_TESTING_SF_WITH_RCA << "------simulate_state() begin------" << std::endl;
	// Simulate CA based on state and get resulted turn_state.

	const unit_map &units_ = state.get_units();
	const std::vector<team> &teams_ = state.get_teams();

	// Simulate.
	// First set current data structure according to previous simulation.
	*resources::units = units_;
	*resources::teams = teams_;
	init_side();

	// Second run RCA based on current decision on current data structures.
	DBG_AI_TESTING_SF_WITH_RCA << "Set strategy flag: " << decision_no_ << std::endl;
	decision_no_ == 0 ? set_offense() : set_defense();
	rca_->do_play_stage();

	// Third return the data structures after simulation.
	boost::shared_ptr<turn_state> state_next(new turn_state(this->get_side(), state.get_turn_no()+1, *resources::units, *resources::teams));
	const decision &decision_ = state.get_decision();
	if(decision_.is_valid()){
		state_next->set_decision(decision_.get_decision_no());
	} else {
		state_next->set_decision(decision_no_);
	}

	DBG_AI_TESTING_SF_WITH_RCA << "Clear strategy flag" << std::endl;
	clear_strategy();

	DBG_AI_TESTING_SF_WITH_RCA << "------simulate_state() end------" << std::endl;
	return *state_next;
}

void strategy_formulation_with_rca::switch_side()
{
	DBG_AI_TESTING_SF_WITH_RCA << "------switch_side() begin------" << std::endl;
	size_t current_side = this->get_side();
	if(current_side+1 <= resources::teams->size())
		this->set_side(current_side+1);
	else
		this->set_side(1);

	resources::tod_manager->next_turn(resources::gamedata);

	DBG_AI_TESTING_SF_WITH_RCA << "switch to turn " << resources::tod_manager->turn() << std::endl;
	DBG_AI_TESTING_SF_WITH_RCA << "------switch_side() end------" << std::endl;
}

void strategy_formulation_with_rca::init_side()
{
	recalculate_move_maps();
	invalidate_defensive_position_cache();
	invalidate_keeps_cache();
	clear_additional_targets();
	unit_stats_cache().clear();

	DBG_AI_TESTING_SF_WITH_RCA << "------init_side() begin------" << std::endl;
	unit_map& units = *resources::units;
	for(unit_map::unit_iterator ui = units.begin(); ui != units.end(); ++ui){
		if(ui->side() == this->get_side()){
			ui->new_turn();
		}
	}
	(*resources::teams)[this->get_side()-1].new_turn();
	calculate_healing(this->get_side(), false);

	DBG_AI_TESTING_SF_WITH_RCA << "------init_side() end------" << std::endl;
}

strategy_formulation_with_rca::~strategy_formulation_with_rca()
{
}


// Implement class turn_state.
turn_state::turn_state(const int own_side_, const int turn_no_, const unit_map &units_, const std::vector<team> &teams_)
	: own_side_(own_side_)
	, turn_no_(turn_no_)
	, state_score_(0.0)
	, units_(units_)
	, teams_(teams_)
	, decision_(-1)
{
}

turn_state::~turn_state()
{
}

void turn_state::scoring_state(){
	DBG_AI_TESTING_SF_WITH_RCA << "\t\t------turn_state::scoring_state() begin------" << std::endl;
	// A very simple scoring function.

	state_score_ = 0.0;

	const gamemap &map = resources::gameboard->map();

	const int total_team = teams_.size();
	std::vector<double> state(total_team, 0.0);
	std::vector<int> total_level(total_team, 0);

	// Sum up the units' score, with simple HP, EXP and terrain weight.
	for(unit_map::const_unit_iterator ui = units_.begin(); ui != units_.end(); ++ui) {
		int current_side = ui->side() - 1;

		double c = static_cast<double>(ui->cost());
		double h = static_cast<double>(ui->hitpoints());
		double mh = static_cast<double>(ui->max_hitpoints());
		double e = static_cast<double>(ui->experience());
		double me = static_cast<double>(ui->max_experience());
		double def = static_cast<double>(ui->defense_modifier(map.get_terrain(ui->get_location()))/100.0);
		double unit_score = c * (h*h)/(mh*mh) * (1+(e*e)/(me*me)) * def;
		LOG_AI_TESTING_SF_WITH_RCA << "\t\t\tside " << current_side+1 << " unit " << ui->type_name() << "'s score is " << unit_score << std::endl;
		state[current_side] += unit_score;
		total_level[current_side] += ui->level();
	}


	const int total_turns = 3; // Calculate 3 turns ahead.
	std::vector<int> upkeep_per_turn(total_team, 0);
	std::vector<int> income_per_turn(total_team, 0);
	std::vector<double> gold(total_team, 0.0);

	// Add the gold that current have and that intend to
	// get as income in future totally(subtract the upkeep),
	// for each team.
	for(std::vector<team>::const_iterator ti = teams_.begin(); ti != teams_.end(); ++ti) {
		int current_side = ti->side() - 1;
		upkeep_per_turn[current_side] = total_level[current_side]>ti->support() ? total_level[current_side]-ti->support() : 0;
		income_per_turn[current_side] = ti->total_income() - upkeep_per_turn[current_side];

		// The further, the rougher. So drop 0.3 weight for each turn.
		double cg = static_cast<double>(ti->gold());
		double i = static_cast<double>(income_per_turn[current_side]);
		double weight = (1.0+(1.0-0.3*(total_turns-1.0)))*total_turns/2.0;
		gold[current_side] = cg + i * weight;
		state[current_side] += gold[current_side];

		LOG_AI_TESTING_SF_WITH_RCA << "\t\t\tside " << current_side+1 << " will totally get " << gold[current_side] << " gold." << std::endl;
	}


	for(std::vector<team>::const_iterator ti = teams_.begin(); ti != teams_.end(); ++ti) {
		int current_side = ti->side() - 1;
		LOG_AI_TESTING_SF_WITH_RCA << "\t\t\tside " << current_side+1 << "'s state: " << state[current_side] << std::endl;
		if(ti->is_enemy(own_side_)){ // 'is_enemy()' based on 1.
			state_score_ -= state[current_side];
		} else {
			state_score_ += state[current_side];
		}
	}

	LOG_AI_TESTING_SF_WITH_RCA << "\t\tState constructed with score " << state_score_ << std::endl;
	DBG_AI_TESTING_SF_WITH_RCA << "\t\t------turn_state::scoring_state() end------" << std::endl;
}

// Implement class decision and overload operator<<.
std::ostream& operator<<(std::ostream &output, const decision &decision_)
{
	std::string strategy = "";

	switch(decision_.decision_no_){
		case 0:
			strategy = "offensively";
			break;
		case 1:
			strategy = "defensively";
			break;
		default:
			strategy = "unknown";
			break;
	}

	output << "The global optimal strategy of this turn is to play "
		<< strategy;

	return output;
}

} // end of namespace testing_ai_default

} // end of namespace ai
