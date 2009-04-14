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

#include "ai_actions.hpp"
#include "../log.hpp"

#define DBG_AI_ACTIONS LOG_STREAM(debug, ai_actions)
#define LOG_AI_ACTIONS LOG_STREAM(info, ai_actions)
#define WRN_AI_ACTIONS LOG_STREAM(warn, ai_actions)
#define ERR_AI_ACTIONS LOG_STREAM(err, ai_actions)

// =======================================================================
// AI ACTIONS
// =======================================================================
ai_action_result::ai_action_result()
{
}


ai_action_result::~ai_action_result()
{
	if (tried_ && !return_value_checked_) {
		ERR_AI_ACTIONS << "Return value of AI ACTION was not checked. This may cause bugs!" << std::endl;
	}
}


void ai_action_result::check_after()
{
	do_check_after();
}


void ai_action_result::check_before()
{
	init_for_check_before();
	do_check_before();
}


void ai_action_result::execute()
{
	check_before();
	if (is_success()){
		init_for_execution();
		do_execute();	
	}
	if (is_success()){
		check_after();
	}
}


void ai_action_result::init_for_check_before()
{
	do_init_for_check_before();
}


void ai_action_result::init_for_execution()
{
	tried_ = true;
	return_value_checked_ = false;
	status_ = ai_action_result::AI_ACTION_STARTED;
	do_init_for_execution();
}


bool ai_action_result::is_ok()
{
	return_value_checked_ = true;
	return is_success();
}

bool ai_action_result::is_success()
{
	return (status_ == ai_action_result::AI_ACTION_SUCCESS);
}


// ai_attack_result

void ai_attack_result::do_check_before()
{
}


void ai_attack_result::do_check_after()
{
}


void ai_attack_result::do_execute()
{
}


void ai_attack_result::do_init_for_check_before()
{
}


void ai_attack_result::do_init_for_execution()
{
}


// ai_move_result

void ai_move_result::do_check_before()
{
}


void ai_move_result::do_check_after()
{
}


void ai_move_result::do_execute()
{
}


void ai_move_result::do_init_for_check_before()
{
}


void ai_move_result::do_init_for_execution()
{
}


// ai_recruit_result

void ai_recruit_result::do_check_before()
{
}


void ai_recruit_result::do_check_after()
{
}


void ai_recruit_result::do_execute()
{
}


void ai_recruit_result::do_init_for_check_before()
{
}


void ai_recruit_result::do_init_for_execution()
{
}


// ai_stopunit_result

void ai_stopunit_result::do_check_before()
{
}


void ai_stopunit_result::do_check_after()
{
}


void ai_stopunit_result::do_execute()
{
}


void ai_stopunit_result::do_init_for_check_before()
{
}


void ai_stopunit_result::do_init_for_execution()
{
}


// =======================================================================
// STATELESS INTERFACE TO AI ACTIONS
// =======================================================================

std::auto_ptr< ai_attack_result > ai_actions::execute_attack_action( int /*side*/,
	bool execute,
	const map_location& /*attacker_loc*/,
	const map_location& /*defender_loc*/,
	int /*attacks*/)
{
	std::auto_ptr< ai_attack_result > ai_action(new ai_attack_result());
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;
}


std::auto_ptr< ai_move_result > ai_actions::execute_move_action( int /*side*/,
	bool execute,
	const map_location& /*from*/,
	const map_location& /*to*/,
	bool /*remove_movement*/)
{
	std::auto_ptr< ai_move_result > ai_action(new ai_move_result());
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;

}


std::auto_ptr< ai_recruit_result > ai_actions::execute_recruit_action( int /*side*/,
	bool execute,
	const std::string& /*unit_name*/,
	const map_location& /*where*/)
{
	std::auto_ptr< ai_recruit_result > ai_action(new ai_recruit_result());
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;

}


std::auto_ptr< ai_stopunit_result > ai_actions::execute_stopunit_action( int /*side*/,
	bool execute,
	const map_location& /*unit_location*/,
	bool /*remove_movement*/,
	bool /*remove_attacks*/)
{
	std::auto_ptr< ai_stopunit_result > ai_action(new ai_stopunit_result());
	execute ? ai_action->execute() : ai_action->check_before();
	return ai_action;

}
