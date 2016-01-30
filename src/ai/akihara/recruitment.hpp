/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#ifndef AI_AKIHARA_RECRUITMENT_HPP_INCLUDED
#define AI_AKIHARA_RECRUITMENT_HPP_INCLUDED

#include "../composite/rca.hpp"
#include "../../team.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace akihara_recruitment {

class recruitment : public candidate_action {
public:

	recruitment( rca_context &context , const config &cfg );
	virtual ~recruitment();
	virtual double evaluate();
	virtual void execute();
	void do_describe(class situation);

private:
	int depth_;

	std::vector<team> ally_;
	std::vector<team> enemy_;


	void do_recruit(int max_units_to_recruit, double quality_factor);

	situation add_new_unit(std::string unit, situation current);

	double analyze_situation();

	situation do_min_max(situation current);

	team get_current_team_recruit(int side);

	situation get_next_situation(situation& current, std::string unit);


};

class situation {

public:
	situation(int depth, int ai_team);
	situation(const situation& my_situation);
	//virtual ~situation();

	void evaluate();
	void describe();
	double evaluate_unit();
	double get_battle_score(const unit& attacker, const unit& defender, const map_location& att_loc, const map_location& def_loc);

	int getDepth();
	double getScore();
	int getSide();
	int getAISide();
	const team& getAITeam();
	std::set<std::string> getNewUnit();
	std::set<std::string> getNewEnemyUnit();

	void setScore(double score);
	void setSide(int side);
	void setDepth(int depth);
	void setNewUnit(std::set<std::string> unit);
	void setNewEnemyUnit(std::set<std::string> unit);




private:
	int depth_;
	//Score of the current situation;
	double score_;
	//Current team to analyze
	int current_team_side_;
	int ai_team_;
	//New unit of AI
	//std::string new_unit_;
	//New unit
	std::set<std::string> new_unit_;
	//New unit of enemies
	std::set<std::string> enemy_new_unit_;

};

} // of namespace testing_ai_default

namespace akihara_terrain_analyze {

struct field_couple {
	field_couple(const map_location&, const map_location&);

	std::pair<int, int> first_location;
	std::pair<int, int> second_location;
	std::string first_field_type;
	std::string second_field_type;
	int number;
};

class terrain_analyze {
public:
	terrain_analyze();

	void analyze();
	void describe_terrain();

private:
	std::map<std::string, double> terrain_percentage_;

	std::vector<field_couple> couple_field_list_;

	std::vector<map_location> village_location_;


	void search();
	void get_village_terrain();
	void add_terrain(const std::string& terrain);
	void add_village_battle(const map_location& village, const map_location& adj);
	bool compareFieldCouple(field_couple& couple, std::string& first, std::string& second);
};

}

} // of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

