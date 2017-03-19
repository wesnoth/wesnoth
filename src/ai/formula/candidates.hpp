/*
   Copyright (C) 2009 - 2017 by Bartosz Waresiak <dragonking@o2.pl>
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
 * Defines formula ai candidate actions - headers
 * */

#ifndef AI_FORMULA_CANDIDATES_HPP_INCLUDED
#define AI_FORMULA_CANDIDATES_HPP_INCLUDED

#include <set>

class unit_map;
class config;

namespace ai {
class formula_ai;
}

namespace game_logic {

class base_candidate_action;

typedef std::map< std::string, game_logic::const_formula_ptr > candidate_action_filters;
typedef std::shared_ptr<game_logic::base_candidate_action> candidate_action_ptr;

//every new candidate action type should be derived from this class
//and should complete evaluate and update_callable_map methods
class base_candidate_action {
public:
	base_candidate_action(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table);

	virtual ~base_candidate_action() {}

	//evaluate candidate action using eval_ formula
	virtual void evaluate(ai::formula_ai* /*ai*/, unit_map& /*units*/) {}

	//adds needed callable objects to callable map
	virtual void update_callable_map(game_logic::map_formula_callable& /*callable*/) {}


	//return score of last evaluation
	int get_score() const {return score_;}

	const_formula_ptr& get_action() {return action_;}

	const std::string& get_name() const { return name_;}
	const std::string& get_type() const { return type_;}

protected:
	int execute_formula(const const_formula_ptr& formula,
			const game_logic::formula_callable& callable, const ai::formula_ai* ai);

	std::string name_;
	std::string type_;
	const_formula_ptr eval_;
	const_formula_ptr action_;
	int score_;
};


class candidate_action_with_filters : public base_candidate_action {
public:
	candidate_action_with_filters(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table);
protected:
        variant do_filtering(ai::formula_ai* ai, variant& input, game_logic::const_formula_ptr formula);

	game_logic::candidate_action_filters filter_map_;
};

class move_candidate_action : public candidate_action_with_filters {
public:
	move_candidate_action(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table);

	virtual void evaluate(ai::formula_ai* ai, unit_map& units);

	virtual void update_callable_map(game_logic::map_formula_callable& callable);

protected:
	variant my_unit_;
};

class attack_candidate_action : public candidate_action_with_filters {
public:
	attack_candidate_action(const std::string& name, const std::string& type,const config& cfg, function_symbol_table* function_table);

	virtual void evaluate(ai::formula_ai* ai, unit_map& units);

	virtual void update_callable_map(game_logic::map_formula_callable& callable);
protected:
	variant my_unit_;
	variant enemy_unit_;
};

}

#endif	/* AI_FORMULA_CANDIDATES_HPP_INCLUDED */

