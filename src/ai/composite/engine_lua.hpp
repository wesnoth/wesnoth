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
 * @file ai/composite/engine_lua.hpp
 */

#ifndef AI_COMPOSITE_ENGINE_LUA_HPP_INCLUDED
#define AI_COMPOSITE_ENGINE_LUA_HPP_INCLUDED

#include "../../global.hpp"

#include "engine.hpp"
#include "../contexts.hpp"

class lua_ai_context;

//============================================================================
namespace ai {

class engine_lua : public engine {
public:
	engine_lua( readonly_context &context, const config &cfg );

	virtual ~engine_lua();

	/**
	 * Taka a config (with engine=lua in it)
	 * and parse several (usually, 1) candidate actions out of it
	 */
	virtual void do_parse_candidate_action_from_config( rca_context &context, const config &cfg, std::back_insert_iterator<std::vector< candidate_action_ptr > > b );

	/**
	 * Taka a config (with engine=lua in it)
	 * and parse several (usually, 1) stages out of it
	 */
	virtual void do_parse_stage_from_config( ai_context &context, const config &cfg, std::back_insert_iterator<std::vector< stage_ptr > > b );

	virtual std::string evaluate(const std::string &str);

	/**
	 * Serialize to config
	 */
	virtual config to_config() const;

	/**
	 * Method to inject AI context into the engine.
	 * The context includes all that in necessary for the AI -
	 * , like access to game state and movement/attack routines.
	 */
	virtual void set_ai_context(ai_context *context);
private:
	//There is one lua engine per AI. So, it can hold state
	boost::shared_ptr<lua_ai_context> lua_ai_context_;
};

} //end of namespace ai

#endif
