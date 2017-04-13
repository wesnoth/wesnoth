/*
   Copyright (C) 2014 - 2017 by Guorui Xi <kevin.xgr@gmail.com>
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
 * Implement simulated actions
 * @file
 */

#include "ai/simulated_actions.hpp"

#include "game_board.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "recall_list_manager.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/helper.hpp"
#include "units/ptr.hpp"
#include "units/types.hpp"

namespace ai {

static lg::log_domain log_ai_sim_actions("ai/sim_actions");
#define DBG_AI_SIM_ACTIONS LOG_STREAM(debug, log_ai_sim_actions)
#define LOG_AI_SIM_ACTIONS LOG_STREAM(info, log_ai_sim_actions)
#define WRN_AI_SIM_ACTIONS LOG_STREAM(warn, log_ai_sim_actions)
#define ERR_AI_SIM_ACTIONS LOG_STREAM(err, log_ai_sim_actions)

void helper_check_village(const map_location& loc, int side);
void helper_place_unit(const unit& u, const map_location& loc);
void helper_advance_unit(const map_location& loc);

bool simulated_attack(const map_location& attacker_loc, const map_location& defender_loc, double attacker_hp, double defender_hp){
	LOG_AI_SIM_ACTIONS << "Simulated attack" << std::endl;

	unit_map::iterator attack_unit = resources::gameboard->units().find(attacker_loc);
	unit_map::iterator defend_unit = resources::gameboard->units().find(defender_loc);

	LOG_AI_SIM_ACTIONS << attack_unit->type_name() << " at " << attacker_loc << " attack "
		<< defend_unit->type_name() << " at " << defender_loc << std::endl;
	LOG_AI_SIM_ACTIONS << "attacker's hp before attack: " << attack_unit->hitpoints() << std::endl;
	LOG_AI_SIM_ACTIONS << "defender's hp before attack: " << defend_unit->hitpoints() << std::endl;

	attack_unit->set_hitpoints(static_cast<int>(attacker_hp));
	defend_unit->set_hitpoints(static_cast<int>(defender_hp));

	LOG_AI_SIM_ACTIONS << "attacker's hp after attack: " << attack_unit->hitpoints() << std::endl;
	LOG_AI_SIM_ACTIONS << "defender's hp after attack: " << defend_unit->hitpoints() << std::endl;

	int attacker_xp = defend_unit->level();
	int defender_xp = attack_unit->level();
	bool attacker_died = false;
	bool defender_died = false;
	if(attack_unit->hitpoints() <= 0){
		attacker_xp = 0;
		defender_xp = game_config::kill_xp(attack_unit->level());
		(resources::gameboard->units()).erase(attacker_loc);
		attacker_died = true;
	}

	if(defend_unit->hitpoints() <= 0){
		defender_xp = 0;
		attacker_xp = game_config::kill_xp(defend_unit->level());
		(resources::gameboard->units()).erase(defender_loc);
		defender_died = true;
	}

	if(!attacker_died){
		attack_unit->set_experience(attack_unit->experience()+attacker_xp);
		helper_advance_unit(attacker_loc);
		simulated_stopunit(attacker_loc, true, true);
	}

	if(!defender_died){
		defend_unit->set_experience(defend_unit->experience()+defender_xp);
		helper_advance_unit(defender_loc);
		simulated_stopunit(defender_loc, true, true);
	}

	return true;
}

bool simulated_move(int side, const map_location& from, const map_location& to, int steps, map_location& unit_location){
	LOG_AI_SIM_ACTIONS << "Simulated move" << std::endl;

	// In simulation, AI should not know if there is a enemy's ambusher.
	std::pair<unit_map::unit_iterator, bool> unit_move = resources::gameboard->units().move(from, to);
	bool is_ok = unit_move.second;
	if(!is_ok){
		unit_location = to;	// This happened because in some CAs like get_village_phase and move_leader_to_keep phase,
							// if the destination is already occupied will not be checked before execute. Just silent
							// errors in ai/actions and tell rca the game state isn't changed.
		return false;
	}
	unit_map::unit_iterator move_unit = unit_move.first;
	move_unit->set_movement(move_unit->movement_left()-steps);	// Following original logic, remove_movement_ will be considered outside.

	unit_location = move_unit->get_location();	// For check_after.

	LOG_AI_SIM_ACTIONS << move_unit->type_name() << " move from " << from << " to " << to << std::endl;

	if(resources::gameboard->map().is_village(to)){
		helper_check_village(to, side);
	}

	return true;
}

bool simulated_recall(int side, const std::string& unit_id, const map_location& recall_location){
	LOG_AI_SIM_ACTIONS << "Simulated recall" << std::endl;

	team own_team = resources::gameboard->teams()[side-1];
	unit_ptr recall_unit = own_team.recall_list().extract_if_matches_id(unit_id);

	helper_place_unit(*recall_unit, recall_location);

	own_team.spend_gold(recall_unit->recall_cost()<0 ? own_team.recall_cost() : recall_unit->recall_cost());

	LOG_AI_SIM_ACTIONS << "recall " << recall_unit->type_name() << " at "
		<< recall_location << " spend " << own_team.recall_cost() << " gold" << std::endl;

	return true;
}

bool simulated_recruit(int side, const unit_type* u, const map_location& recruit_location){
	LOG_AI_SIM_ACTIONS << "Simulated recruit" << std::endl;

	const unit recruit_unit(*u, side, false);	// Random traits, name and gender are not needed. This will cause "duplicate id conflicts" inside unit_map::insert(), but engine will manage this issue correctly.
	helper_place_unit(recruit_unit, recruit_location);

	resources::gameboard->teams()[side-1].spend_gold(u->cost());

	LOG_AI_SIM_ACTIONS << "recruit " << u->type_name() << " at "
		<< recruit_location << " spend " << u->cost() << " gold" << std::endl;

	return true;
}

bool simulated_stopunit(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	LOG_AI_SIM_ACTIONS << "Simulated stopunit" << std::endl;

	unit_map::iterator stop_unit = resources::gameboard->units().find(unit_location);
	bool changed = false;
	if(remove_movement){
		stop_unit->set_movement(0, true);
		LOG_AI_SIM_ACTIONS << "remove (" << stop_unit->get_location() << ") " << stop_unit->type_name() << "'s movement" << std::endl;
		changed = true;
	}
	if(remove_attacks){
		stop_unit->set_attacks(0);
		LOG_AI_SIM_ACTIONS << "remove (" << stop_unit->get_location() << ") " << stop_unit->type_name() << "'s attacks" << std::endl;
		changed = true;
	}

	return changed;
}

bool simulated_synced_command(){
	LOG_AI_SIM_ACTIONS << "Simulated synced_command" << std::endl;

	DBG_AI_SIM_ACTIONS << "Trigger dummy synced_command_result::do_execute()" << std::endl;

	return false;
}

// Helper functions.
void helper_check_village(const map_location& loc, int side){
	std::vector<team> &teams = resources::gameboard->teams();
	team *t = unsigned(side - 1) < teams.size() ? &teams[side - 1] : nullptr;
	if(t && t->owns_village(loc)){
		return;
	}

	bool has_leader = resources::gameboard->units().find_leader(side).valid();

	// Strip the village off all other sides.
	int old_owner_side = 0;
	for(std::vector<team>::iterator i = teams.begin(); i != teams.end(); ++i){
		int i_side = i - teams.begin() + 1;
		if(!t || has_leader || t->is_enemy(i_side)){
			if(i->owns_village(loc)){
				old_owner_side = i_side;
				i->lose_village(loc);
				DBG_AI_SIM_ACTIONS << "side " << i_side << " losts village at " << loc << std::endl;
			}
		}
	}

	// Get the village if have leader.
	if (!t) return;

	if(has_leader){
		t->get_village(loc, old_owner_side, nullptr);
		DBG_AI_SIM_ACTIONS << "side " << side << " gets village at " << loc << std::endl;
	}
}

void helper_place_unit(const unit& u, const map_location& loc){
	unit new_unit = u;
	new_unit.set_movement(0, true);
	new_unit.set_attacks(0);
	new_unit.heal_fully();

	std::pair<unit_map::iterator, bool> add_result = resources::gameboard->units().add(loc, new_unit);
	assert(add_result.second);
	unit_map::iterator& new_unit_itor = add_result.first;

	if(resources::gameboard->map().is_village(loc)){
		helper_check_village(loc, new_unit_itor->side());
	}
}

void helper_advance_unit(const map_location& loc){
	// Choose advanced unit type randomly.
	// First check if the unit has enough experience and can advance.
	// Then get all possible options, include modification advancements, like {AMLA DEFAULT} in cfg.
	// And then randomly choose one to advanced to.

	unit_map::iterator advance_unit = resources::gameboard->units().find(loc);

	if(!unit_helper::will_certainly_advance(advance_unit))
		return;

	const std::vector<std::string>& options = advance_unit->advances_to();
	std::vector<config> mod_options = advance_unit->get_modification_advances();
	int options_num = unit_helper::number_of_possible_advances(*advance_unit);

	size_t advance_choice = rand() % options_num;
	unit advanced_unit(*advance_unit);

	if(advance_choice < options.size()){
		std::string advance_unit_typename = options[advance_choice];
		const unit_type *advanced_type = unit_types.find(advance_unit_typename);
		if(!advanced_type) {
			ERR_AI_SIM_ACTIONS << "Simulating advancing to unknown unit type: " << advance_unit_typename;
			assert(false && "simulating to unknown unit type");
		}
		advanced_unit.set_experience(advanced_unit.experience() - advanced_unit.max_experience());
		advanced_unit.advance_to(*advanced_type);
		advanced_unit.heal_fully();
		advanced_unit.set_state(unit::STATE_POISONED, false);
		advanced_unit.set_state(unit::STATE_SLOWED, false);
		advanced_unit.set_state(unit::STATE_PETRIFIED, false);
	}else{
		const config &mod_option = mod_options[advance_choice-options.size()];
		advanced_unit.set_experience(advanced_unit.experience()-advanced_unit.max_experience());
		advanced_unit.add_modification("advancement", mod_option);
	}

	resources::gameboard->units().replace(loc, advanced_unit);
	LOG_AI_SIM_ACTIONS << advance_unit->type_name() << " at " << loc << " advanced to " << advanced_unit.type_name() << std::endl;
}

}// End namespace
