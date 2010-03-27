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
 * @file ai/composite/aspect.cpp
 */

#include "aspect.hpp"
#include "../manager.hpp"
#include "../../log.hpp"

namespace ai {

static lg::log_domain log_ai_aspect("ai/aspect");
#define DBG_AI_ASPECT LOG_STREAM(debug, log_ai_aspect)
#define LOG_AI_ASPECT LOG_STREAM(info, log_ai_aspect)
#define WRN_AI_ASPECT LOG_STREAM(warn, log_ai_aspect)
#define ERR_AI_ASPECT LOG_STREAM(err, log_ai_aspect)

aspect::aspect(readonly_context &context, const config &cfg, const std::string &id)
		: valid_(false), valid_variant_(false), cfg_(cfg), invalidate_on_turn_start_(utils::string_bool(cfg["invalidate_on_turn_start"],true)), invalidate_on_tod_change_(utils::string_bool(cfg["invalidate_on_tod_change"],true)), invalidate_on_gamestate_change_(utils::string_bool(cfg["invalidate_on_gamestate_change"])), invalidate_on_minor_gamestate_change_(utils::string_bool(cfg["invalidate_on_minor_gamestate_change"])),engine_(cfg["engine"]),name_(cfg["name"]),id_(id)
	{
		DBG_AI_ASPECT << "creating new aspect: engine=["<<engine_<<"], name=["<<name_<<"], id=["<<id_<<"]"<< std::endl;
		init_readonly_context_proxy(context);
		redeploy(cfg,id);
	}


aspect::~aspect()
	{
		if (invalidate_on_turn_start_) {
			manager::remove_turn_started_observer(this);
		}
		if (invalidate_on_tod_change_) {
			//@todo 1.9 add tod_changed_observer
			//manager::remove_tod_changed_observer(this);
		}
		if (invalidate_on_gamestate_change_) {
			manager::remove_gamestate_observer(this);
		}
		if (invalidate_on_minor_gamestate_change_) {
			//@todo 1.9 add minor_gamestate_change_observer
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


const std::string& aspect::get_id() const
{
	return id_;
}


const std::string& aspect::get_name() const
{
	return name_;
}


const std::string& aspect::get_engine() const
{
	return engine_;
}


bool aspect::redeploy(const config &cfg, const std::string& /*id*/)
{
	if (invalidate_on_turn_start_) {
		manager::remove_turn_started_observer(this);
	}
	if (invalidate_on_tod_change_) {
		//@todo 1.9 add tod_changed_observer
		//manager::remove_tod_changed_observer(this);
	}
	if (invalidate_on_gamestate_change_) {
		manager::remove_gamestate_observer(this);
	}
	if (invalidate_on_minor_gamestate_change_) {
		//@todo 1.9 add minor_gamestate_change_observer
		//manager::remove_minor_gamestate_observer(this);
	}

	valid_ = false;
	valid_variant_ =false;
	cfg_ = cfg;
	invalidate_on_turn_start_ = utils::string_bool(cfg["invalidate_on_turn_start"],true);
	invalidate_on_tod_change_ = utils::string_bool(cfg["invalidate_on_tod_change"],true);
	invalidate_on_gamestate_change_ = utils::string_bool(cfg["invalidate_on_gamestate_change"]);
	invalidate_on_minor_gamestate_change_ = utils::string_bool(cfg["invalidate_on_minor_gamestate_change"]);
	engine_ = cfg["engine"];
	name_ = cfg["name"];
	id_ = cfg["id"];
	DBG_AI_ASPECT << "redeploying aspect: engine=["<<engine_<<"], name=["<<name_<<"], id=["<<id_<<"]"<< std::endl;
	if (invalidate_on_turn_start_) {
		manager::add_turn_started_observer(this);
	}
	if (invalidate_on_tod_change_) {
		//@todo 1.9 add tod_changed_observer
		//manager::add_tod_changed_observer(this);
	}
	if (invalidate_on_gamestate_change_) {
		manager::add_gamestate_observer(this);
	}
	if (invalidate_on_minor_gamestate_change_) {
		//@todo 1.9 add minor_gamestate_change_observer
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
	cfg["engine"] = engine_;
	cfg["name"] = name_;
	cfg["id"] = id_;
	return cfg;
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


} //end of namespace ai
