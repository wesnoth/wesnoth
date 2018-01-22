/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * */

#pragma once

#include "ai/game_info.hpp"

#include "actions/move.hpp"
#include "ai/lua/aspect_advancements.hpp"
#include "units/ptr.hpp"

namespace pathfind {
struct plain_route;
} // of namespace pathfind

class unit;
class unit_type;
class team;

namespace ai {

class action_result {
friend void sim_gamestate_changed(action_result *result, bool gamestate_changed);	// Manage gamestate changed in simulated actions.

public:

	enum result {
		AI_ACTION_SUCCESS = 0,
		AI_ACTION_STARTED = 1,
		AI_ACTION_FAILURE = -1
	};

	virtual ~action_result();

	/* check as must as possible without executing anything */
	void check_before();

	/* execute the action */
	void execute();

	/* has the game state changed during execution ? */
	bool is_gamestate_changed() const;

	/* check the return value of the action. mandatory to call. */
	bool is_ok();

	/* get the return value of the action */
	int get_status() const;

	/* describe the action */
	virtual std::string do_describe() const =0;
protected:
	action_result( side_number side );

	/* do check before execution or just check. setting status_ via set_error to != cancels the execution.*/
	virtual void do_check_before() = 0;

	/* do some additional checks after execution. */
	virtual void do_check_after() = 0;

	/* execute. assert(is_success()) */
	virtual void do_execute() = 0;

	/* runs before cheching before execution */
	virtual void do_init_for_execution() = 0;

	/* are we going to execute the action now ? */
	bool is_execution() const;

	/* return the side number */
	int get_side() const { return side_; }

	/* return real information about the game state */
	game_info& get_info() const;

	team& get_my_team() const;

	/* set error code */
	void set_error(int error_code, bool log_as_error = true);

	/* is error code equal to 0 (no errors)? */
	bool is_success() const;

	/* note that the game state has been changed */
	void set_gamestate_changed();
private:

	/* Check after the execution */
	void check_after();

	/* Initialization before execution */
	void init_for_execution();

	/* set the flag that the return value had been checked */
	void set_ok_checked();

	/* was the return value checked ? */
	bool return_value_checked_;

	/* current side number */
	int side_;

	/* execution status. if 0, all is ok. if !=0, then there were some problems. */
	int status_;

	/* are we going to execute the action now ? */
	bool is_execution_;

	bool is_gamestate_changed_;

};

class attack_result : public action_result {
public:
	attack_result( side_number side,
		const map_location& attacker_loc,
		const map_location& defender_loc,
		int attacker_weapon,
		double aggression,
		const unit_advancements_aspect& advancements = unit_advancements_aspect());

	enum result {
		E_EMPTY_ATTACKER = 1001,
		E_EMPTY_DEFENDER = 1002,
		E_INCAPACITATED_ATTACKER = 1003,
		E_INCAPACITATED_DEFENDER = 1004,
		E_NOT_OWN_ATTACKER = 1005,
		E_NOT_ENEMY_DEFENDER = 1006,
		E_NO_ATTACKS_LEFT = 1007,
		E_WRONG_ATTACKER_WEAPON = 1008,
		E_UNABLE_TO_CHOOSE_ATTACKER_WEAPON = 1009,
		E_ATTACKER_AND_DEFENDER_NOT_ADJACENT = 1010
	};

	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	const map_location& attacker_loc_;
	const map_location& defender_loc_;
	int attacker_weapon_;
	double aggression_;
	const unit_advancements_aspect& advancements_;
};

class move_result : public action_result {
public:
	move_result( side_number side,
		const map_location& from,
		const map_location& to,
		bool remove_movement,
		bool unreach_is_ok);

	enum result {
		E_EMPTY_MOVE = 2001,
		E_NO_UNIT = 2002,
		E_NOT_OWN_UNIT = 2003,
		E_INCAPACITATED_UNIT = 2004,
		E_AMBUSHED = 2005,
		E_FAILED_TELEPORT = 2006,
		E_NOT_REACHED_DESTINATION = 2007,
		E_NO_ROUTE = 2008
	};

	virtual std::string do_describe() const;
	virtual const map_location& get_unit_location() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	const unit *get_unit();
	bool test_route(const unit &un);
	const map_location from_;
	const map_location to_;
	bool remove_movement_;
	std::shared_ptr<pathfind::plain_route> route_;
	map_location unit_location_;
	bool unreach_is_ok_;
	bool has_ambusher_;
	bool has_interrupted_teleport_;
};


class recall_result : public action_result {
public:
	recall_result (side_number side, const std::string &unit_id, const map_location& where, const map_location& from);

	enum result {
		E_NOT_AVAILABLE_FOR_RECALLING = 6001,
		E_NO_GOLD = 6003,
		E_NO_LEADER = 6004,
		E_LEADER_NOT_ON_KEEP = 6005,
		E_BAD_RECALL_LOCATION = 6006
	};

	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	unit_const_ptr get_recall_unit(
		const team& my_team);
	bool test_enough_gold(
		const team& my_team);

	const std::string& unit_id_;
	const map_location where_;
	map_location recall_location_;
	map_location recall_from_;
	bool location_checked_;
};

class recruit_result : public action_result {
public:
	recruit_result( side_number side, const std::string& unit_name, const map_location& where, const map_location& from);

	enum result {
		E_NOT_AVAILABLE_FOR_RECRUITING = 3001,
		E_UNKNOWN_OR_DUMMY_UNIT_TYPE = 3002,
		E_NO_GOLD = 3003,
		E_NO_LEADER = 3004,
		E_LEADER_NOT_ON_KEEP = 3005,
		E_BAD_RECRUIT_LOCATION = 3006
	};

	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	const unit_type *get_unit_type_known(
		const std::string &recruit);
	bool test_enough_gold(
		const team& my_team,
		const unit_type &type );

