/* $Id$ */
/*
   Copyright (C) 2007
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file ai_python.hpp
//!

#ifndef AI_PYTHON_HPP_INCLUDED
#define AI_PYTHON_HPP_INCLUDED

#include "ai_interface.hpp"
#include "menu_events.hpp"
#undef _POSIX_C_SOURCE	// avoids a spurious compiler warning
#include <Python.h>

typedef struct {
	PyObject_HEAD
	const unit_type* unit_type_;
} wesnoth_unittype;

typedef struct {
	PyObject_HEAD
	const team* team_;
} wesnoth_team;

typedef struct {
	PyObject_HEAD
	const unit* unit_;
} wesnoth_unit;

#define W(name) static PyObject *wrapper_##name(PyObject* self, PyObject* args)
class python_ai : public ai_interface
{
public:
	python_ai(ai_interface::info& info);
	virtual ~python_ai();
	virtual void play_turn();

    static PyObject* wrapper_unit_movement_cost(wesnoth_unit*, PyObject* args);
    static PyObject* wrapper_unit_defense_modifier(wesnoth_unit*, PyObject* args);
    static PyObject* wrapper_unittype_movement_cost(wesnoth_unittype*, PyObject* args);
    static PyObject* wrapper_unittype_defense_modifier(wesnoth_unittype*, PyObject* args);

    W(team_targets);
    W(get_units);
    W(log_message);
    W(get_location);
    W(get_map);
    W(get_teams);
    W(get_current_team);
    W(get_src_dst);
    W(get_dst_src);
    W(get_enemy_src_dst);
    W(get_enemy_dst_src);
    W(move_unit);
    W(attack_unit);
    W(get_adjacent_tiles);
    W(recruit_unit);
    W(get_gamestatus);
    W(set_variable);
    W(get_variable);
    W(get_version);
    W(raise_user_interact);
    W(test_move);

	static PyObject* unittype_advances_to( wesnoth_unittype* type, PyObject* args );
	static PyObject* wrapper_team_recruits( wesnoth_team* team, PyObject* args );
	static PyObject* wrapper_unit_find_path( wesnoth_unit* unit, PyObject* args );
	static PyObject* wrapper_unit_attack_statistics(wesnoth_unit* unit, PyObject* args);

	static bool is_unit_valid(const unit* unit);
	std::vector<team>& get_teams() { return get_info().teams; }
    static std::vector<std::string> get_available_scripts();
    static void initialize_python();
    static void invoke(std::string name);

    friend void recalculate_movemaps();
private:
    static bool init_;

    end_level_exception exception;
	ai_interface::move_map src_dst_;
	ai_interface::move_map dst_src_;
	std::map<location,paths> possible_moves_;
	ai_interface::move_map enemy_src_dst_;
	ai_interface::move_map enemy_dst_src_;
	std::map<location,paths> enemy_possible_moves_;
};
#undef W

#endif
