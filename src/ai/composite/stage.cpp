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
 * Stage of a composite AI
 * @file ai/composite/stage.cpp
 */

#include "ai.hpp"
#include "stage.hpp"
#include "../contexts.hpp"
#include "../../foreach.hpp"
#include "../../log.hpp"
#include <map>
#include <string>

namespace ai {

namespace composite_ai {

static lg::log_domain log_ai_composite_stage("ai/composite/stage");
#define DBG_AI_COMPOSITE_STAGE LOG_STREAM(debug, log_ai_composite_stage)
#define LOG_AI_COMPOSITE_STAGE LOG_STREAM(info, log_ai_composite_stage)
#define ERR_AI_COMPOSITE_STAGE LOG_STREAM(err, log_ai_composite_stage)

// =======================================================================
// COMPOSITE AI STAGE
// =======================================================================

stage::stage( composite_ai_context &context, const config &cfg )
	: recursion_counter_(context.get_recursion_count()), cfg_(cfg)
{
	init_composite_ai_context_proxy(context);
}

void stage::on_create()
{
	LOG_AI_COMPOSITE_STAGE << "side "<< get_side() << " : "<<" created stage with name=["<<cfg_["name"]<<"]"<<std::endl;
}

stage::~stage()
{
}

void stage::play_stage()
{
	do_play_stage();
}

int stage::get_recursion_count() const
{
	return recursion_counter_.get_count();
}

// =======================================================================
// COMPOSITE AI IDLE STAGE
// =======================================================================


idle_stage::idle_stage( composite_ai_context &context, const config &cfg )
	: stage(context,cfg)
{
}

idle_stage::~idle_stage()
{
}

void idle_stage::do_play_stage(){
	LOG_AI_COMPOSITE_STAGE << "Turn " << get_info().state.turn() << ": playing idle stage for side: "<< get_side() << std::endl;
}

} //end of namespace composite_ai

} //end of namespace ai
