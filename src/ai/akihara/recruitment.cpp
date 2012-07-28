/*
   Copyright (C) 2009 - 2012 by Aline Riss <aline.riss@gmail.com>
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
* Experimental recruitment phase
*/
#include "recruitment.hpp"

#include <boost/foreach.hpp>

#include "../composite/rca.hpp"
#include "../../resources.hpp"
#include "../../unit_map.hpp"
#include "../../map.hpp"
#include "../../team.hpp"
#include "../../unit_display.hpp"
#include "../../unit_types.hpp"

#include "../../log.hpp"

static lg::log_domain log_ai_aki("ai/aki");
#define DBG_AI_AKI LOG_STREAM(debug, log_ai_aki)
#define LOG_AI_AKI LOG_STREAM(info, log_ai_aki)
#define WRN_AI_AKI LOG_STREAM(warn, log_ai_aki)
#define ERR_AI_AKI LOG_STREAM(err, log_ai_aki)

#include <map>

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace akihara_recruitment {

recruitment::recruitment(rca_context &context, const config &cfg)
	: candidate_action(context,cfg)
	, depth_(2)
	, ally_()
	, enemy_()
{
	BOOST_FOREACH( team &t, *resources::teams) {
		if (current_team().is_enemy(t.side()))
			enemy_.push_back(t);
		else if (!current_team().is_enemy(t.side()))
			ally_.push_back(t);
	}

}

recruitment::~recruitment()
{
}

double recruitment::evaluate()
{
	std::vector<unit_map::unit_iterator> leaders = resources::units->find_leaders(get_side());

	BOOST_FOREACH(unit_map::unit_iterator &leader, leaders){
		if (leader == resources::units->end()) {
			return BAD_SCORE;
		}

		std::set<map_location> checked_hexes;
		const map_location &keep = leader->get_location();
		checked_hexes.insert(keep);

		if (resources::game_map->is_keep(leader->get_location()) && count_free_hexes_in_castle(leader->get_location(), checked_hexes) != 0) {
			return get_score();
		}

	}

	return BAD_SCORE;
}

struct situation recruitment::get_next_stage(std::string unit, situation current) {
	situation new_situation;

	new_situation.ally_new_unit = current.ally_new_unit;
	new_situation.enemy_new_unit = current.enemy_new_unit;

	if (current.current_team_side == current_team().side()) {
		new_situation.new_unit = unit;
	} else {
		new_situation.new_unit = current.new_unit;
		new_situation.enemy_new_unit.insert(unit);
	}

	new_situation.current_team_side = get_next_team(current.current_team_side);

	if (new_situation.current_team_side == -1) {
		LOG_AI_AKI << "Error: get_mext_team";
		exit(0);
	}

	if (current_team().is_enemy(new_situation.current_team_side) && new_situation.current_team_side != enemy_[0].side())
		new_situation.depth = current.depth;
	else
		new_situation.depth = current.depth - 1;




	return new_situation;
}

int recruitment::get_next_team(int current_side) {
	if (current_team().side() == current_side)
		return enemy_[0].side();

	unsigned i;
	for(i = 0; i < enemy_.size()-1; i++) {
		if (enemy_[i].side() == current_side)
			return enemy_[i+1].side();
	}
	if (enemy_[enemy_.size()-1].side() == current_side)
		return current_team().side();

	return -1;
}

struct situation recruitment::do_min_max(situation current) {
	LOG_AI_AKI << "\nDepth : " << current.depth << "; Side : " << current.current_team_side << "\n";

	if (current.depth == 0) {
		LOG_AI_AKI << "On evalue " << current.new_unit << "\n";
		current.score = evaluate_unit(current);
		return current;
	}

	situation best_situation;
	best_situation = do_min_max(get_next_stage("", current));

	std::set<std::string> list = get_current_team_recruit(current.current_team_side).recruits();

	BOOST_FOREACH(std::string unit, list) {
		situation new_situation;
		new_situation = do_min_max(get_next_stage(unit, current));

		if (new_situation.score > best_situation.score)
			best_situation = new_situation;
	}

	return best_situation;
}

team recruitment::get_current_team_recruit(int side) {
	BOOST_FOREACH( team &t, *resources::teams) {
		if (t.side() == side)
			return t;
	}

	return current_team();
}

double recruitment::evaluate_unit(situation current) {

	LOG_AI_AKI << "EVALUATION! unit: " << current.new_unit << "; enemies : \n";
	BOOST_FOREACH(std::string s, current.enemy_new_unit) {
		LOG_AI_AKI << s << "\n";
	}

	double score = 0;

	LOG_AI_AKI << "score = " << score << "\n";

	return score;
}



void recruitment::execute()
{
	LOG_AI_AKI << "Akihara's recruitment begin! \n";
	LOG_AI_AKI << "Init: Depth : " << depth_ << "; Side : " << current_team().side() << "\n";

	struct situation current_situation;
	current_situation.current_team_side = current_team().side();
	current_situation.depth = depth_;

	situation best_situation = do_min_max(current_situation);

	LOG_AI_AKI << "Unit to recruit is.... " << best_situation.new_unit << "\n";
}


} //end of akihara_recruitment

} // end of namespace ai
