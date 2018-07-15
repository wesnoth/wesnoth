/*
   Copyright (C) 2009 - 2018 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Defines formula ai candidate actions
 * */

#include "ai/formula/ai.hpp"
#include "ai/formula/candidates.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "units/unit.hpp"

static lg::log_domain log_formula_ai("ai/engine/fai");
#define ERR_AI LOG_STREAM(err, log_formula_ai)


namespace wfl {

base_candidate_action::base_candidate_action(const std::string& name,
		const std::string& type, const config& cfg,
		function_symbol_table* function_table) :
	name_(name),
	type_(type),
	eval_(new formula(cfg["evaluation"], function_table)),
	action_(new formula(cfg["action"], function_table)),
	score_(0)
{}

int base_candidate_action::execute_formula(const const_formula_ptr& formula,
			const formula_callable& callable, const ai::formula_ai* ai)
{
	int res = 0;
	try {
		res = (formula::evaluate(formula, callable)).as_int();
	} catch(const formula_error& e) {
		ai->handle_exception(e);
		res = 0;
	} catch(const type_error& e) {
		res = 0;
		ERR_AI << "formula type error while evaluating candidate action: " << e.message << std::endl;
	}

	return res;
}

candidate_action_with_filters::candidate_action_with_filters(
		const std::string& name, const std::string& type,
		const config& cfg, function_symbol_table* function_table)
	: base_candidate_action(name, type, cfg, function_table)
	, filter_map_()
{
	const config & filter_params = cfg.child("filter");

	if( filter_params ) {
		for(const config::attribute filter_param : filter_params.attribute_range())
		{
			const_formula_ptr filter_formula(
					new formula(filter_param.second, function_table));

			filter_map_[filter_param.first]=filter_formula;
		}
	}
}

variant candidate_action_with_filters::do_filtering(ai::formula_ai* ai, variant& input, const_formula_ptr formula)
{
	map_formula_callable callable(ai->fake_ptr());
	callable.add("input", input);

	return formula::evaluate(formula, callable);

}

move_candidate_action::move_candidate_action(const std::string& name,
		const std::string& type, const config& cfg,
		function_symbol_table* function_table)
	: candidate_action_with_filters(name, type, cfg, function_table)
	, my_unit_()
{}

void move_candidate_action::evaluate(ai::formula_ai* ai, unit_map& units)
{
	score_ = 0;

	candidate_action_filters::const_iterator me_filter = filter_map_.find("me");

	std::vector<variant> unit_vector;

	for(unit_map::unit_iterator i = units.begin() ; i != units.end() ; ++i)
	{
		if (i->side() == ai->get_side() && i->movement_left() > 0) {
			unit_vector.emplace_back(std::make_shared<unit_callable>(*i));
		}
	}

	variant my_units(unit_vector);

	variant filtered_units;
	try {
		if(me_filter != filter_map_.end() )
			filtered_units = do_filtering(ai, my_units, me_filter->second);
		else
			filtered_units=my_units;
	}
	catch(const formula_error& e) {
		ai->handle_exception(e, "Error while executing filter formula for '" + get_name() + "' Candidate Action");
		return;
	}

	for(variant_iterator i = filtered_units.begin() ; i != filtered_units.end() ; ++i)
	{
			map_formula_callable callable(ai->fake_ptr());
			callable.add("me", *i);

			int res = execute_formula(eval_, callable, ai);

			if(res > score_) {
				score_ = res;
				my_unit_ = *i;
			}
	}
}

void move_candidate_action::update_callable_map(map_formula_callable& callable)
{
	callable.add("me", my_unit_);
}

attack_candidate_action::attack_candidate_action(const std::string& name,
		const std::string& type, const config& cfg,
		function_symbol_table* function_table)
	: candidate_action_with_filters(name, type, cfg, function_table)
	, my_unit_()
	, enemy_unit_()
{}

void attack_candidate_action::evaluate(ai::formula_ai* ai, unit_map& units)
{
	score_ = 0;

	candidate_action_filters::const_iterator me_filter = filter_map_.find("me");
	candidate_action_filters::const_iterator target_filter = filter_map_.find("target");

	std::vector<variant> my_res, enemy_res;

	for(unit_map::unit_iterator i = units.begin() ; i != units.end() ; ++i)
	{
		if (i->side() == ai->get_side())
		{
			if (i->attacks_left()) {
				my_res.emplace_back(std::make_shared<unit_callable>(*i));
			}
		} else
		{
			if (ai->current_team().is_enemy(i->side()) && !i->incapacitated() && !i->invisible(i->get_location(), *resources::gameboard)) {
				enemy_res.emplace_back(std::make_shared<unit_callable>(*i));
			}
		}
	}
	variant my_units(my_res);
	variant enemy_units(enemy_res);

	variant filtered_my_units, filtered_enemy_units;
	try {
		if(me_filter != filter_map_.end() )
			filtered_my_units = do_filtering(ai, my_units, me_filter->second);
		else
			filtered_my_units = my_units;

		if(target_filter != filter_map_.end() )
			filtered_enemy_units = do_filtering(ai, enemy_units, target_filter->second);
		else
			filtered_enemy_units = enemy_units;
	}
	catch(const formula_error& e) {
		ai->handle_exception(e, "Error while executing filter formula for '" + get_name() + "' Candidate Action");
		return;
	}

	try{
		if( !(filtered_enemy_units.num_elements() && filtered_my_units.num_elements() ) )
			return;
	}
	catch(const type_error& e) {
		ERR_AI << "Error while executing filter formulas for '" + get_name() + "' Candidate Action: " << e.message << std::endl;
		return;
	}

	std::vector<variant> my_units_flt;
	std::vector<variant> enemy_units_flt;

	for(variant_iterator i = filtered_my_units.begin() ; i != filtered_my_units.end() ; ++i) {
		auto u_callable = (*i).try_convert<const unit_callable>();
		if(!u_callable) {
			ERR_AI << "ERROR in "<< get_name() << "Candidate Action: Filter formula returned table that does not contain units" << std::endl;
			return;
		}
		my_units_flt.emplace_back(u_callable);
	}

	for(variant_iterator i = filtered_enemy_units.begin() ; i != filtered_enemy_units.end() ; ++i) {
		auto u_callable = (*i).try_convert<const unit_callable>();
		if(!u_callable) {
			ERR_AI << "ERROR in "<< get_name() << "Candidate Action: Filter formula returned table that does not contain units" << std::endl;
			return;
		}
		enemy_units_flt.emplace_back(u_callable);
	}

	for( size_t my_unit = 0 ; my_unit < my_units_flt.size() ; ++my_unit){
		auto my_unit_callable = my_units_flt[my_unit].convert_to<unit_callable>();
		for( size_t enemy_unit = 0 ; enemy_unit < enemy_units_flt.size() ; ++enemy_unit){
			auto enemy_unit_callable = enemy_units_flt[enemy_unit].convert_to<unit_callable>();
			if(ai->can_reach_unit(my_unit_callable->get_location(), enemy_unit_callable->get_location())) {

				map_formula_callable callable(ai->fake_ptr());
				callable.add("me", filtered_my_units[my_unit]);
				callable.add("target", filtered_enemy_units[enemy_unit]);

				int res = execute_formula(eval_, callable, ai);

				if(res > score_) {
					score_ = res;
					my_unit_ = filtered_my_units[my_unit];
					enemy_unit_ = filtered_enemy_units[enemy_unit];
				}
			}
		}
	}
}

void attack_candidate_action::update_callable_map(map_formula_callable& callable)
{
	callable.add("me", my_unit_);
	callable.add("target", enemy_unit_);
}

}
