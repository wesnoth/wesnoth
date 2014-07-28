/*
   Copyright (C) 2014 by Guorui Xi <kevin.xgr@gmail.com>
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

#ifndef AI_TESTING_STAGE_SF_WITH_RCA_HPP_INCLUDED
#define AI_TESTING_STAGE_SF_WITH_RCA_HPP_INCLUDED

#include "stage_rca.hpp"
#include "../composite/stage.hpp"

#include "../../unit_map.hpp"

#include <queue>

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace testing_ai_default {

class turn_state;
class decision;

class strategy_formulation_with_rca: public virtual stage, public virtual rca_context {
public:
	strategy_formulation_with_rca(ai_context &context, const config &cfg);

	~strategy_formulation_with_rca();

	bool do_play_stage();

	void on_create();

	config to_config() const;

	rca_context& get_rca_context();

	void simulate_states_ahead();

	void set_optimal_strategy();

	const turn_state simulate_state(const int decision_no_, const turn_state &state);

	void switch_side();

	void init_side();

private:
	std::queue<turn_state> states_;

	std::vector<bool> enemy_this_turn_;	// Store if is enemy for this turn. Use in the set optimal strategy function.

	boost::shared_ptr<candidate_action_evaluation_loop> rca_;
};

class decision
{
	friend std::ostream& operator<<(std::ostream&, const decision&);

public:
	static const int total_decisions = 2;

	explicit decision(int decision_no_) : decision_no_(decision_no_)
	{
	}

	~decision(){}

	int get_decision_no() const { return decision_no_; }

	decision& set_decision_no(int decision_no_) { this->decision_no_ = decision_no_; return *this; }

	bool is_valid() const { return (decision_no_ >= 0 && decision_no_ < total_decisions); }

private:
	int decision_no_;
};

class turn_state
{
public:
	turn_state(const int own_side_, const int turn_no_, const unit_map &units_, const std::vector<team> &teams_);

	~turn_state();

	void scoring_state();

	int get_own_side() const { return own_side_; }

	int get_turn_no() const { return turn_no_; }

	double get_state_score() const { return state_score_; }

	const unit_map& get_units() const { return units_; }

	const std::vector<team>& get_teams() const { return teams_; }

	const decision& get_decision() const { return decision_; }

	void set_decision(int decision_no_) { decision_.set_decision_no(decision_no_); }

private:
	const int own_side_;

	const int turn_no_;

	double state_score_;

	const unit_map units_;

	const std::vector<team> teams_;

	decision decision_;
};

} // of namespace testing_ai_default

} // of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
