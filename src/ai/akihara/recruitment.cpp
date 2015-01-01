/*
   Copyright (C) 2009 - 2015 by Aline Riss <aline.riss@gmail.com>
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


#include <boost/foreach.hpp>
#include <map>

#include "../../actions/attack.hpp"
#include "../../attack_prediction.hpp"
#include "../../resources.hpp"
#include "../../log.hpp"
#include "../../map.hpp"
#include "../../team.hpp"
#include "../../unit_display.hpp"
#include "../../unit_map.hpp"
#include "../../unit_types.hpp"
#include "../composite/rca.hpp"
#include "../default/ai.hpp"

#include "recruitment.hpp"


static lg::log_domain log_ai_aki("ai/aki");
#define DBG_AI_AKI LOG_STREAM(debug, log_ai_aki)
#define LOG_AI_AKI LOG_STREAM(info, log_ai_aki)
#define WRN_AI_AKI LOG_STREAM(warn, log_ai_aki)
#define ERR_AI_AKI LOG_STREAM(err, log_ai_aki)

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

namespace akihara_terrain_analyze {

field_couple::field_couple(const map_location& first, const map_location& second) :
	first_location(),
	second_location(),
	first_field_type(),
	second_field_type(),
	number(1) {
first_location.first = first.x;
first_location.second = first.y;
first_field_type = resources::game_map->get_terrain_string(first);

second_location.first = second.x;
second_location.second = second.y;
second_field_type = resources::game_map->get_terrain_string(second);

}

terrain_analyze::terrain_analyze():
	terrain_percentage_(),
	couple_field_list_(),
	village_location_()
{
}

void terrain_analyze::analyze() {
	search();
	get_village_terrain();
	describe_terrain();
}

void terrain_analyze::search() {
	for (int x = 0; x < resources::game_map->w(); x++) {
		for(int y = 0; y < resources::game_map->h(); y++) {
			const map_location loc(x,y);
			if (resources::game_map->is_keep(loc)) {
			} else if (resources::game_map->is_village(loc)) {
				village_location_.push_back(loc);
			}
		}
	}
}

void terrain_analyze::describe_terrain() {
	LOG_AI_AKI << "Terrain description! (or not)\n";
	BOOST_FOREACH ( field_couple& couple, couple_field_list_) {
		LOG_AI_AKI << "BATTLE : f1 : " << couple.first_field_type << "; f2 : " << couple.second_field_type
				<< "; num = " << couple.number << "\n";
	}
}

void terrain_analyze::get_village_terrain() {
	BOOST_FOREACH(const map_location& loc, village_location_) {
		map_location adj[6];
		get_adjacent_tiles(loc,adj);

		for(int i = 0; i < 6; i++) {
			add_village_battle(loc, adj[i]);
		}
	}
}

void terrain_analyze::add_terrain(const std::string& terrain) {
	std::map<std::string, double>::iterator i = terrain_percentage_.find(terrain);
	if (i != terrain_percentage_.end()) {
		i->second += 1;
	} else {
		terrain_percentage_[terrain] = 1;
	}
}

void terrain_analyze::add_village_battle(const map_location& village, const map_location& adj) {
	// If the battle is already stored, increment the number
	std::string village_field = resources::game_map->get_terrain_string(village);
	std::string adj_field = resources::game_map->get_terrain_string(adj);

	BOOST_FOREACH(field_couple &couple, couple_field_list_) {
		if (compareFieldCouple(couple, village_field, adj_field)) {
			couple.number++;
			return;
		}
	}

	field_couple new_couple(village, adj);
	couple_field_list_.push_back(new_couple);
}

bool terrain_analyze::compareFieldCouple(field_couple& couple, std::string& first, std::string& second) {
	if (couple.first_field_type == first && couple.second_field_type == second) {
		return true;
	}
	if (couple.first_field_type == second && couple.second_field_type == first) {
		return true;
	}
	return false;
}

}


namespace akihara_recruitment {

situation::situation(int depth, int ai_team):
	depth_(depth),
	score_(0),
	current_team_side_(ai_team),
	ai_team_(ai_team),
	new_unit_(),
	enemy_new_unit_()
{
}

situation::situation(const situation& my_situation) :
			depth_(my_situation.depth_),
			score_(my_situation.score_),
			current_team_side_(my_situation.current_team_side_),
			ai_team_(my_situation.ai_team_),
			new_unit_(my_situation.new_unit_),
			enemy_new_unit_(my_situation.enemy_new_unit_) {

}

// The aim of this function is to determine if the situation is favorable or not
// For this, we need to evaluate the complete situation!
void situation::evaluate() {

	LOG_AI_AKI << "EVALUATION! unit: \n";

	BOOST_FOREACH(const unit& unit, *resources::units) {
		if (unit.side() == ai_team_) {
			score_ += evaluate_unit();
		}
	}

	// we conciderate the new unit
	if (!new_unit_.empty()) {
		score_ += evaluate_unit();
	}

	LOG_AI_AKI << "score = " << score_ << "\n";

}

void situation::describe() {
	LOG_AI_AKI << "Describe situation!\n";
	LOG_AI_AKI << "AI Side : " << ai_team_ << "; Current Side : " << current_team_side_ << "\n";
	LOG_AI_AKI << "New unit";
	BOOST_FOREACH(std::string unit, new_unit_) {
		LOG_AI_AKI << unit << "\n";
	}
	LOG_AI_AKI << "Enemy unit";
	BOOST_FOREACH(std::string unit, enemy_new_unit_) {
		LOG_AI_AKI << unit << "\n";
	}

}

double situation::evaluate_unit() {
	// for each unit on the field
	double score = 0;
	/*const unit_type *type =unit_types.find(my_unit_type);*/
	/*if (ai_team_ == current_team_side_)
	unit attacker(att_type, 3, false);
	unit defender(def_type, 2, false);


	temporary_unit_placer att_place(*resources::units, att_loc, attacker);
	temporary_unit_placer def_place(*resources::units, def_loc, defender);


	BOOST_FOREACH(const unit& enn_unit, *resources::units) {
		if (getAITeam().is_enemy(enn_unit.side())) {
			score += get_combat_score(current_team_side_, *type, *enn_unit.type());
		}
	}

	// for the new enemy unit
	BOOST_FOREACH(const std::string& enn_unit, enemy_new_unit_) {
		if (enn_unit.empty()) {
			const unit_type *enn_new_unit = unit_types.find(enn_unit);
			score += get_combat_score(current_team_side_, *type, *enn_new_unit);
		}
	}*/

	return score;
}

