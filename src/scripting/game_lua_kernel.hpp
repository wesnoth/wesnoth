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

#include "scripting/lua_kernel_base.hpp" // for lua_kernel_base

#include "game_events/action_wml.hpp"   // for wml_action, etc

#include <string>                       // for string

class config;
class unit;
class vconfig;
namespace ai { class engine_lua; }
namespace ai { class lua_ai_action_handler; }
namespace ai { class lua_ai_context; }
namespace game_events { struct queued_event; }

void extract_preload_scripts(config const &);

class LuaKernel : public lua_kernel_base
{
	const config &level_;

public:
	LuaKernel(const config &);

	virtual std::string my_name() { return "Game Lua Kernel"; }

	void initialize();
	void save_game(config &);
	void load_game();
	bool run_event(game_events::queued_event const &);
	void set_wml_action(std::string const &, game_events::wml_action::handler);
	bool run_wml_action(std::string const &, vconfig const &,
		game_events::queued_event const &);
	bool run_filter(char const *name, unit const &u);

	virtual void log_error(char const* msg, char const* context = "Lua error");

	ai::lua_ai_context* create_lua_ai_context(char const *code, ai::engine_lua *engine);
	ai::lua_ai_action_handler* create_lua_ai_action_handler(char const *code, ai::lua_ai_context &context);
};

#endif
