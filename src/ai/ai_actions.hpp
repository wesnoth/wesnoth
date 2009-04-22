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

#include "ai_interface.hpp"
#include "../map.hpp"
#include "../map_location.hpp"
#include "../team.hpp"
#include <memory>
#include <vector>

class ai_action_result {
public:
	static const int AI_ACTION_SUCCESS = 0;
	static const int AI_ACTION_STARTED = 1;
	static const int AI_ACTION_FAILURE = -1;

	virtual ~ai_action_result();

	void check_before();
	void execute();
	bool is_ok();
	virtual std::string do_describe() const =0;
protected:
	ai_action_result( unsigned int side );
	virtual void do_check_before() = 0;
	virtual void do_check_after() = 0;
	virtual void do_execute() = 0;
	virtual void do_init_for_execution() = 0;

	bool is_execution() const;
	unsigned int get_side() const;
	ai_interface::info& get_info() const;
	ai_interface::info& get_subjective_info() const;
	bool using_subjective_info() const;
	team& get_my_team(ai_interface::info info) const;
	void set_error(int error_code);
	bool is_success() const;
private:
	void check_after();
	void init_for_execution();
	void set_ok_checked();
	bool return_value_checked_;
	unsigned int side_;
	int status_;
	bool is_execution_;

};

class ai_attack_result : public ai_action_result {
public:
	ai_attack_result( unsigned int side,
		const map_location& attacker_loc,
		const map_location& defender_loc,
		int attacker_weapon );
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
};

class ai_move_result : public ai_action_result {
public:
	ai_move_result( unsigned int side,
		const map_location& from,
		const map_location& to,
		bool remove_movement );
	static const int E_EMPTY_MOVE = 2001;
	static const int E_NO_UNIT = 2002;
	static const int E_NOT_OWN_UNIT = 2003;
	static const int E_INCAPACITATED_UNIT = 2004;
	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	bool test_unit(unit_map::const_iterator& un, const team& team, const unit_map& units, const std::vector<team>& teams, bool update_knowledge = false );
	bool test_route(const unit_map::const_iterator& un, const team& team, const unit_map& units, const std::vector<team>& teams, const gamemap& map, bool update_knowledge = false );
	const map_location& from_;
	const map_location& to_;
	bool remove_movement_;
	paths::route route_;
};

class ai_recruit_result : public ai_action_result {
public:
	ai_recruit_result( unsigned int side, const std::string& unit_name, const map_location& where);
	static const int E_NOT_AVAILABLE_FOR_RECRUITING = 3001;
	static const int E_UNKNOWN_OR_DUMMY_UNIT_TYPE = 3002;
	static const int E_NO_GOLD = 3003;
	static const int E_NO_LEADER = 3004;
	static const int E_LEADER_NOT_ON_KEEP = 3005;
	static const int E_BAD_RECRUIT_LOCATION = 3006;

	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	bool test_available_for_recruiting(
		const team& team,
		std::set<std::string>::const_iterator& recruit,
		bool update_knowledge = false );
	bool test_unit_type_known(
		const std::set<std::string>::const_iterator& recruit,
		unit_type_data::unit_type_map::const_iterator& unit_type,
		bool update_knowledge = false );
	bool test_enough_gold(
		const team& team,
		const unit_type_data::unit_type_map::const_iterator& unit_type,
		bool update_knowledge = false );
	bool test_leader_present(
		const unit_map& units,
		unit_map::const_iterator& my_leader,
		bool update_knowledge = false );
	bool test_leader_on_keep(
		const gamemap& map,
		const unit_map::const_iterator& my_leader,
		bool update_knowledge = false);
	bool test_suitable_recruit_location (
		const gamemap& map,
		const unit_map& units,
		const unit_map::const_iterator& my_leader,
		bool update_knowledge = false);
	const std::string& unit_name_;
	const map_location& where_;
	map_location recruit_location_;
	unit_type_data::unit_type_map::const_iterator* unit_type_;
	int num_;
};

class ai_stopunit_result : public ai_action_result {
public:
	ai_stopunit_result( unsigned int side,
		const map_location& unit_location,
		bool remove_movement,
		bool remove_attacks );
	static const int E_NO_UNIT = 4002;
	static const int E_NOT_OWN_UNIT = 4003;
	static const int E_INCAPACITATED_UNIT = 4004;

	virtual std::string do_describe() const;
protected:
	virtual void do_check_before();
	virtual void do_check_after();
	virtual void do_execute();
	virtual void do_init_for_execution();
private:
	bool test_unit(unit_map::const_iterator& un, const unit_map& units, bool update_knowledge = false );
	const map_location& unit_location_;
	const bool remove_movement_;
	const bool remove_attacks_;
};


std::ostream &operator<<(std::ostream &s, ai_attack_result const &r);
std::ostream &operator<<(std::ostream &s, ai_move_result const &r);
std::ostream &operator<<(std::ostream &s, ai_recruit_result const &r);
std::ostream &operator<<(std::ostream &s, ai_stopunit_result const &r);

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
static std::auto_ptr<ai_attack_result> execute_attack_action( unsigned int side,
	bool execute,
	const map_location& attacker_loc,
	const map_location& defender_loc,
	int attacker_weapon );


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
static std::auto_ptr<ai_move_result > execute_move_action( unsigned int side,
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
static std::auto_ptr<ai_recruit_result> execute_recruit_action( unsigned int side,
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
static std::auto_ptr<ai_stopunit_result> execute_stopunit_action( unsigned int side,
	bool execute,
	const map_location& unit_location,
	bool remove_movement,
	bool remove_attacks );


};
#endif
