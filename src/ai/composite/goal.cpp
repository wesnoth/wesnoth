/*
	Copyright (C) 2009 - 2024
	by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "ai/composite/goal.hpp"

#include "ai/default/contexts.hpp"
#include "ai/lua/core.hpp"
#include "ai/lua/lua_object.hpp"
#include "game_board.hpp"
#include "log.hpp"
#include "map/location.hpp"
#include "resources.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "units/filter.hpp"

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
	LOG_AI_GOAL << "side " << get_side() << " : " << " created goal with name=[" << cfg_["name"] << "]";
}

// In this case, the API is intended to cause an error with this specific type.
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void goal::on_create(std::shared_ptr<ai::lua_ai_context>)
{
	unrecognized();
}

void goal::unrecognized()
{
	ERR_AI_GOAL << "side " << get_side() << " : " << " tried to create goal with name=[" << cfg_["name"] << "], but the [" << cfg_["engine"] << "] engine did not recognize that type of goal. ";
	ok_ = false;
}

goal::~goal()
{
}

void goal::add_targets(std::back_insert_iterator< std::vector< target >> /*target_list*/)
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
	if (!cfg_["engine"].empty() && cfg_["engine"] != "cpp") {
		unrecognized();
		value_ = 0;
		return;
	}
	if (const config::attribute_value *v = cfg_.get("value")) {
		value_ = v->to_double(0);
	}
}

void target_unit_goal::add_targets(std::back_insert_iterator< std::vector< target >> target_list)
{
	if (!(this)->active()) {
		return;
	}

	auto criteria = cfg_.optional_child("criteria");
	if (!criteria) return;

	//find the enemy leaders and explicit targets
	const unit_filter ufilt{ vconfig(*criteria) };
	for (const unit &u : resources::gameboard->units()) {
		if (ufilt( u )) {
			LOG_AI_GOAL << "found explicit target unit at ... " << u.get_location() << " with value: " << value();
			*target_list = target(u.get_location(), value(), ai_target::type::xplicit);
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
	if (!cfg_["engine"].empty() && cfg_["engine"] != "cpp") {
		unrecognized();
		value_ = 0;
		return;
	}
	if (cfg_.has_attribute("value")) {
		value_ = cfg_["value"].to_double(0);
	}
	auto criteria = cfg_.optional_child("criteria");
	if (criteria) {
		filter_ptr_.reset(new terrain_filter(vconfig(*criteria),resources::filter_con, false));
	}
}

void target_location_goal::add_targets(std::back_insert_iterator< std::vector< target >> target_list)
{
	if (!(this)->active()) {
		return;
	}

	if (!filter_ptr_) return;

	std::set<map_location> items;
	filter_ptr_->get_locations(items);
	for (const map_location &loc : items)
	{
		LOG_AI_GOAL << "found explicit target location ... " << loc << " with value: " << value();
		*target_list = target(loc, value(), ai_target::type::xplicit);
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
	if (!cfg_["engine"].empty() && cfg_["engine"] != "cpp") {
		unrecognized();
		value_ = 0;
		return;
	}
	if (const config::attribute_value *v = cfg_.get("value")) {
		value_ = v->to_double(0);
	}
	if (const config::attribute_value *v = cfg_.get("protect_radius")) {
		radius_ = (*v).to_int(1);
	}

	if (radius_<1) {
		radius_=20;
	}
	auto criteria = cfg_.optional_child("criteria");
	if (criteria) {
		filter_ptr_.reset(new terrain_filter(vconfig(*criteria), resources::filter_con, false));
	}

}

void protect_goal::add_targets(std::back_insert_iterator< std::vector< target >> target_list)
{
	std::string goal_type;
	if (protect_unit_) {
		goal_type = "protect_unit";
	} else {
		goal_type ="protect_location";
	}

	if (!(this)->active()) {
		LOG_AI_GOAL << "skipping " << goal_type << " goal - not active";
		return;
	}

	auto criteria = cfg_.optional_child("criteria");
	if (!criteria) {
		LOG_AI_GOAL << "skipping " << goal_type << " goal - no criteria given";
		return;
	} else {
		DBG_AI_GOAL << "side " << get_side() << ": "<< goal_type << " goal with criteria" << std::endl << cfg_.mandatory_child("criteria");
	}

	unit_map &units = resources::gameboard->units();

	std::set<map_location> items;
	if (protect_unit_) {
		const unit_filter ufilt{ vconfig(*criteria) };
		for (const unit &u : units)
		{
			// 'protect_unit' can be set to any unit of any side -> exclude hidden units
			// unless they are visible to the AI side (e.g. allies with shared vision).
			// As is done in other parts of the AI, units under fog/shroud count as visible to the AI.
			if (ufilt(u)
				&& (!u.invisible(u.get_location()) || u.is_visible_to_team(current_team(), false)))
			{
				DBG_AI_GOAL << "side " << get_side() << ": in " << goal_type << ": " << u.get_location() << " should be protected";
				items.insert(u.get_location());
			}
		}
	} else {
		filter_ptr_->get_locations(items);
	}
	DBG_AI_GOAL << "side " << get_side() << ": searching for threats in "+goal_type+" goal";
	// Look for directions to protect a specific location or specific unit.
	for (const map_location &loc : items)
	{
		for (const unit &u : units)
		{
			int distance = distance_between(u.get_location(), loc);
			if (current_team().is_enemy(u.side()) && distance < radius_ &&
			    !u.invisible(u.get_location()))
			{
				DBG_AI_GOAL << "side " << get_side() << ": in " << goal_type << ": found threat target. " << u.get_location() << " is a threat to "<< loc;
				*target_list = target(u.get_location(),
					value_ * static_cast<double>(radius_ - distance) /
					radius_, ai_target::type::threat);
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
		ERR_AI_GOAL << "side " << get_side() << " : Error creating Lua goal (missing code= key)";
	}
}

void lua_goal::on_create(std::shared_ptr<ai::lua_ai_context> l_ctx)
{
	handler_.reset(resources::lua_kernel->create_lua_ai_action_handler(code_.c_str(), *l_ctx));
}

void lua_goal::add_targets(std::back_insert_iterator< std::vector< target >> target_list)
{
	std::shared_ptr<lua_object<std::vector<target>>> l_obj = std::make_shared<lua_object<std::vector<target>>>();
	config c(cfg_.child_or_empty("args"));
	const config empty_cfg;
	handler_->handle(c, empty_cfg, true, l_obj);

	std::vector < target > targets = *(l_obj->get());

	for (target tg : targets)
	{
		*target_list = tg;
	}
}

// This is defined in the source file so that it can easily access the logger
bool goal_factory::is_duplicate(const std::string& name)
{
	if (get_list().find(name) != get_list().end()) {
		ERR_AI_GOAL << "Error: Attempt to double-register goal " << name;
		return true;
	}
	return false;
}

} //end of namespace ai
