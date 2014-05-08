/*
   Copyright (C) 2009 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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
* Experimental recruitment phase by Floris Kint
*/

#include "ca_testing_recruitment.hpp"
#include "../actions.hpp"
#include "../manager.hpp"
#include "../composite/engine.hpp"
#include "../composite/rca.hpp"
#include "../composite/stage.hpp"
#include "../../gamestatus.hpp"
#include "../../log.hpp"
#include "../../map.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"
#include "../../wml_exception.hpp"
#include "../../pathfind/pathfind.hpp"

#include <boost/foreach.hpp>

#include <numeric>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>


namespace ai {

namespace testing_ai_default {

static lg::log_domain log_aitesting("aitesting");
#define LOG_AIT LOG_STREAM(info, log_aitesting)
//If necessary, this define can be replaced with `#define LOG_AIT std::cout` to restore previous behavior

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
			assert(type);
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
         , gold_(0)
         , extra_units_()
         , recruit_list_()
      {
         reset();
      }

      void reset()
      {
         extra_units_.clear();
         gold_ = target_->gold();
         recruit_list_.clear();
         std::transform(target_->recruits().begin(),target_->recruits().end(),std::back_inserter(recruit_list_), potential_recruit_converter(999,side()));//999 - unlimited recruits of this type
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
         DBG_AI << " Fake recruiting [" << r.id() << "] for side [" << side() << "]"<< std::endl;
         spend_gold(r.cost());
         extra_units_.push_back(r);//@todo: give an id if needed
      }

      std::vector<potential_recruit>& extra_units()
      {
         return extra_units_;
      }

      int get_current_qty(const potential_recruit &r) const
      {
         return get_current_qty(r.id());//@todo: fix
      }

      int get_current_qty(const std::string &name) const
      {
         int counter = 0;
         BOOST_FOREACH(unit &un, *resources::units){
            if(un.side() == side() && un.type().base_id() == name) // @todo: is base_id good?
            {
               counter++;
            }
         }
         return std::count(extra_units_.begin(), extra_units_.end(), name) + counter;//@todo: fix
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

      std::vector<potential_recruit>& recruit_list()
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
      LOG_AI << "The weighting sum is 0 and is ignored.\n";
   }

   //LOG_AI << "average defense of '" << a.id() << "': " << defense << "\n";

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


static int compare_unit_types(const unit_type& a, const unit_type& b)
{
	const int a_effectiveness_vs_b = average_resistance_against(b,a);
	const int b_effectiveness_vs_a = average_resistance_against(a,b);

	DBG_AI << "    comparison of '" << a.id() << " vs " << b.id() << ": "
	       << a_effectiveness_vs_b << " - " << b_effectiveness_vs_a << " = "
	       << (a_effectiveness_vs_b - b_effectiveness_vs_a) << '\n';
	return a_effectiveness_vs_b - b_effectiveness_vs_a;
}


/*static double get_unit_quality(const unit_type &info, fake_team &t, std::vector<fake_team> & fake_teams)
{
   const int hitpoints_const = 100;
   double score = 0;
   double total_weight = 0;
   BOOST_FOREACH(const unit &enemy_unit, *resources::units)
   {
      if(enemy_unit.can_recruit() || !t.is_enemy(enemy_unit.side()))
      {
         continue;
      }
      const unit_type *enemy_info = unit_types.find(enemy_unit.type_id());
      double weight = enemy_unit.hitpoints() * hitpoints_const / enemy_unit.max_hitpoints();
      total_weight += weight;
      VALIDATE(enemy_info, "Unknown unit type : " + enemy_unit.type_id() + " while updating recruit quality.");

      score += compare_unit_types(info, *enemy_info) * weight;
   }
   BOOST_FOREACH(fake_team &enemy_team, fake_teams)
   {
      if(!t.is_enemy(enemy_team.side()))
      {
         continue;
      }
      BOOST_FOREACH(const potential_recruit &enemy_unit,  enemy_team.extra_units())
      {
         const unit_type *enemy_info = enemy_unit.type();
         VALIDATE(enemy_info, "Unknown unit type : " + enemy_unit.id() + " while updating recruit quality.");

         total_weight += hitpoints_const;
         score += compare_unit_types(info, *enemy_info) * hitpoints_const;
      }
   }

   if(total_weight != 0)
   {
      return score / total_weight;
   }
   else
   {
      return score;
   }
}*/

/*static void update_recruit_qualities(fake_team &t, std::vector<fake_team> &fake_teams)
{
   BOOST_FOREACH( potential_recruit &recruit, t.recruit_list() )
   {
      double score = get_unit_quality(*recruit.type(),t,fake_teams);
      recruit.set_quality(score);
   }
}*/

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
      return (a->quality() * quality_factor - a->cost()
			* (1.0 - quality_factor) * max_quality
			/ static_cast<double>(max_cost))
		> (b->quality() * quality_factor - b->cost() * (1.0 - quality_factor)
			* max_quality / static_cast<double>(max_cost));
   }

