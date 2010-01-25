/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
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
#include "play_controller.hpp"
#include "../replay.hpp"
#include "resources.hpp"
#include "../statistics.hpp"
#include "../team.hpp"

#include <boost/bind.hpp>

namespace ai {

static lg::log_domain log_ai_actions("ai/actions");
#define DBG_AI_ACTIONS LOG_STREAM(debug, log_ai_actions)
#define LOG_AI_ACTIONS LOG_STREAM(info, log_ai_actions)
#define WRN_AI_ACTIONS LOG_STREAM(warn, log_ai_actions)
#define ERR_AI_ACTIONS LOG_STREAM(err, log_ai_actions)

// =======================================================================
// AI ACTIONS
// =======================================================================
action_result::action_result( side_number side )
	: return_value_checked_(true),side_(side),status_(AI_ACTION_SUCCESS),is_execution_(false),is_gamestate_changed_(false)
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
		try {
			resources::controller->check_victory();
		} catch (...) {
			is_ok(); //Silences "unchecked result" warning
			throw;
		}
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


void action_result::set_error(int error_code, bool log_as_error){
	status_ = error_code;
	if (is_execution()) {
		if (log_as_error) {
			ERR_AI_ACTIONS << "Error #"<<error_code<<" in "<< do_describe();
		} else {
			LOG_AI_ACTIONS << "Error #"<<error_code<<" in "<< do_describe();
		}
	} else {
		LOG_AI_ACTIONS << "Error #"<<error_code<<" when checking "<< do_describe();
	}
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
attack_result::attack_result( side_number side, const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon, double aggression)
	: action_result(side), attacker_loc_(attacker_loc), defender_loc_(defender_loc), attacker_weapon_(attacker_weapon), aggression_(aggression){
}

const unit* attack_result::get_unit(game_info &info, const map_location &loc) const
{
	unit_map &units = info.units;
	unit_map::const_iterator u = units.find(loc);
	if (u == units.end()){
		return NULL;
	}
	return &u->second;

}

void attack_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const unit *attacker = get_unit(get_info(),attacker_loc_);
	const unit *defender = get_unit(get_info(),defender_loc_);

	if(attacker==NULL)
	{
		LOG_AI_ACTIONS << "attempt to attack without attacker\n";
		set_error(E_EMPTY_ATTACKER);
		return;
	}

	if (defender==NULL)
	{
		LOG_AI_ACTIONS << "attempt to attack without defender\n";
		set_error(E_EMPTY_DEFENDER);
		return;
	}

	if(attacker->incapacitated()) {
		LOG_AI_ACTIONS << "attempt to attack with unit that is petrified\n";
		set_error(E_INCAPACITATED_ATTACKER);
		return;
	}

	if(defender->incapacitated()) {
		LOG_AI_ACTIONS << "attempt to attack unit that is petrified\n";
		set_error(E_INCAPACITATED_DEFENDER);
		return;
	}

	if(!attacker->attacks_left()) {
		LOG_AI_ACTIONS << "attempt to attack with no attacks left\n";
		set_error(E_NO_ATTACKS_LEFT);
		return;
	}

	if(attacker->side()!=get_side()) {
		LOG_AI_ACTIONS << "attempt to attack with not own unit\n";
		set_error(E_NOT_OWN_ATTACKER);
		return;
	}

	if(!get_my_team(get_info()).is_enemy(defender->side())) {
		LOG_AI_ACTIONS << "attempt to attack unit that is not enemy\n";
		set_error(E_NOT_ENEMY_DEFENDER);
		return;
	}

	if (attacker_weapon_!=-1) {
		if ((attacker_weapon_<0)||(attacker_weapon_ > static_cast<int>(attacker->attacks().size()))) {
			LOG_AI_ACTIONS << "invalid weapon selection for the attacker\n";
			set_error(E_WRONG_ATTACKER_WEAPON);
			return;
		}
	}

	if (!tiles_adjacent(attacker_loc_,defender_loc_)) {
		LOG_AI_ACTIONS << "attacker and defender not adjacent\n";
		set_error(E_ATTACKER_AND_DEFENDER_NOT_ADJACENT);
		return;
	}
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
	s << " with aggression "<< aggression_;
	s <<std::endl;
	return s.str();
}


void attack_result::do_execute()
{
	LOG_AI_ACTIONS << "start of execution of: "<< *this << std::endl;
	// Stop the user from issuing any commands while the unit is attacking
	const events::command_disabler disable_commands;

	//@note: yes, this is a decision done here. It's that way because we want to allow a simpler attack 'with whatever weapon is considered best', and because we want to allow the defender to pick it's weapon. That's why aggression is needed. a cleaner solution is needed.
	battle_context bc(get_info().units, attacker_loc_,
		defender_loc_, attacker_weapon_, -1, aggression_);

	int attacker_weapon = bc.get_attacker_stats().attack_num;
	int defender_weapon = bc.get_defender_stats().attack_num;

	if(attacker_weapon < 0) {
		set_error(E_UNABLE_TO_CHOOSE_ATTACKER_WEAPON);
		return;
	}

	const unit *d_ = get_unit(get_info(),defender_loc_);
	const unit *a_ = get_unit(get_info(),attacker_loc_);

	//@todo 1.9: change ToD to be location specific for the defender unit
	recorder.add_attack(attacker_loc_, defender_loc_, attacker_weapon, defender_weapon, a_->type_id(),
		d_->type_id(), a_->level(), d_->level(), resources::tod_manager->turn(),
		resources::tod_manager->get_time_of_day());
	rand_rng::invalidate_seed();
	rand_rng::clear_new_seed_callback();
	while (!rand_rng::has_valid_seed()) {
		manager::raise_user_interact();
		manager::raise_sync_network();
		SDL_Delay(10);
	}
	recorder.add_seed("attack", rand_rng::get_last_seed());
	attack_unit(attacker_loc_, defender_loc_, attacker_weapon, defender_weapon);
	dialogs::advance_unit(attacker_loc_, true);

	const unit_map::const_iterator defender = get_info().units.find(defender_loc_);
	if(defender != get_info().units.end()) {
		const size_t defender_team = size_t(defender->second.side()) - 1;
		if(defender_team < get_info().teams.size()) {
			dialogs::advance_unit(defender_loc_ , !get_info().teams[defender_team].is_human());
		}
	}

	set_gamestate_changed();
	//start of ugly hack. @todo 1.9 rework that via extended event system
	//until event system is reworked, we note the attack this way
	get_info().recent_attacks.insert(defender_loc_);
	//end of ugly hack
	try {
		manager::raise_gamestate_changed();
	} catch (...) {
		is_ok(); //Silences "unchecked result" warning
		throw;
	}
}


void attack_result::do_init_for_execution()
{
}




// move_result
move_result::move_result(side_number side, const map_location& from,
		const map_location& to, bool remove_movement)
	: action_result(side)
	, from_(from)
	, move_spectator_(get_info().units)
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
	if (from_== to_) {
		if (!remove_movement_ || (un.movement_left() == 0) ) {
			set_error(E_EMPTY_MOVE);
			return false;
		}
		return true;
	}