double situation::get_battle_score(const unit& attacker, const unit& defender, const map_location& att_loc, const map_location& def_loc) {
	double score_plus = 0.0;
	double score_min = 0.0;

	int max_att_hp = attacker.hitpoints();
	int max_def_hp = defender.hitpoints();

	int per = 0;
	int nb_sol = 0;
	double weight = attacker.hitpoints() / attacker.max_hitpoints();

	battle_context bc(*resources::units, att_loc, def_loc, -1, -1, 0.5, NULL, &attacker);

	std::vector<double> hp_dist = bc.get_defender_combatant().hp_dist;

	for(unsigned i = 0; i < hp_dist.size(); i++) {
		if (hp_dist[i] != 0) {
			per = (max_def_hp - i) * hp_dist[i] * 100;
			nb_sol++;
			score_plus += per;
		}
	}

	if (nb_sol != 0) {
		score_plus /= nb_sol;
	}

	if (bc.get_defender_combatant().poisoned != 0)
		score_plus += bc.get_defender_combatant().poisoned;
	if (bc.get_defender_combatant().slowed != 0)
		score_plus += bc.get_defender_combatant().slowed;

	nb_sol = 0;
	hp_dist = bc.get_attacker_combatant().hp_dist;

	for(unsigned i = 0; i < hp_dist.size(); i++) {
		if (hp_dist[i] != 0) {
			per = (max_att_hp - i) * hp_dist[i] * 100;
			score_min += per;
			nb_sol++;
		}
	}

	if (nb_sol != 0 ) {
		score_min /= nb_sol;
	}

	if (bc.get_attacker_combatant().poisoned != 0)
		score_min += bc.get_defender_combatant().poisoned;
	if (bc.get_attacker_combatant().slowed != 0)
		score_min += bc.get_defender_combatant().slowed;

	return (score_min - score_plus) * weight;
}

int situation::getDepth() {
	return depth_;
}

int situation::getSide() {
	return current_team_side_;
}

const team& situation::getAITeam() {
	BOOST_FOREACH(const team& team, *resources::teams) {
		if (team.side() == ai_team_)
			return team;
	}

	LOG_AI_AKI << "Error: wrong team returned!\n";
	return resources::teams->at(0);
}

