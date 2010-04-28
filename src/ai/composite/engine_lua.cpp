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
#include "stage.hpp"
#include "../../log.hpp"
#include "../../resources.hpp"
#include "../../scripting/lua.hpp"
#include "../gamestate_observer.hpp"

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
	lua_candidate_action_wrapper( rca_context &context, const config &cfg, lua_ai_context &lua_ai_ctx)
		: candidate_action(context,cfg),evaluation_(cfg["evaluation"]),evaluation_action_handler_(),execution_(cfg["execution"]),execution_action_handler_(),serialized_evaluation_state_()
	{
		evaluation_action_handler_ = boost::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_ai_action_handler(evaluation_.c_str(),lua_ai_ctx));
		execution_action_handler_ = boost::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_ai_action_handler(execution_.c_str(),lua_ai_ctx));
	}

	virtual ~lua_candidate_action_wrapper() {}


	virtual double evaluate()
	{
		serialized_evaluation_state_ = config();
		if (evaluation_action_handler_) {
			evaluation_action_handler_->handle(serialized_evaluation_state_, true);
		} else {
			return BAD_SCORE;
		}

		return lexical_cast_default<double>(serialized_evaluation_state_["score"],BAD_SCORE);
	}


	virtual void execute()
	{
		if (execution_action_handler_) {
			execution_action_handler_->handle(serialized_evaluation_state_);
		}
	}

	virtual config to_config() const
	{
		config cfg = candidate_action::to_config();
		cfg["evaluation"] = evaluation_;
		cfg["execution"] = execution_;
		cfg.add_child("state",serialized_evaluation_state_);
		return cfg;
	}
private:
	std::string evaluation_;
	boost::shared_ptr<lua_ai_action_handler> evaluation_action_handler_;
	std::string execution_;
	boost::shared_ptr<lua_ai_action_handler> execution_action_handler_;
	config serialized_evaluation_state_;
};



class lua_stage_wrapper : public stage {
public:
	lua_stage_wrapper( ai_context &context, const config &cfg, lua_ai_context &lua_ai_ctx )
		: stage(context,cfg),action_handler_(),code_(cfg["code"]),serialized_evaluation_state_(cfg.child_or_empty("state"))
	{
		action_handler_ =  boost::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_ai_action_handler(code_.c_str(),lua_ai_ctx));
	}

	virtual ~lua_stage_wrapper()
	{
	}

	virtual bool do_play_stage()
	{
		gamestate_observer gs_o;
		if (action_handler_) {
			action_handler_->handle(serialized_evaluation_state_);
		}

		return gs_o.is_gamestate_changed();
	}

	virtual config to_config() const
	{
		config cfg = stage::to_config();
		cfg["code"] = code_;
		cfg.add_child("state",serialized_evaluation_state_);
		return cfg;
	}
private:
	boost::shared_ptr<lua_ai_action_handler> action_handler_;
	std::string code_;
	config serialized_evaluation_state_;
};


/**
 * Note that initially we get access only to readonly context (engine is created rather early, when there's no way to move/attack.
 * We inject full ai_context later.
 */
engine_lua::engine_lua( readonly_context &context, const config &cfg )
	: engine(context,cfg)
	, lua_ai_context_(resources::lua_kernel->create_ai_context(
				  cfg["code"].c_str()
				, this)) //will be moved to set_ai_context
{
	name_ = "lua";
}


engine_lua::~engine_lua()
{
}


void engine_lua::do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b ){
	if (!cfg) {
		return;
	}

	if (!lua_ai_context_) {
		return;
	}

	candidate_action_ptr ca_ptr = candidate_action_ptr(new lua_candidate_action_wrapper(context,cfg,*lua_ai_context_));
	if (ca_ptr) {
		*b = ca_ptr;
	}
}

void engine_lua::do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b )
{
	if (!cfg) {
		return;
	}

	if (!lua_ai_context_) {
		return;
	}

	stage_ptr st_ptr = stage_ptr(new lua_stage_wrapper(context,cfg,*lua_ai_context_));
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
