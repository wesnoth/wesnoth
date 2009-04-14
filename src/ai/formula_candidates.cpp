/*
   Copyright (C) 2009 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/formula_candidates.cpp
 * Defines formula ai candidate actions
 * */

#include "formula_ai.hpp"
#include "formula_candidates.hpp"
#include "../log.hpp"

#define LOG_AI LOG_STREAM(info, formula_ai)
#define WRN_AI LOG_STREAM(warn, formula_ai)
#define ERR_AI LOG_STREAM(err, formula_ai)

namespace game_logic {

void candidate_action_manager::load_config(const config& cfg, formula_ai* ai, function_symbol_table* function_table)
{
	// register candidate actions
	foreach (const config &rc_action, cfg.child_range("register_candidate_action"))
	{
		const t_string &name = rc_action["name"];

		try{
			const t_string &type = rc_action["type"];

			candidate_action_ptr new_ca;

			if( type == "movement") {
				new_ca = candidate_action_ptr(new move_candidate_action(name, type, rc_action, function_table ));
			} else if( type == "attack") {
				new_ca = candidate_action_ptr(new attack_candidate_action(name, type, rc_action, function_table ));
			} else if( type == "support") {
				new_ca = candidate_action_ptr(new support_candidate_action(name, type, rc_action, function_table ));
			} else {
				ERR_AI << "Unknown candidate action type: " << type << "\n";
				continue;
			}
				

			candidate_actions_.push_back(new_ca);

		}
		catch(formula_error& e) {
			ai->handle_exception(e, "Error while registering candidate action '" + name + "'");
		}
	}
}

bool candidate_action_manager::evaluate_candidate_actions(formula_ai* ai, unit_map& units)
{
	evaluated_candidate_actions_.clear();

	foreach(candidate_action_ptr cm, candidate_actions_)
	{
		cm->evaluate(ai, units);
		evaluated_candidate_actions_.insert(cm);
	}

	if( evaluated_candidate_actions_.empty() ||
		(*evaluated_candidate_actions_.begin())->get_score() < 1 )
		return false;
	
	return true;
}

base_candidate_action::base_candidate_action(const std::string& name,const std::string& type,const config& cfg, function_symbol_table* function_table) :
	name_(name),
	type_(type),
	eval_(new game_logic::formula(cfg["evaluation"], function_table)),
	action_(new game_logic::formula(cfg["action"], function_table))
{}

int base_candidate_action::execute_formula(const const_formula_ptr& formula,
			const game_logic::formula_callable& callable, const formula_ai* ai)
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

candidate_action_with_filters::candidate_action_with_filters(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table) :
	base_candidate_action(name, type, cfg, function_table)
{
	const config & filter_params = cfg.child("filter");

	if( filter_params ) {
		foreach( const config::attribute filter_param, filter_params.attribute_range() )
		{
			game_logic::const_formula_ptr filter_formula(
					new game_logic::formula(filter_param.second, function_table));

			filter_map_[filter_param.first]=filter_formula;
		}
	}	
}

move_candidate_action::move_candidate_action(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table) :
	candidate_action_with_filters(name, type, cfg, function_table)
{}

void move_candidate_action::evaluate(formula_ai* ai, unit_map& units)
{
	score_ = 0;

	candidate_action_filters::const_iterator me_filter = filter_map_.find("me");

	for(unit_map::unit_iterator i = units.begin() ; i != units.end() ; ++i)
	{
		if( (i->second.side() == ai->get_side() ) &&
				(i->second.has_moved() == false) ) {

			game_logic::map_formula_callable callable(static_cast<const formula_callable*>(ai));
			callable.add_ref();
			callable.add("me", variant(new unit_callable(*i)));

			if(me_filter != filter_map_.end() ) {
				if ( !execute_formula( me_filter->second, callable, ai ) )
					continue;
			}

			int res = execute_formula(eval_, callable, ai);

			if(res > score_) {
				score_ = res;
				my_unit_ = i;
			}
		}
	}
}

void move_candidate_action::update_callable_map(game_logic::map_formula_callable& callable)
{
	variant my_unit_callable(new unit_callable( *my_unit_ ));
	callable.add("me", my_unit_callable);
}

attack_candidate_action::attack_candidate_action(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table) :
	candidate_action_with_filters(name, type, cfg, function_table)
{}

void attack_candidate_action::evaluate(formula_ai* ai, unit_map& units)
{
	std::vector< unit_map::const_unit_iterator > my_units;
	std::vector< unit_map::const_unit_iterator > enemy_units;

	candidate_action_filters::const_iterator me_filter = filter_map_.find("me");
	candidate_action_filters::const_iterator target_filter = filter_map_.find("target");

	for(unit_map::unit_iterator unit = units.begin() ; unit != units.end() ; ++unit)
	{
		if( unit->second.side() == ai->get_side() ) {
			if( unit->second.has_moved() == false ){
				if(me_filter != filter_map_.end() ) {
					game_logic::map_formula_callable callable(static_cast<const formula_callable*>(ai));
					callable.add_ref();
					callable.add("me", variant(new unit_callable(*unit)));

					if ( execute_formula( me_filter->second, callable, ai ) )
						my_units.push_back(unit);
				} else {
					my_units.push_back(unit);
				}
			}
		} else {
			if( ai->current_team().is_enemy(unit->second.side()) ){
				if(target_filter != filter_map_.end() ) {
					game_logic::map_formula_callable callable(static_cast<const formula_callable*>(ai));
					callable.add_ref();
					callable.add("target", variant(new unit_callable(*unit)));

					if ( execute_formula( target_filter->second, callable, ai ) )
						enemy_units.push_back(unit);
				} else {
					enemy_units.push_back(unit);
				}
			}
		}
	}
	
	score_ = 0;
	foreach(unit_map::const_unit_iterator me, my_units)
	{
		foreach(unit_map::const_unit_iterator target, enemy_units)
		{
			if( ai->can_reach_unit(me, target) )
			{	
				game_logic::map_formula_callable callable(static_cast<const formula_callable*>(ai));
				callable.add_ref();
				callable.add("me", variant(new unit_callable(*me)));
				callable.add("target", variant(new unit_callable(*target)));

				int res = execute_formula(eval_, callable, ai);

				if(res > score_) {
					score_ = res;
					my_unit_ = me;
					enemy_unit_ = target;
				}
			}
		}
	}
}

void attack_candidate_action::update_callable_map(game_logic::map_formula_callable& callable)
{
	variant my_unit_callable(new unit_callable( *my_unit_ ));
	callable.add("me", my_unit_callable);
	variant enemy_unit_callable(new unit_callable( *enemy_unit_ ));
	callable.add("target", enemy_unit_callable);
}


support_candidate_action::support_candidate_action(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table) :
	candidate_action_with_filters(name, type, cfg, function_table)
{}

}
