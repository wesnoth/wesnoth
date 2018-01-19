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
 * LUA AI Support engine - creating specific ai components from config
 * @file
 */

#ifndef AI_COMPOSITE_ENGINE_LUA_HPP_INCLUDED
#define AI_COMPOSITE_ENGINE_LUA_HPP_INCLUDED

#include "ai/composite/engine.hpp"

//============================================================================
namespace ai {

class lua_ai_context;

class engine_lua : public engine {
public:
	engine_lua( readonly_context &context, const config &cfg );

	virtual ~engine_lua();

	bool is_ok() const;

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

	/**
	 * Taka a config (with engine=lua in it)
	 * and parse several (usually, 1) aspects out of it
	 */
	virtual void do_parse_aspect_from_config( const config &cfg, const std::string &id, std::back_insert_iterator<std::vector< aspect_ptr > > b );

	virtual void do_parse_goal_from_config(const config &cfg, std::back_insert_iterator<std::vector< goal_ptr > > b );

	virtual std::string evaluate(const std::string &str);

	/**
	 * Method that pushes the AI table of the lua_context on the stack
	 * for debugging purposes
	 */

	virtual void push_ai_table();

	/**
	 * Serialize to config
	 */
	virtual config to_config() const;

// 	/**
// 	 * Method to inject AI context into the engine.
// 	 * The context includes all that in necessary for the AI -
// 	 * , like access to game state and movement/attack routines.
// 	 */
// 	virtual void set_ai_context(ai_context *context);

private:

	/**
	 * The underlying lua code
	 */
	std::string code_;

	//There is one lua engine per AI. So, it can hold state
	std::shared_ptr<lua_ai_context> lua_ai_context_;

	std::string get_engine_code(const config&) const;

};

} //end of namespace ai

#endif