   int max_cost;
   double max_quality;
   double quality_factor;
};
static std::vector<potential_recruit> ai_choose_best_recruits(fake_team &t, int max_units_to_recruit, double quality_factor, bool counter_recruit)
{
	LOG_AI << "Running simple "<< (counter_recruit ? "counter-":"")<< "recruit selection algorithm for side "<< t.side() << std::endl;
	std::vector<potential_recruit> recruits;
	const std::vector<potential_recruit> &recruit_list = t.recruit_list();
	if(recruit_list.empty())
	{
		return recruits;
	}

   std::map<std::string, int> current_units;
   BOOST_FOREACH(const potential_recruit &i, t.extra_units())
   {
      current_units[(i.id())]++;
   }
   BOOST_FOREACH(const unit &i, *resources::units)
   {
	if (i.side()==t.side())
	{
		current_units[i.type().base_id()]++;
	}
   }
   int gold = t.gold();
   double max_quality = recruit_list[0].quality();
   int max_cost = recruit_list[0].cost();
   std::vector<const potential_recruit*> sorted = std::vector<const potential_recruit*>();
   BOOST_FOREACH(const potential_recruit &i, recruit_list)
   {
      if(i.cost() > max_cost)
      {
         max_cost = i.cost();
      }
      if(i.quality() > max_quality)
      {
         max_quality = i.quality();
      }

      LOG_AI <<"IN "<< (counter_recruit ? "COUNTER-" : "") <<"RECRUIT: side=["<<t.side()<<"] unit=["<<i.id()<<"] quality=["<<i.quality()<<"] cost=["<<i.cost()<<"]"<< std::endl;
      sorted.push_back(&i);
   }
   potential_recruit_sorter sorter(max_cost, max_quality, quality_factor);
   std::sort(sorted.begin(), sorted.end(), sorter);
   int recruited = 0;
   BOOST_FOREACH(const potential_recruit *i, sorted)
   {
      if(recruited < max_units_to_recruit)
      {
         int possible_amount = gold / i->cost();
         if(possible_amount > max_units_to_recruit - recruited)
         {
            possible_amount = max_units_to_recruit - recruited;
         }
         if(possible_amount > i->max_qty() -  current_units[i->id()])
         {
            possible_amount = i->max_qty() - current_units[i->id()];
         }
         for(int j = 0; j < possible_amount; j++)
         {
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
   LOG_AI << "Finished simple recruit selection algorithm for side "<< t.side() << std::endl;
   return recruits;
}
static void ai_choose_recruits(fake_team &t, int max_units_to_recruit, double quality_factor, bool counter_recruit)
{
	std::vector<potential_recruit> recruits = ai_choose_best_recruits(t, max_units_to_recruit, quality_factor, counter_recruit);
   BOOST_FOREACH(potential_recruit &i, recruits) {
      t.fake_recruit(i);
   }

}

/*struct unit_data{
   unit_data(const std::string _id, const unit_type *_type):id(_id), type(*_type)
   {}
   const std::string id;
   const unit_type &type;
};*/
//class defender_pair_type{
//public:
//   //defender_pair_type(const unit_data *_defender):defender(_defender){}
//   defender_pair_type(const unit_type *_defender):defender(_defender){}
////   ~defender_pair_type(){
////      delete defender;
////      for(unsigned int i = 0; i < enemies.size(); i++){
////         delete enemies[i];
////      }
////   }
////   const unit_data* defender;
////   std::vector<unit_data*> enemies;
////   void add_enemy(unit_data *data){
////      enemies.push_back(data);
////   }
//   const unit_type *defender;
//   std::vector<const unit_type*> enemies;
//   void add_enemy(const unit_type *type){
//      enemies.push_back(type);
//   }
//};
//class enemy_pair_type{
//public:
////   enemy_pair_type(const unit_data *_enemy):enemy(_enemy){
////      score = 0;
////
////   }
////   ~enemy_pair_type(){
////      delete enemy;
////      for(unsigned int i = 0; i < defenders.size(); i++){
////         delete defenders[i];
////      }
////   }
////   void add_defender(unit_data *data){
////      defenders.push_back(data);
////   }
////   const unit_data* enemy;
////   std::vector<unit_data*> defenders;
//   enemy_pair_type(const unit_type *_enemy):enemy(_enemy){
//      score = 0;
//   }
//   void add_defender(const unit_type *defender){
//      defenders.push_back(defender);
//   }
//   const unit_type *enemy;
//   std::vector<const unit_type*> defenders;
//   double score;
//};

static void get_recruit_qualities(std::vector<potential_recruit> &recruit_list, fake_team &t, std::vector<fake_team> &fake_teams)
{
   //DBG_AI << "start of get_recruit_qualities" << std::endl;
   typedef std::map<const unit_type*, std::vector<double> > unit_map;
   unit_map enemies;
   BOOST_FOREACH(unit &un, *resources::units){
	   if(t.is_enemy(un.side()) && !un.can_recruit()){
		   enemies[&un.type()].push_back(
			  static_cast<double>(un.hitpoints())
			/ static_cast<double>(un.max_hitpoints()));
	   }
   }
   DBG_AI << "before extra_units of fake_teams: enemies.size() = " << enemies.size() << std::endl;
   BOOST_FOREACH(fake_team &tmp_t, fake_teams)
   {
	   if (t.is_enemy(tmp_t.side())) {
		   BOOST_FOREACH(potential_recruit &rec, tmp_t.extra_units())
		   {
			   enemies[rec.type()].push_back(1.0);
		   }
	   }
   }
   DBG_AI << "after extra_units of fake_teams: enemies.size() = " << enemies.size() << std::endl;

   BOOST_FOREACH(potential_recruit &rec, recruit_list) {
	   double score = 0;
	   double weighting = 0;
	   BOOST_FOREACH(unit_map::value_type &enemy, enemies) {
		   double hitpoints_sum = std::accumulate(enemy.second.begin(),enemy.second.end(),0);
		   score += compare_unit_types(*rec.type(), *enemy.first) * hitpoints_sum;
		   weighting += hitpoints_sum;
	   }
	   if (weighting!=0) {
		   score /= weighting;
	   }
	   rec.set_quality(score);
	   LOG_AI <<"side=["<<t.side()<<"] unit=["<<rec.id()<<"] quality=["<<rec.quality()<<"] cost=["<<rec.cost()<<"]"<< std::endl;
   }

}

/*
class unit_type_health{
public:
   unit_type_health(const unit_type *type, double health):type_(type), health_(health){

   }
   double health(){return health_;}
   const unit_type* type(){return type_;}
protected:
   const unit_type* type_;
   double health_;
};
*/

static void get_recruit_qualities(fake_team &t, std::vector<fake_team> &fake_teams)
{
	get_recruit_qualities(t.recruit_list(),t,fake_teams);
}

static void get_recruit_quality(potential_recruit &rec, fake_team &t, std::vector<fake_team> &fake_teams)
{
	std::vector<potential_recruit> recruits;
	recruits.push_back(rec);
	get_recruit_qualities(recruits,t,fake_teams);
	rec.set_quality(recruits[0].quality());
}

/*static double get_combat_score2(fake_team &t, std::vector<fake_team> &fake_teams)
{
   std::vector<defender_pair_type*> defenders;
   std::vector<enemy_pair_type*> enemies;
   BOOST_FOREACH(unit &un, *resources::units)
   {
      if(t.is_enemy(un.side()))
      {
//         const unit_type &enemy_type = *un.type();
         //enemy_pair_type *pair = new enemy_pair_type(new unit_data(un.id(), un.type()));
         enemy_pair_type *pair = new enemy_pair_type(un.type());
         BOOST_FOREACH(unit &defender, *resources::units)
         {
            if(!t.is_enemy(defender.side())){
               //int score = compare_unit_types(*defender.type(), enemy_type);
               //if(score >= 0)
               //{
                  //pair->add_defender(new unit_data(defender.id(), defender.type()));
               pair->add_defender(defender.type());
               //}
            }
         }
         BOOST_FOREACH(fake_team &tmp_t, fake_teams){
            if(!t.is_enemy(tmp_t.side())){
               BOOST_FOREACH(potential_recruit &rec, tmp_t.extra_units()){
         //         int score = compare_unit_types(*rec.type(), enemy_type);
         //         if(score >= 0){
                     //pair->add_defender(new unit_data(rec.id(), rec.type()));
                     pair->add_defender(rec.type());
         //         }
               }
            }
         }
         enemies.push_back(pair);
      }
      else
      {
         //const unit_type &defender_type = *un.type();
         //defender_pair_type *pair = new defender_pair_type(new unit_data(un.id(), un.type()));
         defender_pair_type *pair = new defender_pair_type(un.type());
         BOOST_FOREACH(unit &enemy, *resources::units)
         {
            //int score = compare_unit_types(defender_type, *enemy.type());
            //if(score >= 0)
            //{
               //pair->add_enemy(new unit_data(enemy.id(), enemy.type()));
            pair->add_enemy(enemy.type());
            //}
         }
         BOOST_FOREACH(fake_team &tmp_t, fake_teams){
            if(t.is_enemy(tmp_t.side())){
               BOOST_FOREACH(potential_recruit &rec, tmp_t.extra_units()){
                  //int score = compare_unit_types(defender_type, *rec.type());
                  //if(score >= 0){
                     //pair->add_enemy(new unit_data(rec.id(), rec.type()));
                  pair->add_enemy(rec.type());
                  //}
               }
            }
         }
         defenders.push_back(pair);
      }
   }
   BOOST_FOREACH(fake_team &tmp_t, fake_teams)
   {
      BOOST_FOREACH(potential_recruit &rec, tmp_t.extra_units()){
         if(t.is_enemy(tmp_t.side())){
            //const unit_type &enemy_type = *rec.type();
            //enemy_pair_type *pair = new enemy_pair_type(new unit_data(rec.id(), rec.type()));
            enemy_pair_type *pair = new enemy_pair_type(rec.type());
            BOOST_FOREACH(unit &defender, *resources::units){
               if(t.is_enemy(defender.side())){
                  continue;
               }
               //int score = compare_unit_types(*defender.type(), enemy_type);
               //if(score >= 0){
                  //pair->add_defender(new unit_data(defender.id(), defender.type()));
               pair->add_defender(defender.type());
               //}
            }
            //HIER
            BOOST_FOREACH(fake_team &sub_t, fake_teams){
               if(t.is_enemy(sub_t.side())){
                  continue;
               }
               BOOST_FOREACH(potential_recruit &sub_rec, sub_t.extra_units()){
                  //int score = compare_unit_types(*sub_rec.type(), enemy_type);
                  //if(score >= 0){
                     //pair->add_defender(new unit_data(sub_rec.id(), sub_rec.type()));
                  pair->add_defender(sub_rec.type());
                  //}
               }
            }
            enemies.push_back(pair);
         }else{
            //const unit_type &defender_type = *rec.type();
            //defender_pair_type *pair = new defender_pair_type(new unit_data(rec.id(), rec.type()));
            defender_pair_type *pair = new defender_pair_type(rec.type());
            BOOST_FOREACH(unit &enemy, *resources::units)
            {
               if(!t.is_enemy(enemy.side())){
                  continue;
               }
            //   int score = compare_unit_types(defender_type, *enemy.type());
            //   if(score >= 0){
                  //pair->add_enemy(new unit_data(rec.id(), rec.type()));
               pair->add_enemy(rec.type());
            //   }
            }
            BOOST_FOREACH(fake_team &sub_t, fake_teams){
               if(!t.is_enemy(sub_t.side())){
                  continue;
               }
               BOOST_FOREACH(potential_recruit &sub_rec, sub_t.extra_units()){
               //   int score = compare_unit_types(defender_type, *sub_rec.type());
               //   if(score >= 0){
                     //pair->add_enemy(new unit_data(sub_rec.id(), sub_rec.type()));
                  pair->add_enemy(sub_rec.type());
               //   }
               }
            }
            defenders.push_back(pair);
         }
      }
   }
   double min_score = 0;
   double max_score = 0;
   BOOST_FOREACH(enemy_pair_type *pair, enemies)
   {
//      if(pair->defenders.size() == 0)
//      {
//         pair->score = -10000;
//      }else{
         //BOOST_FOREACH(unit_data *defender, pair->defenders)
         BOOST_FOREACH(unit_type *defender, pair->defenders)
         {
            unsigned int defender_enemies = 0;
            BOOST_FOREACH(defender_pair_type *defender_p, defenders)
            {
               //if(defender->id == defender_p->defender->id){
               if(defender->type_name()() == defender_p->defender->type_name()){
                  defender_enemies = defender_p->enemies.size();
                  break;
               }
            }
            //double tmpscore =(compare_unit_types(defender->type, pair->enemy->type) / ((defender_enemies != 0) ? defender_enemies : 1 ));
            double tmpscore =(compare_unit_types(defender, pair->enemy) / ((defender_enemies != 0) ? defender_enemies : 1 ));
            if(tmpscore > 0){
               pair->score += tmpscore;
               LOG_AIT << defender->type_name() << " resistance against " << pair->enemy->type_name() << " is: " << tmpscore << ", new score = " << pair->score << std::endl;
            }
         }
         if(pair->score > max_score)
            max_score = pair->score;
         if(pair->score < min_score)
            min_score = pair->score;
         LOG_AIT << pair->enemy->id << " resistance = " << pair->score << std::endl;
      //}
   }
   double score = 0;
   score -= max_score - min_score;
   BOOST_FOREACH(enemy_pair_type *pair, enemies)
   {
      score += pair->score;
   }

   for(unsigned int i = 0; i < enemies.size(); i++)
   {
      delete enemies[i];
   }
   for(unsigned int i = 0; i < defenders.size(); i++)
   {
      delete defenders[i];
   }
   return score;
}*/
/*static double get_combat_score(fake_team &t, std::vector<fake_team> &fake_teams)
{
   typedef std::map<const unit_type*, std::vector<double> > unit_map;
   unit_map enemies;
   unit_map defenders;
   BOOST_FOREACH(unit &un, *resources::units){
      if(t.is_enemy(un.side())){
         enemies[un.type()].push_back((double)un.hitpoints() / (double)un.max_hitpoints());
      }else{
         defenders[un.type()].push_back(un.hitpoints() / un.max_hitpoints());
      }
   }
   BOOST_FOREACH(fake_team &tmp_t, fake_teams)
   {
      BOOST_FOREACH(potential_recruit &rec, tmp_t.extra_units())
      {
         if(t.is_enemy(tmp_t.side())){
            enemies[rec.type()].push_back(1.0);
         }else{
            defenders[rec.type()].push_back(1.0);
         }
      }
   }
   double result = 0;
   BOOST_FOREACH(unit_map::value_type &defender, defenders)
   {
      double defenders_score = 0;
      BOOST_FOREACH(unit_map::value_type &enemy, enemies)
      {
         double hitpoints_sum = 0;
         BOOST_FOREACH(double i, enemy.second)
         {
            hitpoints_sum += i;
         }
         defenders_score += compare_unit_types(*defender.first, *enemy.first) / ((hitpoints_sum == 0)?1:hitpoints_sum);

      }
      double hitpoints_sum = 0;
      BOOST_FOREACH(double i, defender.second)
      {
         hitpoints_sum += i;
      }
      defenders_score *= hitpoints_sum;

      result += defenders_score;
   }
   return result;
//   vector<defender_pair> defenders;
//   vector<enemy_pair> enemies;
//   //BOOST_FOREACH(unit &un, *resources::units)
//   //{
//   //   if(t.is_enemy(un.side()))
//   //   {
//   //      enemies.push_back(un.id());
//   //   }else{
//   //      defenders.push_back(un.id());
//   //   }
//   //}
//   BOOST_FOREACH(unit &un, *resources::units)
//   {
//      if(t.is_enemy(un.side()))
//      {
//         const unit_type &enemy_type = un.type();
//         enemy_pair pair;
//         pair.enemy = new unit_data(un.id(), enemy_type);
//         BOOST_FOREACH(unit &defender, *resources::units)
//         {
//            int score = compare_unit_types(defender.type(), enemy_type);
//            if(score >= 0)
//            {
//               pair.defenders.push_back(new unit_data(defender.id(), defender.type()));
//            }
//         }
//         defenders.push_back(pair);
//      }
//      else
//      {
//         const unit_type &defender_type = un.type();
//         defender_pair pair;
//         pair.defender = new unit_data(defender.id(), defender_type);
//         BOOST_FOREACH(unit &enemy, *resources::units)
//         {
//            int score = compare_unit_types(defender_type, enemy.type());
//            if(score >= 0)
//            {
//               pair.enemies.push_back(new unit_data(enemy.id(), enemy.type()));
//               //pair.score += score;
//            }
//         }
//         enemies.push_back(pair);
//      }
//   }
//   BOOST_FOREACH(fake_team &tmp_t, fake_teams)
//   {
//      if(t.is_enemy(tmp_t.side())){
//         BOOST_FOREACH(potential_recruit &rec, tmp_t.extra_units())
//         {
//
//         }
//      }else{
//
//      }
//
//   }
//   //std::vector<unit> no_defense_enemies;
//   BOOST_FOREACH(enemy_pair &pair, enemies)
//   {
//      if(pair.defenders.size() == 0)
//      {
//         //no_defense_enemies.push_back(pair.enemy);
//
//         pair.score = 0;
//      }else{
//         BOOST_FOREACH(unit_data &defender, pair.defenders)
//         {
//            //unit_type &defender = defender_data.type;
//            unsigned int defender_enemies = 0;
//            BOOST_FOREACH(defender_pair &defender_p, defenders)
//            {
//               if(defender.id == defender_p.defender.id){
//                  defender_enemies = defender_p.enemies.size();
//                  break;
//               }
//            }
//
//            pair.score += (compare_unit_types(defender.type, pair.enemy.type) / ((defender_enemies != 0) ? defender_enemies : 1 ));
//         }
//      }
//   }
//   //enemy_pair *worst_pair = &enemies[0];
//   double total_score = 0;
//   BOOST_FOREACH(enemy_pair &pair, enemies)
//   {
//      total_score += pair.score;
//   }
//   return worst_pair->enemy;
}*/
//static void check_worst_defense(fake_team &t)
//{
//   //std::vector<unit_pair> pairs;
//   //unit &worst_def_unit;
//   vector<defender_pair> defenders;
//   vector<enemy_pair> enemies;
//   /*BOOST_FOREACH(unit &un, *resources::units)
//   {
//      if(t.is_enemy(un.side()))
//      {
//         enemies.push_back(un.id());
//      }else{
//         defenders.push_back(un.id());
//      }
//   }*/
//   BOOST_FOREACH(unit &un, *resources::units)
//   {
//      if(t.is_enemy(un.side()))
//      {
//         const unit_type &enemy_type = un.type();
//         enemy_pair pair;
//         pair.enemy = un;
//         BOOST_FOREACH(unit &defender, *resources::units)
//         {
//            int score = compare_unit_types(defender.type(), enemy_type);
//            if(score >= 0)
//            {
//               pair.defenders.push_back(defender);
//            }
//         }
//         defenders.push_back(pair);
//      }
//      else
//      {
//         const unit_type &defender_type = un.type();
//         defender_pair pair;
//         pair.defender = un;
//         BOOST_FOREACH(unit &enemy, *resources::units)
//         {
//            int score = compare_unit_types(defender_type, enemy.type());
//            if(score >= 0)
//            {
//               pair.enemies.push_back(enemy);
//               //pair.score += score;
//            }
//         }
//         enemies.push_back(pair);
//      }
//   }
//   //std::vector<unit> no_defense_enemies;
//   BOOST_FOREACH(enemy_pair &pair, enemies)
//   {
//      if(pair.defenders.size() == 0)
//      {
//         //no_defense_enemies.push_back(pair.enemy);
//         return pair.enemy;
//         //pair.score = 0;
//      }else{
//         BOOST_FOREACH(unit &defender, pair.defenders)
//         {
//            unsigned int defender_enemies = 0;
//            BOOST_FOREACH(defender_pair &defender_p, defenders)
//            {
//               if(defender.id() == defender_p.defender.id()){
//                  defender_enemies = defender_p.enemies.size();
//                  break;
//               }
//            }
//
//            pair.score += (compare_unit_types(defender, pair.enemy) / ((defender_enemies != 0) ? defender_enemies : 1 ));
//         }
//      }
//   }
//   enemy_pair *worst_pair = &enemies[0];
//   BOOST_FOREACH(enemy_pair &pair, enemies)
//   {
//      if(pair.score < worst_pair.score)
//      {
//         worst_pair = &pair;
//      }
//   }
//   return worst_pair->enemy;
//}

void testing_recruitment_phase::do_recruit(int max_units_to_recruit, double quality_factor)
{
   std::vector<fake_team> tmp_fake_teams (resources::teams->begin(), resources::teams->end());
   std::vector<fake_team> fake_teams;
   fake_team *ai_t = 0;
   for(int i = get_side() - 1
		   ; static_cast<unsigned int>(i) < tmp_fake_teams.size()
		   ; i++)
   {
      fake_teams.push_back(tmp_fake_teams[i]);

   }
   for(int i = 0; i < get_side() - 1; i++)
   {
      fake_teams.push_back(tmp_fake_teams[i]);
   }
   ai_t = &fake_teams[0];
   if(ai_t->recruit_list().empty())
   {
      return;
   }
   for(int recruited_amount = 0; recruited_amount < max_units_to_recruit; recruited_amount++)
   {

      BOOST_FOREACH(fake_team &t, fake_teams)
      {
         t.reset();
      }

      std::vector<potential_recruit> ai_recruit_list = ai_t->recruit_list();

      BOOST_FOREACH(potential_recruit &recruit_type, ai_recruit_list)
      {
         BOOST_FOREACH(fake_team &t, fake_teams)
         {
            t.reset();
         }

         if(ai_t->gold() < recruit_type.cost())
         {
            continue;
         }
         if(ai_t->get_current_qty(recruit_type) >= recruit_type.max_qty())
         {
            continue;
         }
         LOG_AI << "Pretend that we recruited: " << recruit_type.id() << std::endl;
         ai_t->fake_recruit(recruit_type);
         BOOST_FOREACH(fake_team &t, fake_teams)
         {
            if(ai_t->side() == t.side())
            {
               continue;
            }
            LOG_AI << "evaluating reaction of fake_team " << t.side() << std::endl;

            //@todo: enemy_max_units: for each enemy leader, find nearest keep, find free space near that keep, sum
            int enemy_max_units = 5;
            //@todo: enemy_quality_factor. will be taken later from parameter
            double enemy_quality_factor = 1.0;

            //update quality ratings for enemy
            get_recruit_qualities(t, fake_teams);

            ai_choose_recruits(t, enemy_max_units, enemy_quality_factor, true);

         }
         get_recruit_quality(recruit_type,*ai_t, fake_teams);
      }
      ai_t->recruit_list() = ai_recruit_list;
      // choose the best unit
      std::vector<potential_recruit> recruit_result = ai_choose_best_recruits(*ai_t, 1, quality_factor,false);
      if(recruit_result.empty())
      {
         LOG_AIT << "recruit_result = empty" << std::endl;
         break;
      }
      const potential_recruit &recruit_unit = recruit_result[0];
      LOG_AIT << "recruit: " << recruit_unit.id() << std::endl;
      if(ai_t->gold() >= recruit_unit.cost())
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
      else
      {
         LOG_AIT << "gold not ok" << std::endl;
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
   LOG_AIT << "execute floris' recruitment algorithm" << std::endl;
   int max_units_to_recruit = 1;
   double quality_factor = 1.0;
   do_recruit(max_units_to_recruit, quality_factor);
}

} // end of namespace testing_ai_default

} // end of namespace ai
