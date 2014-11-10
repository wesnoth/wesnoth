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
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "mt_rng.hpp"
#include "scripting/lua_api.hpp"
#include "scripting/lua_common.hpp"

#include <cstring>
#include <string>
#include <boost/bind.hpp>

static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)
#define DBG_NG LOG_STREAM(debug, log_mapgen)

// Add compiler directive suppressing unused variable warning
#if defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__)
#define ATTR_UNUSED( x ) __attribute__((unused)) x
#else
#define ATTR_UNUSED( x ) x
#endif

// Begin lua rng bindings

using rand_rng::mt_rng;

static const char * Rng = "Rng";

static int impl_rng_create(lua_State* L)
{
	mt_rng * ATTR_UNUSED(rng) = new ( lua_newuserdata(L, sizeof(mt_rng)) ) mt_rng();
	luaL_setmetatable(L, Rng);

	return 1;
}

static int impl_rng_destroy(lua_State* L)
{
	mt_rng * d = static_cast< mt_rng *> (luaL_testudata(L, 1, Rng));
	if (d == NULL) {
		ERR_NG << "rng_destroy called on data of type: " << lua_typename( L, lua_type( L, 1 ) ) << std::endl;
		ERR_NG << "This may indicate a memory leak, please report at bugs.wesnoth.org" << std::endl;
	} else {
		d->~mt_rng();
	}
	return 0;
}

static int impl_rng_seed(lua_State* L)
{
	mt_rng * rng = static_cast<mt_rng *>(luaL_checkudata(L, 1, Rng));
	std::string seed = luaL_checkstring(L, 2);

	rng->seed_random(seed);
	return 0;
}

static int impl_rng_draw(lua_State* L)
{
	mt_rng * rng = static_cast<mt_rng *>(luaL_checkudata(L, 1, Rng));

	lua_pushnumber(L, rng->get_next_random());
	return 1;
}

// End Lua Rng bindings

mapgen_lua_kernel::mapgen_lua_kernel()
	: lua_kernel_base()
{
	lua_State *L = mState;

	// Add mersenne twister rng wrapper
	luaL_newmetatable(L, Rng);

	static luaL_Reg const callbacks[] = {
		{ "create",         &impl_rng_create},
		{ "__gc",           &impl_rng_destroy},
		{ "seed", 	    &impl_rng_seed},
		{ "draw",	    &impl_rng_draw},
		{ NULL, NULL }
	};
	luaL_setfuncs(L, callbacks, 0);

	lua_pushvalue(L, -1); //make a copy of this table, set it to be its own __index table
	lua_setfield(L, -2, "__index");

	lua_setglobal(L, Rng);
}

void mapgen_lua_kernel::run_generator(const char * prog, const config & generator)
{
	load_string(prog, boost::bind(&lua_kernel_base::throw_exception, this, _1, _2));
	luaW_pushconfig(mState, generator);
	protected_call(1, 1, boost::bind(&lua_kernel_base::throw_exception, this, _1, _2));
}

std::string mapgen_lua_kernel::create_map(const char * prog, const config & generator) // throws game::lua_error
{
	run_generator(prog, generator);

	if (!lua_isstring(mState,-1)) {
		std::string msg = "expected a string, found a ";
		msg += lua_typename(mState, lua_type(mState, -1));
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
		throw game::lua_error(msg.c_str(),"bad return value");
	}
	config result;
	if (!luaW_toconfig(mState, -1, result)) {
		std::string msg = "expected a config, it seems malformed ";
		throw game::lua_error(msg.c_str(),"bad return value");
	}
	return result;
}
