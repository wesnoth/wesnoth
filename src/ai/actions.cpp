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
 * @file ai/actions.cpp
 */

/**
 * A small explanation about what's going on here:
 * Each action has access to two game_info objects
 * First is 'info' - real information
 * Second is 'subjective info' - AIs perception of what's going on
 * So, when we check_before action, we use 'subjective info' and don't
 * touch real 'info' at all.
 * But when we actually want to execute an action, we firstly check
 * 'subjective info' and then (if subjective check is ok) do the same
 * check on  real 'info'. There's a caveat: if we fail an action based
 * on real 'info', then we NEED to update AIs knowledge to avoid the ai
 * doing the same thing again.
 * So far the use of 'subjective info' is stubbed out.
 */

#include "actions.hpp"
#include "manager.hpp"

#include "../actions.hpp"
#include "../dialogs.hpp"
#include "../game_end_exceptions.hpp"
#include "../game_preferences.hpp"
#include "../log.hpp"
#include "../mouse_handler_base.hpp"
#include "../pathfind.hpp"
#include "../replay.hpp"
#include "../statistics.hpp"
#include "../team.hpp"

namespace ai {

static lg::log_domain log_ai_actions("ai/actions");
#define DBG_AI_ACTIONS LOG_STREAM(debug, log_ai_actions)
#define LOG_AI_ACTIONS LOG_STREAM(info, log_ai_actions)
#define WRN_AI_ACTIONS LOG_STREAM(warn, log_ai_actions)
#define ERR_AI_ACTIONS LOG_STREAM(err, log_ai_actions)

using namespace ai;

// =======================================================================
// AI ACTIONS
// =======================================================================
action_result::action_result( side_number side )
	: return_value_checked_(true),side_(side),status_(AI_ACTION_SUCCESS),is_execution_(false)
{
}


action_result::~action_result()
{
	if (!return_value_checked_) {
		ERR_AI_ACTIONS << "Return value of AI ACTION was not checked. This may cause bugs! " << std::endl;
	}
}


void action_result::check_after()
{
	do_check_after();
}


void action_result::check_before()
{
	do_check_before();
}


void action_result::execute()
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

void action_result::init_for_execution()
{
	return_value_checked_ = false;
	is_gamestate_changed_ = false;
	status_ =  action_result::AI_ACTION_SUCCESS;
	do_init_for_execution();
}


bool action_result::is_gamestate_changed() const
{
	return is_gamestate_changed_;
}


bool action_result::is_ok()
{
	return_value_checked_ = true;
	return is_success();
}


void action_result::set_error(int error_code){
	status_ = error_code;
	ERR_AI_ACTIONS << "Error #"<<error_code<<" in "<< do_describe();
}


void action_result::set_gamestate_changed()
{
	is_gamestate_changed_ = true;
}


int action_result::get_status(){
	return status_;
}

bool action_result::is_success() const
{
	return (status_ == action_result::AI_ACTION_SUCCESS);
}


bool action_result::is_execution() const
{
	return is_execution_;
}

game_info& action_result::get_info() const
{
	return manager::get_active_ai_info_for_side(get_side());
}


game_info& action_result::get_subjective_info() const
{
	return get_info();
}


bool action_result::using_subjective_info() const
{
	return false;
}


team& action_result::get_my_team(game_info& info) const
{
	return info.teams[side_-1];
}

const team& action_result::get_my_team(const game_info& info) const
{
	return info.teams[side_-1];
}

// attack_result
attack_result::attack_result( side_number side, const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon)
	: action_result(side), attacker_loc_(attacker_loc), defender_loc_(defender_loc), attacker_weapon_(attacker_weapon){
}

void attack_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
}


void attack_result::do_check_after()
{
}


std::string attack_result::do_describe() const
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


