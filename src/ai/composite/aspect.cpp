/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "ai/composite/aspect.hpp"
#include "ai/manager.hpp"
#include "log.hpp"

namespace ai {

static lg::log_domain log_ai_aspect("ai/aspect");
#define DBG_AI_ASPECT LOG_STREAM(debug, log_ai_aspect)
#define LOG_AI_ASPECT LOG_STREAM(info, log_ai_aspect)
#define WRN_AI_ASPECT LOG_STREAM(warn, log_ai_aspect)
#define ERR_AI_ASPECT LOG_STREAM(err, log_ai_aspect)

aspect::aspect(readonly_context &context, const config &cfg, const std::string &id):
	time_of_day_(cfg["time_of_day"]),turns_(cfg["turns"]),
	valid_(false), valid_variant_(false), valid_lua_(false), cfg_(cfg),
	invalidate_on_turn_start_(cfg["invalidate_on_turn_start"].to_bool(true)),
	invalidate_on_tod_change_(cfg["invalidate_on_tod_change"].to_bool(true)),
	invalidate_on_gamestate_change_(cfg["invalidate_on_gamestate_change"].to_bool()),
	invalidate_on_minor_gamestate_change_(cfg["invalidate_on_minor_gamestate_change"].to_bool()),
	engine_(cfg["engine"]), name_(cfg["name"]), id_(id)
	{
		DBG_AI_ASPECT << "creating new aspect: engine=["<<engine_<<"], name=["<<name_<<"], id=["<<id_<<"]"<< std::endl;
		init_readonly_context_proxy(context);
		redeploy(cfg,id);
		DBG_AI_ASPECT << "aspect has time_of_day=["<<time_of_day_<<"], turns=["<<turns_<<"]" << std::endl;
	}


aspect::~aspect()
	{
		if (invalidate_on_turn_start_) {
			manager::remove_turn_started_observer(this);
		}
		if (invalidate_on_tod_change_) {
			manager::remove_tod_changed_observer(this);
		}
		if (invalidate_on_gamestate_change_) {
			manager::remove_gamestate_observer(this);
		}
		if (invalidate_on_minor_gamestate_change_) {
			///@todo 1.9 add minor_gamestate_change_observer
			//manager::remove_minor_gamestate_observer(this);
		}
	}

lg::log_domain& aspect::log()
{
	return log_ai_aspect;
}

void aspect::on_create()
{
}

bool aspect::redeploy(const config &cfg, const std::string& /*id*/)
{
	if (invalidate_on_turn_start_) {
		manager::remove_turn_started_observer(this);
	}
	if (invalidate_on_tod_change_) {
		manager::remove_tod_changed_observer(this);
	}
	if (invalidate_on_gamestate_change_) {
		manager::remove_gamestate_observer(this);
	}
	if (invalidate_on_minor_gamestate_change_) {
		///@todo 1.9 add minor_gamestate_change_observer
		//manager::remove_minor_gamestate_observer(this);
	}

	valid_ = false;
	valid_variant_ =false;
	valid_lua_ = false;
	cfg_ = cfg;
	invalidate_on_turn_start_ = cfg["invalidate_on_turn_start"].to_bool(true);
	invalidate_on_tod_change_ = cfg["invalidate_on_tod_change"].to_bool(true);
	invalidate_on_gamestate_change_ = cfg["invalidate_on_gamestate_change"].to_bool();
	invalidate_on_minor_gamestate_change_ = cfg["invalidate_on_minor_gamestate_change"].to_bool();
	engine_ = cfg["engine"].str();
	name_ = cfg["name"].str();
	id_ = cfg["id"].str();
	DBG_AI_ASPECT << "redeploying aspect: engine=["<<engine_<<"], name=["<<name_<<"], id=["<<id_<<"]"<< std::endl;
	if (invalidate_on_turn_start_) {
		manager::add_turn_started_observer(this);
	}
	if (invalidate_on_tod_change_) {
		manager::add_tod_changed_observer(this);
	}
	if (invalidate_on_gamestate_change_) {
		manager::add_gamestate_observer(this);
	}
	if (invalidate_on_minor_gamestate_change_) {
		///@todo 1.9 add minor_gamestate_change_observer
		//manager::add_minor_gamestate_observer(this);
	}
	return true;
}

config aspect::to_config() const
{
	config cfg;
	cfg["invalidate_on_turn_start"] = invalidate_on_turn_start_;
	cfg["invalidate_on_tod_change"] = invalidate_on_tod_change_;
	cfg["invalidate_on_gamestate_change"] = invalidate_on_gamestate_change_;
	cfg["invalidate_on_minor_gamestate_change"] = invalidate_on_minor_gamestate_change_;
	if (!time_of_day_.empty()) {
		cfg["time_of_day"] = time_of_day_;
	}
	if (!turns_.empty()) {
		cfg["turns"] = turns_;
	}
	cfg["engine"] = engine_;
	cfg["name"] = name_;
	cfg["id"] = id_;
	return cfg;
}

bool aspect::active() const
{
	return this->is_active(time_of_day_,turns_);
}

bool aspect::delete_all_facets()
{
	return false;
}

known_aspect::known_aspect(const std::string &name)
	: name_(name)
{
}

const std::string& known_aspect::get_name() const
{
	return name_;
}

known_aspect::~known_aspect()
{
}

std::string lua_aspect_visitor::quote_string(const std::string& s)
{
	if (s.find_first_of('"') == std::string::npos) {
		return '"' + s + '"';
	} else if (s.find_first_of("'") == std::string::npos) {
		return "'" + s + "'";
	} else {
		return "[=====[" + s + "]=====]";
	}
}

// This is defined in the source file so that it can easily access the logger
bool aspect_factory::is_duplicate(const std::string& name)
{
	if (get_list().find(name) != get_list().end()) {
		ERR_AI_ASPECT << "Error: Attempt to double-register aspect " << name << std::endl;
		return true;
	}
	return false;
}

} //end of namespace ai
