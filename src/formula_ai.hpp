/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_AI_HPP_INCLUDED
#define FORMULA_AI_HPP_INCLUDED

#include "ai.hpp"
#include "ai_interface.hpp"
#include "callable_objects.hpp"
#include "formula.hpp"
#include "formula_fwd.hpp"
#include "formula_callable.hpp"
#include "formula_function.hpp"

// Forward declaration needed for ai function symbol table
class formula_ai;

namespace game_logic {

class candidate_move {
public:
	candidate_move(std::string name, std::string type, const_formula_ptr eval, const_formula_ptr move) :
		name_(name),
		type_(type),
		eval_(eval),
		move_(move),
		score_(-1),
		action_unit_(),
		enemy_unit_()
	{};

	void evaluate_move(const formula_ai* ai, unit_map& units, size_t team_num);

	int get_score() const {return score_;}
	std::string get_type() const {return type_;}
	unit_map::unit_iterator get_action_unit() {return action_unit_;}
	unit_map::unit_iterator get_enemy_unit() {return enemy_unit_;}
	const_formula_ptr get_move() {return move_;}

	struct move_compare {
		bool operator() (const boost::shared_ptr<candidate_move> lmove,
				const boost::shared_ptr<candidate_move> rmove) const
		{
			return lmove->get_score() < rmove->get_score();
		}
	};

private:

	std::string name_;
	std::string type_;
	const_formula_ptr eval_;
	const_formula_ptr move_;
	int score_;
	unit_map::unit_iterator action_unit_;
	unit_map::unit_iterator enemy_unit_;
};


typedef boost::shared_ptr<candidate_move> candidate_move_ptr;
typedef std::set<game_logic::candidate_move_ptr, game_logic::candidate_move::move_compare> candidate_move_set;

class ai_function_symbol_table : public function_symbol_table {

public:
	explicit ai_function_symbol_table(formula_ai& ai) :
		ai_(ai),
		move_functions(),
		candidate_moves()
	{}

	void register_candidate_move(const std::string name, const std::string type,
			const_formula_ptr formula, const_formula_ptr eval,
			const_formula_ptr precondition, const std::vector<std::string>& args);

	std::vector<candidate_move_ptr>::iterator candidate_move_begin() {
		return candidate_moves.begin();
	}

	std::vector<candidate_move_ptr>::iterator candidate_move_end() {
		return candidate_moves.end();
	}

private:
	formula_ai& ai_;
	std::set<std::string> move_functions;
	std::vector<candidate_move_ptr> candidate_moves;
	expression_ptr create_function(const std::string& fn,
	                               const std::vector<expression_ptr>& args) const;
};

}

class formula_ai : public ai {
public:
	explicit formula_ai(info& i);
	virtual ~formula_ai() {};
	virtual void play_turn();
	virtual void new_turn();

	using ai_interface::get_info;
	using ai_interface::current_team;
	using ai_interface::move_map;

	const move_map& srcdst() const { if(!move_maps_valid_) { prepare_move(); } return srcdst_; }

	std::string evaluate(const std::string& formula_str);

	struct move_map_backup {
		move_map_backup() :
			move_maps_valid(false),
			srcdst(),
			dstsrc(),
			full_srcdst(),
			full_dstsrc(),
			enemy_srcdst(),
			enemy_dstsrc(),
			attacks_cache()
		{
		}

		bool move_maps_valid;
		move_map srcdst, dstsrc, full_srcdst, full_dstsrc, enemy_srcdst, enemy_dstsrc;
		variant attacks_cache;
	};

        // If the AI manager should manager the AI once constructed.
        virtual bool manager_manage_ai() const { return true ; } ;

	void swap_move_map(move_map_backup& backup);

	variant get_keeps() const;

	const variant& get_keeps_cache() const { return keeps_cache_; }

	// Check if given unit loc can reach attack range of enemy loc
	bool can_attack (const map_location, const map_location) const;

	const std::map<location,paths>& get_possible_moves() const { prepare_move(); return possible_moves_; }

	void handle_exception(game_logic::formula_error& e) const;
	void handle_exception(game_logic::formula_error& e, const std::string& failed_operation) const;

        void invalidate_move_maps() const { move_maps_valid_ = false; }

private:
	void display_message(const std::string& msg) const;
	bool do_recruitment();
	bool make_move(game_logic::const_formula_ptr formula_, const game_logic::formula_callable& variables);
	bool execute_variant(const variant& var, bool commandline=false);
	virtual variant get_value(const std::string& key) const;
	virtual void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	game_logic::const_formula_ptr recruit_formula_;
	game_logic::const_formula_ptr move_formula_;

	mutable std::map<location,paths> possible_moves_;

	void prepare_move() const;
	void build_move_list();
	void make_candidate_moves();
	mutable bool move_maps_valid_;
	mutable move_map srcdst_, dstsrc_, full_srcdst_, full_dstsrc_, enemy_srcdst_, enemy_dstsrc_;
	mutable variant attacks_cache_;
	mutable variant keeps_cache_;

	game_logic::map_formula_callable vars_;
	game_logic::ai_function_symbol_table function_table;

	game_logic::candidate_move_set candidate_moves_;
	bool use_eval_lists_;
	friend class ai;
};

#endif