	if (un.movement_left() == 0 ) {
		set_error(E_EMPTY_MOVE);
		return false;
	}
	const pathfind::shortest_path_calculator calc(un, my_team, units, teams,map);

	//allowed teleports
	std::set<map_location> allowed_teleports = pathfind::get_teleport_locations(un, units, my_team, true);//@todo 1.9: see_all -> false

	//do an A*-search
	route_ = pathfind::a_star_search(un.get_location(), to_, 10000.0, &calc, map.w(), map.h(), &allowed_teleports);
	if (route_.steps.empty()) {
		set_error(E_NO_ROUTE);
		return false;
	}
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


const move_unit_spectator& move_result::get_move_spectator() const
{
	return move_spectator_;
}

void move_result::do_check_after()
{
	if (move_spectator_.get_ambusher().valid()) {
		set_error(E_AMBUSHED,false);
		return;
	}
	if (move_spectator_.get_failed_teleport().valid()) {
		set_error(E_FAILED_TELEPORT);
		return;
	}
	//@todo 1.9 add 'new units spotted' failure mode

	if (unit_location_!=to_) {
		set_error(E_NOT_REACHED_DESTINATION);
		return;
	}
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

	move_spectator_.set_unit(get_info().units.find(from_));

	if (from_ != to_) {
		move_unit(
			/*move_unit_spectator* move_spectator*/ &move_spectator_,
			/*std::vector<map_location> route*/ route_.steps,
			/*replay* move_recorder*/ &recorder,
			/*undo_list* undo_stack*/ NULL,
			/*bool show_move*/ preferences::show_ai_moves(),
			/*map_location *next_unit*/ NULL,
			/*bool continue_move*/ true, //@todo: 1.9 set to false after implemeting interrupt awareness
			/*bool should_clear_shroud*/ true,
			/*bool is_replay*/ false);

		if ( move_spectator_.get_ambusher().valid() || !move_spectator_.get_seen_enemies().empty() || !move_spectator_.get_seen_friends().empty() ) {
			set_gamestate_changed();
		} else if (move_spectator_.get_unit().valid()){
			unit_location_ = move_spectator_.get_unit()->first;
			if (unit_location_ != from_) {
				set_gamestate_changed();
			}
		}
	} else {
		assert(remove_movement_);
	}

	if (move_spectator_.get_unit().valid()){
		unit_location_ = move_spectator_.get_unit()->first;
		if ( remove_movement_ && ( move_spectator_.get_unit()->second.movement_left() > 0 ) && (unit_location_==to_)) {
			stopunit_result_ptr stopunit_res = actions::execute_stopunit_action(get_side(),true,unit_location_,true,false);
			if (!stopunit_res->is_ok()) {
				set_error(stopunit_res->get_status());
			}
			if (stopunit_res->is_gamestate_changed()) {
				set_gamestate_changed();
			}
		}
	} else {
		unit_location_ = map_location();
	}

	if (is_gamestate_changed()) {
		try {
			manager::raise_gamestate_changed();
		} catch (...) {
			is_ok(); //Silences "unchecked result" warning
			throw;
		}
	}
}


void move_result::do_init_for_execution()
{
	move_spectator_.reset(get_info().units);
}



// recall_result
recall_result::recall_result(side_number side,
		const std::string& unit_id, const map_location& where)
	: action_result(side)
	, unit_id_(unit_id)
	, where_(where)
	, recall_location_(where)
{
}

bool recall_result::test_available_for_recalling(const team &my_team, bool)
{
	const std::vector<unit>::const_iterator rec = std::find_if(my_team.recall_list().begin(), my_team.recall_list().end(), boost::bind(&unit::matches_id, _1, unit_id_));
	if (rec == my_team.recall_list().end()) {
		set_error(E_NOT_AVAILABLE_FOR_RECALLING);
		return false;
	}
	return true;
}


bool recall_result::test_enough_gold(const team &my_team,  bool)
{
	if (my_team.gold() < game_config::recall_cost ) {
		set_error(E_NO_GOLD);
		return false;
	}
	return true;
}

const unit *recall_result::get_leader(const unit_map& units, bool)
{
	unit_map::const_iterator my_leader = units.find_leader(get_side());
	if (my_leader == units.end()){
		set_error(E_NO_LEADER);
		return NULL;
	}
	return &my_leader->second;

}

bool recall_result::test_leader_on_keep(const gamemap &map, const unit &my_leader, bool)
{
	if (!map.is_keep(my_leader.get_location())) {
		set_error(E_LEADER_NOT_ON_KEEP);
		return false;
	}
	return true;
}

bool recall_result::test_suitable_recall_location(const gamemap &map, const unit_map &units, const unit &my_leader, bool)
{
	recall_location_ = where_;

	//if we have not-on-board location, such as null_location, then the caller wants us to recall on 'any' possible tile.
	if (!map.on_board(recall_location_)) {
		recall_location_ = pathfind::find_vacant_tile(map, units, my_leader.get_location(), pathfind::VACANT_CASTLE);
	}

	if (!can_recruit_on(map, my_leader.get_location(), recall_location_)) {
		set_error(E_BAD_RECALL_LOCATION);
		return false;
	}
	return true;
}

void recall_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const game_info& s_info = get_subjective_info();
	const game_info& info = get_info();

