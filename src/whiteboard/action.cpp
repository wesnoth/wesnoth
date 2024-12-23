/*
	Copyright (C) 2010 - 2024
	by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file
 */

#include "whiteboard/action.hpp"
#include "whiteboard/move.hpp"
#include "whiteboard/attack.hpp"
#include "whiteboard/recruit.hpp"
#include "whiteboard/recall.hpp"
#include "whiteboard/suppose_dead.hpp"

#include "config.hpp"
#include "game_board.hpp"
#include "resources.hpp"
#include "units/unit.hpp"

namespace wb {

std::ostream& operator<<(std::ostream& s, const action_ptr& action)
{
	assert(action);
	return action->print(s);
}

std::ostream& operator<<(std::ostream& s, const action_const_ptr& action)
{
	assert(action);
	return action->print(s);
}

std::ostream& action::print(std::ostream& s) const
{
	return s;
}

config action::to_config() const
{
	config final_cfg;
	final_cfg["type"]="action";
	final_cfg["team_index_"]=static_cast<int>(team_index_);
	return final_cfg;
}

/* static */
action_ptr action::from_config(const config& cfg, bool hidden)
{
	std::string type = cfg["type"];

	try {
		if(type == "move")
			return std::make_shared<move>(cfg, hidden);
		else if(type == "attack")
			return std::make_shared<attack>(cfg, hidden);
		else if(type == "recruit")
			return std::make_shared<recruit>(cfg, hidden);
		else if(type == "recall")
			return std::make_shared<recall>(cfg, hidden);
		else if(type == "suppose_dead")
			return std::make_shared<suppose_dead>(cfg, hidden);
	} catch(const action::ctor_err&) {
	}

	return nullptr;
}

void action::hide()
{
	if(hidden_)
		return;
	hidden_ = true;
	do_hide();
}

void action::show()
{
	if(!hidden_)
		return;
	hidden_ = false;
	do_show();
}

action::action(std::size_t team_index, bool hidden)
	: team_index_(team_index)
	, hidden_(hidden)
{
}

action::action(const config& cfg, bool hidden)
	: team_index_()
	, hidden_(hidden)
{
	// Construct and validate team_index_
	int team_index_temp = cfg["team_index_"].to_int(-1); //default value: -1
	if(team_index_temp < 0
			|| team_index_temp >= static_cast<int>(resources::gameboard->teams().size()))
		throw ctor_err("action: Invalid team_index_");
	team_index_ = team_index_temp;
}

action::~action()
{
}

std::size_t action::get_unit_id() const
{
	unit_ptr ret = get_unit();
	return ret ? ret->underlying_id() : 0;
}

} // end namespace wb
