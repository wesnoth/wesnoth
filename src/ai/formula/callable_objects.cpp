/*
   Copyright (C) 2009 - 2018 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "ai/formula/ai.hpp"
#include "attack_prediction.hpp"
#include "game_board.hpp"
#include "ai/formula/callable_objects.hpp"
#include "ai/composite/contexts.hpp"
#include "resources.hpp"
#include "map/map.hpp"
#include "ai/game_info.hpp"
#include "ai/actions.hpp"
#include "units/formula_manager.hpp"
#include "units/unit.hpp"
#include "log.hpp"
#include "menu_events.hpp" // for fallback_ai_to_human_exception

static lg::log_domain log_formula_ai("ai/engine/fai");
#define DBG_AI LOG_STREAM(debug, log_formula_ai)
#define LOG_AI LOG_STREAM(info, log_formula_ai)
#define WRN_AI LOG_STREAM(warn, log_formula_ai)
#define ERR_AI LOG_STREAM(err, log_formula_ai)

namespace ai {

ai_context& get_ai_context(wfl::const_formula_callable_ptr for_fai) {
	auto fai = std::dynamic_pointer_cast<const formula_ai>(for_fai);
	assert(fai != nullptr);
	return *std::const_pointer_cast<formula_ai>(fai)->ai_ptr_;
}

}

namespace wfl {
	using namespace ai;

variant move_map_callable::get_value(const std::string& key) const
{
	if(key == "moves") {
		std::vector<variant> vars;
		for(move_map::const_iterator i = srcdst_.begin(); i != srcdst_.end(); ++i) {
                        if( i->first == i->second || units_.count(i->second) == 0) {
                            auto item = std::make_shared<move_callable>(i->first, i->second);
                            vars.emplace_back(item);
                        }
		}

		return variant(vars);
	} else if(key == "has_moves") {
		return variant(!srcdst_.empty());
	} else {
		return variant();
	}
}

void move_map_callable::get_inputs(formula_input_vector& inputs) const
{
	add_input(inputs, "moves");
}

int move_callable::do_compare(const formula_callable* callable) const
{
	const move_callable* mv_callable = dynamic_cast<const move_callable*>(callable);
	if(mv_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	const map_location& other_src = mv_callable->src_;
	const map_location& other_dst = mv_callable->dst_;

	if (int cmp = src_.do_compare(other_src)) {
		return cmp;
	}

	return dst_.do_compare(other_dst);
}

variant move_callable::execute_self(variant ctxt) {
	ai_context& ai = get_ai_context(ctxt.as_callable());
	move_result_ptr move_result = ai.execute_move_action(src_, dst_, true);

	if(!move_result->is_ok()) {
		LOG_AI << "ERROR #" << move_result->get_status() << " while executing 'move' formula function\n" << std::endl;
		return variant(std::make_shared<safe_call_result>(fake_ptr(), move_result->get_status(), move_result->get_unit_location()));
	}

	return variant(move_result->is_gamestate_changed());
}

int move_partial_callable::do_compare(const formula_callable* callable) const
{
	const move_partial_callable* mv_callable = dynamic_cast<const move_partial_callable*>(callable);
	if(mv_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	const map_location& other_src = mv_callable->src_;
	const map_location& other_dst = mv_callable->dst_;

	if (int cmp = src_.do_compare(other_src)) {
		return cmp;
	}

	return dst_.do_compare(other_dst);
}

variant move_partial_callable::execute_self(variant ctxt) {
	ai_context& ai = get_ai_context(ctxt.as_callable());
	move_result_ptr move_result = ai.execute_move_action(src_, dst_, false);

	if(!move_result->is_ok()) {
		LOG_AI << "ERROR #" << move_result->get_status() << " while executing 'move_partial' formula function\n" << std::endl;
		return variant(std::make_shared<safe_call_result>(fake_ptr(), move_result->get_status(), move_result->get_unit_location()));
	}

	return variant(move_result->is_gamestate_changed());
}

variant position_callable::get_value(const std::string& key) const {
	if(key == "chance") {
		return variant(chance_);
	} else {
		return variant();
	}
}

void position_callable::get_inputs(formula_input_vector& inputs) const {
	add_input(inputs, "chance");
}

variant outcome_callable::get_value(const std::string& key) const {
	if(key == "hitpoints_left") {
		return variant(hitLeft_);
	} else if(key == "probability") {
		return variant(prob_);
	} else if(key == "possible_status") {
		return variant(status_);
	} else {
		return variant();
	}
}

void outcome_callable::get_inputs(formula_input_vector& inputs) const {
	add_input(inputs, "hitpoints_left");
	add_input(inputs, "probability");
	add_input(inputs, "possible_status");
}


attack_callable::attack_callable(const map_location& move_from,
				    const map_location& src, const map_location& dst, int weapon)
	: move_from_(move_from), src_(src), dst_(dst),
	bc_(resources::gameboard->units(), src, dst, weapon, -1, 1.0, nullptr,
		&*resources::gameboard->units().find(move_from))
{
      type_ = ATTACK_C;
}


variant attack_callable::get_value(const std::string& key) const {
	if(key == "attack_from") {
		return variant(std::make_shared<location_callable>(src_));
	} else if(key == "defender") {
		return variant(std::make_shared<location_callable>(dst_));
	} else if(key == "move_from") {
		return variant(std::make_shared<location_callable>(move_from_));
	} else {
		return variant();
	}
}

void attack_callable::get_inputs(formula_input_vector& inputs) const {
	add_input(inputs, "attack_from");
	add_input(inputs, "defender");
	add_input(inputs, "move_from");
}

int attack_callable::do_compare(const wfl::formula_callable* callable)
	const {
	const attack_callable* a_callable = dynamic_cast<const attack_callable*>(callable);
	if(a_callable == nullptr) {
		return formula_callable::do_compare(callable);
	}

	const map_location& other_from = a_callable->move_from();

	if (int cmp = move_from_.do_compare(other_from)) {
		return cmp;
	}
	const map_location& other_src = a_callable->src();
	if (int cmp = src_.do_compare(other_src)) {
		return cmp;
	}
	const map_location& other_dst = a_callable->dst();
	if (int cmp = dst_.do_compare(other_dst)) {
		return cmp;
	}
	const int other_weapon = a_callable->weapon();
	if (int cmp = (this->weapon() - other_weapon)) {
		return cmp;
	}
	const int other_def_weapon = a_callable->defender_weapon();
	return this->defender_weapon() - other_def_weapon;
}

variant attack_callable::execute_self(variant ctxt) {
	ai_context& ai = get_ai_context(ctxt.as_callable());
	bool gamestate_changed = false;
	move_result_ptr move_result;

	if(move_from_ != src_) {
		move_result = ai.execute_move_action(move_from_, src_, false);
		gamestate_changed |= move_result->is_gamestate_changed();

		if(!move_result->is_ok()) {
			//move part failed
			LOG_AI << "ERROR #" << move_result->get_status() << " while executing 'attack' formula function\n" << std::endl;
			return variant(std::make_shared<safe_call_result>(fake_ptr(), move_result->get_status(), move_result->get_unit_location()));
		}
	}

	if(!move_result || move_result->is_ok()) {
		//if move wasn't done at all or was done successfully
		attack_result_ptr attack_result = ai.execute_attack_action(src_, dst_, weapon());
		gamestate_changed |= attack_result->is_gamestate_changed();
		if(!attack_result->is_ok()) {
			//attack failed
			LOG_AI << "ERROR #" << attack_result->get_status() << " while executing 'attack' formula function\n" << std::endl;
			return variant(std::make_shared<safe_call_result>(fake_ptr(), attack_result->get_status()));
		}
	}

	return variant(gamestate_changed);
}

variant attack_map_callable::get_value(const std::string& key) const {
	if(key == "attacks") {
		std::vector<variant> vars;
		for(move_map::const_iterator i = ai_.get_srcdst().begin(); i != ai_.get_srcdst().end(); ++i) {
			/* for each possible move check all adjacent tiles for enemies */
			if(units_.count(i->second) == 0) {
				collect_possible_attacks(vars, i->first, i->second);
			}
		}
		/* special case, when unit moved toward enemy and can only attack */
		for(unit_map::const_iterator i = resources::gameboard->units().begin(); i != resources::gameboard->units().end(); ++i) {
			if (i->side() == ai_.get_side() && i->attacks_left() > 0) {
				collect_possible_attacks(vars, i->get_location(), i->get_location());
			}
		}
		return variant(vars);
	} else {
		return variant();
	}
}

