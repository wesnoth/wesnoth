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
 * LUA AI Support engine - creating specific ai components from config
 * @file
 */

#include "ai/lua/engine_lua.hpp"
#include "ai/composite/ai.hpp"
#include "ai/composite/goal.hpp"
#include "ai/composite/rca.hpp"
#include "ai/composite/stage.hpp"
#include "ai/composite/aspect.hpp"

#include "ai/gamestate_observer.hpp"

#include "log.hpp"
#include "resources.hpp"
#include "ai/lua/core.hpp"
#include "ai/lua/lua_object.hpp"
#include "game_board.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"


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

typedef std::shared_ptr< lua_object<int> > lua_int_obj;

class lua_candidate_action_wrapper_base : public candidate_action {

public:
	lua_candidate_action_wrapper_base( rca_context &context, const config &cfg)
		: candidate_action(context, cfg),evaluation_action_handler_(),execution_action_handler_(),serialized_evaluation_state_(cfg.child_or_empty("args"))
	{
		// do nothing
	}

	virtual ~lua_candidate_action_wrapper_base() {}

	virtual double evaluate()
	{
		lua_int_obj l_obj = lua_int_obj(new lua_object<int>());

		if (evaluation_action_handler_) {
			evaluation_action_handler_->handle(serialized_evaluation_state_, true, l_obj);
		} else {
			return BAD_SCORE;
		}

		std::shared_ptr<int> result = l_obj->get();

		return result ? *result : 0.0;
	}


	virtual void execute()	{
		if (execution_action_handler_) {
			lua_object_ptr nil;
			execution_action_handler_->handle(serialized_evaluation_state_, false, nil);
		}
	}

	virtual config to_config() const {
		config cfg = candidate_action::to_config();
		cfg.add_child("args",serialized_evaluation_state_);
		return cfg;
	}

protected:
	std::shared_ptr<lua_ai_action_handler> evaluation_action_handler_;
	std::shared_ptr<lua_ai_action_handler> execution_action_handler_;
	config serialized_evaluation_state_;
};

class lua_candidate_action_wrapper : public lua_candidate_action_wrapper_base {

public:
	lua_candidate_action_wrapper( rca_context &context, const config &cfg, lua_ai_context &lua_ai_ctx)
		: lua_candidate_action_wrapper_base(context,cfg),evaluation_(cfg["evaluation"]),execution_(cfg["execution"])
	{
		evaluation_action_handler_ = std::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_lua_ai_action_handler(evaluation_.c_str(),lua_ai_ctx));
		execution_action_handler_ = std::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_lua_ai_action_handler(execution_.c_str(),lua_ai_ctx));
	}

	virtual ~lua_candidate_action_wrapper() {}

	virtual config to_config() const
	{
		config cfg = lua_candidate_action_wrapper_base::to_config();
		cfg["evaluation"] = evaluation_;
		cfg["execution"] = execution_;
		return cfg;
	}

private:
	std::string evaluation_;
	std::string execution_;
};

class lua_candidate_action_wrapper_external : public lua_candidate_action_wrapper_base {
public:
	lua_candidate_action_wrapper_external(rca_context& context, const config& cfg, lua_ai_context &lua_ai_ctx)
		: lua_candidate_action_wrapper_base(context,cfg), location_(cfg["location"]), use_parms_(false)
	{
		if (cfg.has_attribute("exec_parms") || cfg.has_attribute("eval_parms")) {
			use_parms_ = true;
			exec_parms_ = cfg["exec_parms"].str();
			eval_parms_ = cfg["eval_parms"].str();
		}
		std::string eval_code;
		std::string exec_code;
		generate_code(eval_code, exec_code);

		evaluation_action_handler_ = std::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_lua_ai_action_handler(eval_code.c_str(),lua_ai_ctx));
		execution_action_handler_ = std::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_lua_ai_action_handler(exec_code.c_str(),lua_ai_ctx));
	}

	virtual ~lua_candidate_action_wrapper_external() {}

	virtual config to_config() const
	{
		config cfg = lua_candidate_action_wrapper_base::to_config();
		cfg["location"] = location_;
		if (use_parms_) {
			cfg["eval_parms"] = eval_parms_;
			cfg["exec_parms"] = exec_parms_;
		}
		return cfg;
	}

