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
 * Candidate actions evaluator
 * @file ai/testing/stage_rca.cpp
 */

#include "stage_rca.hpp"

#include "../composite/ai.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"

namespace testing_ai_default {

static lg::log_domain log_ai_testing_rca_default("ai/testing/rca_default");
#define DBG_AI_TESTING_RCA_DEFAULT LOG_STREAM(debug, log_ai_testing_rca_default)
#define LOG_AI_TESTING_RCA_DEFAULT LOG_STREAM(info, log_ai_testing_rca_default)
#define ERR_AI_TESTING_RCA_DEFAULT LOG_STREAM(err, log_ai_testing_rca_default)

candidate_action_evaluation_loop::candidate_action_evaluation_loop( ai::composite_ai::composite_ai_context &context, const config &cfg)
	: stage(context,cfg),cfg_(cfg)
{
}

void candidate_action_evaluation_loop::on_create()
{
	//init the candidate actions
	foreach(const config &cfg_element, cfg_.child_range("candidate_action")){
		ai::composite_ai::engine::parse_candidate_action_from_config(*this,cfg_element,back_inserter(candidate_actions_));
	}
}

void candidate_action_evaluation_loop::do_play_stage()
{
	LOG_AI_TESTING_RCA_DEFAULT << "Starting candidate action evaluation loop for side "<< get_side() << std::endl;
	const static double STOP_VALUE = 0;

	foreach(ai::composite_ai::candidate_action_ptr ca, candidate_actions_){
		ca->enable();
	}

	bool executed = false;
	do {
		executed = false;
		double best_score = ai::composite_ai::candidate_action::BAD_SCORE;
		ai::composite_ai::candidate_action_ptr best_ptr;

		//Evaluation
		foreach(ai::composite_ai::candidate_action_ptr ca_ptr, candidate_actions_){
			if (!ca_ptr->is_enabled()){
				continue;
			}

			double score = STOP_VALUE;
			try {
				DBG_AI_TESTING_RCA_DEFAULT << "Evaluating candidate action: "<< *ca_ptr << std::endl;
				score = ca_ptr->evaluate();
				DBG_AI_TESTING_RCA_DEFAULT << "Evaluated candidate action to score "<< score << " : " << *ca_ptr << std::endl;
			} catch (ai::composite_ai::candidate_action_evaluation_exception &caee) {
				ERR_AI_TESTING_RCA_DEFAULT << "Candidate action evaluation threw an exception: " << caee << std::endl;
				ca_ptr->disable();
				continue;
			}

			if (score>best_score) {
				best_score = score;
				best_ptr = ca_ptr;
			}
		}

		//Execution
		if (best_score>ai::composite_ai::candidate_action::BAD_SCORE) {
			try {
				DBG_AI_TESTING_RCA_DEFAULT << "Best candidate action: "<< *best_ptr << std::endl;
				executed = best_ptr->execute();
			} catch (ai::composite_ai::candidate_action_execution_exception &caee) {
				ERR_AI_TESTING_RCA_DEFAULT << "Candidate action execution threw an exception: " << caee << std::endl;
				executed = false;
			}
			if (!executed) {
				//this means that this CA has lied to us in evaluate()
				//we punish it by disabling it
				best_ptr->disable();
				//since we don't re-enable at this play_stage, if we disable this CA, other may get the change to go.
				executed = true;
			}
		} else {
			LOG_AI_TESTING_RCA_DEFAULT << "Ending candidate action evaluation loop due to best score "<< best_score<<"<="<< ai::composite_ai::candidate_action::BAD_SCORE<<std::endl;
		}
	} while (executed);
	LOG_AI_TESTING_RCA_DEFAULT << "Ended candidate action evaluation loop for side "<< get_side() << std::endl;
}

ai::composite_ai::rca_context& candidate_action_evaluation_loop::get_rca_context()
{
	return *this;
}

candidate_action_evaluation_loop::~candidate_action_evaluation_loop()
{
}

} // of namespace testing_ai_default
