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
 * Managing the AI-Game interaction - AI actions and their results
 * @file ai/ai_actions.cpp
 */

/**
 * A small explanation about what's going on here:
 * Each action has access to two ai_interface::info objects
 * First is 'info' - real information
 * Second is 'subjective info' - AIs perception of what's going on
 * So, when we check_before action, we use 'subjective info' and don't 
 * touch real 'info' at all.
 * But when we actually want to execute an action, we firstly check
 * 'subjective info' and then (if subjective check is ok) do the same 
 * check on  real 'info'. There's a caveat: if we fail an action based
 * on real 'info', then we NEED to update AIs knowlegge to avoid the ai
 * doing the same thing again.
 * So far the use of 'subjective info' is stubbed out.
 */

#include "ai_actions.hpp"
#include "ai_manager.hpp"
#include "../actions.hpp"
#include "../game_preferences.hpp"
#include "../log.hpp"
#include "../pathfind.hpp"
#include "../replay.hpp"
#include "../statistics.hpp"


#define DBG_AI_ACTIONS LOG_STREAM(debug, ai_actions)
#define LOG_AI_ACTIONS LOG_STREAM(info, ai_actions)
#define WRN_AI_ACTIONS LOG_STREAM(warn, ai_actions)
#define ERR_AI_ACTIONS LOG_STREAM(err, ai_actions)

// =======================================================================
// AI ACTIONS
// =======================================================================
ai_action_result::ai_action_result( unsigned int side )
	: return_value_checked_(true),side_(side),status_(AI_ACTION_SUCCESS),is_execution_(false)
{
}


ai_action_result::~ai_action_result()
{
	if (!return_value_checked_) {
		ERR_AI_ACTIONS << "Return value of AI ACTION was not checked. This may cause bugs! " <<  std::endl;
	}
}


void ai_action_result::check_after()
{
	do_check_after();
}


void ai_action_result::check_before()
{
	do_check_before();
}


void ai_action_result::execute()
{
	is_execution_ = true;
	init_for_execution();
	check_before();
	if (is_success()){
		do_execute();	
	}
	if (is_success()){
		check_after();
	}
	is_execution_ = false;
}

void ai_action_result::init_for_execution()
{
	return_value_checked_ = false;
	status_ =  ai_action_result::AI_ACTION_SUCCESS;
	do_init_for_execution();
}


bool ai_action_result::is_ok()
{
	return_value_checked_ = true;
	return is_success();
}


void ai_action_result::set_error(int error_code){
	status_ = error_code;
	ERR_AI_ACTIONS << "Error #"<<error_code<<" in "<< do_describe();
}


bool ai_action_result::is_success() const
{
	return (status_ == ai_action_result::AI_ACTION_SUCCESS);
}


bool ai_action_result::is_execution() const
{
	return is_execution_;
}


unsigned int ai_action_result::get_side() const
{
	return side_;
}


ai_interface::info& ai_action_result::get_info() const
{
	return ai_manager::get_active_ai_info_for_side(get_side());
}


ai_interface::info& ai_action_result::get_subjective_info() const
{
	return get_info();
}


bool ai_action_result::using_subjective_info() const
{
	return false;
}


team& ai_action_result::get_my_team(ai_interface::info info) const
{
	return info.teams[side_-1];
}

// ai_attack_result
ai_attack_result::ai_attack_result( unsigned int side, const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon)
	: ai_action_result(side), attacker_loc_(attacker_loc), defender_loc_(defender_loc), attacker_weapon_(attacker_weapon){
}

void ai_attack_result::do_check_before()
{
}


void ai_attack_result::do_check_after()
{
}


std::string ai_attack_result::do_describe() const
{
	std::stringstream s;
	s << "attack by side ";
	s << get_side();
	s << " from location "<<attacker_loc_;
	s << " to location "<<defender_loc_;
	s << " using weapon "<< attacker_weapon_;
	s <<std::endl;
	return s.str();
}


void ai_attack_result::do_execute()
{
}


void ai_attack_result::do_init_for_execution()
{
}


// ai_move_result
ai_move_result::ai_move_result( unsigned int side, const map_location& from, const map_location& to, bool remove_movement)
	: ai_action_result(side), from_(from), to_(to), remove_movement_(remove_movement){
}

void ai_move_result::do_check_before()
{
}


void ai_move_result::do_check_after()
{
}


std::string ai_move_result::do_describe() const
{
	std::stringstream s;
	if (remove_movement_){
		s << "full move by side ";
	} else {
		s << "partial move by side ";
	}
	s << get_side();
	s << " from location "<<from_;
	s << " to location "<<to_;
	s <<std::endl;
	return s.str();
}


void ai_move_result::do_execute()
{
}


void ai_move_result::do_init_for_execution()
{
}


