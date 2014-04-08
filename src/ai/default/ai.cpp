/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 * Artificial intelligence - The computer commands the enemy.
 */

#include "ai.hpp"

#include "../actions.hpp"
#include "../manager.hpp"
#include "../formula/ai.hpp"

#include "../../array.hpp"
#include "../../dialogs.hpp"
#include "../../game_events/pump.hpp"
#include "../../gamestatus.hpp"
#include "../../log.hpp"
#include "../../mouse_handler_base.hpp"
#include "../../resources.hpp"
#include "../../terrain_filter.hpp"
#include "../../unit_display.hpp"
#include "../../wml_exception.hpp"

#include "../../pathfind/pathfind.hpp"

#include <boost/foreach.hpp>

#include <iterator>
#include <algorithm>
#include <fstream>

static lg::log_domain log_ai("ai/general");
#define DBG_AI LOG_STREAM(debug, log_ai)
#define LOG_AI LOG_STREAM(info, log_ai)
#define WRN_AI LOG_STREAM(warn, log_ai)
#define ERR_AI LOG_STREAM(err, log_ai)

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace ai {

typedef util::array<map_location,6> adjacent_tiles_array;

idle_ai::idle_ai(readwrite_context &context, const config& /*cfg*/)
	: recursion_counter_(context.get_recursion_count())
{
	init_readwrite_context_proxy(context);
}

std::string idle_ai::describe_self() const
{
	return "[idle_ai]";
}


void idle_ai::new_turn()
{
}


void idle_ai::switch_side(side_number side)
{
	set_side(side);
}


config idle_ai::to_config() const
{
	config cfg;
	cfg["ai_algorithm"]= "idle_ai";
	return cfg;
}


int idle_ai::get_recursion_count() const
{
	return recursion_counter_.get_count();
}


void idle_ai::play_turn()
{
	game_events::fire("ai turn");
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif


ai_default_recruitment_stage::recruit_situation_change_observer::recruit_situation_change_observer()
	: valid_(false)
{
	manager::add_recruit_list_changed_observer(this);
	manager::add_turn_started_observer(this);
}


void ai_default_recruitment_stage::recruit_situation_change_observer::handle_generic_event(const std::string &/*event_name*/)
{
	valid_ = false;
}


ai_default_recruitment_stage::recruit_situation_change_observer::~recruit_situation_change_observer()
{
	manager::remove_recruit_list_changed_observer(this);
	manager::remove_turn_started_observer(this);
}


bool ai_default_recruitment_stage::recruit_situation_change_observer::get_valid()
{
	return valid_;
}


void ai_default_recruitment_stage::recruit_situation_change_observer::set_valid(bool valid)
{
	valid_ = valid;
}


void ai_default_recruitment_stage::on_create() {
	stage::on_create();
	BOOST_FOREACH(const config &c, cfg_.child_range("limit")) {
		if (c.has_attribute("type") && c.has_attribute("max") ) {
			maximum_counts_.insert(std::make_pair(c["type"],c["max"].to_int(0)));
		}
	}
}

config ai_default_recruitment_stage::to_config() const
{
	config cfg = stage::to_config();
	for (std::map<std::string,int>::const_iterator i = maximum_counts_.begin(); i!= maximum_counts_.end(); ++i) {
		config lim;
		lim["type"] = i->first;
		lim["max"] = str_cast(i->second);
		cfg.add_child("limit",lim);
	}
	return cfg;
}


void ai_default_recruitment_stage::analyze_all()
{
	if (!recruit_situation_change_observer_.get_valid()) {
		not_recommended_units_.clear();
		unit_movement_scores_.clear();
		unit_combat_scores_.clear();
		analyze_potential_recruit_movements();
		analyze_potential_recruit_combat();
		recruit_situation_change_observer_.set_valid(true);
	}
}

bool ai_default_recruitment_stage::recruit_usage(const std::string& usage)
{
	raise_user_interact();
	analyze_all();

	const int min_gold = 0;

	log_scope2(log_ai, "recruiting troops");
	LOG_AI << "recruiting '" << usage << "'\n";

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
			LOG_AI << name << " considered for " << usage << " recruitment\n";
			found = true;

			if (current_team().gold() - ut->cost() < min_gold) {
				LOG_AI << name << " rejected, cost too high (cost: " << ut->cost() << ", current gold: " << current_team().gold() <<", min_gold: " << min_gold << ")\n";
				continue;
			}

			if (not_recommended_units_.count(name))
			{
				LOG_AI << name << " rejected, bad terrain or combat\n";
				continue;
			}


			std::map<std::string,int>::iterator imc = maximum_counts_.find(name);

			if (imc != maximum_counts_.end()) {
				int count_active = 0;
				for (unit_map::const_iterator u = resources::units->begin(); u != resources::units->end(); ++u) {
					if (u->side() == get_side() && !u->incapacitated() && u->type().base_id() == name) {
						++count_active;
					}
				}

				if (count_active >= imc->second) {
					LOG_AI << name << " rejected, too many in the field\n";
					continue;
				}
			}

			LOG_AI << "recommending '" << name << "'\n";
			options.push_back(name);
		}
	}

	// From the available options, choose one at random
	if(options.empty() == false) {
		const int option = rand()%options.size();
		recruit_result_ptr recruit_res = check_recruit_action(options[option]);
		if (recruit_res->is_ok()) {
			recruit_res->execute();
			if (!recruit_res->is_ok()) {
				ERR_AI << "recruitment failed "<< std::endl;
			}
		}
		return recruit_res->is_gamestate_changed();
	}
	if (found) {
		LOG_AI << "No available units to recruit that come under the price.\n";
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
		WRN_AI << warning;
		// Uncommented until the recruitment limiting macro can be fixed to not trigger this warning.
		//lg::wml_error << warning;
		//@fixme
		//return current_team_w().remove_recruitment_pattern_entry(usage);
		return false;
	}
	return false;
}


