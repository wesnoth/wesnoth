/*
   Copyright (C) 2009 - 2016 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "ai.hpp"
#include "../../attack_prediction.hpp"
#include "../../game_board.hpp"
#include "callable_objects.hpp"
#include "../../resources.hpp"


namespace game_logic {

variant move_map_callable::get_value(const std::string& key) const
{
	using namespace game_logic;
	if(key == "moves") {
		std::vector<variant> vars;
		for(move_map::const_iterator i = srcdst_.begin(); i != srcdst_.end(); ++i) {
                        if( i->first == i->second || units_.count(i->second) == 0) {
                            move_callable* item = new move_callable(i->first, i->second);
                            vars.push_back(variant(item));
                        }
		}

		return variant(&vars);
	} else if(key == "has_moves") {
		return variant(!srcdst_.empty());
	} else {
		return variant();
	}
}

void move_map_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const
{
	using game_logic::FORMULA_READ_ONLY;
	inputs->push_back(game_logic::formula_input("moves", FORMULA_READ_ONLY));
}

int move_callable::do_compare(const formula_callable* callable) const
{
	const move_callable* mv_callable = dynamic_cast<const move_callable*>(callable);
	if(mv_callable == NULL) {
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
	if(mv_callable == NULL) {
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

void position_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("chance", game_logic::FORMULA_READ_ONLY));
}

variant outcome_callable::get_value(const std::string& key) const {
	if(key == "hitpoints_left") {
		return variant(new std::vector<variant>(hitLeft_));
	} else if(key == "probability") {
		return variant(new std::vector<variant>(prob_));
	} else if(key == "possible_status") {
		return variant(new std::vector<variant>(status_));
	} else {
		return variant();
	}
}

void outcome_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("hitpoints_left", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("probability", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("possible_status", game_logic::FORMULA_READ_ONLY));
}


attack_callable::attack_callable(const map_location& move_from,
				    const map_location& src, const map_location& dst, int weapon)
	: move_from_(move_from), src_(src), dst_(dst),
	bc_(*resources::units, src, dst, weapon, -1, 1.0, NULL,
		&*resources::units->find(move_from))
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

void attack_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("attack_from", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("defender", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("move_from", game_logic::FORMULA_READ_ONLY));
}

int attack_callable::do_compare(const game_logic::formula_callable* callable)
	const {
	const attack_callable* a_callable = dynamic_cast<const attack_callable*>(callable);
	if(a_callable == NULL) {
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
		for(unit_map::const_iterator i = resources::units->begin(); i != resources::units->end(); ++i) {
			if (i->side() == ai_.get_side() && i->attacks_left() > 0) {
				collect_possible_attacks(vars, i->get_location(), i->get_location());
			}
		}
		return variant(&vars);
	} else {
		return variant();
	}
}

void attack_map_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("attacks", game_logic::FORMULA_READ_ONLY));
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
		    unit->invisible(unit->get_location()))
			continue;
		/* add attacks with default weapon */
		attack_callable* item = new attack_callable(attacker_location, attack_position, adj[n], -1);
		vars.push_back(variant(item));
	}
}


variant recall_callable::get_value(const std::string& key) const {
	if( key == "id")
		return variant(id_);
	if( key == "loc")
		return variant(new location_callable(loc_));
	return variant();
}

void recall_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("id", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("loc", game_logic::FORMULA_READ_ONLY));
}



variant recruit_callable::get_value(const std::string& key) const {
	if( key == "unit_type")
		return variant(type_);
	if( key == "recruit_loc")
		return variant(new location_callable(loc_));
	return variant();
}

void recruit_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("unit_type", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("recruit_loc", game_logic::FORMULA_READ_ONLY));
}


variant set_var_callable::get_value(const std::string& key) const {
	if(key == "key")
		return variant(key_);

	if(key == "value")
		return value_;

	return variant();
}

void set_var_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("key", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("value", game_logic::FORMULA_READ_ONLY));
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

void set_unit_var_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("loc", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("key", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("value", game_logic::FORMULA_READ_ONLY));
}


variant safe_call_callable::get_value(const std::string& key) const {
	if(key == "main")
		return variant(main_);

	if(key == "backup")
		return variant(backup_);

	return variant();
}

void safe_call_callable::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("main", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("backup", game_logic::FORMULA_READ_ONLY));
}


variant safe_call_result::get_value(const std::string& key) const {
	if(key == "status")
		return variant(status_);

	if(key == "object") {
		if( failed_callable_ != NULL)
			return variant(failed_callable_);
		else
			return variant();
	}

	if(key == "current_loc" && current_unit_location_ != map_location())
		return variant(new location_callable(current_unit_location_));

	return variant();
}

void safe_call_result::get_inputs(std::vector<game_logic::formula_input>* inputs) const {
	inputs->push_back(game_logic::formula_input("status", game_logic::FORMULA_READ_ONLY));
	inputs->push_back(game_logic::formula_input("object", game_logic::FORMULA_READ_ONLY));
	if( current_unit_location_ != map_location() )
		inputs->push_back(game_logic::formula_input("current_loc", game_logic::FORMULA_READ_ONLY));
}

}