void attack_result::do_execute()
{
	LOG_AI_ACTIONS << "start of execution of: "<< *this << std::endl;
	// Stop the user from issuing any commands while the unit is attacking
	const events::command_disabler disable_commands;

	if(!get_info().units.count(attacker_loc_))
	{
		ERR_AI_ACTIONS << "attempt to attack without attacker\n";
		set_error(E_EMPTY_ATTACKER);
		return;
	}

	if (!get_info().units.count(defender_loc_))
	{
		ERR_AI_ACTIONS << "attempt to attack without defender\n";
		set_error(E_EMPTY_DEFENDER);
		return;
	}

	if(get_info().units.find(attacker_loc_)->second.incapacitated()) {
		ERR_AI_ACTIONS << "attempt to attack with unit that is petrified\n";
		set_error(E_INCAPACITATED_ATTACKER);
		return;
	}

	if(get_info().units.find(defender_loc_)->second.incapacitated()) {
		ERR_AI_ACTIONS << "attempt to attack unit that is petrified\n";
		set_error(E_INCAPACITATED_DEFENDER);
		return;
	}

	if(!get_info().units.find(attacker_loc_)->second.attacks_left()) {
		ERR_AI_ACTIONS << "attempt to attack with no attacks left\n";
		set_error(E_NO_ATTACKS_LEFT);
		return;
	}

	//CHECK OWN(ATTACKER)
	//CHECK ENEMY(DEFENDER)
	//CHECK ATTACKER WEAPON

	battle_context bc(get_info().map, get_info().teams, get_info().units,
		get_info().state, get_info().tod_manager_, attacker_loc_,
		defender_loc_, attacker_weapon_, -1, get_my_team(get_info()).aggression());

	int attacker_weapon = bc.get_attacker_stats().attack_num;
	int defender_weapon = bc.get_defender_stats().attack_num;

	if(attacker_weapon_ >= 0) {
		recorder.add_attack(attacker_loc_,defender_loc_,attacker_weapon,defender_weapon);
	}
	try {
		attack(get_info().disp, get_info().map, get_info().teams, attacker_loc_,
			defender_loc_,attacker_weapon,defender_weapon,	get_info().units, 
			get_info().state, get_info().tod_manager_);
	}
	catch (end_level_exception&)
	{
		dialogs::advance_unit(get_info().map,get_info().units,attacker_loc_,get_info().disp,true);

		const unit_map::const_iterator defender = get_info().units.find(defender_loc_);
		if(defender != get_info().units.end()) {
			const size_t defender_team = size_t(defender->second.side()) - 1;
			if(defender_team < get_info().teams.size()) {
				dialogs::advance_unit(get_info().map, get_info().units,
						defender_loc_, get_info().disp, !get_info().teams[defender_team].is_human());
			}
		}

		throw;
	}
	dialogs::advance_unit(get_info().map,get_info().units,attacker_loc_,get_info().disp,true);

	const unit_map::const_iterator defender = get_info().units.find(defender_loc_);
	if(defender != get_info().units.end()) {
		const size_t defender_team = size_t(defender->second.side()) - 1;
		if(defender_team < get_info().teams.size()) {
			dialogs::advance_unit(get_info().map, get_info().units,
					defender_loc_ , get_info().disp, !get_info().teams[defender_team].is_human());
		}
	}

	check_victory(get_info().state,get_info().units,get_info().teams, get_info().disp);
	manager::raise_enemy_attacked();

}


void attack_result::do_init_for_execution()
{
}




// move_result
move_result::move_result(side_number side, const map_location& from,
		const map_location& to, bool remove_movement)
	: action_result(side)
	, from_(from)
	, to_(to)
	, remove_movement_(remove_movement)
	, route_()
	, unit_location_(from)
{
}


const unit *move_result::get_unit(const unit_map &units, const std::vector<team> &, bool)
{
	unit_map::const_iterator un = units.find(from_);
	if (un==units.end()){
		set_error(E_NO_UNIT);
		return NULL;
	}
	const unit *u = &un->second;
	if (u->side() != get_side()) {
		set_error(E_NOT_OWN_UNIT);
		return NULL;
	}
	if (u->incapacitated()) {
		set_error(E_INCAPACITATED_UNIT);
		return NULL;
	}
	return u;
}


