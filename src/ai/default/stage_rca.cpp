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
 * Candidate actions evaluator
 * @file
 */

#include "ai/default/stage_rca.hpp"

#include "ai/manager.hpp"
#include "ai/composite/ai.hpp"
#include "ai/composite/engine.hpp"
#include "ai/composite/property_handler.hpp"
#include "ai/composite/rca.hpp"
#include "ai/gamestate_observer.hpp"
#include "log.hpp"

#include "utils/functional.hpp"

namespace ai {

namespace ai_default_rca {

static lg::log_domain log_ai_testing_rca_default("ai/stage/rca");
#define DBG_AI_TESTING_RCA_DEFAULT LOG_STREAM(debug, log_ai_testing_rca_default)
#define LOG_AI_TESTING_RCA_DEFAULT LOG_STREAM(info, log_ai_testing_rca_default)
#define ERR_AI_TESTING_RCA_DEFAULT LOG_STREAM(err, log_ai_testing_rca_default)

candidate_action_evaluation_loop::candidate_action_evaluation_loop( ai_context &context, const config &cfg)
	: stage(context,cfg)
	, candidate_actions_()
	, cfg_(cfg)
{
}

void candidate_action_evaluation_loop::on_create()
{
	//init the candidate actions
	for (const config &cfg_element : cfg_.child_range("candidate_action")) {
		engine::parse_candidate_action_from_config(*this,cfg_element,back_inserter(candidate_actions_));
	}

	std::function<void(std::vector<candidate_action_ptr>&, const config&)> factory_candidate_actions = [this](std::vector<candidate_action_ptr> &candidate_actions, const config &cfg)
	{
		engine::parse_candidate_action_from_config(*this, cfg, std::back_inserter(candidate_actions));
	};
	register_vector_property(property_handlers(),"candidate_action",candidate_actions_, factory_candidate_actions);

}

config candidate_action_evaluation_loop::to_config() const
{
	config cfg = stage::to_config();
	for (candidate_action_ptr ca : candidate_actions_) {
		cfg.add_child("candidate_action",ca->to_config());
	}
	return cfg;
}


class desc_sorter_of_candidate_actions {
public:
	bool operator()(const candidate_action_ptr &a, const candidate_action_ptr &b) const
	{
		return a->get_max_score() > b->get_max_score();
	}
};

bool candidate_action_evaluation_loop::do_play_stage()
{
	LOG_AI_TESTING_RCA_DEFAULT << "Starting candidate action evaluation loop for side "<< get_side() << std::endl;

	for (candidate_action_ptr ca : candidate_actions_) {
		ca->enable();
	}

	//sort candidate actions by max_score DESC
	std::sort(candidate_actions_.begin(),candidate_actions_.end(),desc_sorter_of_candidate_actions());

	bool executed = false;
	bool gamestate_changed = false;
	do {
		executed = false;
		double best_score = candidate_action::BAD_SCORE;
		candidate_action_ptr best_ptr;

		//Evaluation
		for (candidate_action_ptr ca_ptr : candidate_actions_) {
			if (!ca_ptr->is_enabled()){
				DBG_AI_TESTING_RCA_DEFAULT << "Skipping disabled candidate action: "<< *ca_ptr << std::endl;
				continue;
			}

			if (ca_ptr->get_max_score()<=best_score) {
				DBG_AI_TESTING_RCA_DEFAULT << "Ending candidate action evaluation loop because current score "<<best_score<<" is greater than the upper bound of score for remaining candidate actions "<< ca_ptr->get_max_score()<< std::endl;
				break;
			}

			DBG_AI_TESTING_RCA_DEFAULT << "Evaluating candidate action: "<< *ca_ptr << std::endl;
			double score = ca_ptr->evaluate();
			DBG_AI_TESTING_RCA_DEFAULT << "Evaluated candidate action to score "<< score << " : " << *ca_ptr << std::endl;

			if (score>best_score) {
				best_score = score;
				best_ptr = ca_ptr;
			}
		}

		//Execution
		if (best_score>candidate_action::BAD_SCORE) {
			DBG_AI_TESTING_RCA_DEFAULT << "Executing best candidate action: "<< *best_ptr << std::endl;
			gamestate_observer gs_o;
			best_ptr->execute();
			executed = true;
			if (!gs_o.is_gamestate_changed()) {
				//this means that this CA has lied to us in evaluate()
				//we punish it by disabling it
				DBG_AI_TESTING_RCA_DEFAULT << "Disabling candidate action because it failed to change the game state: "<< *best_ptr << std::endl;
				best_ptr->disable();
				//since we don't re-enable at this play_stage, if we disable this CA, other may get the chance to go.
			} else {
				gamestate_changed = true;
			}
		} else {
			LOG_AI_TESTING_RCA_DEFAULT << "Ending candidate action evaluation loop due to best score "<< best_score<<"<="<< candidate_action::BAD_SCORE<<std::endl;
		}
	} while (executed);
	LOG_AI_TESTING_RCA_DEFAULT << "Ended candidate action evaluation loop for side "<< get_side() << std::endl;
	remove_completed_cas();
	return gamestate_changed;
}

void candidate_action_evaluation_loop::remove_completed_cas()
{
	std::vector<size_t> tbr; // indexes of elements to be removed

	for (size_t i = 0; i != candidate_actions_.size(); ++i)
	{
		if (candidate_actions_[i]->to_be_removed())
		{
			tbr.push_back(i); // so we fill the array with the indexes
		}
	}

	for (size_t i = 0; i != tbr.size(); ++i)
	{
		// we should go downwards, so that index shifts don't affect us
		size_t index = tbr.size() - i - 1; // downcounting for is not possible using unsigned counters, so we hack around
		std::string path = "stage[" + this->get_id() + "].candidate_action[" + candidate_actions_[tbr[index]]->get_name() + "]";

		config cfg = config();
		cfg["path"] = path;
		cfg["action"] = "delete";

		ai::manager::get_singleton().modify_active_ai_for_side(this->get_side(), cfg); // we remove the CA
	}


// @note: this code might be more convenient, but is obviously faulty and incomplete, because of iterator invalidation rules
//	  If you see a way to complete it, please contact me(Nephro).
// 	for (std::vector<candidate_action_ptr>::iterator it = candidate_actions_.begin(); it != candidate_actions_.end(); )
// 	{
// 		if ((*it)->to_be_removed())
// 		{
// 			// code to remove a CA
// 			std::string path = "stage[" + this->get_id() + "].candidate_action[" + (*it)->get_name() + "]";
//
// 			config cfg = config();
// 			cfg["path"] = path;
// 			cfg["action"] = "delete";
//
// 			ai::manager::get_singleton().modify_active_ai_for_side(this->get_side(), cfg);
// 		}
// 		else
// 		{
// 			++it; // @note: should I modify this to a while loop?
// 		}
// 	}
}

rca_context& candidate_action_evaluation_loop::get_rca_context()
{
	return *this;
}

candidate_action_evaluation_loop::~candidate_action_evaluation_loop()
{
}

} // end of namespace testing_ai_default

} // end of namespace ai