namespace {

/** A structure for storing an item we're trying to protect. */
struct protected_item {
	protected_item(double value, int radius, const map_location& loc) :
		value(value), radius(radius), loc(loc) {}

	double value;
	int radius;
	map_location loc;
};

}

class remove_wrong_targets {
public:
	remove_wrong_targets(const readonly_context &context)
		:avoid_(context.get_avoid()), map_(*resources::game_map)
	{
	}

bool operator()(const target &t){
	if (!map_.on_board(t.loc)) {
		DBG_AI << "removing target "<< t.loc << " due to it not on_board" << std::endl;
		return true;
	}

	if (t.value<=0) {
		DBG_AI << "removing target "<< t.loc << " due to value<=0" << std::endl;
		return true;
	}

	if (avoid_.match(t.loc)) {
		DBG_AI << "removing target "<< t.loc << " due to 'avoid' match" << std::endl;
		return true;
	}

	return false;
}
private:
	const terrain_filter &avoid_;
	const gamemap &map_;

};


int ai_default_recruitment_stage::average_resistance_against(const unit_type& a, const unit_type& b) const
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
		ERR_AI << "The weighting sum is 0 and is ignored.\n";
	}

	LOG_AI << "average defense of '" << a.id() << "': " << defense << "\n";

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

int ai_default_recruitment_stage::compare_unit_types(const unit_type& a, const unit_type& b) const
{
	const int a_effectiveness_vs_b = average_resistance_against(b,a);
	const int b_effectiveness_vs_a = average_resistance_against(a,b);

	LOG_AI << "comparison of '" << a.id() << " vs " << b.id() << ": "
		<< a_effectiveness_vs_b << " - " << b_effectiveness_vs_a << " = "
		<< (a_effectiveness_vs_b - b_effectiveness_vs_a) << '\n';
	return a_effectiveness_vs_b - b_effectiveness_vs_a;
}

void ai_default_recruitment_stage::get_combat_score_vs(const unit_type& ut, const std::string &enemy_type_id, int &score, int &weighting, int hitpoints, int max_hitpoints) const
{
	const unit_type *enemy_info = unit_types.find(enemy_type_id);
	VALIDATE(enemy_info, "Unknown unit type : " + enemy_type_id + " while scoring units.");
	int weight = ut.cost();
	if ((hitpoints>0) && (max_hitpoints>0)) {
		weight = weight * hitpoints / max_hitpoints;
	}

	weighting += weight;
	score += compare_unit_types(ut, *enemy_info) * weight;
}