void attack_map_callable::get_inputs(formula_input_vector& inputs) const {
	add_input(inputs, "attacks");
}

/* add to vars all attacks on enemy units around <attack_position> tile. attacker_location is tile where unit is currently standing. It's moved to attack_position first and then performs attack.*/
void attack_map_callable::collect_possible_attacks(std::vector<variant>& vars, map_location attacker_location, map_location attack_position) const {
	adjacent_loc_array_t adj;
	get_adjacent_tiles(attack_position, adj.data());

	for(unsigned n = 0; n < adj.size(); ++n) {
		/* if adjacent tile is outside the board */
		if (! resources::gameboard->map().on_board(adj[n]))
			continue;
		unit_map::const_iterator unit = units_.find(adj[n]);
		/* if tile is empty */
		if (unit == units_.end())
			continue;
		/* if tile is occupied by friendly or petrified/invisible unit */
		if (!ai_.current_team().is_enemy(unit->side())  ||
		    unit->incapacitated() ||
		    unit->invisible(unit->get_location(), *resources::gameboard))
			continue;
		/* add attacks with default weapon */
		auto item = std::make_shared<attack_callable>(attacker_location, attack_position, adj[n], -1);
		vars.emplace_back(item);
	}
}


variant recall_callable::get_value(const std::string& key) const {
	if( key == "id")
		return variant(id_);
	if( key == "loc")
		return variant(std::make_shared<location_callable>(loc_));
	return variant();
}

