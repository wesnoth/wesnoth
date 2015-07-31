/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef AI_DEFAULT_AI_HPP_INCLUDED
#define AI_DEFAULT_AI_HPP_INCLUDED

#include "../interface.hpp"
#include "../composite/stage.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif


namespace pathfind {

struct plain_route;

} // of namespace pathfind


namespace ai {

class formula_ai;

/** A trivial ai that sits around doing absolutely nothing. */
class idle_ai : public readwrite_context_proxy, public interface {
public:
	idle_ai(readwrite_context &context, const config& /*cfg*/);
	void play_turn();
	void new_turn();
	std::string describe_self() const;
	void switch_side(side_number side);
	int get_recursion_count() const;
	virtual config to_config() const;
private:
	recursion_counter recursion_counter_;
};

class ai_default_recruitment_stage : public stage {
public:
	ai_default_recruitment_stage(ai_context &context, const config &cfg);
	virtual ~ai_default_recruitment_stage();
	void on_create();
	bool do_play_stage();
	config to_config() const;
	int get_combat_score(const unit_type& ut) const;

private:

	void get_combat_score_vs(const unit_type& ut, const std::string &enemy_type_id, int &score, int &weighting, int hitpoints, int max_hitpoints) const;

	virtual bool recruit_usage(const std::string& usage);


	class recruit_situation_change_observer : public events::observer {
	public:
		recruit_situation_change_observer();
		~recruit_situation_change_observer();

		void handle_generic_event(const std::string& /*event_name*/);

		bool get_valid();
		void set_valid(bool valid);
	private:
		bool valid_;

	};

	/**
	 * initialize recruitment recommendations
	 */
	void analyze_all();

	/**
	 * Analyze all the units that this side can recruit
	 * and rate their movement types.
	 * Ratings will be placed in 'unit_movement_scores_',
	 * with lower scores being better,
	 * and the lowest possible rating being '10'.
	 */
	virtual void analyze_potential_recruit_movements();

	std::string find_suitable_recall_id();

	std::map<std::string,int> best_usage_;

	config cfg_;

	std::map<std::string,int> maximum_counts_;

	std::set<std::string> not_recommended_units_;

	std::vector<std::pair<std::string,double> > recall_list_scores_;

	recruit_situation_change_observer recruit_situation_change_observer_;

	std::map<std::string,int> unit_combat_scores_;

	std::map<std::string,int> unit_movement_scores_;

	/**
	 * Analyze all the units that this side can recruit
	 * and rate their fighting suitability against enemy units.
	 * Ratings will be placed in 'unit_combat_scores_',
	 * with a '0' rating indicating that the unit is 'average' against enemy units,
	 * negative ratings meaning they are poorly suited,
	 * and positive ratings meaning they are well suited.
	 */
	virtual void analyze_potential_recruit_combat();


	bool analyze_recall_list();

	/**
	 * Rates two unit types for their suitability against each other.
	 * Returns 0 if the units are equally matched,
	 * a positive number if a is suited against b,
	 * and a negative number if b is suited against a.
	 */
	virtual int compare_unit_types(const unit_type& a, const unit_type& b) const;

	/**
	 * calculates the average resistance unit type a has against the attacks of
	 * unit type b.
	 */
	virtual int average_resistance_against(const unit_type& a, const unit_type& b) const;


};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif
