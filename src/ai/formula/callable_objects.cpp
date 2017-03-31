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

#include "ai/formula/ai.hpp"
#include "attack_prediction.hpp"
#include "game_board.hpp"
#include "ai/formula/callable_objects.hpp"
#include "resources.hpp"
#include "map/map.hpp"

namespace game_logic {

variant move_map_callable::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "moves") {
		std::vector<variant> vars;
		for(move_map::const_iterator i = srcdst_.begin(); i != srcdst_.end(); ++i) {
                        if( i->first == i->second || units_.count(i->second) == 0) {
                            move_callable* item = new move_callable(i->first, i->second);
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

void move_map_callable::get_inputs(formula_input_vector* inputs) const
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

variant position_callable::get_value(const std::string& key) const {
	if(key == "chance") {
		return variant(chance_);
	} else {
		return variant();
	}
}

void position_callable::get_inputs(formula_input_vector* inputs) const {
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

void outcome_callable::get_inputs(formula_input_vector* inputs) const {
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
		return variant(new location_callable(src_));
	} else if(key == "defender") {
		return variant(new location_callable(dst_));
	} else if(key == "move_from") {
		return variant(new location_callable(move_from_));
	} else {
		return variant();
	}
}

void attack_callable::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "attack_from");
	add_input(inputs, "defender");
	add_input(inputs, "move_from");
}

int attack_callable::do_compare(const game_logic::formula_callable* callable)
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

void attack_map_callable::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "attacks");
}

/* add to vars all attacks on enemy units around <attack_position> tile. attacker_location is tile where unit is currently standing. It's moved to attack_position first and then performs attack.*/
void attack_map_callable::collect_possible_attacks(std::vector<variant>& vars, map_location attacker_location, map_location attack_position) const {
	map_location adj[6];
	get_adjacent_tiles(attack_position, adj);

	for(int n = 0; n != 6; ++n) {
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
		attack_callable* item = new attack_callable(attacker_location, attack_position, adj[n], -1);
		vars.emplace_back(item);
	}
}


variant recall_callable::get_value(const std::string& key) const {
	if( key == "id")
		return variant(id_);
	if( key == "loc")
		return variant(new location_callable(loc_));
	return variant();
}

void recall_callable::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "id");
	add_input(inputs, "loc");
}



variant recruit_callable::get_value(const std::string& key) const {
	if( key == "unit_type")
		return variant(type_);
	if( key == "recruit_loc")
		return variant(new location_callable(loc_));
	return variant();
}

void recruit_callable::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "unit_type");
	add_input(inputs, "recruit_loc");
}


variant set_var_callable::get_value(const std::string& key) const {
	if(key == "key")
		return variant(key_);

	if(key == "value")
		return value_;

	return variant();
}

void set_var_callable::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "key");
	add_input(inputs, "value");
}


variant set_unit_var_callable::get_value(const std::string& key) const {
	if(key == "loc")
		return variant(new location_callable(loc_));

	if(key == "key")
		return variant(key_);

	if(key == "value")
		return value_;

	return variant();
}

void set_unit_var_callable::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "loc");
	add_input(inputs, "key");
	add_input(inputs, "value");
}


variant safe_call_callable::get_value(const std::string& key) const {
	if(key == "main")
		return variant(main_);

	if(key == "backup")
		return variant(backup_);

	return variant();
}

void safe_call_callable::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "main");
	add_input(inputs, "backup");
}


variant safe_call_result::get_value(const std::string& key) const {
	if(key == "status")
		return variant(status_);

	if(key == "object") {
		if( failed_callable_ != nullptr)
			return variant(failed_callable_);
		else
			return variant();
	}

	if(key == "current_loc" && current_unit_location_ != map_location())
		return variant(new location_callable(current_unit_location_));

	return variant();
}

void safe_call_result::get_inputs(formula_input_vector* inputs) const {
	add_input(inputs, "status");
	add_input(inputs, "object");
	if( current_unit_location_ != map_location() )
		add_input(inputs, "current_loc");
}

}