bool move_result::test_route(const unit &un, const team &my_team, const unit_map &units, const std::vector<team> &teams, const gamemap &map, bool)
{
	if (from_==to_) {
		set_error(E_EMPTY_MOVE);
		return false;
	}
	const shortest_path_calculator calc(un, my_team, units, teams,map);

	//allowed teleports
	std::set<map_location> allowed_teleports;

	//@todo 1.7: calculate allowed teleports

	//do an A*-search
	route_ = a_star_search(un.get_location(), to_, 10000.0, &calc, map.w(), map.h(), &allowed_teleports);
	return true;
}

void move_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const game_info& s_info = get_subjective_info();
	const game_info& info = get_info();

	const unit_map& s_units = s_info.units;
	const unit_map& units = info.units;

	const gamemap& s_map = s_info.map;

	const team& s_my_team = get_my_team(s_info);

	const std::vector<team> &s_teams = s_info.teams;
	const std::vector<team> &teams = info.teams;

	const unit *s_unit = get_unit(s_units, s_teams);
	if (!s_unit || (is_execution() && using_subjective_info() && !get_unit(units, teams, true)))
	{
		return;
	}

	if (!test_route(*s_unit, s_my_team, s_units, s_teams, s_map)) {
		//impossible to test using real info without revealing anything
		//prematurely, since moves are done 'in steps'
		return;
	}
}


const map_location& move_result::get_unit_location() const
{
	return unit_location_;
}


void move_result::do_check_after()
{
}


std::string move_result::do_describe() const
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


void move_result::do_execute()
{
	LOG_AI_ACTIONS << "start of execution of: "<< *this << std::endl;
	assert(is_success());

	game_info& info = get_info();

	move_unit(
		/*game_display* disp*/ &info.disp,
                /*const gamemap& map*/ info.map,
                /*unit_map& units*/ info.units,
		/*std::vector<team>& teams*/ info.teams,
                /*std::vector<map_location> route*/ route_.steps,
                /*replay* move_recorder*/ &recorder,
		/*undo_list* undo_stack*/ NULL,
                /*map_location *next_unit*/ NULL,
		/*bool continue_move*/ true, //@todo: 1.7 set to false after implemeting interrupt awareness
                /*bool should_clear_shroud*/ true,
		/*bool is_replay*/ false);
	unit_location_ = to_;//@todo: 1.7 modify move_unit to get this info from it

}


void move_result::do_init_for_execution()
{
}




// recruit_result
recruit_result::recruit_result(side_number side,
		const std::string& unit_name, const map_location& where)
	: action_result(side)
	, unit_name_(unit_name)
	, where_(where)
	, recruit_location_(where)
	, num_(0)
{
}

const std::string &recruit_result::get_available_for_recruiting(const team &my_team, bool)
{
	const std::set<std::string> &recruit_set = my_team.recruits();
	std::set<std::string>::const_iterator recruit = recruit_set.find(unit_name_);
	if (recruit == recruit_set.end()) {
		set_error(E_NOT_AVAILABLE_FOR_RECRUITING);
		static std::string dummy;
		return dummy;
	}
	num_ = std::distance(recruit_set.begin(),recruit);
	return *recruit;
}

const unit_type *recruit_result::get_unit_type_known(const std::string &recruit, bool)
{
	unit_type_data::unit_type_map::const_iterator type = unit_type_data::types().find_unit_type(recruit);
	if (type == unit_type_data::types().end() || type->first == "dummy_unit") {
		set_error(E_UNKNOWN_OR_DUMMY_UNIT_TYPE);
		return NULL;
	}
	return &type->second;
}

bool recruit_result::test_enough_gold(const team &my_team, const unit_type &type, bool)
{
	if (my_team.gold() < type.cost()) {
		set_error(E_NO_GOLD);
		return false;
	}
	return true;
}

const unit *recruit_result::get_leader(const unit_map& units, bool)
{
	unit_map::const_iterator my_leader = units.find_leader(get_side());
	if (my_leader == units.end()){
		set_error(E_NO_LEADER);
		return NULL;
	}
	return &my_leader->second;

}

