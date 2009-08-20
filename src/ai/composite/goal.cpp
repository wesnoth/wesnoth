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

#include <boost/lexical_cast.hpp>

namespace ai {

static lg::log_domain log_ai_composite_goal("ai/composite/goal");
#define DBG_AI_COMPOSITE_GOAL LOG_STREAM(debug, log_ai_composite_goal)
#define LOG_AI_COMPOSITE_GOAL LOG_STREAM(info, log_ai_composite_goal)
#define ERR_AI_COMPOSITE_GOAL LOG_STREAM(err, log_ai_composite_goal)

goal::goal(readonly_context &/*context*/, const config &cfg)
	: cfg_(cfg), value_()
{
	if (cfg.has_attribute("value")) {
		try {
			value_ = boost::lexical_cast<double>(cfg["value"]);
		} catch (boost::bad_lexical_cast){
			ERR_AI_COMPOSITE_GOAL << "bad value of goal"<<std::endl;
			value_ = 0;
		}
	}
}



void goal::on_create()
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
	return cfg_;
}


} //end of namespace ai
