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
 * @file ai/ai_actions.hpp
 * */

#ifndef AI_AI_ACTIONS_HPP_INCLUDED
#define AI_AI_ACTIONS_HPP_INCLUDED

#include "../global.hpp"

#include "../map_location.hpp"
#include <memory>

class ai_action_result {
public:
	static const int AI_ACTION_SUCCESS = 0;
	static const int AI_ACTION_STARTED = 1;

	ai_action_result();
	virtual ~ai_action_result();

	void check_before();
	void execute();
	bool is_ok();
protected:
	virtual void do_check_before() = 0;
	virtual void do_check_after() = 0;
	virtual void do_execute() = 0;	
	virtual void do_init_for_check_before() = 0;
	virtual void do_init_for_execution() = 0;
private:
	void check_after();
	void init_for_check_before();
	void init_for_execution();
	void set_ok_checked();
	bool init_for_execution_and_check();
	int status_;
	bool is_success();
	bool return_value_checked_;
	bool tried_;

};

class ai_attack_result : public ai_action_result {
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();	
	virtual void do_init_for_check_before();
	virtual void do_init_for_execution();
};

class ai_move_result : public ai_action_result {
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();	
	virtual void do_init_for_check_before();
	virtual void do_init_for_execution();
};

class ai_recruit_result : public ai_action_result {
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();	
	virtual void do_init_for_check_before();
	virtual void do_init_for_execution();
};
	
class ai_stopunit_result : public ai_action_result {
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();	
	virtual void do_init_for_check_before();
	virtual void do_init_for_execution();
};


class ai_actions {

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
 * @retval possible result: ok
 * @retval possible result: something wrong
 * @retval possible result: attacker and/or defender are invalid
 * @retval possible result: attacker doesn't have the specified weapon
 */
static std::auto_ptr<ai_attack_result> execute_attack_action( int side,
	bool execute,
	const map_location& attacker_loc,
	const map_location& defender_loc, 
	int attacks );


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
static std::auto_ptr<ai_move_result > execute_move_action( int side,
	bool execute,
	const map_location& from,
	const map_location& to,
	bool remove_movement );


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
static std::auto_ptr<ai_recruit_result> execute_recruit_action( int side,
	bool execute,
	const std::string& unit_name,
	const map_location& where );


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
static std::auto_ptr<ai_stopunit_result> execute_stopunit_action( int side,
	bool execute,
	const map_location& unit_location,
	bool remove_movement,
	bool remove_attacks );


};
#endif
