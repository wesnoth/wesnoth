/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCRIPTING_LUA_HPP
#define SCRIPTING_LUA_HPP

#include "game_events.hpp"

struct lua_State;

/**
 * Proxy table for the AI context
 */
class lua_ai_context
{
private:
	lua_State *L;
	int num_;
public:
	lua_ai_context(lua_State *l, int num) : L(l), num_(num)
	{
	}
	~lua_ai_context();
	void load();
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
public:
	lua_ai_action_handler(lua_State *l, lua_ai_context &context, int num) : L(l), context_(context),num_(num)
	{
	}
	~lua_ai_action_handler();
	void handle(config &);
};


class LuaKernel
{
	lua_State *mState;
	bool execute(char const *, int, int);
public:
	LuaKernel();
	~LuaKernel();
	void run_event(vconfig const &, game_events::queued_event const &);
	bool run_filter(char const *name, unit const &u);
	/** Runs a plain script. */
	void run(char const *prog) { execute(prog, 0, 0); }
	lua_ai_context* create_ai_context(char const *code, int side);
	lua_ai_action_handler* create_ai_action_handler(char const *code, lua_ai_context &context);
};

#endif