	const unit_map& s_units = s_info.units;
	const unit_map& units = info.units;

	const team& s_my_team = get_my_team(s_info);
	const team& my_team = get_my_team(info);

	//Unit available for recalling?
	if ( !test_available_for_recalling(s_my_team) ||
	    (is_execution() && using_subjective_info() &&
	     !test_available_for_recalling(my_team, true)))
	{
		return;
	}

	//Enough gold?
	if (!test_enough_gold(s_my_team) ||
	    (is_execution() && using_subjective_info() &&
	     !test_enough_gold(my_team, true)))
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

	//Try to get suitable recall location. Is suitable location available ?
	if (!test_suitable_recall_location(s_map, s_units, *s_my_leader) ||
	    (is_execution() && using_subjective_info() &&
	     !test_suitable_recall_location(map, units, *s_my_leader, true)))
	{
		return;
	}

}


void recall_result::do_check_after()
{
	const game_info& info = get_info();
	const gamemap& map = info.map;
	if (!map.on_board(recall_location_)){
		set_error(AI_ACTION_FAILURE);
		return;
	}

	const unit_map& units = info.units;
	unit_map::const_iterator unit = units.find(recall_location_);
	if (unit==units.end()){
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (unit->second.side()!=get_side()){
		set_error(AI_ACTION_FAILURE);
		return;
	}

}

std::string recall_result::do_describe() const
{
	std::stringstream s;
	s << "recall by side ";
	s << get_side();
	s << " of unit id ["<<unit_id_;
	if (where_ != map_location::null_location){
		s << "] on location "<<where_;
	} else {
		s << "] on any suitable location";
	}
	s <<std::endl;
	return s.str();
}


void recall_result::do_execute()
{
	LOG_AI_ACTIONS << "start of execution of: " << *this << std::endl;
	assert(is_success());

	game_info& info = get_info();
	team& my_team = get_my_team(info);

	const events::command_disabler disable_commands;

	std::vector<unit>::iterator rec = std::find_if(my_team.recall_list().begin(), my_team.recall_list().end(), boost::bind(&unit::matches_id, _1, unit_id_));

	assert(rec != my_team.recall_list().end());

	const std::string &err = find_recruit_location(get_side(), recall_location_);
	if(!err.empty()) {
		set_error(AI_ACTION_FAILURE);
		return;
	} else {

		unit &un = *rec;
		recorder.add_recall(un.id(), recall_location_);
		un.set_game_context(&info.units);
		place_recruit(un, recall_location_, true, true);
		statistics::recall_unit(un);
		my_team.spend_gold(game_config::recall_cost);

		my_team.recall_list().erase(rec);
		if (resources::screen!=NULL) {
			resources::screen->invalidate_game_status();
			resources::screen->invalidate_all();
		}
		recorder.add_checksum_check(recall_location_);
		set_gamestate_changed();
		try {
			manager::raise_gamestate_changed();
		} catch (...) {
			is_ok(); //Silences "unchecked result" warning
			throw;
		}
	}

}


void recall_result::do_init_for_execution()
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
	const unit_type *type = unit_types.find(recruit);
	if (!type) {
		set_error(E_UNKNOWN_OR_DUMMY_UNIT_TYPE);
		return NULL;
	}
	return type;
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
		recruit_location_ = pathfind::find_vacant_tile(map, units, my_leader.get_location(), pathfind::VACANT_CASTLE);
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
		return;
	}

	const unit_map& units = info.units;
	unit_map::const_iterator unit = units.find(recruit_location_);
	if (unit==units.end()){
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (unit->second.side()!=get_side()){
		set_error(AI_ACTION_FAILURE);
		return;
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
	const unit_type *u = unit_types.find(unit_name_);
	const events::command_disabler disable_commands;
	const std::string recruit_err = find_recruit_location(get_side(), recruit_location_);
	if(recruit_err.empty()) {
		const unit new_unit(&info.units, u, get_side(), true);
		place_recruit(new_unit, recruit_location_, false, preferences::show_ai_moves());
		statistics::recruit_unit(new_unit);
		get_my_team(info).spend_gold(u->cost());
		// Confirm the transaction - i.e. don't undo recruitment
		replay_guard.confirm_transaction();
		set_gamestate_changed();
		try {
			manager::raise_gamestate_changed();
		} catch (...) {
			is_ok(); //Silences "unchecked result" warning
			throw;
		}
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
		s << " : remove movement ";
	}
	if (remove_movement_ && remove_attacks_){
		s << "and ";
	}
	if (remove_attacks_){
		s << " remove attacks ";
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
	try {
		if (remove_movement_){
			un->second.remove_movement_ai();
			set_gamestate_changed();
			manager::raise_gamestate_changed();
		}
		if (remove_attacks_){
			un->second.remove_attacks_ai();
			set_gamestate_changed();
			manager::raise_gamestate_changed();//to be on the safe side
		}
	} catch (...) {
		is_ok(); //Silences "unchecked result" warning
		throw;
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
	int attacker_weapon,
	double aggression)
{
	attack_result_ptr action(new attack_result(side,attacker_loc,defender_loc,attacker_weapon,aggression));
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


recall_result_ptr actions::execute_recall_action( side_number side,
	bool execute,
	const std::string& unit_id,
	const map_location& where)
{
	recall_result_ptr action(new recall_result(side,unit_id,where));
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


std::ostream &operator<<(std::ostream &s, ai::recall_result const &r) {
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
