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
 * @file ai/composite/goal.cpp
 */

#include "goal.hpp"
#include "../manager.hpp"
#include "../../log.hpp"
#include "../../gamestatus.hpp"
#include "../../variable.hpp"

namespace ai {

static lg::log_domain log_ai_composite_aspect("ai/composite/goal");
#define DBG_AI_COMPOSITE_GOAL LOG_STREAM(debug, log_ai_composite_goal)
#define LOG_AI_COMPOSITE_GOAL LOG_STREAM(info, log_ai_composite_goal)
#define ERR_AI_COMPOSITE_GOAL LOG_STREAM(err, log_ai_composite_goal)

goal::goal(const config &cfg)
	: cfg_(cfg),loc_(),value_(),type_()
{
	//@todo 1.7: parse config of goal
}


goal::goal(const map_location &loc, double value, TYPE type)
	: cfg_(),loc_(loc), value_(value), type_(type)
{
}


goal::~goal()
{
}


//@todo: push to subclass
bool goal::matches_unit(unit_map::const_iterator u)
{
	if (!u.valid()) {
		return false;
	}
	config &criteria = cfg_.child("criteria");
	return u->second.matches_filter(vconfig(criteria),u->first);
}


config goal::to_config() const
{
	config cfg;
	//@todo 1.7 recreate config
	return cfg;
}


} //end of namespace ai
