/* $Id: ca_testing_move_to_targets.cpp 46187 2010-09-01 22:19:28Z crab $ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Experimental recruitment phase by floris
 */

#include "ca_testing_recruitment.hpp"
#include "../actions.hpp"
#include "../manager.hpp"
#include "../composite/engine.hpp"
#include "../composite/rca.hpp"
#include "../composite/stage.hpp"
#include "../../gamestatus.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"
#include "../../map.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"
#include "../../wml_exception.hpp"
#include "../../pathfind/pathfind.hpp"


#include <numeric>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>


namespace ai {

namespace testing_ai_default {

static lg::log_domain log_ai_ca_testing_recruitment("ai/ca/testing_recruitment");
#define DBG_AI LOG_STREAM(debug, log_ai_ca_testing_recruitment)
#define LOG_AI LOG_STREAM(info, log_ai_ca_testing_recruitment)
#define WRN_AI LOG_STREAM(warn, log_ai_ca_testing_recruitment)
#define ERR_AI LOG_STREAM(err, log_ai_ca_testing_recruitment)


testing_recruitment_phase::testing_recruitment_phase( rca_context &context, const config &cfg )
: candidate_action(context,cfg)
{
}


testing_recruitment_phase::~testing_recruitment_phase()
{
}


double testing_recruitment_phase::evaluate()
{
	const unit_map::const_iterator leader = resources::units->find_leader(get_side());
	if(leader == resources::units->end()) {
		return BAD_SCORE;
	}
	if (!resources::game_map->is_keep(leader->get_location())) {
		return BAD_SCORE;
	}

	std::set<map_location> checked_hexes;
	checked_hexes.insert(leader->get_location());
	if (count_free_hexes_in_castle(leader->get_location(), checked_hexes)==0) {
		return BAD_SCORE;
	}
	return get_score();
}


class potential_recruit
{
	public:
		bool operator==(const std::string &i) const
		{
			return id() == i;
		}

		potential_recruit(int cost, int max_qty, double quality, int side, const unit_type *type)
			: cost_(cost), max_qty_(max_qty), quality_(quality), side_(side), type_(type)
		{}

		const std::string& id() const
		{
			return type_->id();
		}
		int cost() const
		{
			return cost_;
		}
		double quality() const
		{
			return quality_;
		}
		void set_quality(double quality)
		{
			quality_ = quality;
		}
		int max_qty() const
		{
			return max_qty_;
		}
		int side() const
		{
			return side_;
		}
		const unit_type *type() const
		{
			return type_;
		}
	private:
		int cost_;
		int max_qty_;
		double quality_;
		int side_;
		const unit_type *type_;
};

class potential_recruit_converter
{
	public:
		potential_recruit_converter(int max_qty, int side )
			: max_qty_ (max_qty), side_(side)
		{
		}
		potential_recruit operator()(const std::string &id)
		{
			const unit_type *type = unit_types.find(id);
			return potential_recruit(type->cost(),max_qty_,0,side_,type);
		}
	private:
		int max_qty_;
		int side_;
};

class fake_team
{
	public:
		fake_team(const team &target)
			: target_(&target)
		{
			reset();
		}

		void reset()
		{
			extra_units_.clear();
			gold_ = target_->gold();
			recruit_list_.clear();
			std::transform(target_->recruits().begin(),target_->recruits().end(),std::back_inserter(recruit_list_), potential_recruit_converter(999,side()));
		}

		int gold()
		{
			return gold_;
		}

		void set_gold(int gold)
		{
			gold_ = gold;
		}

		void fake_recruit(const potential_recruit &r)
		{
			if (gold()<r.cost()) {
				ERR_AI <<  "ERROR: cannot fake recruit "<<r.id()<<", not enough gold" << std::endl;
				return;
			}
			if (get_current_qty(r)>=r.max_qty()) {
				ERR_AI << "ERROR: cannot fake recruit "<<r.id()<<", too many in the field" << std::endl;
				return;
			}
			DBG_AI << " Fake recruiting [" << r.id() << "]" << std::endl;
			spend_gold(r.cost());
			extra_units_.push_back(r);
		}

		const std::vector<potential_recruit>& extra_units() const
		{
			return extra_units_;
		}

		int get_current_qty(const potential_recruit &r) const
		{
			return get_current_qty(r.id());//@todo: fix
		}

		int get_current_qty(const std::string &name) const
		{
			return std::count(extra_units_.begin(), extra_units_.end(), name);//@todo: fix
		}

		void spend_gold(int gold)
		{
			gold_-=gold;
		}

		int side() const
		{
			return target_->side();
		}

		bool is_enemy(int side) const
		{
			return target_->is_enemy(side);
		}

