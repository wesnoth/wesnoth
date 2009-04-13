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
 * @file formula_candidates.hpp
 * Defines formula ai candidate moves - headers
 * */

#ifndef _FORMULA_CANDIDATES_HPP
#define	_FORMULA_CANDIDATES_HPP

namespace game_logic {

class base_candidate_move;

typedef std::map< std::string, game_logic::const_formula_ptr > candidate_move_filters;
typedef boost::shared_ptr<game_logic::base_candidate_move> candidate_move_ptr;

//every new candidate move type should be derived from this class
//and should complete evaluate and update_callable_map methods
class base_candidate_move {
public:
	base_candidate_move(const config& cfg, function_symbol_table* function_table);

	virtual ~base_candidate_move() {}

	//evaluate candidate move using eval_ formula
	virtual void evaluate(formula_ai* /*ai*/, unit_map& /*units*/) {}

	//adds needed callable objects to callable map
	virtual void update_callable_map(game_logic::map_formula_callable& /*callable*/) {}


	//return score of last evaluation
	int get_score() const {return score_;}
	
	const_formula_ptr& get_action() {return action_;}
	
protected:
	int execute_formula(const const_formula_ptr& formula, 
			const game_logic::formula_callable& callable, const formula_ai* ai);

	std::string name_;
	std::string type_;
	const_formula_ptr eval_;
	const_formula_ptr action_;
	int score_;
};

struct candidate_move_compare {
	bool operator() (const candidate_move_ptr lmove,
			const candidate_move_ptr rmove) const
	{
		return lmove->get_score() > rmove->get_score();
	}
};

typedef std::set<game_logic::candidate_move_ptr, game_logic::candidate_move_compare> candidate_move_set;

//this class is responsible for managing candidate moves
class candidate_move_manager {
public:
	candidate_move_manager() {}
	
	//register candidate moves from config
	void load_config(const config& cfg, formula_ai* ai, function_symbol_table* function_table);

	//evaluate candidate moves, return true if we have candidate moves that have score > 0
	bool evaluate_candidate_moves(formula_ai* ai, unit_map& units);
	
	const_formula_ptr get_best_move_formula() {
		if( evaluated_candidate_moves_.empty() )
			return game_logic::formula_ptr();
		return (*evaluated_candidate_moves_.begin())->get_action();
	}

	//calls same method from best candidate move
	void update_callable_map(game_logic::map_formula_callable& callable){
		if( evaluated_candidate_moves_.empty() )
			return;
		(*evaluated_candidate_moves_.begin())->update_callable_map(callable);
	}

	void register_candidate_move(candidate_move_ptr& candidate_move){
		candidate_moves_.push_back(candidate_move);
	}

	bool has_candidate_moves() { return !candidate_moves_.empty(); }

	void clear() {
		candidate_moves_.clear();
		evaluated_candidate_moves_.clear();
	}

private:
	game_logic::candidate_move_set evaluated_candidate_moves_;
	std::vector<candidate_move_ptr> candidate_moves_;
};


class candidate_move_with_filters : public base_candidate_move {
public:
	candidate_move_with_filters(const config& cfg, function_symbol_table* function_table);
protected:

	game_logic::candidate_move_filters filter_map_;
};

class move_candidate_move : public candidate_move_with_filters {
public:
	move_candidate_move(const config& cfg, function_symbol_table* function_table);

	virtual void evaluate(formula_ai* ai, unit_map& units);

	virtual void update_callable_map(game_logic::map_formula_callable& callable);

protected:
	unit_map::unit_iterator my_unit_;
};

class attack_candidate_move : public candidate_move_with_filters {
public:
	attack_candidate_move(const config& cfg, function_symbol_table* function_table);

	virtual void evaluate(formula_ai* ai, unit_map& units);

	virtual void update_callable_map(game_logic::map_formula_callable& callable);
protected:
	unit_map::const_unit_iterator my_unit_;
	unit_map::const_unit_iterator enemy_unit_;
};

class support_candidate_move : public candidate_move_with_filters {
public:
	support_candidate_move(const config& cfg, function_symbol_table* function_table);
};

}

#endif	/* _FORMULA_CANDIDATES_HPP */