int ai_default_recruitment_stage::get_combat_score(const unit_type& ut) const
{
	int score = 0, weighting = 0;
	const unit_map & units_ = *resources::units;
	for(unit_map::const_iterator j = units_.begin(); j != units_.end(); ++j) {
		if (!current_team().is_enemy(j->side())) {
			continue;
		}

		if (j->can_recruit()) {

			team &enemy_team = (*resources::teams)[j->side() - 1];
			const std::set<std::string> &recruits = enemy_team.recruits();
			BOOST_FOREACH(const std::string &rec, recruits) {
				get_combat_score_vs(ut,rec,score,weighting,0,0);
			}
			continue;
		}

		get_combat_score_vs(ut, j->type().base_id(), score, weighting, j->hitpoints(), j->max_hitpoints());
	}

	if(weighting != 0) {
		score /= weighting;
	}
	return score;
}

void ai_default_recruitment_stage::analyze_potential_recruit_combat()
{
	if(unit_combat_scores_.empty() == false ||
			get_recruitment_ignore_bad_combat()) {
		return;
	}

	log_scope2(log_ai, "analyze_potential_recruit_combat()");

	// Records the best combat analysis for each usage type.
	best_usage_.clear();

	const std::set<std::string>& recruits = current_team().recruits();
	std::set<std::string>::const_iterator i;
	for(i = recruits.begin(); i != recruits.end(); ++i) {
		const unit_type *info = unit_types.find(*i);
		if (!info || not_recommended_units_.count(*i)) {
			continue;
		}

		int score = get_combat_score(*info);
		LOG_AI << "combat score of '" << *i << "': " << score << "\n";
		unit_combat_scores_[*i] = score;

		if(best_usage_.count(info->usage()) == 0 ||
				score > best_usage_[info->usage()]) {
			best_usage_[info->usage()] = score;
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

		if(unit_combat_scores_[*i] + 600 < best_usage_[info->usage()]) {
			LOG_AI << "recommending not to use '" << *i << "' because of poor combat performance "
				<< unit_combat_scores_[*i] << "/" << best_usage_[info->usage()] << "\n";
			not_recommended_units_.insert(*i);
		}
	}
}

namespace {

struct target_comparer_distance {
	target_comparer_distance(const map_location& loc) : loc_(loc) {}

	bool operator()(const ai::target& a, const ai::target& b) const {
		return distance_between(a.loc,loc_) < distance_between(b.loc,loc_);
	}

private:
	map_location loc_;
};

}


ai_default_recruitment_stage::ai_default_recruitment_stage(ai_context &context, const config &cfg)
	: stage(context,cfg),
	  best_usage_(),
	  cfg_(cfg),
	  maximum_counts_(),
	  not_recommended_units_(),
	  recall_list_scores_(),
	  recruit_situation_change_observer_(),
	  unit_combat_scores_(),
	  unit_movement_scores_()

{
}


ai_default_recruitment_stage::~ai_default_recruitment_stage()
{
}

void ai_default_recruitment_stage::analyze_potential_recruit_movements()
{
	unit_map &units_ = *resources::units;
	gamemap &map_ = *resources::game_map;

	if(unit_movement_scores_.empty() == false ||
			get_recruitment_ignore_bad_movement()) {
		return;
	}

	const unit_map::const_iterator leader = units_.find_leader(get_side());
	if(leader == units_.end()) {
		return;
	}

	const map_location& start = nearest_keep(leader->get_location());
	if(map_.on_board(start) == false) {
		return;
	}

	log_scope2(log_ai, "analyze_potential_recruit_movements()");

	const unsigned int max_targets = 5;

	const move_map dstsrc;
	std::vector<target> targets = find_targets(dstsrc);
	if(targets.size() > max_targets) {
		std::sort(targets.begin(),targets.end(),target_comparer_distance(start));
		targets.erase(targets.begin()+max_targets,targets.end());
	}

	const std::set<std::string>& recruits = current_team().recruits();

	LOG_AI << "targets: " << targets.size() << "\n";

	std::map<std::string,int> best_scores;

	for(std::set<std::string>::const_iterator i = recruits.begin(); i != recruits.end(); ++i) {
		const unit_type *info = unit_types.find(*i);
		if (!info) {
			continue;
		}

		const unit_type &ut = *info;
		///@todo 1.9: we give max movement, but recruited will get 0? Seems inaccurate
		//but keep it like that for now
		// pathfinding ignoring other units and terrain defense
		const pathfind::move_type_path_calculator calc(ut.movement_type(), ut.movement(), ut.movement(), current_team(),map_);

		int cost = 0;
		int targets_reached = 0;
		int targets_missed = 0;

		for(std::vector<target>::const_iterator t = targets.begin(); t != targets.end(); ++t) {
			LOG_AI << "analyzing '" << *i << "' getting to target...\n";
			pathfind::plain_route route = a_star_search(start, t->loc, 100.0, &calc,
					resources::game_map->w(), resources::game_map->h());

			if (!route.steps.empty()) {
				LOG_AI << "made it: " << route.move_cost << "\n";
				cost += route.move_cost;
				++targets_reached;
			} else {
				LOG_AI << "failed\n";
				++targets_missed;
			}
		}

		if(targets_reached == 0 || targets_missed >= targets_reached*2) {
			unit_movement_scores_[*i] = 100000;
			not_recommended_units_.insert(*i);
		} else {
			const int average_cost = cost/targets_reached;
			const int score = (average_cost * (targets_reached+targets_missed))/targets_reached;
			unit_movement_scores_[*i] = score;

			const std::map<std::string,int>::const_iterator current_best = best_scores.find(ut.usage());
			if(current_best == best_scores.end() || score < current_best->second) {
				best_scores[ut.usage()] = score;
			}
		}
	}

	for(std::map<std::string,int>::iterator j = unit_movement_scores_.begin();
			j != unit_movement_scores_.end(); ++j) {

		const unit_type *info = unit_types.find(j->first);

		if (!info) {
			continue;
		}

		const int best_score = best_scores[info->usage()];
		if(best_score > 0) {
			j->second = (j->second*10)/best_score;
			if(j->second > 15) {
				LOG_AI << "recommending against recruiting '" << j->first << "' (score: " << j->second << ")\n";
				not_recommended_units_.insert(j->first);
			} else {
				LOG_AI << "recommending recruit of '" << j->first << "' (score: " << j->second << ")\n";
			}
		}
	}

	if(not_recommended_units_.size() == unit_movement_scores_.size()) {
		not_recommended_units_.clear();
	}
}


std::string ai_default_recruitment_stage::find_suitable_recall_id()
{
	if (recall_list_scores_.empty()) {
		return "";
	}
	std::string best_id = recall_list_scores_.back().first;
	recall_list_scores_.pop_back();
	return best_id;
}

class unit_combat_score_getter {
public:
	unit_combat_score_getter(const ai_default_recruitment_stage &s)
		: stage_(s)
	{
	}
	std::pair<std::string, double> operator()(const unit &u) {
		std::pair<std::string,int> p;
		p.first = u.id();
		const unit_type& u_type = u.type();

		double xp_ratio = 0;
		if (u.can_advance() && (u.max_experience()>0)) {
			xp_ratio = u.experience()/u.max_experience();
		}

		p.second = (1-xp_ratio) * stage_.get_combat_score(u_type);
		double recall_cost = game_config::recall_cost != 0 ? game_config::recall_cost : 1;

		p.second *= static_cast<double>(u_type.cost())/recall_cost;
		if (u.can_advance() && (xp_ratio>0) ) {
		        double best_combat_score_of_advancement = 0;
			bool best_combat_score_of_advancement_found = false;
			int best_cost = recall_cost;
			BOOST_FOREACH(const std::string &i, u.advances_to()) {
				const unit_type *ut = unit_types.find(i);
				if (!ut) {
					continue;
				}

				int combat_score_of_advancement = stage_.get_combat_score(*ut);
				if (!best_combat_score_of_advancement_found || (best_combat_score_of_advancement<combat_score_of_advancement)) {
					best_combat_score_of_advancement = combat_score_of_advancement;
					best_combat_score_of_advancement_found = true;
					best_cost = ut->cost();
				}

			}
			p.second += xp_ratio*best_combat_score_of_advancement*best_cost/recall_cost;
		}

		return p;

	}
private:
	const ai_default_recruitment_stage &stage_;
};


template <class T, class V>
bool smaller_mapped_value(const std::pair<T,V>& a, const std::pair<T,V>& b)
{
  return a.second < b.second;
}

class bad_recalls_remover {
public:
	bad_recalls_remover(const std::map<std::string, int>& unit_combat_scores)
		: allow_any_(false), best_combat_score_()
	{
		std::map<std::string, int>::const_iterator cs = std::min_element(unit_combat_scores.begin(),unit_combat_scores.end(),&smaller_mapped_value<std::string,int>);

		if (cs == unit_combat_scores.end()) {
			allow_any_ = true;
		} else {
			best_combat_score_ = cs->second;
		}
	}
	bool operator()(const std::pair<std::string,double>& p) {
		if (allow_any_) {
			return false;
		}
		if (p.second>=best_combat_score_) {
			return false;
		}
		return true;
	}

private:
	bool allow_any_;
	double best_combat_score_;
};


class combat_score_less {
public:
	bool operator()(const std::pair<std::string,double> &s1, const std::pair<std::string,double> &s2)
	{
		return s1.second<s2.second;
	}
};

static void debug_print_recall_list_scores(const std::vector< std::pair<std::string,double> > &recall_list_scores,const char *message)
{
	if (!lg::debug.dont_log(log_ai)) {
		std::stringstream s;
		s << message << std::endl;
		for (std::vector< std::pair<std::string,double> >::const_iterator p = recall_list_scores.begin(); p!=recall_list_scores.end();++p) {
			s << p->first << " ["<<p->second<<"]"<<std::endl;
		}
		DBG_AI << s.str();
	}


}

bool ai_default_recruitment_stage::analyze_recall_list()
{
	if (current_team().gold() < current_team().recall_cost() ) {
		return false;
	}

	const std::vector<unit> &recalls = current_team().recall_list();

	if (recalls.empty()) {
		return false;
	}

	std::transform(recalls.begin(), recalls.end(), std::back_inserter< std::vector <std::pair<std::string,double> > > (recall_list_scores_), unit_combat_score_getter(*this) );

	debug_print_recall_list_scores(recall_list_scores_,"Recall list, after scoring:");

	recall_list_scores_.erase( std::remove_if(recall_list_scores_.begin(), recall_list_scores_.end(), bad_recalls_remover(unit_combat_scores_)), recall_list_scores_.end() );

	debug_print_recall_list_scores(recall_list_scores_,"Recall list, after erase:");

	if (recall_list_scores_.empty()) {
		return false;
	}

	sort(recall_list_scores_.begin(),recall_list_scores_.end(),combat_score_less());

	debug_print_recall_list_scores(recall_list_scores_,"Recall list, after sort (worst to best):");

	return !(recall_list_scores_.empty());
}


bool ai_default_recruitment_stage::do_play_stage()
{
	const unit_map &units_ = *resources::units;

	const unit_map::const_iterator leader = units_.find_leader(get_side());
	if(leader == units_.end()) {
		return false;
	}

	const map_location& start_pos = nearest_keep(leader->get_location());

	analyze_all();

	//handle recalls
	//if there any recalls left which have a better combat score/cost ratio, get them
	bool gamestate_changed = false;
	std::string id;
	if (analyze_recall_list()) {
		while ( !(id = find_suitable_recall_id()).empty() ) {

			recall_result_ptr recall_res = check_recall_action(id);
			if (recall_res->is_ok()) {
				recall_res->execute();
				if (!recall_res->is_ok()) {
					ERR_AI << "recall failed "<< std::endl;
					break;
				}
			}
			gamestate_changed |= recall_res->is_gamestate_changed();

		}
	}

	std::vector<std::string> options = get_recruitment_pattern();
	if (std::count(options.begin(), options.end(), "scout") > 0) {
		size_t neutral_villages = 0;

		// We recruit the initial allocation of scouts
		// based on how many neutral villages there are
		// that are closer to us than to other keeps.
		const std::vector<map_location>& villages = resources::game_map->villages();
		for(std::vector<map_location>::const_iterator v = villages.begin(); v != villages.end(); ++v) {
			const int owner = village_owner(*v);
			if(owner == -1) {
				const size_t distance = distance_between(start_pos,*v);

				bool closest = true;
				for(std::vector<team>::const_iterator i = resources::teams->begin(); i != resources::teams->end(); ++i) {
					const int index = i - resources::teams->begin() + 1;
					const map_location& loc = resources::game_map->starting_position(index);
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

		LOG_AI << "scouts_wanted: " << neutral_villages << "/"
			<< villages_per_scout << " = " << scouts_wanted << "\n";

		std::map<std::string,int> unit_types;

		for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u) {
			if (u->side() == get_side()) {
				++unit_types[u->usage()];
			}
		}

		LOG_AI << "we have " << unit_types["scout"] << " scouts already and we want "
			<< scouts_wanted << " in total\n";

		while(unit_types["scout"] < scouts_wanted) {
			if(recruit_usage("scout") == false)
				break;
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
		gamestate_changed = true;
		options = get_recruitment_pattern();
		if (options.empty()) {
			options.push_back("");
		}
	}

	return gamestate_changed;
}

variant attack_analysis::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "target") {
		return variant(new location_callable(target));
	} else if(key == "movements") {
		std::vector<variant> res;
		for(size_t n = 0; n != movements.size(); ++n) {
			map_formula_callable* item = new map_formula_callable(NULL);
			item->add("src", variant(new location_callable(movements[n].first)));
			item->add("dst", variant(new location_callable(movements[n].second)));
			res.push_back(variant(item));
		}

		return variant(&res);
	} else if(key == "units") {
		std::vector<variant> res;
		for(size_t n = 0; n != movements.size(); ++n) {
			res.push_back(variant(new location_callable(movements[n].first)));
		}

		return variant(&res);
	} else if(key == "target_value") {
		return variant(static_cast<int>(target_value*1000));
	} else if(key == "avg_losses") {
		return variant(static_cast<int>(avg_losses*1000));
	} else if(key == "chance_to_kill") {
		return variant(static_cast<int>(chance_to_kill*100));
	} else if(key == "avg_damage_inflicted") {
		return variant(static_cast<int>(avg_damage_inflicted));
	} else if(key == "target_starting_damage") {
		return variant(target_starting_damage);
	} else if(key == "avg_damage_taken") {
		return variant(static_cast<int>(avg_damage_taken));
	} else if(key == "resources_used") {
		return variant(static_cast<int>(resources_used));
	} else if(key == "terrain_quality") {
		return variant(static_cast<int>(terrain_quality));
	} else if(key == "alternative_terrain_quality") {
		return variant(static_cast<int>(alternative_terrain_quality));
	} else if(key == "vulnerability") {
		return variant(static_cast<int>(vulnerability));
	} else if(key == "support") {
		return variant(static_cast<int>(support));
	} else if(key == "leader_threat") {
		return variant(leader_threat);
	} else if(key == "uses_leader") {
		return variant(uses_leader);
	} else if(key == "is_surrounded") {
		return variant(is_surrounded);
	} else {
		return variant();
	}
}

void attack_analysis::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using namespace game_logic;
	inputs->push_back(formula_input("target", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("movements", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("units", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("target_value", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_losses", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("chance_to_kill", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_damage_inflicted", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("target_starting_damage", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("avg_damage_taken", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("resources_used", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("terrain_quality", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("alternative_terrain_quality", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("vulnerability", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("support", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("leader_threat", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("uses_leader", FORMULA_READ_ONLY));
	inputs->push_back(formula_input("is_surrounded", FORMULA_READ_ONLY));
}

} //end of namespace ai