// ai_recruit_result
ai_recruit_result::ai_recruit_result( unsigned int side, const std::string& unit_name, const map_location& where)
	: ai_action_result(side), unit_name_(unit_name), where_(where), recruit_location_(where){
}

bool ai_recruit_result::test_available_for_recruiting( const team& team, std::set<std::string>::const_iterator& recruit, bool /*update_knowledge*/ )
{
    	const std::set<std::string>& recruit_set = team.recruits();
	recruit = recruit_set.find(unit_name_);
        if(recruit == recruit_set.end()) {
                set_error(E_NOT_AVAILABLE_FOR_RECRUITING);
                return false;
        }
	num_ = std::distance(recruit_set.begin(),recruit);
	return true;
}

bool ai_recruit_result::test_unit_type_known( const std::set<std::string>::const_iterator& recruit, unit_type_data::unit_type_map::const_iterator& unit_type, bool /*update_knowledge*/ )
{
	unit_type = unit_type_data::types().find_unit_type(*recruit);
        if(unit_type == unit_type_data::types().end() || unit_type->first == "dummy_unit") {
                set_error(E_UNKNOWN_OR_DUMMY_UNIT_TYPE);
                return false;
        }
	return true;
}

bool ai_recruit_result::test_enough_gold( const team& team, const unit_type_data::unit_type_map::const_iterator& unit_type, bool /*update_knowledge*/ )
{
	if(team.gold() < unit_type->second.cost()) {
		set_error(E_NO_GOLD);
		return false;
	}
	return true;
}

bool ai_recruit_result::test_leader_present( const unit_map& units, unit_map::const_iterator& my_leader, bool /*update_knowledge*/ )
{
	my_leader = find_leader(units,get_side());
	if (my_leader == units.end()){
		set_error(E_NO_LEADER);
		return false;
	}
	return true;

}

bool ai_recruit_result::test_leader_on_keep( const gamemap& map, const unit_map::const_iterator& my_leader, bool /*update_knowledge*/ )
{
	if (!map.is_keep(my_leader->first)){
		set_error(E_LEADER_NOT_ON_KEEP);
		return false;
	}
	return true;
}

bool ai_recruit_result::test_suitable_recruit_location( const gamemap& map, const unit_map& units, const unit_map::const_iterator& my_leader, bool /*update_knowledge*/ )
{
	recruit_location_ = where_;

	//if we have not-on-board location, such as null_location, then the caller wants us to recruit on 'any' possible tile.
        if(!map.on_board(recruit_location_)) {
                recruit_location_ = find_vacant_tile(map,units,my_leader->first, VACANT_CASTLE);
	}

	if (!can_recruit_on(map,my_leader->first,recruit_location_)){
		set_error(E_BAD_RECRUIT_LOCATION);
		return false;
	}
	return true;
}

void ai_recruit_result::do_check_before()
{
	DBG_AI_ACTIONS << " check_before " << *this << std::endl;
	const ai_interface::info& s_info = get_subjective_info();
	const ai_interface::info& info = get_info();

	const unit_map& s_units = s_info.units;
	const unit_map& units = info.units;

	const team& s_team = get_my_team(s_info);
	const team& team = get_my_team(info);

	//Unit available for recruiting?
	std::set<std::string>::const_iterator s_recruit;
	std::set<std::string>::const_iterator recruit;

	if ( !test_available_for_recruiting(s_team,s_recruit) ||
		( is_execution() && using_subjective_info() &&
		!test_available_for_recruiting(team,recruit,true) ) ){
		return;
	}

	//Unit type known ?
	unit_type_data::unit_type_map::const_iterator s_unit_type;
	unit_type_data::unit_type_map::const_iterator unit_type;

	if ( !test_unit_type_known(s_recruit,s_unit_type) ||
		( is_execution() && using_subjective_info() &&
		!test_unit_type_known(recruit,s_unit_type,true) ) ){ 
		return;
	}

	//Enough gold?		
	if (!test_enough_gold(s_team,s_unit_type) ||
		( is_execution() && using_subjective_info() &&
		!test_enough_gold(team,unit_type,true) ) ){
		return;
	}

	//Leader present?
	unit_map::const_iterator s_my_leader;
	unit_map::const_iterator my_leader;

	if (!test_leader_present(s_units,s_my_leader) ||
		( is_execution() && using_subjective_info() &&
		!test_leader_present(units,my_leader,true) ) ){
		return;
	}

	//Leader on keep?
	const gamemap& s_map = s_info.map;
	const gamemap& map = info.map;

	if (!test_leader_on_keep(s_map,s_my_leader) ||
		( is_execution() && using_subjective_info() &&
		!test_leader_on_keep(map,my_leader,true) ) ){
		return;
	}

	//Try to get suitable recruit location. Is suitable location available ?
	if (!test_suitable_recruit_location(s_map,s_units,s_my_leader) ||
		( is_execution() && using_subjective_info() &&
		!test_suitable_recruit_location(map,units,my_leader,true) ) ){
		return;
	}

}