bool recruit_result::test_leader_on_keep(const gamemap &map, const unit &my_leader, bool)
{
	if (!map.is_keep(my_leader.get_location())) {
		set_error(E_LEADER_NOT_ON_KEEP);
		return false;
	}
	return true;
}

bool recruit_result::test_suitable_recruit_location(const gamemap &map, const unit_map &units, const unit &my_leader, bool)
{
	recruit_location_ = where_;

	//if we have not-on-board location, such as null_location, then the caller wants us to recruit on 'any' possible tile.
	if (!map.on_board(recruit_location_)) {
		recruit_location_ = find_vacant_tile(map, units, my_leader.get_location(), VACANT_CASTLE);
	}

	if (!can_recruit_on(map, my_leader.get_location(), recruit_location_)) {
		set_error(E_BAD_RECRUIT_LOCATION);
		return false;
	}
	return true;
}

void recruit_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const game_info& s_info = get_subjective_info();
	const game_info& info = get_info();

	const unit_map& s_units = s_info.units;
	const unit_map& units = info.units;

	const team& s_my_team = get_my_team(s_info);
	const team& my_team = get_my_team(info);

	//Unit available for recruiting?
	const std::string &s_recruit = get_available_for_recruiting(s_my_team);

	if (s_recruit.empty() ||
	    (is_execution() && using_subjective_info() &&
	     get_available_for_recruiting(my_team, true).empty()))
	{
		return;
	}

	//Unit type known ?
	const unit_type *s_type = get_unit_type_known(s_recruit);
	if (!s_type ||
	    (is_execution() && using_subjective_info() &&
	     !get_unit_type_known(s_recruit, true)))
	{
		return;
	}

	//Enough gold?
	if (!test_enough_gold(s_my_team, *s_type) ||
	    (is_execution() && using_subjective_info() &&
	     !test_enough_gold(my_team, *s_type, true)))
	{
		return;
	}

	//Leader present?
	const unit *s_my_leader = get_leader(s_units);

	if (!s_my_leader ||
	    (is_execution() && using_subjective_info() &&
	     !get_leader(units, true)))
	{
		return;
	}

	//Leader on keep?
	const gamemap& s_map = s_info.map;
	const gamemap& map = info.map;

	if (!test_leader_on_keep(s_map, *s_my_leader) ||
	    (is_execution() && using_subjective_info() &&
	     !test_leader_on_keep(map, *s_my_leader, true)))
	{
		return;
	}

	//Try to get suitable recruit location. Is suitable location available ?
	if (!test_suitable_recruit_location(s_map, s_units, *s_my_leader) ||
	    (is_execution() && using_subjective_info() &&
	     !test_suitable_recruit_location(map, units, *s_my_leader, true)))
	{
		return;
	}

}


void recruit_result::do_check_after()
{
	const game_info& info = get_info();
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

std::string recruit_result::do_describe() const
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


void recruit_result::do_execute()
{
	LOG_AI_ACTIONS << "start of execution of: " << *this << std::endl;
	assert(is_success());
	game_info& info = get_info();
	// We have to add the recruit command now, because when the unit
	// is created it has to have the recruit command in the recorder
	// to be able to put random numbers into to generate unit traits.
	// However, we're not sure if the transaction will be successful,
	// so use a replay_undo object to cancel it if we don't get
	// a confirmation for the transaction.
	recorder.add_recruit(num_,recruit_location_);
	replay_undo replay_guard(recorder);
	unit_type_data::unit_type_map::const_iterator u = unit_type_data::types().find_unit_type(unit_name_);
	unit new_unit(&info.units,&info.map,&info.state,&info.tod_manager_,&info.teams,&u->second,get_side(),true);
	std::string recruit_err = recruit_unit(info.map,get_side(),info.units,new_unit,recruit_location_,false,preferences::show_ai_moves());
	if(recruit_err.empty()) {
		statistics::recruit_unit(new_unit);
		get_my_team(info).spend_gold(u->second.cost());
		// Confirm the transaction - i.e. don't undo recruitment
		replay_guard.confirm_transaction();
		manager::raise_unit_recruited();
	} else {
		set_error(AI_ACTION_FAILURE);
	}


}


void recruit_result::do_init_for_execution()
{
}





// stopunit_result
stopunit_result::stopunit_result( side_number side, const map_location& unit_location, bool remove_movement, bool remove_attacks)
	: action_result(side), unit_location_(unit_location), remove_movement_(remove_movement), remove_attacks_(remove_attacks)
{
}

const unit *stopunit_result::get_unit(const unit_map &units, bool)
{
	unit_map::const_iterator un = units.find(unit_location_);
	if (un==units.end()){
		set_error(E_NO_UNIT);
		return NULL;
	}
	const unit *u = &un->second;
	if (u->side() != get_side()) {
		set_error(E_NOT_OWN_UNIT);
		return NULL;
	}
	if (u->incapacitated()) {
		set_error(E_INCAPACITATED_UNIT);
		return NULL;
	}
	return u;
}

void stopunit_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const game_info& s_info = get_subjective_info();
	const game_info& info = get_info();

	const unit_map& s_units = s_info.units;
	const unit_map& units = info.units;

	if (!get_unit(s_units) || (is_execution() && using_subjective_info() && !get_unit(units, true)))
	{
		return;
	}

}


