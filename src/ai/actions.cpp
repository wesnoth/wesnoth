/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Managing the AI-Game interaction - AI actions and their results
 * @file
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

#include "ai/actions.hpp"
#include "ai/manager.hpp"
#include "ai/simulated_actions.hpp"

#include "actions/attack.hpp"
#include "actions/create.hpp"
#include "attack_prediction.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "mouse_handler_base.hpp"
#include "pathfind/teleport.hpp"
#include "play_controller.hpp"
#include "playsingle_controller.hpp"
#include "recall_list_manager.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"
#include "whiteboard/manager.hpp"

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
		DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; //Demotes to DBG "unchecked result" warning
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
		try {
			do_execute();
		} catch (return_to_play_side_exception&) {
			if (!is_ok()) { DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; } //Demotes to DBG "unchecked result" warning
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
			ERR_AI_ACTIONS << "Error #"<<error_code<<" ("<<actions::get_error_name(error_code)<<") in "<< do_describe();
		} else {
			LOG_AI_ACTIONS << "Error #"<<error_code<<" ("<<actions::get_error_name(error_code)<<") in "<< do_describe();
		}
	} else {
		LOG_AI_ACTIONS << "Error #"<<error_code<<" ("<<actions::get_error_name(error_code)<<") when checking "<< do_describe();
	}
}

void action_result::set_gamestate_changed()
{
	is_gamestate_changed_ = true;
}