void ai_recruit_result::do_check_after()
{
	const ai_interface::info& info = get_info();
	const gamemap& map = info.map;
	if (!map.on_board(recruit_location_)){
		set_error(AI_ACTION_FAILURE);
	}	

	const unit_map& units = info.units;
	unit_map::const_iterator unit = units.find(recruit_location_);
	if (unit==units.end()){
		set_error(AI_ACTION_FAILURE);
	}
	if (unit->second.side()!=get_side()){
		set_error(AI_ACTION_FAILURE);
	}
	
}

std::string ai_recruit_result::do_describe() const
{
	std::stringstream s;
	s << "recruitment by side ";
	s << get_side();
	s << " of unit type ["<<unit_name_;
	if (where_ != map_location::null_location){
		s << "] on location "<<where_;
	} else {
		s << "] on any suitable location";
	}
	s <<std::endl;
	return s.str();
}


void ai_recruit_result::do_execute()
{
	DBG_AI_ACTIONS << " execute: " << do_describe() << std::endl;
	assert(is_success());
	const ai_interface::info& info = get_info();
	// We have to add the recruit command now, because when the unit
	// is created it has to have the recruit command in the recorder
	// to be able to put random numbers into to generate unit traits.
	// However, we're not sure if the transaction will be successful,
	// so use a replay_undo object to cancel it if we don't get
	// a confirmation for the transaction.
	recorder.add_recruit(num_,recruit_location_);
	replay_undo replay_guard(recorder);
	unit_type_data::unit_type_map::const_iterator u = unit_type_data::types().find_unit_type(unit_name_);
	unit new_unit(&get_info().units,&get_info().map,&get_info().state,&get_info().teams,&u->second,get_side(),true);
	std::string recruit_err = recruit_unit(get_info().map,get_side(),get_info().units,new_unit,recruit_location_,false,preferences::show_ai_moves());
	if(recruit_err.empty()) {
		statistics::recruit_unit(new_unit);
		get_my_team(info).spend_gold(u->second.cost());
		// Confirm the transaction - i.e. don't undo recruitment
		replay_guard.confirm_transaction();
		ai_manager::raise_unit_recruited();
	} else {
		set_error(AI_ACTION_FAILURE);
	}


}


void ai_recruit_result::do_init_for_execution()
{
}

std::ostream &operator<<(std::ostream &s, ai_recruit_result const &r) {
        s << r.do_describe();
        return s;
}



// ai_stopunit_result
ai_stopunit_result::ai_stopunit_result( unsigned int side, const map_location& unit_location, bool remove_movement, bool remove_attacks)
	: ai_action_result(side), unit_location_(unit_location), remove_movement_(remove_movement), remove_attacks_(remove_attacks)
{
}

void ai_stopunit_result::do_check_before()
{
}


void ai_stopunit_result::do_check_after()
{
}

std::string ai_stopunit_result::do_describe() const
{
	std::stringstream s;
	s <<" stopunit by side ";
	s << get_side();
	if (remove_movement_){
		s << " : remove movenent";
	}
	if (remove_attacks_){
		s << " remove attacks";
	} 
	s << "from unit on location "<<unit_location_;
	s <<std::endl;
	return s.str();
}

void ai_stopunit_result::do_execute()
{
}


void ai_stopunit_result::do_init_for_execution()
{
}


// =======================================================================
// STATELESS INTERFACE TO AI ACTIONS
// =======================================================================

std::auto_ptr< ai_attack_result > ai_actions::execute_attack_action( unsigned int side,
	bool execute,
	const map_location& attacker_loc,
	const map_location& defender_loc,
	int attacker_weapon)
{
	std::auto_ptr< ai_attack_result > ai_action(new ai_attack_result(side,attacker_loc,defender_loc,attacker_weapon));
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;
}


std::auto_ptr< ai_move_result > ai_actions::execute_move_action( unsigned int side,
	bool execute,
	const map_location& from,
	const map_location& to,
	bool remove_movement)
{
	std::auto_ptr< ai_move_result > ai_action(new ai_move_result(side,from,to,remove_movement));
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;

}


std::auto_ptr< ai_recruit_result > ai_actions::execute_recruit_action( unsigned int side,
	bool execute,
	const std::string& unit_name,
	const map_location& where)
{
	std::auto_ptr< ai_recruit_result > ai_action(new ai_recruit_result(side,unit_name,where));
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;

}


std::auto_ptr< ai_stopunit_result > ai_actions::execute_stopunit_action( unsigned int side,
	bool execute,
	const map_location& unit_location,
	bool remove_movement,
	bool remove_attacks)
{
	std::auto_ptr< ai_stopunit_result > ai_action(new ai_stopunit_result(side,unit_location,remove_movement,remove_attacks));
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;

}
