
/*
   Copyright (C) 2009 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "goal.hpp"
#include "global.hpp"

#include "ai/default/contexts.hpp"
#include "ai/lua/core.hpp"
#include "ai/lua/lua_object.hpp"
#include "ai/manager.hpp"
#include "filter_context.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "map/location.hpp"
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "units/filter.hpp"
#include "wml_exception.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <set>
#include <sstream>

namespace ai {

static lg::log_domain log_ai_goal("ai/goal");
#define DBG_AI_GOAL LOG_STREAM(debug, log_ai_goal)
#define LOG_AI_GOAL LOG_STREAM(info, log_ai_goal)
#define ERR_AI_GOAL LOG_STREAM(err, log_ai_goal)

goal::goal(readonly_context &context, const config &cfg)
	: readonly_context_proxy(), cfg_(cfg), ok_(true)
{
	init_readonly_context_proxy(context);
}



void goal::on_create()
{
	LOG_AI_GOAL << "side " << get_side() << " : " << " created goal with name=[" << cfg_["name"] << "]" << std::endl;
}

void goal::on_create(boost::shared_ptr<ai::lua_ai_context>)
{
	unrecognized();
}

void goal::unrecognized()
{
	ERR_AI_GOAL << "side " << get_side() << " : " << " tried to create goal with name=[" << cfg_["name"] << "], but the [" << cfg_["engine"] << "] engine did not recognize that type of goal. " << std::endl;
	ok_ = false;
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

std::string goal::get_id() const
{
	return cfg_["id"];
}

std::string goal::get_name() const
{
	return cfg_["id"];
}

std::string goal::get_engine() const
{
	return cfg_["engine"];
}


bool goal::redeploy(const config &cfg)
{
	cfg_ = cfg;
	on_create();
	return true;
}


bool goal::ok() const
{
	return ok_;
}


bool goal::active() const
{
	return is_active(cfg_["time_of_day"],cfg_["turns"]);
}


void target_unit_goal::on_create()
{
	goal::on_create();
	if (cfg_["engine"] != "cpp") {
		unrecognized();
		value_ = 0;
		return;
	}
	if (const config::attribute_value *v = cfg_.get("value")) {
		try {
			value_ = boost::lexical_cast<double>(*v);
		} catch (boost::bad_lexical_cast){
			ERR_AI_GOAL << "bad value of goal"<<std::endl;
			value_ = 0;
		}
	}
}

void target_unit_goal::add_targets(std::back_insert_iterator< std::vector< target > > target_list)
{
	if (!(this)->active()) {
		return;
	}

	const config &criteria = cfg_.child("criteria");
	if (!criteria) return;

	//find the enemy leaders and explicit targets
	const unit_filter ufilt(vconfig(criteria), resources::filter_con);
	BOOST_FOREACH(const unit &u, *resources::units) {
		if (ufilt( u )) {
			LOG_AI_GOAL << "found explicit target unit at ... " << u.get_location() << " with value: " << value() << "\n";
			*target_list = target(u.get_location(), value(), target::TYPE::EXPLICIT);
		}
	}


}


target_unit_goal::target_unit_goal(readonly_context &context, const config &cfg)
	: goal(context,cfg)
	, value_(0.0)
{
}


void target_location_goal::on_create()
{
	goal::on_create();
	if (cfg_["engine"] != "cpp") {
		unrecognized();
		value_ = 0;
		return;
	}
	if (cfg_.has_attribute("value")) {
		try {
			value_ = boost::lexical_cast<double>(cfg_["value"]);
		} catch (boost::bad_lexical_cast){
			ERR_AI_GOAL << "bad value of goal"<<std::endl;
			value_ = 0;
		}
	}
	const config &criteria = cfg_.child("criteria");
	if (criteria) {
		filter_ptr_ = boost::shared_ptr<terrain_filter>(new terrain_filter(vconfig(criteria),resources::filter_con));
	}
}

void target_location_goal::add_targets(std::back_insert_iterator< std::vector< target > > target_list)
{
	if (!(this)->active()) {
		return;
	}

	if (!filter_ptr_) return;

	std::set<map_location> items;
	filter_ptr_->get_locations(items);
	BOOST_FOREACH(const map_location &loc, items)
	{
		LOG_AI_GOAL << "found explicit target location ... " << loc << " with value: " << value() << std::endl;
		*target_list = target(loc, value(), target::TYPE::EXPLICIT);
	}

}

target_location_goal::target_location_goal(readonly_context &context, const config &cfg)
	: goal(context,cfg)
	, filter_ptr_()
	, value_(0.0)
{
}



void protect_goal::on_create()
{
	goal::on_create();
	if (cfg_["engine"] != "cpp") {
		unrecognized();
		value_ = 0;
		return;
	}
	if (const config::attribute_value *v = cfg_.get("value")) {
		try {
			value_ = boost::lexical_cast<double>(*v);
		} catch (boost::bad_lexical_cast){
			ERR_AI_GOAL << "bad value of protect_goal"<<std::endl;
			value_ = 0;
		}
	}
	if (const config::attribute_value *v = cfg_.get("protect_radius")) {
		try {
			radius_ = boost::lexical_cast<int>(*v);
		} catch (boost::bad_lexical_cast){
			ERR_AI_GOAL << "bad protection radius of protect_goal"<<std::endl;
			radius_ = 1;
		}
	}

	if (radius_<1) {
		radius_=20;
	}
	const config &criteria = cfg_.child("criteria");
	if (criteria) {
		filter_ptr_ = boost::shared_ptr<terrain_filter>(new terrain_filter(vconfig(criteria),resources::filter_con));
	}


}


void protect_goal::add_targets(std::back_insert_iterator< std::vector< target > > target_list)
{
	std::string goal_type;
	if (protect_unit_) {
		goal_type = "protect_unit";
	} else {
		goal_type ="protect_location";
	}

	if (!(this)->active()) {
		LOG_AI_GOAL << "skipping " << goal_type << " goal - not active" << std::endl;
		return;
	}

	const config &criteria = cfg_.child("criteria");
	if (!criteria) {
		LOG_AI_GOAL << "skipping " << goal_type << " goal - no criteria given" << std::endl;
		return;
	} else {
		DBG_AI_GOAL << "side " << get_side() << ": "<< goal_type << " goal with criteria" << std::endl << cfg_.child("criteria") << std::endl;
	}

	unit_map &units = *resources::units;

	std::set<map_location> items;
	if (protect_unit_) {
		const unit_filter ufilt(vconfig(criteria), resources::filter_con);
		BOOST_FOREACH(const unit &u, units)
		{
			//TODO: we will protect hidden units, by not testing for invisibility to current side
			if (ufilt(u)) {
				DBG_AI_GOAL << "side " << get_side() << ": in " << goal_type << ": " << u.get_location() << " should be protected\n";
				items.insert(u.get_location());
			}
		}
	} else {
		filter_ptr_->get_locations(items);
	}
	DBG_AI_GOAL << "side " << get_side() << ": seaching for threats in "+goal_type+" goal" << std::endl;
	// Look for directions to protect a specific location or specific unit.
	BOOST_FOREACH(const map_location &loc, items)
	{
		BOOST_FOREACH(const unit &u, units)
		{
			int distance = distance_between(u.get_location(), loc);
			if (current_team().is_enemy(u.side()) && distance < radius_ &&
			    !u.invisible(u.get_location()))
			{
				DBG_AI_GOAL << "side " << get_side() << ": in " << goal_type << ": found threat target. " << u.get_location() << " is a threat to "<< loc << '\n';
				*target_list = target(u.get_location(),
					value_ * double(radius_ - distance) /
					radius_, target::TYPE::THREAT);
			}
		}
	}


}


protect_goal::protect_goal(readonly_context &context, const config &cfg, bool protect_unit)
	: goal(context,cfg)
	, filter_ptr_()
	, protect_unit_(protect_unit)
	, radius_(20) //this default radius is taken from old code
	, value_(1.0) //this default value taken from old code
{
}

lua_goal::lua_goal(readonly_context &context, const config &cfg)
	: goal(context, cfg)
	, code_()
	, handler_()
{
	if (cfg.has_attribute("code")) {
		code_ = cfg["code"].str();
	}
	else
	{
		ERR_AI_GOAL << "side " << get_side() << " : Error creating Lua goal (missing code= key)" << std::endl;
	}
}

void lua_goal::on_create(boost::shared_ptr<ai::lua_ai_context> l_ctx)
{
	handler_ = boost::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_lua_ai_action_handler(code_.c_str(), *l_ctx));
}

void lua_goal::add_targets(std::back_insert_iterator< std::vector< target > > target_list)
{
	boost::shared_ptr< lua_object< std::vector < target > > > l_obj
		= boost::shared_ptr< lua_object< std::vector < target > > >(new lua_object< std::vector < target > >());
	config c = config();
	handler_->handle(c, true, l_obj);
	try {
		std::vector < target > targets = *(l_obj->get());

		BOOST_FOREACH(target tg, targets)
		{
			*target_list = tg;
		}
	} catch(bad_enum_cast& e) {
		ERR_AI_GOAL << "A Lua goal returned a target of an unknown type (\"" << e.value() << "\"; unfortunately, the engine cannot recover from this error. As a result, all targets returned by the goal have been lost.\n";
	}

}



} //end of namespace ai
