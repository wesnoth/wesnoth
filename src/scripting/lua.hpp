/*
   Copyright (C) 2009 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCRIPTING_LUA_HPP
#define SCRIPTING_LUA_HPP

#include "game_events/action_wml.hpp"

class  unit;
struct lua_State;

namespace ai {
class lua_ai_action_handler;
class lua_ai_context;
class engine_lua;
} // of namespace ai

namespace game_events {
	struct queued_event;
}

void extract_preload_scripts(config const &);

class LuaKernel
{
	lua_State *mState;
	const config &level_;
	bool execute(char const *, int, int);
public:
	LuaKernel(const config &);
	~LuaKernel();
	void initialize();
	void save_game(config &);
	void load_game();
	bool run_event(game_events::queued_event const &);
	void set_wml_action(std::string const &, game_events::wml_action::handler);
	bool run_wml_action(std::string const &, vconfig const &,
		game_events::queued_event const &);
	bool run_filter(char const *name, unit const &u);
	/** Runs a plain script. */
	void run(char const *prog) { execute(prog, 0, 0); }
	ai::lua_ai_context* create_lua_ai_context(char const *code, ai::engine_lua *engine);
	ai::lua_ai_action_handler* create_lua_ai_action_handler(char const *code, ai::lua_ai_context &context);
	void load_package();
};

#endif