std::set<std::string> situation::getNewUnit() {
	return new_unit_;
}

std::set<std::string> situation::getNewEnemyUnit() {
	return enemy_new_unit_;
}

double situation::getScore() {
	return score_;
}

void situation::setScore(double score) {
	score_ = score;
}

void situation::setSide(int side) {
	current_team_side_ = side;
}

void situation::setDepth(int depth) {
	depth_ = depth;
}

void situation::setNewEnemyUnit(std::set<std::string> unit) {
	enemy_new_unit_ = unit;
}

void situation::setNewUnit(std::set<std::string> unit) {
	new_unit_ = unit;
}

int situation::getAISide() {
	return ai_team_;
}


recruitment::recruitment(rca_context &context, const config &cfg)
: candidate_action(context,cfg),
  depth_(2),
  ally_(),
  enemy_()
{
	BOOST_FOREACH( team &t, *resources::teams) {
		if (current_team().is_enemy(t.side())) {
			enemy_.push_back(t);
		} else if (!current_team().is_enemy(t.side())) {
			ally_.push_back(t);
		}
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

situation recruitment::get_next_situation(situation& current, std::string unit) {
	situation new_situation(current);

	bool change = false;

	if (new_situation.getDepth() == 1) {
		new_situation.setDepth(new_situation.getDepth() - 1);
		return new_situation;
	}

	//depth INT
	if (new_situation.getSide() == new_situation.getAISide()) {
		LOG_AI_AKI << "Set enemy 0 side!\n";
		new_situation.setSide(enemy_[0].side());
		LOG_AI_AKI << "TEST : " << new_situation.getSide() << "\n";
		change = true;
	} else if (new_situation.getSide() == enemy_[enemy_.size() - 1].side()) {
		LOG_AI_AKI << "Set ai side!\n";
		new_situation.setSide(new_situation.getAISide());
		change = true;
	} else {
		LOG_AI_AKI << "Set next enemy side\n";
		for(unsigned i = 0; i < enemy_.size(); i++) {
			if (new_situation.getSide() == enemy_[i].side()) {
				new_situation.setSide(enemy_[i+1].side());
				break;
			}
		}
	}

	if (change) {
		new_situation.setDepth(new_situation.getDepth() - 1);
		new_situation.setNewEnemyUnit(current.getNewUnit());
		new_situation.setNewUnit(current.getNewEnemyUnit());

		change = false;
	}

	new_situation.getNewUnit().insert(unit);

	return new_situation;
}

situation recruitment::do_min_max(situation current) {
	LOG_AI_AKI << "\nDepth : " << current.getDepth() << "; Side : " << current.getSide() << "\n";

	if (current.getDepth() == 0) {
		LOG_AI_AKI << "EVAL\n";
		//current.evaluate();
		current.describe();
		return current;
	}

	//situation best_situation = do_min_max(get_next_stage("", current));
	//bool enemy = current_team().is_enemy(current.getSide()) && current.getSide() != enemy_[0].side() ? false : true;
	situation best_situation(do_min_max(get_next_situation(current, "")));

	std::set<std::string> list = get_current_team_recruit(current.getSide()).recruits();

	BOOST_FOREACH(std::string unit, list) {
		situation new_situation(do_min_max(get_next_situation(current, unit)));

		if (new_situation.getScore() < best_situation.getScore())
			best_situation = new_situation;
	}

	best_situation.setScore(-best_situation.getScore());
	return best_situation;
}

team recruitment::get_current_team_recruit(int side) {
	BOOST_FOREACH( team &t, *resources::teams) {
		if (t.side() == side)
			return t;
	}

	return current_team();
}


void recruitment::execute()
{
	LOG_AI_AKI << "Akihara's recruitment begin! \n";
	//akihara_terrain_analyze::terrain_analyze test;
	//test.analyze();
	LOG_AI_AKI << "Init: Depth : " << depth_ << "; Side : " << current_team().side() << "\n";

	BOOST_FOREACH( team &t, enemy_) {
		LOG_AI_AKI << t.side() << "\n";
	}

	situation current_situation(depth_, current_team().side());

	situation best_situation(do_min_max(current_situation));

	LOG_AI_AKI << "END \n";
}


} //end of akihara_recruitment

} // end of namespace ai