		std::vector<potential_recruit> &recruit_list()
		{
			return recruit_list_;
		}

		const std::vector<potential_recruit> &recruit_list() const
		{
			return recruit_list_;
		}

	private:
		const team *target_;
		int gold_;
		std::vector<potential_recruit> extra_units_;
		std::vector<potential_recruit> recruit_list_;
};

static int average_resistance_against(const unit_type& a, const unit_type& b)
{
	gamemap &map_ = *resources::game_map;

	int weighting_sum = 0, defense = 0;
	const std::map<t_translation::t_terrain, size_t>& terrain =
		map_.get_weighted_terrain_frequencies();

	for (std::map<t_translation::t_terrain, size_t>::const_iterator j = terrain.begin(),
			j_end = terrain.end(); j != j_end; ++j)
	{
		// Use only reachable tiles when computing the average defense.
		if (a.movement_type().movement_cost(map_, j->first) < unit_movement_type::UNREACHABLE) {
			defense += a.movement_type().defense_modifier(map_, j->first) * j->second;
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
			defense += a.movement_type().defense_modifier(map_, jj->first) * jj->second;
			weighting_sum += jj->second;
		}
	}

	if(weighting_sum != 0) {
		defense /= weighting_sum;
	} else {
		LOG_AI << "The weighting sum is 0 and is ignored.\n";
	}

	LOG_AI << "average defense of '" << a.id() << "': " << defense << "\n";

	int sum = 0, weight_sum = 0;

	// calculation of the average damage taken
	bool steadfast = a.has_ability_by_id("steadfast");
	bool living = !a.not_living();
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
		if (living && cth != 0 && i->get_special_bool("poison", true)) {
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

static int compare_unit_types(const unit_type& a, const unit_type& b)
{
	const int a_effectiveness_vs_b = average_resistance_against(b,a);
	const int b_effectiveness_vs_a = average_resistance_against(a,b);

	LOG_AI << "comparison of '" << a.id() << " vs " << b.id() << ": "
		<< a_effectiveness_vs_b << " - " << b_effectiveness_vs_a << " = "
		<< (a_effectiveness_vs_b - b_effectiveness_vs_a) << '\n';
	return a_effectiveness_vs_b - b_effectiveness_vs_a;
}


//@todo: you really need weight there
static double get_unit_quality(const unit_type &info, fake_team &t, std::vector<potential_recruit> &extra_units, std::vector<fake_team> & fake_teams)
{
	const int hitpoints_const = 100;
	double score = 0;
	foreach(const unit &enemy_unit, *resources::units)
	{
		if(enemy_unit.can_recruit() || !t.is_enemy(enemy_unit.side()))
		{
			continue;
		}
		const unit_type *enemy_info = unit_types.find(enemy_unit.type_id());
		//VALIDATE(enemy_info, "Unknown unit type : " + enemy_unit->type_id() + " while updating recruit quality.");

		score += compare_unit_types(info, *enemy_info) * (enemy_unit.hitpoints() / enemy_unit.max_hitpoints()) * hitpoints_const;
	}
	foreach(fake_team &enemy_team, fake_teams)
	{
		if(!t.is_enemy(enemy_team.side()))
		{
			continue;
		}
		foreach(const potential_recruit &enemy_unit,  enemy_team.extra_units())
		{
			const unit_type *enemy_info = enemy_unit.type();
			//VALIDATE(enemy_info, "Unknown unit type : " + enemy_unit->name() + " while updating recruit quality.");

			score += compare_unit_types(info, *enemy_info) * hitpoints_const;
		}
	}
	foreach(const potential_recruit &extra_unit, extra_units)
	{
		if(!t.is_enemy(extra_unit.side()))
		{
			continue;
		}
		const unit_type *enemy_info = extra_unit.type();
		//VALIDATE(enemy_info, "Unknown unit type : " + enemy_unit->type_id() + " while updating recruit quality.");

		score += compare_unit_types(info, *enemy_info) * hitpoints_const;
	}

	return score;//@todo: divide by weight (if weight!=0)
}

static void update_recruit_qualities(fake_team &t, std::vector<potential_recruit> &extra_units, std::vector<fake_team> &fake_teams)
{
	foreach ( potential_recruit &recruit, t.recruit_list() )
	{
		double score = get_unit_quality(*recruit.type(),t,extra_units,fake_teams);
		recruit.set_quality(score);
	}
}

static int analyze_recruit_combat(fake_team &t, const unit_map &units, std::vector<potential_recruit> &extra_units)
{
	const int hitpoints_const = 100;
	int score = 0;
	for(unit_map::const_iterator test_unit = units.begin(); test_unit != units.end(); test_unit++)
	{
		if(test_unit->can_recruit() || test_unit->side() != t.side())
		{
			continue;
		}
		const unit_type *info = unit_types.find(test_unit->type_id());
		for(unit_map::const_iterator unit = units.begin(); unit != units.end(); unit++)
		{
			if (unit->can_recruit() || !t.is_enemy(unit->side()))
			{
				continue;
			}

			const unit_type *enemy_info = unit_types.find(unit->type_id());
			//VALIDATE(enemy_info, "Unknown unit type : " + unit->type_id() + " while scoring units.");

			//int weight = un.cost() * un.hitpoints() / un.max_hitpoints();
			//weighting += weight;

			//@todo: own hitpoints?
			score += compare_unit_types(*info, *enemy_info) * ((unit->hitpoints() / unit->max_hitpoints()) * hitpoints_const);
		}
		for(std::vector<potential_recruit>::const_iterator unit = extra_units.begin(); unit != extra_units.end(); unit ++)
		{
			if(!t.is_enemy(unit->side()))
			{
				continue;
			}
			const unit_type *enemy_info = (*unit).type();
			//VALIDATE(enemy_info, "Unknown unit type : " +(*unit).name() + " while scoring units (extra_unit).");

			//int weight = (*unit).cost();
			//weighting += weight;

			//@todo: own hitpoints?
			score += compare_unit_types(*info, *enemy_info) * hitpoints_const; // no hp-factor, because new units are expected to have full hp
		}
	}
	for(std::vector<potential_recruit>::const_iterator test_unit = extra_units.begin(); test_unit != extra_units.end(); test_unit ++)
	{
		if(test_unit->side() != t.side()){
			continue;
		}
		const unit_type *info = test_unit->type();
		for(unit_map::const_iterator unit = units.begin(); unit != units.end(); unit++)
		{
			if(unit->can_recruit() || !t.is_enemy(unit->side())){
				continue;
			}
			//unit const &un = *unit;
			const unit_type *enemy_info = unit_types.find(unit->type_id());
			//VALIDATE(enemy_info, "Unknown unit type : " + unit->type_id() + " while scoring units.");

			//int weight = un.cost() * un.hitpoints() / un.max_hitpoints();
			//weighting += weight;

			score += compare_unit_types(*info, *enemy_info) * ((unit->hitpoints() / unit->max_hitpoints()) * hitpoints_const);

		}
		for(std::vector<potential_recruit>::iterator unit = extra_units.begin(); unit != extra_units.end(); unit++)
		{
			if(!t.is_enemy(unit->side()))
			{
				continue;
			}
			const unit_type *enemy_info = unit->type();
			//VALIDATE(enemy_info, "Unknown unit type : " + unit->name() + " while scoring units (extra_unit).");
			score += compare_unit_types(*info, *enemy_info) * hitpoints_const;
		}
	}
	score /= hitpoints_const;
	return score;
}
//------------------------
struct potential_recruit_sorter
{
	potential_recruit_sorter():max_cost(0), max_quality(0), quality_factor(0)
	{
	}
	potential_recruit_sorter(int max_cost_, double max_quality_, double quality_factor_) : max_cost(max_cost_), max_quality(max_quality_), quality_factor(quality_factor_)
	{
	}
	bool operator()(const potential_recruit *a, const potential_recruit *b)
	{
		return (a->quality() * quality_factor - a->cost() * (1.0 - quality_factor) * max_quality / (double)max_cost) > (b->quality() * quality_factor - b->cost() * (1.0 - quality_factor) * max_quality / (double)max_cost);
	}

	int max_cost;
	double max_quality;
	double quality_factor;
};
static std::vector<potential_recruit> ai_choose_best_recruits(fake_team &t, int max_units_to_recruit, double quality_factor)
{
	std::map<std::string, int> current_units;
	foreach (const potential_recruit &i, t.extra_units())
	{
		current_units[(i.id())]++;
	}
	foreach (const unit &i, *resources::units)
	{
		current_units[(i.type_id())]++;
	}
	const std::vector<potential_recruit> &recruit_list = t.recruit_list();
	int gold = t.gold();
	std::vector<potential_recruit> recruits;
	if(recruit_list.empty())
	{
		return recruits;
	}
	double max_quality = recruit_list[0].quality();
	int max_cost = recruit_list[0].cost();
	std::vector<const potential_recruit*> sorted = std::vector<const potential_recruit*>();
	foreach(const potential_recruit &i, recruit_list)
	{
		if(i.cost() > max_cost)
		{
			max_cost = i.cost();
		}
		if(i.quality() > max_quality)
		{
			max_quality = i.quality();
		}

		sorted.push_back(&i);
	}
	potential_recruit_sorter sorter(max_cost, max_quality, quality_factor);
	std::sort(sorted.begin(), sorted.end(), sorter);
	int recruited = 0;
	foreach(const potential_recruit *i, sorted)
	{
		if(recruited < max_units_to_recruit)
		{
			int possible_amount = (int)(gold / i->cost());
			if(possible_amount > max_units_to_recruit - recruited)
			{
				possible_amount = max_units_to_recruit - recruited;
			}
			//if(possible_amount > (*i)->max_qty() - t.get_current_qty((*i)->name()) current_units[(*i)->name()])
			if(possible_amount > i->max_qty() -  current_units[i->id()])
			{
				//	possible_amount = (*i)->max_qty() - t.get_current_qty((*i)->name()) current_units[(*i)->name()];
				possible_amount = i->max_qty() - current_units[i->id()];
			}
			for(int j = 0; j < possible_amount; j++)
			{
				//recruit(t, *(*i));
				//(*i)->set_side(t.side());
				recruits.push_back(*i);
			}
			gold -= possible_amount * i->cost();
			recruited += possible_amount;
		}
		else
		{
			break;
		}
	}
	return recruits;
}
static void ai_choose_recruits(fake_team &t, int max_units_to_recruit, double quality_factor)
{
	std::vector<potential_recruit> recruits = ai_choose_best_recruits(t, max_units_to_recruit, quality_factor);
	foreach(potential_recruit &i, recruits) {
		t.fake_recruit(i);
	}

}
void testing_recruitment_phase::do_recruit(int max_units_to_recruit, double quality_factor)
{
	std::vector<fake_team> fake_teams;
	std::copy(resources::teams->begin(), resources::teams->end(), std::back_inserter(fake_teams));
	fake_team &ai_t = fake_teams[get_side()-1];

	//@todo: reorder fake_teams according to turn order

	if(ai_t.recruit_list().empty())
	{
		return;
	}

	for(int recruited_amount = 0; recruited_amount < max_units_to_recruit; recruited_amount++)
	{
		foreach(potential_recruit &recruit_type, ai_t.recruit_list() )
		{
			std::vector<potential_recruit> extra_units;
			extra_units.push_back(recruit_type);

			foreach(fake_team &t, fake_teams)
			{
				if(!ai_t.is_enemy(t.side())) //@todo:really, we might need to take allied recruiting into account, as well
				{
					continue;
				}
				t.reset();

				//@todo: enemy_max_units: for each enemy leader, find nearest keep, find free space near that keep, sum
				int enemy_max_units = 5;
				//@todo: enemy_quality_factor. will be taken later from parameter
				double enemy_quality_factor = 1.0;

				//update quality ratings for enemy
				update_recruit_qualities(t, extra_units,fake_teams);

				ai_choose_recruits(t, enemy_max_units, enemy_quality_factor);

			}
			double quality = get_unit_quality(*recruit_type.type(), ai_t, extra_units, fake_teams);
			recruit_type.set_quality(quality);
		}
		// choose the best unit
		std::vector<potential_recruit> recruit_result = ai_choose_best_recruits(ai_t, 1, quality_factor);
		if(recruit_result.empty())
		{
			break;
		}
		potential_recruit recruit_unit = recruit_result[0];

		if(ai_t.gold() >= recruit_unit.cost())
		{
			recruit_result_ptr recruit_action = check_recruit_action(recruit_unit.id());
			if (recruit_action->is_ok())
			{
				recruit_action->execute();
				if (!recruit_action->is_ok()){
					ERR_AI << "recruit failed" << std::endl;
				}
			} else {
				return;
			}
		}
	}
}

/*
   int main(int argc, char ** argv)
   {
//a small testcase
fake_team t;
t.set_gold(120);
t.add_unit("Spearman");
std::vector<potential_recruit> recruit_list;
recruit_list.push_back(potential_recruit("Spearman", 13, 12.6 ,10));
recruit_list.push_back(potential_recruit("Royal Guard", 30, 40.0, 2));
expect(t,"Spearman", 1);
expect(t,"Royal Guard", 0);
ai_choose_recruits(t, 5, 1, recruit_list);

ai_recruit(t, 5, 1, recruit_list);

//expect(t,"Spearman", 4);
//expect(t,"Royal Guard", 2);
return 0;
}
 */


void testing_recruitment_phase::execute()
{
	int max_units_to_recruit = 1;
	double quality_factor = 1.0;
	do_recruit(max_units_to_recruit, quality_factor);
}

} // end of namespace testing_ai_default

} // end of namespace ai

