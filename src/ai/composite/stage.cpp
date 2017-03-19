/*
   Copyright (C) 2009 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
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
 * Stage of a composite AI
 * @file
 */

#include "ai/composite/ai.hpp"
#include "ai/composite/engine.hpp"
#include "ai/composite/stage.hpp"
#include "ai/contexts.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "tod_manager.hpp"
#include <map>
#include <string>

namespace ai {

static lg::log_domain log_ai_stage("ai/stage");
#define DBG_AI_STAGE LOG_STREAM(debug, log_ai_stage)
#define LOG_AI_STAGE LOG_STREAM(info, log_ai_stage)
#define ERR_AI_STAGE LOG_STREAM(err, log_ai_stage)

// =======================================================================
// COMPOSITE AI STAGE
// =======================================================================

stage::stage( ai_context &context, const config &cfg )
	: recursion_counter_(context.get_recursion_count()), cfg_(cfg)
{
	init_ai_context_proxy(context);
}

void stage::on_create()
{
	LOG_AI_STAGE << "side "<< get_side() << " : "<<" created stage with name=["<<cfg_["name"]<<"]"<<std::endl;
}

stage::~stage()
{
}

bool stage::play_stage()
{
	return do_play_stage();
}

int stage::get_recursion_count() const
{
	return recursion_counter_.get_count();
}

config stage::to_config() const
{
	config cfg;
	cfg["engine"] = cfg_["engine"];
	cfg["name"] = cfg_["name"];
	cfg["id"] = cfg_["id"];
	return cfg;
}

std::string stage::get_id() const
{
	return cfg_["id"];
}

std::string stage::get_engine() const
{
	return cfg_["engine"];
}

std::string stage::get_name() const
{
	return cfg_["name"];
}




// =======================================================================
// COMPOSITE AI IDLE STAGE
// =======================================================================


idle_stage::idle_stage( ai_context &context, const config &cfg )
	: stage(context,cfg)
{
}

idle_stage::~idle_stage()
{
}

bool idle_stage::do_play_stage(){
	LOG_AI_STAGE << "Turn " << resources::tod_manager->turn() << ": playing idle stage for side: "<< get_side() << std::endl;
	return false;
}


// This is defined in the source file so that it can easily access the logger
bool stage_factory::is_duplicate(const std::string& name)
{
	if (get_list().find(name) != get_list().end()) {
		ERR_AI_STAGE << "Error: Attempt to double-register stage " << name << std::endl;
		return true;
	}
	return false;
}


} //end of namespace ai
