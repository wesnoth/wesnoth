/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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
action_ptr action::from_config(config const& cfg)
{
	if(cfg["type"]=="move")
		return action_ptr(new move(cfg));
	else if(cfg["type"]=="attack")
		return action_ptr(new attack(cfg));
	else if(cfg["type"]=="recruit")
		return action_ptr(new recruit(cfg));
	else if(cfg["type"]=="recall")
		return action_ptr(new recall(cfg));
	else if(cfg["type"]=="suppose_dead")
		return action_ptr(new suppose_dead(cfg));

	return action_ptr();
}

action::action(size_t team_index)
	: team_index_(team_index)
{
}

action::action(config const& cfg)
	: team_index_(cfg["team_index_"])
{
}

action::~action()
{
}

} // end namespace wb
