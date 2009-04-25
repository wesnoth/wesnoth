/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Base class for the AI and AI-ai_manager contract.
 * @file ai/ai_interface.cpp
 */

#include "ai_actions.hpp"
#include "ai_interface.hpp"
#include "ai_manager.hpp"

// =======================================================================
//
// =======================================================================
void ai_interface::raise_user_interact() const
{
	ai_manager::raise_user_interact();
}

void ai_interface::raise_unit_recruited() const
{
	ai_manager::raise_unit_recruited();
}

void ai_interface::raise_unit_moved() const
{
	ai_manager::raise_unit_moved();
}

void ai_interface::raise_enemy_attacked() const
{
	ai_manager::raise_enemy_attacked();
}


std::auto_ptr<ai_attack_result> ai_interface::execute_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon){
	return ai_actions::execute_attack_action(get_side(),true,attacker_loc,defender_loc,attacker_weapon);
}

std::auto_ptr<ai_attack_result> ai_interface::check_attack_action(const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon){
	return ai_actions::execute_attack_action(get_side(),false,attacker_loc,defender_loc,attacker_weapon);
}


std::auto_ptr<ai_move_result> ai_interface::execute_move_action(const map_location& from, const location& to, bool remove_movement){
	return ai_actions::execute_move_action(get_side(),true,from,to,remove_movement);
}

std::auto_ptr<ai_move_result> ai_interface::check_move_action(const map_location& from, const location& to, bool remove_movement){
	return ai_actions::execute_move_action(get_side(),false,from,to,remove_movement);
}


std::auto_ptr<ai_recruit_result> ai_interface::execute_recruit_action(const std::string& unit_name, const location &where){
	return ai_actions::execute_recruit_action(get_side(),true,unit_name,where);
}

std::auto_ptr<ai_recruit_result> ai_interface::check_recruit_action(const std::string& unit_name, const location &where){
	return ai_actions::execute_recruit_action(get_side(),false,unit_name,where);
}


std::auto_ptr<ai_stopunit_result> ai_interface::execute_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	return ai_actions::execute_stopunit_action(get_side(),true,unit_location,remove_movement,remove_attacks);
}

std::auto_ptr<ai_stopunit_result> ai_interface::check_stopunit_action(const map_location& unit_location, bool remove_movement, bool remove_attacks){
	return ai_actions::execute_stopunit_action(get_side(),false,unit_location,remove_movement,remove_attacks);
}