	const std::string& unit_name_;
	const map_location& where_;
	map_location recruit_location_;
	map_location recruit_from_;
	bool location_checked_;
};

class stopunit_result : public action_result {
public:
	stopunit_result( side_number side,
		const map_location& unit_location,
		bool remove_movement,
		bool remove_attacks );

	enum result {
		E_NO_UNIT = 4002,
		E_NOT_OWN_UNIT = 4003,
		E_INCAPACITATED_UNIT = 4004
	};

	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	const unit *get_unit();
	const map_location& unit_location_;
	const bool remove_movement_;
	const bool remove_attacks_;
};

class synced_command_result : public action_result {
public:
	synced_command_result( side_number side,
		const std::string& lua_code,
		const map_location& location );

	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	const std::string& lua_code_;
	const map_location& location_;
};


class actions {

public:
// =======================================================================
// Stateless interface to actions
// =======================================================================


/**
 * Ask the game to attack an enemy defender using our unit attacker from attackers current location,
 * @param side the side which tries to execute the move
 * @param execute should move be actually executed or not
 * @param attacker_loc location of attacker
 * @param defender_loc location of defender
 * @param attacker_weapon weapon of attacker
 * @param aggression aggression of attacker, is used to determine attacker's weapon if it is not specified
 * @retval possible result: ok
 * @retval possible result: something wrong
 * @retval possible result: attacker and/or defender are invalid
 * @retval possible result: attacker doesn't have the specified weapon
 */
static attack_result_ptr execute_attack_action( side_number side,
	bool execute,
	const map_location& attacker_loc,
	const map_location& defender_loc,
	int attacker_weapon,
	double aggression,
	const unit_advancements_aspect& advancements = unit_advancements_aspect());


/**
 * Ask the game to move our unit from location 'from' to location 'to', optionally - doing a partial move
 * @param side the side which tries to execute the move
 * @param execute should move be actually executed or not
 * @param from location of our unit
 * @param to where to move
 * @param remove_movement set unit movement to 0 in case of successful move
 * @retval possible result: ok
 * @retval possible result: something wrong
 * @retval possible result: move is interrupted
 * @retval possible result: move is impossible
 */
static move_result_ptr execute_move_action( side_number side,
	bool execute,
	const map_location& from,
	const map_location& to,
	bool remove_movement,
	bool unreach_is_ok = false);



/**
 * Ask the game to recall a unit for us on specified location
 * @param side the side which tries to execute the move
 * @param execute should move be actually executed or not
 * @param unit_id the id of the unit to be recalled.
 * @param where location where the unit is to be recalled.
 * @retval possible result: ok
 * @retval possible_result: something wrong
 * @retval possible_result: leader not on keep
 * @retval possible_result: no free space on keep
 * @retval possible_result: not enough gold
 */
static recall_result_ptr execute_recall_action( side_number side,
	bool execute,
	const std::string& unit_id,
	const map_location& where,
	const map_location& from);



/**
 * Ask the game to recruit a unit for us on specified location
 * @param side the side which tries to execute the move
 * @param execute should move be actually executed or not
 * @param unit_name the name of the unit to be recruited.
 * @param where location where the unit is to be recruited.
 * @retval possible result: ok
 * @retval possible_result: something wrong
 * @retval possible_result: leader not on keep
 * @retval possible_result: no free space on keep
 * @retval possible_result: not enough gold
 */
static recruit_result_ptr execute_recruit_action( side_number side,
	bool execute,
	const std::string& unit_name,
	const map_location& where,
	const map_location& from);


/**
 * Ask the game to remove unit movements and/or attack
 * @param side the side which tries to execute the move
 * @param execute should move be actually executed or not
 * @param unit_location the location of our unit
 * @param remove_movement set remaining movements to 0
 * @param remove_attacks set remaining attacks to 0
 * @retval possible result: ok
 * @retval possible_result: something wrong
 * @retval possible_result: nothing to do
 */
static stopunit_result_ptr execute_stopunit_action( side_number side,
	bool execute,
	const map_location& unit_location,
	bool remove_movement,
	bool remove_attacks );


/**
 * Ask the game to run Lua code
 * @param side the side which tries to execute the move
 * @param execute should move be actually executed or not
 * @param lua_code the code to be run
 * @param location location to be passed to the code as x1/y1
 * @retval possible result: ok
 * @retval possible_result: something wrong
 * @retval possible_result: nothing to do
 */
static synced_command_result_ptr execute_synced_command_action( side_number side,
	bool execute,
	const std::string& lua_code,
	const map_location& location );


/**
 * get human-readable name of the error by code.
 * @param error_code error code.
 * @retval result the name of the error.
 */
const static std::string& get_error_name(int error_code);

private:

static std::map<int,std::string> error_names_;

};


///@todo 1.7.11 important! Add an ai action (and fai function) to set a goto on a unit
///@todo 1.7.11 important! Add an ai action (and fai function) to send a chat message to a player

} //end of namespace ai

std::ostream &operator<<(std::ostream &s, const ai::attack_result& r);
std::ostream &operator<<(std::ostream &s, const ai::move_result& r);
std::ostream &operator<<(std::ostream &s, const ai::recall_result& r);
std::ostream &operator<<(std::ostream &s, const ai::recruit_result& r);
std::ostream &operator<<(std::ostream &s, const ai::stopunit_result& r);
std::ostream &operator<<(std::ostream &s, const ai::synced_command_result& r);

