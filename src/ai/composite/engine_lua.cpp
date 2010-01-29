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
 * LUA AI Support engine - creating specific ai components from config
 * @file ai/composite/engine_lua.cpp
 */

#include "ai.hpp"
#include "engine_lua.hpp"
#include "rca.hpp"
#include "../../log.hpp"

namespace ai {

static lg::log_domain log_ai_engine_lua("ai/engine/lua");
#define DBG_AI_LUA LOG_STREAM(debug, log_ai_engine_lua)
#define LOG_AI_LUA LOG_STREAM(info, log_ai_engine_lua)
#define WRN_AI_LUA LOG_STREAM(warn, log_ai_engine_lua)
#define ERR_AI_LUA LOG_STREAM(err, log_ai_engine_lua)

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

class lua_candidate_action_wrapper : public candidate_action {
public:
	lua_candidate_action_wrapper( rca_context &context, const config &cfg )
		: candidate_action(context,cfg),cfg_(cfg)
	{

	}

	virtual ~lua_candidate_action_wrapper() {}


	virtual double evaluate()
	{
		//@todo: lua must evaluate the score and return it
		return 0;
	}


	virtual void execute()
	{
		//@todo: lua must do the actions.
	}

	virtual config to_config() const
	{
		return cfg_;//or we can serialize our current state to config instead of using original cfg
	}
private:
	const config cfg_;
};


/**
 * Note that initially we get access only to readonly context (engine is created rather early, when there's no way to move/attack.
 * We inject full ai_context later.
 */
engine_lua::engine_lua( readonly_context &context, const config &cfg )
	: engine(context,cfg)
{
	name_ = "lua";
}


engine_lua::~engine_lua()
{
}


void engine_lua::do_parse_candidate_action_from_config( rca_context & /*context*/, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b ){
	if (!cfg) {
		return;
	}
	candidate_action_ptr ca_ptr;
	//@todo : need to code an adapter class which implements the candidate action interface
	//ca_ptr = candidate_action_ptr(new lua_candidate_action_wrapper(context,cfg));
	if (ca_ptr) {
		*b = ca_ptr;
	}
}

void engine_lua::do_parse_stage_from_config( ai_context & /*context*/, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b )
{
	if (!cfg) {
		return;
	}
	stage_ptr st_ptr;

	//@todo : need to code an adapter class which implements the stage interface
	//it's simple - the main part is do_play_stage() method
	//st_ptr = stage_ptr(new lua_stage_wrapper(context,cfg));
	if (st_ptr) {
		st_ptr->on_create();
		*b = st_ptr;
	}
}

std::string engine_lua::evaluate(const std::string &/*str*/)
{
	//@todo: this is not mandatory, but if we want to allow lua to evaluate
	// something 'in context' of this ai, this will be useful
	return "";
}


void engine_lua::set_ai_context(ai_context * /*context*/)
{
	//this function is called when the ai is fully initialized
}


config engine_lua::to_config() const
{
	config cfg = engine::to_config();
	//we can modify the cfg here
	return cfg;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} //end of namespace ai