int action_result::get_status() const
{
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

team& action_result::get_my_team() const
{
	return resources::gameboard->teams()[side_-1];
}


// attack_result
attack_result::attack_result( side_number side, const map_location& attacker_loc, const map_location& defender_loc, int attacker_weapon, double aggression, const unit_advancements_aspect& advancements)
	: action_result(side), attacker_loc_(attacker_loc), defender_loc_(defender_loc), attacker_weapon_(attacker_weapon), aggression_(aggression), advancements_(advancements){
}

void attack_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const unit_map::const_iterator attacker = resources::gameboard->units().find(attacker_loc_);
	const unit_map::const_iterator defender = resources::gameboard->units().find(defender_loc_);

	if(attacker==resources::gameboard->units().end())
	{
		LOG_AI_ACTIONS << "attempt to attack without attacker\n";
		set_error(E_EMPTY_ATTACKER);
		return;
	}

	if (defender==resources::gameboard->units().end())
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

	if(!get_my_team().is_enemy(defender->side())) {
		LOG_AI_ACTIONS << "attempt to attack unit that is not enemy\n";
		set_error(E_NOT_ENEMY_DEFENDER);
		return;
	}

	if (attacker_weapon_!=-1) {
		if ((attacker_weapon_<0)||(attacker_weapon_ >= static_cast<int>(attacker->attacks().size()))) {
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
	battle_context bc(resources::gameboard->units(), attacker_loc_,
		defender_loc_, attacker_weapon_, -1, aggression_);

	int attacker_weapon = bc.get_attacker_stats().attack_num;
	int defender_weapon = bc.get_defender_stats().attack_num;

	if(attacker_weapon < 0) {
		set_error(E_UNABLE_TO_CHOOSE_ATTACKER_WEAPON);
		return;
	}

	const unit_map::const_iterator a_ = resources::gameboard->units().find(attacker_loc_);
	const unit_map::const_iterator d_ = resources::gameboard->units().find(defender_loc_);

	if(resources::simulation_){
		bool gamestate_changed = simulated_attack(attacker_loc_, defender_loc_, bc.get_attacker_combatant().average_hp(), bc.get_defender_combatant().average_hp());

		sim_gamestate_changed(this, gamestate_changed);

		return;
	}

	//FIXME: find a way to 'ask' the ai which advancement should be chosen from synced_commands.cpp .
	if(!synced_context::is_synced()) //RAII block for set_scontext_synced
	{
		wb::real_map rm;
		//we don't use synced_context::run_in_synced_context because that wouldn't allow us to pass advancements_
		resources::recorder->add_synced_command("attack", replay_helper::get_attack(attacker_loc_, defender_loc_, attacker_weapon, defender_weapon, a_->type_id(),
			d_->type_id(), a_->level(), d_->level(), resources::tod_manager->turn(),
			resources::tod_manager->get_time_of_day()));
		set_scontext_synced sync;
		attack_unit_and_advance(attacker_loc_, defender_loc_, attacker_weapon, defender_weapon, true, advancements_);
		resources::controller->check_victory();
		resources::controller->maybe_throw_return_to_play_side();
		sync.do_final_checkup();
	}
	else
	{
		attack_unit_and_advance(attacker_loc_, defender_loc_, attacker_weapon, defender_weapon, true, advancements_);
	}


	set_gamestate_changed();
	//start of ugly hack. @todo 1.9 rework that via extended event system
	//until event system is reworked, we note the attack this way
	get_info().recent_attacks.insert(defender_loc_);
	//end of ugly hack
	try {
		manager::raise_gamestate_changed();
	} catch (...) {
		if (!is_ok()) { DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; } //Demotes to DBG "unchecked result" warning
		throw;
	}
}

void attack_result::do_init_for_execution()
{
}


// move_result
move_result::move_result(side_number side, const map_location& from,
			 const map_location& to, bool remove_movement, bool unreach_is_ok)
	: action_result(side)
	, from_(from)
	, to_(to)
	, remove_movement_(remove_movement)
	, route_()
	, unit_location_(from)
	, unreach_is_ok_(unreach_is_ok)
	, has_ambusher_(false)
	, has_interrupted_teleport_(false)
{
}

const unit *move_result::get_unit()
{
	unit_map::const_iterator un = resources::gameboard->units().find(from_);
	if (un==resources::gameboard->units().end()){
		set_error(E_NO_UNIT);
		return nullptr;
	}
	const unit *u = &*un;
	if (u->side() != get_side()) {
		set_error(E_NOT_OWN_UNIT);
		return nullptr;
	}
	if (u->incapacitated()) {
		set_error(E_INCAPACITATED_UNIT);
		return nullptr;
	}
	return u;
}

bool move_result::test_route(const unit &un)
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

	if (!to_.valid()) {
		set_error(E_NO_ROUTE);
		return false;
	}

	team &my_team = get_my_team();
	const pathfind::shortest_path_calculator calc(un, my_team, resources::gameboard->teams(), resources::gameboard->map());

	//allowed teleports
	pathfind::teleport_map allowed_teleports = pathfind::get_teleport_locations(un, my_team, true);///@todo 1.9: see_all -> false

	//do an A*-search
	route_ = std::shared_ptr<pathfind::plain_route>( new pathfind::plain_route(pathfind::a_star_search(un.get_location(), to_, 10000.0, calc, resources::gameboard->map().w(), resources::gameboard->map().h(), &allowed_teleports)));
	if (route_->steps.empty()) {
		set_error(E_NO_ROUTE);
		return false;
	}
	return true;
}

void move_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const unit *u = get_unit();
	if (!u) {
		return;
	}
	if (!test_route(*u)) {
		return;
	}
}

const map_location& move_result::get_unit_location() const
{
	return unit_location_;
}