void stopunit_result::do_check_after()
{
	const game_info& info = get_info();
	unit_map::const_iterator un = info.units.find(unit_location_);
	if (un==info.units.end()){
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (remove_movement_ && (un->second.movement_left()!=0) ){
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (remove_attacks_ && (un->second.attacks_left()!=0) ) {
		set_error(AI_ACTION_FAILURE);
		return;
	}
}

std::string stopunit_result::do_describe() const
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

void stopunit_result::do_execute()
{
	LOG_AI_ACTIONS << "start of execution of: " << *this << std::endl;
	assert(is_success());
	const game_info& info = get_info();
	unit_map::iterator un = info.units.find(unit_location_);
	if (remove_movement_){
		un->second.set_movement(0);
	}
	if (remove_attacks_){
		un->second.set_attacks(0);
	}
}


void stopunit_result::do_init_for_execution()
{
}




// =======================================================================
// STATELESS INTERFACE TO AI ACTIONS
// =======================================================================

attack_result_ptr actions::execute_attack_action( side_number side,
	bool execute,
	const map_location& attacker_loc,
	const map_location& defender_loc,
	int attacker_weapon)
{
	attack_result_ptr action(new attack_result(side,attacker_loc,defender_loc,attacker_weapon));
	execute ? action->execute() : action->check_before();
	return action;
}


move_result_ptr actions::execute_move_action( side_number side,
	bool execute,
	const map_location& from,
	const map_location& to,
	bool remove_movement)
{
	move_result_ptr action(new move_result(side,from,to,remove_movement));
	execute ? action->execute() : action->check_before();
	return action;

}


recruit_result_ptr actions::execute_recruit_action( side_number side,
	bool execute,
	const std::string& unit_name,
	const map_location& where)
{
	recruit_result_ptr action(new recruit_result(side,unit_name,where));
	execute ? action->execute() : action->check_before();
	return action;

}


stopunit_result_ptr actions::execute_stopunit_action( side_number side,
	bool execute,
	const map_location& unit_location,
	bool remove_movement,
	bool remove_attacks)
{
	stopunit_result_ptr action(new stopunit_result(side,unit_location,remove_movement,remove_attacks));
	execute ? action->execute() : action->check_before();
	return action;

}


} //end of namespace ai


std::ostream &operator<<(std::ostream &s, ai::attack_result const &r) {
        s << r.do_describe();
        return s;
}


std::ostream &operator<<(std::ostream &s, ai::move_result const &r) {
        s << r.do_describe();
        return s;
}


std::ostream &operator<<(std::ostream &s, ai::recruit_result const &r) {
        s << r.do_describe();
        return s;
}


std::ostream &operator<<(std::ostream &s, ai::stopunit_result const &r) {
        s << r.do_describe();
        return s;
}