void recall_callable::get_inputs(formula_input_vector& inputs) const {
	add_input(inputs, "id");
	add_input(inputs, "loc");
}

variant recall_callable::execute_self(variant ctxt) {
	ai_context& ai = get_ai_context(ctxt.as_callable());
	recall_result_ptr recall_result = ai.check_recall_action(id_, loc_);

	if(recall_result->is_ok()) {
		recall_result->execute();
	} else {
		LOG_AI << "ERROR #" << recall_result->get_status() << " while executing 'recall' formula function\n" << std::endl;
		return variant(std::make_shared<safe_call_result>(fake_ptr(), recall_result->get_status()));
	}

	return variant(recall_result->is_gamestate_changed());
}

variant recruit_callable::get_value(const std::string& key) const {
	if( key == "unit_type")
		return variant(type_);
	if( key == "recruit_loc")
		return variant(std::make_shared<location_callable>(loc_));
	return variant();
}

void recruit_callable::get_inputs(formula_input_vector& inputs) const {
	add_input(inputs, "unit_type");
	add_input(inputs, "recruit_loc");
}

variant recruit_callable::execute_self(variant ctxt) {
	ai_context& ai = get_ai_context(ctxt.as_callable());
	recruit_result_ptr recruit_result = ai.check_recruit_action(type_, loc_);

	//is_ok()==true means that the action is successful (eg. no unexpected events)
	//is_ok() must be checked or the code will complain :)
	if(recruit_result->is_ok()) {
		recruit_result->execute();
	} else {
		LOG_AI << "ERROR #" << recruit_result->get_status() << " while executing 'recruit' formula function\n" << std::endl;
		return variant(std::make_shared<safe_call_result>(fake_ptr(), recruit_result->get_status()));
	}

	//is_gamestate_changed()==true means that the game state was somehow changed by action.
	//it is believed that during a turn, a game state can change only a finite number of times
	return variant(recruit_result->is_gamestate_changed());
}

variant set_unit_var_callable::get_value(const std::string& key) const {
	if(key == "loc")
		return variant(std::make_shared<location_callable>(loc_));

	if(key == "key")
		return variant(key_);

	if(key == "value")
		return value_;

	return variant();
}

void set_unit_var_callable::get_inputs(formula_input_vector& inputs) const {
	add_input(inputs, "loc");
	add_input(inputs, "key");
	add_input(inputs, "value");
}

variant set_unit_var_callable::execute_self(variant ctxt) {
	int status = 0;
	unit_map::iterator unit;
	unit_map& units = resources::gameboard->units();

/*	if(!infinite_loop_guardian_.set_unit_var_check()) {
		status = 5001; //exceeded nmber of calls in a row - possible infinite loop
	} else*/ if((unit = units.find(loc_)) == units.end()) {
		status = 5002; //unit not found
	} else if(unit->side() != get_ai_context(ctxt.as_callable()).get_side()) {
		status = 5003;//unit does not belong to our side
	}

	if(status == 0) {
		LOG_AI << "Setting unit variable: " << key_ << " -> " << value_.to_debug_string() << "\n";
		unit->formula_manager().add_formula_var(key_, value_);
		return variant(true);
	}

	ERR_AI << "ERROR #" << status << " while executing 'set_unit_var' formula function" << std::endl;
	return variant(std::make_shared<safe_call_result>(fake_ptr(), status));
}

variant fallback_callable::execute_self(variant) {
	// We want give control of the side to human for the rest of this turn
	throw fallback_ai_to_human_exception();
}

}