private:
	std::string location_;
	std::string eval_parms_;
	std::string exec_parms_;
	bool use_parms_;

	void generate_code(std::string& eval, std::string& exec) {
		std::string preamble = "local self, params, data = ...\n";
		std::string load = "wesnoth.require(\"" + location_ + "\")";
		if (use_parms_) {
			eval = preamble + "return " + load + ":evaluation(ai, {" + eval_parms_ + "}, {data = data})";
			exec = preamble + load + ":execution(ai, {" + exec_parms_ + "}, {data = data})";
		} else {
			eval = preamble + "return " + load + ".evaluation(self, params, data)";
			exec = preamble + load + ".execution(self, params, data)";
		}
	}
};

class lua_sticky_candidate_action_wrapper : public lua_candidate_action_wrapper {
public:
	lua_sticky_candidate_action_wrapper( rca_context &context, const config &cfg, lua_ai_context &lua_ai_ctx)
		: lua_candidate_action_wrapper(context, cfg, lua_ai_ctx)
		, bound_unit_()
	{
		map_location loc(cfg["unit_x"], cfg["unit_y"], wml_loc()); // lua and c++ coords differ by one
		bound_unit_ = unit_ptr(new unit(*resources::gameboard->units().find(loc)));
	}

	virtual double evaluate()
	{
		if (resources::gameboard->units().find(bound_unit_->underlying_id()).valid())
		{
			return lua_candidate_action_wrapper_base::evaluate();
		}
		else
		{
			this->set_to_be_removed();
			return 0; // Is 0 what we return when we don't want the action to be executed?
		}
	}

	virtual void execute()
	{
		lua_candidate_action_wrapper_base::execute();
		this->disable(); // we do not want to execute the same sticky CA twice -> will be moved out to Lua later
	}
private:
	unit_ptr bound_unit_;

};

class lua_stage_wrapper : public stage {
public:
	lua_stage_wrapper( ai_context &context, const config &cfg, lua_ai_context &lua_ai_ctx )
		: stage(context,cfg),action_handler_(),code_(cfg["code"]),serialized_evaluation_state_(cfg.child_or_empty("args"))
	{
		action_handler_ =  std::shared_ptr<lua_ai_action_handler>(resources::lua_kernel->create_lua_ai_action_handler(code_.c_str(),lua_ai_ctx));
	}

	virtual ~lua_stage_wrapper()
	{
	}

	virtual bool do_play_stage()
	{
		gamestate_observer gs_o;

		if (action_handler_) {
			lua_object_ptr nil;
			action_handler_->handle(serialized_evaluation_state_, false, nil);
		}

		return gs_o.is_gamestate_changed();
	}

	virtual config to_config() const
	{
		config cfg = stage::to_config();
		cfg["code"] = code_;
		cfg.add_child("args",serialized_evaluation_state_);
		return cfg;
	}
private:
	std::shared_ptr<lua_ai_action_handler> action_handler_;
	std::string code_;
	config serialized_evaluation_state_;
};


/**
 * Note that initially we get access only to readonly context (engine is created rather early, when there's no way to move/attack.
 * We inject full ai_context later.
 */
engine_lua::engine_lua( readonly_context &context, const config &cfg )
	: engine(context,cfg)
	, code_(get_engine_code(cfg))
	, lua_ai_context_(resources::lua_kernel->create_lua_ai_context(
		get_engine_code(cfg).c_str(), this))
{
	name_ = "lua";
	config data(cfg.child_or_empty("data"));
	config args(cfg.child_or_empty("args"));

	if (lua_ai_context_) { // The context might be nullptr if the config contains errors
		lua_ai_context_->set_persistent_data(data);
		lua_ai_context_->set_arguments(args);
		lua_ai_context_->update_state();
	}
}