void move_result::do_check_after()
{
	if (has_ambusher_) {
		set_error(E_AMBUSHED,false);
		return;
	}
	if (has_interrupted_teleport_) {
		set_error(E_FAILED_TELEPORT);
		return;
	}
	///@todo 1.9 add 'new units spotted' failure mode

	if (!unreach_is_ok_ && unit_location_!=to_) {
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

	if(resources::simulation_){
		bool gamestate_changed = false;
		if(from_ != to_){
			int step = route_->steps.size();
			gamestate_changed = simulated_move(get_side(), from_, to_, step, unit_location_);
		} else {
			assert(remove_movement_);
		}

		unit_map::const_iterator un = resources::gameboard->units().find(unit_location_);
		if(remove_movement_ && un->movement_left() > 0 && unit_location_ == to_){
			gamestate_changed = simulated_stopunit(unit_location_, true, false);
		}

		sim_gamestate_changed(this, gamestate_changed);

		return;
	}

	::actions::move_unit_spectator move_spectator(resources::gameboard->units());
	move_spectator.set_unit(resources::gameboard->units().find(from_));

	if (from_ != to_) {
		size_t num_steps = ::actions::move_unit_and_record(
			/*std::vector<map_location> steps*/ route_->steps,
			/*::actions::undo_list* undo_stack*/ nullptr,
			/*bool continue_move*/ true, ///@todo 1.9 set to false after implemeting interrupt awareness
			/*bool show_move*/ !preferences::skip_ai_moves(),
			/*bool* interrupted*/ nullptr,
			/*::actions::move_unit_spectator* move_spectator*/ &move_spectator);

		if ( num_steps > 0 ) {
			set_gamestate_changed();
		} else if ( move_spectator.get_ambusher().valid() ) {
			// Unlikely, but some types of strange WML (or bad pathfinding)
			// could cause an ambusher to be found without moving.
			set_gamestate_changed();
		}
	} else {
		assert(remove_movement_);
	}

	if (move_spectator.get_unit().valid()){
		unit_location_ = move_spectator.get_unit()->get_location();
		if (remove_movement_ && move_spectator.get_unit()->movement_left() > 0 && unit_location_ == to_)
		{
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

	has_ambusher_ = move_spectator.get_ambusher().valid();
	has_interrupted_teleport_ = move_spectator.get_failed_teleport().valid();

	if (is_gamestate_changed()) {
		try {
			manager::raise_gamestate_changed();
		} catch (...) {
			if (!is_ok()) { DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; } //Demotes to DBG "unchecked result" warning
			throw;
		}
	}
}

void move_result::do_init_for_execution()
{
}


// recall_result
recall_result::recall_result(side_number side,
		const std::string& unit_id, const map_location& where, const map_location& from)
	: action_result(side)
	, unit_id_(unit_id)
	, where_(where)
	, recall_location_(where)
	, recall_from_(from)
	, location_checked_(false)
{
}

unit_const_ptr recall_result::get_recall_unit(const team &my_team)
{
	unit_const_ptr rec = my_team.recall_list().find_if_matches_id(unit_id_);
	if (!rec) {
		set_error(E_NOT_AVAILABLE_FOR_RECALLING);
	}
	return rec;
}

bool recall_result::test_enough_gold(const team &my_team)
{
	if (my_team.gold() < my_team.recall_cost() ) {
		set_error(E_NO_GOLD);
		return false;
	}
	return true;
}

void recall_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const team& my_team = get_my_team();
	const bool location_specified = recall_location_.valid();

	//Enough gold?
	if (!test_enough_gold(my_team)) {
		return;
	}

	//Unit available for recalling?
	const unit_const_ptr & to_recall = get_recall_unit(my_team);
	if ( !to_recall ) {
		return;
	}

	// Leader available for recalling?
	switch ( ::actions::check_recall_location(get_side(), recall_location_,
	                                          recall_from_, *to_recall) )
	{
	case ::actions::RECRUIT_NO_LEADER:
	case ::actions::RECRUIT_NO_ABLE_LEADER:
		set_error(E_NO_LEADER);
		return;

	case ::actions::RECRUIT_NO_KEEP_LEADER:
		set_error(E_LEADER_NOT_ON_KEEP);
		return;

	case ::actions::RECRUIT_NO_VACANCY:
		set_error(E_BAD_RECALL_LOCATION);
		return;

	case ::actions::RECRUIT_ALTERNATE_LOCATION:
		if ( location_specified ) {
			set_error(E_BAD_RECALL_LOCATION);
			return;
		}
		// No break. If the location was not specified, this counts as "OK".
	case ::actions::RECRUIT_OK:
		location_checked_ = true;
	}
}

void recall_result::do_check_after()
{
	if (!resources::gameboard->map().on_board(recall_location_)){
		set_error(AI_ACTION_FAILURE);
		return;
	}

	unit_map::const_iterator unit = resources::gameboard->units().find(recall_location_);
	if (unit==resources::gameboard->units().end()){
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (unit->side() != get_side()){
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
	if (where_ != map_location::null_location()){
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

	const events::command_disabler disable_commands;

	// Assert that recall_location_ has been validated.
	// This should be implied by is_success() once check_before() has been
	// called, so this is a guard against future breakage.
	assert(location_checked_);

	if(resources::simulation_){
		bool gamestate_changed = simulated_recall(get_side(), unit_id_, recall_location_);

		sim_gamestate_changed(this, gamestate_changed);

		return;
	}

	// Do the actual recalling.
	// We ignore possible errors (=unit doesn't exist on the recall list)
	// because that was the previous behavior.
	synced_context::run_in_synced_context_if_not_already("recall",
		replay_helper::get_recall(unit_id_, recall_location_, recall_from_),
		false,
		!preferences::skip_ai_moves(),
		synced_context::ignore_error_function);

	set_gamestate_changed();
	try {
		manager::raise_gamestate_changed();
	} catch (...) {
		if (!is_ok()) { DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; } //Demotes to DBG "unchecked result" warning
		throw;
	}
}

void recall_result::do_init_for_execution()
{
}


// recruit_result
recruit_result::recruit_result(side_number side,
		const std::string& unit_name, const map_location& where, const map_location& from)
	: action_result(side)
	, unit_name_(unit_name)
	, where_(where)
	, recruit_location_(where)
	, recruit_from_(from)
	, location_checked_(false)
{
}

const unit_type *recruit_result::get_unit_type_known(const std::string &recruit)
{
	const unit_type *type = unit_types.find(recruit);
	if (!type) {
		set_error(E_UNKNOWN_OR_DUMMY_UNIT_TYPE);
		return nullptr;
	}
	return type;
}

bool recruit_result::test_enough_gold(const team &my_team, const unit_type &type)
{
	if (my_team.gold() < type.cost()) {
		set_error(E_NO_GOLD);
		return false;
	}
	return true;
}

void recruit_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
	const team& my_team = get_my_team();
	const bool location_specified = recruit_location_.valid();

	//Unit type known ?
	const unit_type *s_type = get_unit_type_known(unit_name_);
	if (!s_type) {
		return;
	}

	//Enough gold?
	if (!test_enough_gold(my_team, *s_type)) {
		return;
	}

	// Leader available for recruiting?
	switch ( ::actions::check_recruit_location(get_side(), recruit_location_,
	                                           recruit_from_, unit_name_) )
	{
	case ::actions::RECRUIT_NO_LEADER:
	case ::actions::RECRUIT_NO_ABLE_LEADER:
		set_error(E_NO_LEADER);
		return;

	case ::actions::RECRUIT_NO_KEEP_LEADER:
		set_error(E_LEADER_NOT_ON_KEEP);
		return;

	case ::actions::RECRUIT_NO_VACANCY:
		set_error(E_BAD_RECRUIT_LOCATION);
		return;

	case ::actions::RECRUIT_ALTERNATE_LOCATION:
		if ( location_specified ) {
			set_error(E_BAD_RECRUIT_LOCATION);
			return;
		}
		// No break. If the location was not specified, this counts as "OK".
	case ::actions::RECRUIT_OK:
		location_checked_ = true;
	}
}

void recruit_result::do_check_after()
{
	if (!resources::gameboard->map().on_board(recruit_location_)) {
		set_error(AI_ACTION_FAILURE);
		return;
	}

	unit_map::const_iterator unit = resources::gameboard->units().find(recruit_location_);
	if (unit==resources::gameboard->units().end()) {
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (unit->side() != get_side()) {
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
	if (where_ != map_location::null_location()){
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

	const unit_type *u = unit_types.find(unit_name_);
	const events::command_disabler disable_commands;

	// Assert that recruit_location_ has been validated.
	// This should be implied by is_success() once check_before() has been
	// called, so this is a guard against future breakage.
	assert(location_checked_  &&  u != nullptr);

	if(resources::simulation_){
		bool gamestate_changed = simulated_recruit(get_side(), u, recruit_location_);

		sim_gamestate_changed(this, gamestate_changed);

		return;
	}

	synced_context::run_in_synced_context_if_not_already("recruit", replay_helper::get_recruit(u->id(), recruit_location_, recruit_from_), false, !preferences::skip_ai_moves());
	//TODO: should we do something to pass use_undo = false in replays and ai moves ?
	//::actions::recruit_unit(*u, get_side(), recruit_location_, recruit_from_,
	//                        !preferences::skip_ai_moves(), false);

	set_gamestate_changed();
	try {
		manager::raise_gamestate_changed();
	} catch (...) {
		if (!is_ok()) { DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; } //Demotes to DBG "unchecked result" warning
		throw;
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

const unit *stopunit_result::get_unit()
{
	unit_map::const_iterator un = resources::gameboard->units().find(unit_location_);
	if (un==resources::gameboard->units().end()){
		set_error(E_NO_UNIT);
		return nullptr;
	}
	const unit *u = &*un;
	if (u->side() != get_side()) {
		set_error(E_NOT_OWN_UNIT);
		return nullptr;
	}
	if (u->incapacitated()) {
		set_error(E_INCAPACITATED_UNIT);
		return nullptr;
	}
	return u;
}

void stopunit_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;

	if (!get_unit()) {
		return;
	}
}

void stopunit_result::do_check_after()
{
	unit_map::const_iterator un = resources::gameboard->units().find(unit_location_);
	if (un==resources::gameboard->units().end()){
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (remove_movement_ && un->movement_left() != 0) {
		set_error(AI_ACTION_FAILURE);
		return;
	}
	if (remove_attacks_ && un->attacks_left() != 0) {
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
	unit_map::iterator un = resources::gameboard->units().find(unit_location_);

	if(resources::simulation_){
		bool gamestate_changed = simulated_stopunit(unit_location_, remove_movement_, remove_attacks_);

		sim_gamestate_changed(this, gamestate_changed);

		return;
	}

	try {
		if (remove_movement_){
			un->remove_movement_ai();
			set_gamestate_changed();
			manager::raise_gamestate_changed();
		}
		if (remove_attacks_){
			un->remove_attacks_ai();
			set_gamestate_changed();
			manager::raise_gamestate_changed();//to be on the safe side
		}
	} catch (...) {
		if (!is_ok()) { DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; } //Demotes to DBG "unchecked result" warning
		throw;
	}
}

void stopunit_result::do_init_for_execution()
{
}


// synced_command_result
synced_command_result::synced_command_result( side_number side, const std::string& lua_code, const map_location& location )
	: action_result(side), lua_code_(lua_code), location_(location)
{
}

void synced_command_result::do_check_before()
{
	LOG_AI_ACTIONS << " check_before " << *this << std::endl;
}

void synced_command_result::do_check_after()
{
}

std::string synced_command_result::do_describe() const
{
	std::stringstream s;
	s <<" synced_command by side ";
	s << get_side();
	s <<std::endl;
	return s.str();
}

void synced_command_result::do_execute()
{
	if(resources::simulation_){
		bool gamestate_changed = simulated_synced_command();

		sim_gamestate_changed(this, gamestate_changed);

		return;
	}

	LOG_AI_ACTIONS << "start of execution of: " << *this << std::endl;
	assert(is_success());

	std::stringstream s;
	if (location_ != map_location::null_location()){
		s << "local x1 = " << location_.wml_x() << " local y1 = " << location_.wml_y() << " ";
	}
	s << lua_code_;

	synced_context::run_in_synced_context_if_not_already("lua_ai", replay_helper::get_lua_ai(s.str()));

	try {
		set_gamestate_changed();
		manager::raise_gamestate_changed();
	} catch (...) {
		if (!is_ok()) { DBG_AI_ACTIONS << "Return value of AI ACTION was not checked." << std::endl; } //Demotes to DBG "unchecked result" warning
		throw;
	}
}

void synced_command_result::do_init_for_execution()
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
	double aggression,
	const unit_advancements_aspect& advancements)
{
	attack_result_ptr action(new attack_result(side,attacker_loc,defender_loc,attacker_weapon,aggression,advancements));
	execute ? action->execute() : action->check_before();
	return action;
}

move_result_ptr actions::execute_move_action( side_number side,
	bool execute,
	const map_location& from,
	const map_location& to,
	bool remove_movement,
	bool unreach_is_ok)
{
	move_result_ptr action(new move_result(side,from,to,remove_movement,unreach_is_ok));
	execute ? action->execute() : action->check_before();
	return action;
}

recall_result_ptr actions::execute_recall_action( side_number side,
	bool execute,
	const std::string& unit_id,
	const map_location& where,
	const map_location& from)
{
	recall_result_ptr action(new recall_result(side,unit_id,where,from));
	execute ? action->execute() : action->check_before();
	return action;
}

recruit_result_ptr actions::execute_recruit_action( side_number side,
	bool execute,
	const std::string& unit_name,
	const map_location& where,
	const map_location& from)
{
	recruit_result_ptr action(new recruit_result(side,unit_name,where,from));
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

synced_command_result_ptr actions::execute_synced_command_action( side_number side,
	bool execute,
	const std::string& lua_code,
	const map_location& location)
{
	synced_command_result_ptr action(new synced_command_result(side,lua_code,location));
	execute ? action->execute() : action->check_before();
	return action;
}

const std::string& actions::get_error_name(int error_code)
{
	if (error_names_.empty()){
		error_names_.emplace(action_result::AI_ACTION_SUCCESS, "action_result::AI_ACTION_SUCCESS");
		error_names_.emplace(action_result::AI_ACTION_STARTED, "action_result::AI_ACTION_STARTED");
		error_names_.emplace(action_result::AI_ACTION_FAILURE, "action_result::AI_ACTION_FAILURE");

		error_names_.emplace(attack_result::E_EMPTY_ATTACKER, "attack_result::E_EMPTY_ATTACKER");
		error_names_.emplace(attack_result::E_EMPTY_DEFENDER, "attack_result::E_EMPTY_DEFENDER");
		error_names_.emplace(attack_result::E_INCAPACITATED_ATTACKER, "attack_result::E_INCAPACITATED_ATTACKER");
		error_names_.emplace(attack_result::E_INCAPACITATED_DEFENDER, "attack_result::E_INCAPACITATED_DEFENDER");
		error_names_.emplace(attack_result::E_NOT_OWN_ATTACKER, "attack_result::E_NOT_OWN_ATTACKER");
		error_names_.emplace(attack_result::E_NOT_ENEMY_DEFENDER, "attack_result::E_NOT_ENEMY_DEFENDER");
		error_names_.emplace(attack_result::E_NO_ATTACKS_LEFT, "attack_result::E_NO_ATTACKS_LEFT");
		error_names_.emplace(attack_result::E_WRONG_ATTACKER_WEAPON, "attack_result::E_WRONG_ATTACKER_WEAPON");
		error_names_.emplace(attack_result::E_UNABLE_TO_CHOOSE_ATTACKER_WEAPON, "attack_result::E_UNABLE_TO_CHOOSE_ATTACKER_WEAPON");
		error_names_.emplace(attack_result::E_ATTACKER_AND_DEFENDER_NOT_ADJACENT," attack_result::E_ATTACKER_AND_DEFENDER_NOT_ADJACENT");

		error_names_.emplace(move_result::E_EMPTY_MOVE, "move_result::E_EMPTY_MOVE");
		error_names_.emplace(move_result::E_NO_UNIT, "move_result::E_NO_UNIT");
		error_names_.emplace(move_result::E_NOT_OWN_UNIT, "move_result::E_NOT_OWN_UNIT");
		error_names_.emplace(move_result::E_INCAPACITATED_UNIT, "move_result::E_INCAPACITATED_UNIT");
		error_names_.emplace(move_result::E_AMBUSHED, "move_result::E_AMBUSHED");
		error_names_.emplace(move_result::E_FAILED_TELEPORT, "move_result::E_FAILED_TELEPORT");
		error_names_.emplace(move_result::E_NOT_REACHED_DESTINATION, "move_result::E_NOT_REACHED_DESTINATION");
		error_names_.emplace(move_result::E_NO_ROUTE, "move_result::E_NO_ROUTE");

		error_names_.emplace(recall_result::E_NOT_AVAILABLE_FOR_RECALLING, "recall_result::E_NOT_AVAILABLE_FOR_RECALLING");
		error_names_.emplace(recall_result::E_NO_GOLD, "recall_result::E_NO_GOLD");
		error_names_.emplace(recall_result::E_NO_LEADER," recall_result::E_NO_LEADER");
		error_names_.emplace(recall_result::E_LEADER_NOT_ON_KEEP, "recall_result::E_LEADER_NOT_ON_KEEP");
		error_names_.emplace(recall_result::E_BAD_RECALL_LOCATION, "recall_result::E_BAD_RECALL_LOCATION");

		error_names_.emplace(recruit_result::E_NOT_AVAILABLE_FOR_RECRUITING, "recruit_result::E_NOT_AVAILABLE_FOR_RECRUITING");
		error_names_.emplace(recruit_result::E_UNKNOWN_OR_DUMMY_UNIT_TYPE, "recruit_result::E_UNKNOWN_OR_DUMMY_UNIT_TYPE");
		error_names_.emplace(recruit_result::E_NO_GOLD, "recruit_result::E_NO_GOLD");
		error_names_.emplace(recruit_result::E_NO_LEADER, "recruit_result::E_NO_LEADER");
		error_names_.emplace(recruit_result::E_LEADER_NOT_ON_KEEP, "recruit_result::E_LEADER_NOT_ON_KEEP");
		error_names_.emplace(recruit_result::E_BAD_RECRUIT_LOCATION, "recruit_result::E_BAD_RECRUIT_LOCATION");

		error_names_.emplace(stopunit_result::E_NO_UNIT, "stopunit_result::E_NO_UNIT");
		error_names_.emplace(stopunit_result::E_NOT_OWN_UNIT, "stopunit_result::E_NOT_OWN_UNIT");
		error_names_.emplace(stopunit_result::E_INCAPACITATED_UNIT, "stopunit_result::E_INCAPACITATED_UNIT");
	}
	std::map<int,std::string>::iterator i = error_names_.find(error_code);
	if (i==error_names_.end()){
		ERR_AI_ACTIONS << "error name not available for error #"<<error_code << std::endl;
		i = error_names_.find(-1);
		assert(i != error_names_.end());
	}
	return i->second;
}

std::map<int,std::string> actions::error_names_;

void sim_gamestate_changed(action_result *result, bool gamestate_changed){
	if(gamestate_changed){
		result->set_gamestate_changed();
		manager::raise_gamestate_changed();
	}
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

std::ostream &operator<<(std::ostream &s, ai::synced_command_result const &r) {
	s << r.do_describe();
	return s;
}
