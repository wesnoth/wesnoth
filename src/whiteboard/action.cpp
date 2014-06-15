/*
 Copyright (C) 2010 - 2014 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

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

#include "action.hpp"
#include "move.hpp"
#include "attack.hpp"
#include "recruit.hpp"
#include "recall.hpp"
#include "suppose_dead.hpp"

#include "game_board.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "unit.hpp"

namespace wb {

std::ostream& operator<<(std::ostream& s, action_ptr action)
{
	assert(action);
	return action->print(s);
}

std::ostream& operator<<(std::ostream& s, action_const_ptr action)
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
action_ptr action::from_config(config const& cfg, bool hidden)
{
	std::string type = cfg["type"];

	try {
		if(type=="move")
			return action_ptr(new move(cfg,hidden));
		else if(type=="attack")
			return action_ptr(new attack(cfg,hidden));
		else if(type=="recruit")
			return action_ptr(new recruit(cfg,hidden));
		else if(type=="recall")
			return action_ptr(new recall(cfg,hidden));
		else if(type=="suppose_dead")
			return action_ptr(new suppose_dead(cfg,hidden));
	} catch(action::ctor_err const&) {}

	return action_ptr();
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

action::action(size_t team_index, bool hidden)
	: team_index_(team_index)
	, hidden_(hidden)
{
}

action::action(config const& cfg, bool hidden)
	: team_index_()
	, hidden_(hidden)
{
	// Construct and validate team_index_
	int team_index_temp = cfg["team_index_"].to_int(-1); //default value: -1
	if(team_index_temp < 0
			|| team_index_temp >= static_cast<int>(resources::teams->size()))
		throw ctor_err("action: Invalid team_index_");
	team_index_ = team_index_temp;
}

action::~action()
{
}

size_t action::get_unit_id() const
{
	UnitPtr ret = get_unit();
	return ret ? ret->underlying_id() : 0;
}

} // end namespace wb
