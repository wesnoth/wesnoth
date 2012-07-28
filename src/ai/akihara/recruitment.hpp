/* $Id: ca_testing_recruitment.hpp 52533 2012-01-07 02:35:17 +0000 (Sat, 07 Jan 2012) shadowmaster $ */
/*
   Copyright (C) 2009 - 2012 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * @file
 * Strategic recruitment routine, for experimentation
 */

#ifndef AKIHARA_AI_HPP_INCLUDED
#define AKIHARA_AI_HPP_INCLUDED

#include "../composite/rca.hpp"
#include "../../team.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace akihara_recruitment {

struct situation {

	situation()
		: depth(0)
		, score(0.)
		, current_team_side(0)
		, new_unit()
		, ally_new_unit()
		, enemy_new_unit()
	{
	}

	int depth;

	//Score of the current situation;
	double score;

	//Current team to analyze
	int current_team_side;

	//New unit of AI
	std::string new_unit;

	//New unit of allies
	std::set<std::string> ally_new_unit;

	//New unit of enemies
	std::set<std::string> enemy_new_unit;
};

class recruitment : public candidate_action {
public:

	recruitment( rca_context &context , const config &cfg );

	virtual ~recruitment();

	virtual double evaluate();

	virtual void execute();

	void do_describe(struct situation);

private:
	int depth_;

	std::vector<team> ally_;
	std::vector<team> enemy_;


	void do_recruit(int max_units_to_recruit, double quality_factor);

	struct situation get_next_stage(std::string unit, situation current);

	double analyze_situation();
	double evaluate_unit(situation current);
	situation do_min_max(situation current);
	int get_next_team(int current_side);
	team get_current_team_recruit(int side);


};

} // of namespace testing_ai_default

} // of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

