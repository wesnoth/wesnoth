/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file recruit.cpp
 */

#include "recruit.hpp"

#include "visitor.hpp"

#include "game_display.hpp"
#include "menu_events.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "unit_types.hpp"

namespace wb
{

std::ostream& operator<<(std::ostream& s, recruit_ptr recruit)
{
	assert(recruit);
	return recruit->print(s);
}
std::ostream& operator<<(std::ostream& s, recruit_const_ptr recruit)
{
	assert(recruit);
	return recruit->print(s);
}

recruit::recruit(const std::string& unit_name, const map_location& recruit_hex):
		unit_name_(unit_name),
		recruit_hex_(recruit_hex),
		valid_(true),
		temp_cost_()
{
}

recruit::~recruit()
{
}

void recruit::accept(visitor& v)
{
	v.visit_recruit(shared_from_this());
}

bool recruit::execute()
{
	assert(valid_);
	int side_num = resources::screen->viewing_side();
	const std::set<std::string>& recruits = (*resources::teams)[side_num - 1].recruits();
	if (recruits.find(unit_name_) != recruits.end())
	{
		resources::controller->get_menu_handler().do_recruit(unit_name_, side_num, recruit_hex_);
	}
	else
	{
		LOG_WB << "Planned recruit impossible to execute since unit is not in recruit list anymore.\n";
	}
	return true;
}

void recruit::apply_temp_modifier(unit_map& unit_map)
{
	assert(valid_);
	unit_type const* type = unit_types.find(unit_name_);
	assert(type);
	int side_num = resources::screen->viewing_side();
	unit* temp_unit = new unit(type, side_num, true);

	unit_map.add(recruit_hex_, *temp_unit);
	//unit map takes ownership of temp_unit

	temp_cost_ = type->cost();
	//TODO: add cost to money spent on recruits, need variable in side_actions to track this.
}

void recruit::remove_temp_modifier(unit_map& unit_map)
{
	unit_map.extract(recruit_hex_);

	//TODO: remove cost from money spent on recruits, need variable in side_actions to track this.
}

}
