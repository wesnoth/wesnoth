/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
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

goal::goal(readonly_context &context, const config &cfg)
	: readonly_context_proxy(), cfg_(cfg)
{
	init_readonly_context_proxy(context);
}



void goal::on_create()
{
}


goal::~goal()
{
}


void goal::add_targets(std::back_insert_iterator< std::vector< target > > /*target_list*/)
{
}



config goal::to_config() const
{
	return cfg_;
}


const std::string& goal::get_id() const
{
	return cfg_["id"];
}



const std::string& goal::get_name() const
{
	return cfg_["id"];
}



const std::string& goal::get_engine() const
{
	return cfg_["engine"];
}


bool goal::redeploy(const config &cfg)
{
	cfg_ = cfg;
	on_create();
	return true;
}


bool goal::active() const
{
	return is_active(cfg_["time_of_day"],cfg_["turns"]);
}


void target_unit_goal::on_create()
{
	goal::on_create();
	if (cfg_.has_attribute("value")) {
		try {
			value_ = boost::lexical_cast<double>(cfg_["value"]);
		} catch (boost::bad_lexical_cast){
			ERR_AI_COMPOSITE_GOAL << "bad value of goal"<<std::endl;
			value_ = 0;
		}
	}
}


bool target_unit_goal::matches_unit(unit_map::const_iterator u)
{
	if (!u.valid()) {
		return false;
	}
	const config &criteria = cfg_.child("criteria");
	if (!criteria) {
		return false;
	}
	return u->second.matches_filter(vconfig(criteria),u->first);
}


void target_unit_goal::add_targets(std::back_insert_iterator< std::vector< target > > target_list)
{
	if (!(this)->active()) {
		return;
	}

	unit_map &units = get_info().units;

	//find the enemy leaders and explicit targets
	unit_map::const_iterator u;
	for(u = units.begin(); u != units.end(); ++u) {
		if (this->matches_unit(u)) {
			LOG_AI_COMPOSITE_GOAL << "found explicit target... " << u->first << " with value: " << this->value() << "\n";
			*target_list = target(u->first,this->value(),target::EXPLICIT);
		}
	}


}


target_unit_goal::target_unit_goal(readonly_context &context, const config &cfg)
	: goal(context,cfg)
	, value_(0.0)
{
}


void protect_goal::on_create()
{
	goal::on_create();
	if (cfg_.has_attribute("value")) {
		try {
			value_ = boost::lexical_cast<double>(cfg_["value"]);
		} catch (boost::bad_lexical_cast){
			ERR_AI_COMPOSITE_GOAL << "bad value of protect_goal"<<std::endl;
			value_ = 0;
		}
	}
	if (cfg_.has_attribute("protect_radius")) {
		try {
			radius_ = boost::lexical_cast<int>(cfg_["protect_radius"]);
		} catch (boost::bad_lexical_cast){
			ERR_AI_COMPOSITE_GOAL << "bad protection radius of protect_goal"<<std::endl;
			radius_ = 1;
		}
	}

	if (radius_<1) {
		radius_=20;
	}
	const config &criteria = cfg_.child("criteria");
	if (criteria) {
		filter_ptr_ = boost::shared_ptr<terrain_filter>(new terrain_filter(vconfig(criteria),get_info().units));
	}


}


void protect_goal::add_targets(std::back_insert_iterator< std::vector< target > > target_list)
{
	if (!(this)->active()) {
		LOG_AI_COMPOSITE_GOAL << "skipping protect_goal - not active" << std::endl;
		return;
	}

	if (!filter_ptr_) {
		LOG_AI_COMPOSITE_GOAL << "skipping protect_goal - no criteria given" << std::endl;
		return;
	} else {
		DBG_AI_COMPOSITE_GOAL << "protect_goal with criteria" << std::endl << cfg_.child("criteria") << std::endl;
	}

	unit_map &units = get_info().units;
	std::vector<team> &teams = get_info().teams;


	std::set<map_location> items;
	filter_ptr_->get_locations(items);
	DBG_AI_COMPOSITE_GOAL << "seaching for threats in protect_goal" << std::endl;
	// Look for directions to protect a specific location.
	foreach (const map_location &loc, items)
	{
		for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
			const int distance = distance_between(u->first,loc);
			if(current_team().is_enemy(u->second.side()) && distance < radius_
			&& !u->second.invisible(u->first, units, teams)) {
				DBG_AI_COMPOSITE_GOAL << "found threat target... " << u->first << " is a threat to "<< loc << std::endl;
				*target_list = target(u->first, value_ * double(radius_-distance) /
							double(radius_),target::THREAT);
			}
		}
	}


}


protect_goal::protect_goal(readonly_context &context, const config &cfg)
	: goal(context,cfg)
	, filter_ptr_()
	, radius_(20)
	, value_(1.0) //these defaults are taken from old code
{
}


} //end of namespace ai
