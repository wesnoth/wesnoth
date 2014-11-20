/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/mapgen_lua_kernel.hpp"

#include "config.hpp"
#include "game_errors.hpp"
#include "log.hpp"
#include "lua/lua.h"
#include "scripting/lua_api.hpp"
#include "scripting/lua_rng.hpp"

#include <ostream>
#include <string>
#include <boost/bind.hpp>

static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)
#define DBG_NG LOG_STREAM(debug, log_mapgen)

struct lua_State;

mapgen_lua_kernel::mapgen_lua_kernel()
	: lua_kernel_base(NULL)
{
	lua_State *L = mState;
	lua_settop(L, 0);
}

void mapgen_lua_kernel::run_generator(const char * prog, const config & generator)
{
	load_string(prog, boost::bind(&lua_kernel_base::throw_exception, this, _1, _2));
	luaW_pushconfig(mState, generator);
	protected_call(1, 1, boost::bind(&lua_kernel_base::throw_exception, this, _1, _2));
}

void mapgen_lua_kernel::user_config(const char * prog, const config & generator)
{
	run_generator(prog, generator);
}

std::string mapgen_lua_kernel::create_map(const char * prog, const config & generator) // throws game::lua_error
{
	run_generator(prog, generator);

	if (!lua_isstring(mState,-1)) {
		std::string msg = "expected a string, found a ";
		msg += lua_typename(mState, lua_type(mState, -1));
		lua_pop(mState, 1);
		throw game::lua_error(msg.c_str(),"bad return value");
	}

	return lua_tostring(mState, -1);
}

config mapgen_lua_kernel::create_scenario(const char * prog, const config & generator) // throws game::lua_error
{
	run_generator(prog, generator);

	if (!lua_istable(mState, -1)) {
		std::string msg = "expected a config (table), found a ";
		msg += lua_typename(mState, lua_type(mState, -1));
		lua_pop(mState, 1);
		throw game::lua_error(msg.c_str(),"bad return value");
	}
	config result;
	if (!luaW_toconfig(mState, -1, result)) {
		std::string msg = "expected a config, but it is malformed ";
		lua_pop(mState, 1);
		throw game::lua_error(msg.c_str(),"bad return value");
	}
	return result;
}