std::string engine_lua::get_engine_code(const config &cfg) const
{
	if (cfg.has_attribute("code")) {
		return cfg["code"].str();
	}
	// If there is no engine defined we create a dummy engine
	std::string code = "wesnoth.require(\"ai/lua/dummy_engine_lua.lua\")";
	return code;
}

engine_lua::~engine_lua()
{
}

bool engine_lua::is_ok() const
{
	return lua_ai_context_ ? true : false;
}

void engine_lua::push_ai_table()
{
	if (game_config::debug)
	{
		lua_ai_context_->push_ai_table();
	}
}

void engine_lua::do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b ){
	if (!cfg) {
		return;
	}

	if (!lua_ai_context_) {
		return;
	}

	candidate_action_ptr ca_ptr;
	if (!cfg["sticky"].to_bool())
	{
		if (cfg.has_attribute("location")) {
			ca_ptr = candidate_action_ptr(new lua_candidate_action_wrapper_external(context,cfg,*lua_ai_context_));
		} else {
			ca_ptr = candidate_action_ptr(new lua_candidate_action_wrapper(context,cfg,*lua_ai_context_));
		}
	}
	else
	{
		ca_ptr = candidate_action_ptr(new lua_sticky_candidate_action_wrapper(context,cfg,*lua_ai_context_));
	}

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

void engine_lua::do_parse_aspect_from_config( const config &cfg, const std::string &id, std::back_insert_iterator<std::vector< aspect_ptr > > b )
{
	const std::string aspect_factory_key = id+"*lua_aspect"; // @note: factory key for a lua_aspect
	lua_aspect_factory::factory_map::iterator f = lua_aspect_factory::get_list().find(aspect_factory_key);

	if (f == lua_aspect_factory::get_list().end()){
		ERR_AI_LUA << "side "<<ai_.get_side()<< " : UNKNOWN aspect["<<aspect_factory_key<<"]" << std::endl;
		DBG_AI_LUA << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	aspect_ptr new_aspect = f->second->get_new_instance(ai_,cfg,id,lua_ai_context_);
	if (!new_aspect) {
		ERR_AI_LUA << "side "<<ai_.get_side()<< " : UNABLE TO CREATE aspect, key=["<<aspect_factory_key<<"]"<< std::endl;
		DBG_AI_LUA << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	*b = new_aspect;
}

void engine_lua::do_parse_goal_from_config(const config &cfg, std::back_insert_iterator<std::vector< goal_ptr > > b )
{
	goal_factory::factory_map::iterator f = goal_factory::get_list().find(cfg["name"]);
	if (f == goal_factory::get_list().end()){
		ERR_AI_LUA << "side "<<ai_.get_side()<< " : UNKNOWN goal["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_LUA << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	goal_ptr new_goal = f->second->get_new_instance(ai_,cfg);
	new_goal->on_create(lua_ai_context_);
	if (!new_goal || !new_goal->ok()) {
		ERR_AI_LUA << "side "<<ai_.get_side()<< " : UNABLE TO CREATE goal["<<cfg["name"]<<"]"<< std::endl;
		DBG_AI_LUA << "config snippet contains: " << std::endl << cfg << std::endl;
		return;
	}
	*b = new_goal;
}


std::string engine_lua::evaluate(const std::string &/*str*/)
{
	///@todo this is not mandatory, but if we want to allow lua to evaluate
	// something 'in context' of this ai, this will be useful
	return "";
}

config engine_lua::to_config() const
{
	config cfg = engine::to_config();

	cfg["id"] = get_id();
	cfg["code"] = this->code_;

	if (lua_ai_context_) {
		config data = config();
		lua_ai_context_->get_persistent_data(data);
		cfg.add_child("data") = data;
	}

	return cfg;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} //end of namespace ai
