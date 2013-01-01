/*
   Copyright (C) 2009 - 2013 by Bartosz Waresiak <dragonking@o2.pl>
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
 * Defines formula ai candidate actions
 * */

#include "ai.hpp"
#include "candidates.hpp"
#include "../../log.hpp"
#include "../../resources.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_formula_ai("ai/engine/fai");
#define ERR_AI LOG_STREAM(err, log_formula_ai)


namespace game_logic {

void candidate_action_manager::load_config(const config& cfg, ai::formula_ai* ai, function_symbol_table* function_table)
{
	// register candidate actions
	BOOST_FOREACH(const config &rc_action, cfg.child_range("register_candidate_action"))
	{
		candidate_action_ptr new_ca = load_candidate_action_from_config(rc_action,ai,function_table);

		if (new_ca) {
			candidate_actions_.push_back(new_ca);
		}

	}
}

candidate_action_ptr candidate_action_manager::load_candidate_action_from_config(const config& rc_action, ai::formula_ai* ai, function_symbol_table* function_table)
{
	candidate_action_ptr new_ca;
	const t_string &name = rc_action["name"];
	try {
		const t_string &type = rc_action["type"];

		if( type == "movement") {
			new_ca = candidate_action_ptr(new move_candidate_action(name, type, rc_action, function_table ));
		} else if( type == "attack") {
			new_ca = candidate_action_ptr(new attack_candidate_action(name, type, rc_action, function_table ));
		} else {
			ERR_AI << "Unknown candidate action type: " << type << "\n";
		}
	} catch(formula_error& e) {
		ai->handle_exception(e, "Error while registering candidate action '" + name + "'");
	}
	return new_ca;
}

bool candidate_action_manager::evaluate_candidate_actions(ai::formula_ai* ai, unit_map& units)
{
	evaluated_candidate_actions_.clear();

	BOOST_FOREACH(candidate_action_ptr cm, candidate_actions_)
	{
		cm->evaluate(ai, units);
		evaluated_candidate_actions_.insert(cm);
	}

	if( evaluated_candidate_actions_.empty() ||
		(*evaluated_candidate_actions_.begin())->get_score() <= 0 ) //@note ai::candidate_action::BAD_SCORE )
		return false;

	return true;
}

base_candidate_action::base_candidate_action(const std::string& name,
		const std::string& type, const config& cfg,
		function_symbol_table* function_table) :
	name_(name),
	type_(type),
	eval_(new game_logic::formula(cfg["evaluation"], function_table)),
	action_(new game_logic::formula(cfg["action"], function_table)),
	score_(0)
{}

int base_candidate_action::execute_formula(const const_formula_ptr& formula,
			const game_logic::formula_callable& callable, const ai::formula_ai* ai)
{
	int res = 0;
	try {
		res = (formula::evaluate(formula, callable)).as_int();
	} catch(formula_error& e) {
		ai->handle_exception(e);
		res = 0;
	} catch(type_error& e) {
		res = 0;
		ERR_AI << "formula type error while evaluating candidate action: " << e.message << "\n";
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
		BOOST_FOREACH( const config::attribute filter_param, filter_params.attribute_range() )
		{
			game_logic::const_formula_ptr filter_formula(
					new game_logic::formula(filter_param.second, function_table));

			filter_map_[filter_param.first]=filter_formula;
		}
	}
}

variant candidate_action_with_filters::do_filtering(ai::formula_ai* ai, variant& input, game_logic::const_formula_ptr formula)
{
	game_logic::map_formula_callable callable(static_cast<const formula_callable*>(ai));
	callable.add_ref();
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

	std::vector<variant> res;

	for(unit_map::unit_iterator i = units.begin() ; i != units.end() ; ++i)
	{
		if (i->side() == ai->get_side() && i->movement_left() > 0) {
			res.push_back(variant(new unit_callable(*i)));
		}
	}

	variant my_units(&res);

	variant filtered_units;
	try {
		if(me_filter != filter_map_.end() )
			filtered_units = do_filtering(ai, my_units, me_filter->second);
		else
			filtered_units=my_units;
	}
	catch(formula_error& e) {
		ai->handle_exception(e, "Error while executing filter formula for '" + get_name() + "' Candidate Action");
		return;
	}

	for(variant_iterator i = filtered_units.begin() ; i != filtered_units.end() ; ++i)
	{
			game_logic::map_formula_callable callable(static_cast<const formula_callable*>(ai));
			callable.add_ref();
			callable.add("me", *i);

			int res = execute_formula(eval_, callable, ai);

			if(res > score_) {
				score_ = res;
				my_unit_ = *i;
			}
	}
}

void move_candidate_action::update_callable_map(game_logic::map_formula_callable& callable)
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
				my_res.push_back(variant(new unit_callable(*i)));
			}
		} else
		{
			if (ai->current_team().is_enemy(i->side()) && !i->incapacitated() && !i->invisible(i->get_location())) {
				enemy_res.push_back(variant(new unit_callable(*i)));
			}
		}
	}
	variant my_units(&my_res);
	variant enemy_units(&enemy_res);

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
	catch(formula_error& e) {
		ai->handle_exception(e, "Error while executing filter formula for '" + get_name() + "' Candidate Action");
		return;
	}

	try{
		if( !(filtered_enemy_units.num_elements() && filtered_my_units.num_elements() ) )
			return;
	}
	catch(type_error& e) {
		ERR_AI << "Error while executing filter formulas for '" + get_name() + "' Candidate Action: " << e.message << std::endl;
		return;
	}

	std::vector< const unit_callable* > my_units_flt;
	std::vector< const unit_callable* > enemy_units_flt;

	for(variant_iterator i = filtered_my_units.begin() ; i != filtered_my_units.end() ; ++i) {
		const unit_callable* u_callable = dynamic_cast<const unit_callable*>( (*i).as_callable() );
		if(u_callable == NULL) {
			ERR_AI << "ERROR in "<< get_name() << "Candidate Action: Filter formula returned table that does not contain units\n";
			return;
		}
		my_units_flt.push_back(u_callable);
	}

	for(variant_iterator i = filtered_enemy_units.begin() ; i != filtered_enemy_units.end() ; ++i) {
		const unit_callable* u_callable = dynamic_cast<const unit_callable*>( (*i).as_callable() );
		if(u_callable == NULL) {
			ERR_AI << "ERROR in "<< get_name() << "Candidate Action: Filter formula returned table that does not contain units\n";
			return;
		}
		enemy_units_flt.push_back(u_callable);
	}

	for( size_t my_unit = 0 ; my_unit < my_units_flt.size() ; ++my_unit){
		const unit_callable* my_unit_callalbe = my_units_flt[my_unit];
		for( size_t enemy_unit = 0 ; enemy_unit < enemy_units_flt.size() ; ++enemy_unit){
			if( ai->can_reach_unit( my_unit_callalbe->get_location(), enemy_units_flt[enemy_unit]->get_location() )) {

				game_logic::map_formula_callable callable(static_cast<const formula_callable*>(ai));
				callable.add_ref();
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

void attack_candidate_action::update_callable_map(game_logic::map_formula_callable& callable)
{
	callable.add("me", my_unit_);
	callable.add("target", enemy_unit_);
}

}
