/*
   Copyright (C) 2010 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef AI_LUA_CORE_HPP
#define AI_LUA_CORE_HPP

#include <memory>

struct lua_State;
class game_lua_kernel;
class config;

namespace ai {

class engine_lua;
class lua_object_base;
typedef std::shared_ptr<lua_object_base> lua_object_ptr;

/**
 * Proxy table for the AI context
 */
class lua_ai_context
{
private:
	lua_State *L;
	int num_;
	int side_;
	lua_ai_context(lua_State *l, int num, int side) : L(l), num_(num), side_(side)
	{
	}
	static lua_ai_context* create(lua_State *L, char const *code, engine_lua *engine);
public:
	~lua_ai_context();
	void update_state();
	void get_persistent_data(config &) const;
	void set_persistent_data(const config &);
	void get_arguments(config &) const;
	void set_arguments(const config &);
	void push_ai_table();
	static void init(lua_State *L);
	friend class ::game_lua_kernel;
	friend class lua_ai_load;
};

class lua_ai_load
{
	lua_State* L;
	static int refcount;
public:
	bool was_readonly;
	lua_ai_load(lua_ai_context& ctx, bool read_only);
	~lua_ai_load();
};

/**
 * Proxy class for calling AI action handlers defined in Lua.
 */
class lua_ai_action_handler
{
private:
	lua_State *L;
	lua_ai_context &context_;
	int num_;
	lua_ai_action_handler(lua_State *l, lua_ai_context &context, int num) : L(l), context_(context),num_(num)
	{
	}
	static lua_ai_action_handler* create(lua_State *L, char const *code, lua_ai_context &context);
public:
	~lua_ai_action_handler();
	void handle(const config &cfg, bool read_only, lua_object_ptr l_obj);
	friend class ::game_lua_kernel;
};

}//of namespace ai

#endif
