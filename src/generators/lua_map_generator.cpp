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

#include "lua_map_generator.hpp"

#include "config.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"

#include "mt_rng.hpp"

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include "scripting/lua_api.hpp"

#include <string>

#include <boost/foreach.hpp>


// Add compiler directive suppressing unused variable warning
#if defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__)
#define ATTR_UNUSED( x ) __attribute__((unused)) x
#else
#define ATTR_UNUSED( x ) x
#endif

// Begin lua rng bindings

using rand_rng::mt_rng;

static const char * Rng = "Rng";

static int impl_rng_create(lua_State* L);
static int impl_rng_destroy(lua_State* L);
static int impl_rng_seed(lua_State* L);
static int impl_rng_draw(lua_State* L);

static void initialize_lua_state(lua_State * L)
{
	// Open safe libraries.
	// Debug and OS are not, but most of their functions will be disabled below.
	static const luaL_Reg safe_libs[] = {
		{ "",       luaopen_base   },
		{ "table",  luaopen_table  },
		{ "string", luaopen_string },
		{ "math",   luaopen_math   },
		{ "debug",  luaopen_debug  },
		{ "os",     luaopen_os     },
		{ NULL, NULL }
	};
	for (luaL_Reg const *lib = safe_libs; lib->func; ++lib)
	{
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);  /* remove lib */
	}

	// Disable functions from os which we don't want.
	lua_getglobal(L, "os");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0) {
		lua_pop(L, 1);
		char const* function = lua_tostring(L, -1);
		if(strcmp(function, "clock") == 0 || strcmp(function, "date") == 0
			|| strcmp(function, "time") == 0 || strcmp(function, "difftime") == 0) continue;
		lua_pushnil(L);
		lua_setfield(L, -3, function);
	}
	lua_pop(L, 1);

	// Disable functions from debug which we don't want.
	lua_getglobal(L, "debug");
	lua_pushnil(L);
	while(lua_next(L, -2) != 0) {
		lua_pop(L, 1);
		char const* function = lua_tostring(L, -1);
		if(strcmp(function, "traceback") == 0) continue;
		lua_pushnil(L);
		lua_setfield(L, -3, function);
	}
	lua_pop(L, 1);

	lua_settop(L, 0);

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

int impl_rng_create(lua_State* L)
{
	mt_rng * ATTR_UNUSED(rng) = new ( lua_newuserdata(L, sizeof(mt_rng)) ) mt_rng();
	luaL_setmetatable(L, Rng);

	return 1;
}
int impl_rng_destroy(lua_State* L)
{
	static_cast< mt_rng * >( lua_touserdata( L , 1 ) )->~mt_rng();
	return 0;
}
int impl_rng_seed(lua_State* L)
{
	mt_rng * rng = static_cast<mt_rng *>(luaL_checkudata(L, 1, Rng));
	std::string seed = luaL_checkstring(L, 2);

	rng->seed_random(seed);
	return 0;
}
int impl_rng_draw(lua_State* L)
{
	mt_rng * rng = static_cast<mt_rng *>(luaL_checkudata(L, 1, Rng));

	lua_pushnumber(L, rng->get_next_random());
	return 1;
}

// End Lua Rng bindings

lua_map_generator::lua_map_generator(const config & cfg)
	: id_(cfg["id"])
	, config_name_(cfg["config_name"])
	, create_map_(cfg["create_map"])
	, create_scenario_(cfg["create_scenario"])
	, mState_(luaL_newstate())
{
	const char* required[] = {"id", "config_name", "create_map"};
	BOOST_FOREACH(std::string req, required) {
		if (!cfg.has_attribute(req)) {
			std::string msg = "Error when constructing a lua map generator -- missing a required attribute '";
			msg += req + "'\n";
			msg += "Config was '" + cfg.debug() + "'";
			throw mapgen_exception(msg);
		}
	}

	initialize_lua_state(mState_);
}

lua_map_generator::~lua_map_generator()
{
	lua_close(mState_);
}

std::string lua_map_generator::create_map()
{
	{
		int errcode = luaL_loadstring(mState_, create_map_.c_str());
		if (errcode != LUA_OK) {
			std::string msg = "Error when running lua_map_generator create_map.\n";
			msg += "The generator was: " + config_name_ + "\n";
			msg += "Error when parsing create_map function. ";
			if (errcode == LUA_ERRSYNTAX) {
				msg += "There was a syntax error:\n";
			} else {
				msg += "There was a memory error:\n";
			}
			msg += lua_tostring(mState_, -1);
			throw mapgen_exception(msg);
		}
	}
	{
		int errcode = lua_pcall(mState_, 0, 1, 0);
		if (errcode != LUA_OK) {
			std::string msg = "Error when running lua_map_generator create_map.\n";
			msg += "The generator was: " + config_name_ + "\n";
			msg += "Error when running create_map function. ";
			if (errcode == LUA_ERRRUN) {
				msg += "There was a runtime error:\n";
			} else if (errcode == LUA_ERRERR) {
				msg += "There was an error with the attached debugger:\n";
			} else {
				msg += "There was a memory or garbage collection error:\n";
			}
			msg += lua_tostring(mState_, -1);
			throw mapgen_exception(msg);
		}
	}
	if (!lua_isstring(mState_,-1)) {
		std::string msg = "Error when running lua_map_generator create_map.\n";
		msg += "The generator was: " + config_name_ + "\n";
		msg += "create_map did not return a string, instead it returned '";
		msg += lua_typename(mState_, lua_type(mState_, -1));
		msg += "'";
		throw mapgen_exception(msg);
	}
	return lua_tostring(mState_, -1);
}

config lua_map_generator::create_scenario()
{
	if (!create_scenario_.size()) {
		return map_generator::create_scenario();
	}

	{
		int errcode = luaL_loadstring(mState_, create_scenario_.c_str());
		if (errcode != LUA_OK) {
			std::string msg = "Error when running lua_map_generator create_scenario.\n";
			msg += "The generator was: " + config_name_ + "\n";
			msg += "Error when parsing create_scenario function. ";
			if (errcode == LUA_ERRSYNTAX) {
				msg += "There was a syntax error:\n";
			} else {
				msg += "There was a memory error:\n";
			}
			msg += lua_tostring(mState_, -1);
			throw mapgen_exception(msg);
		}
	}
	{
		int errcode = lua_pcall(mState_, 0, 1, 0);
		if (errcode != LUA_OK) {
			std::string msg = "Error when running lua_map_generator create_scenario.\n";
			msg += "The generator was: " + config_name_ + "\n";
			msg += "Error when running create_scenario function. ";
			if (errcode == LUA_ERRRUN) {
				msg += "There was a runtime error:\n";
			} else if (errcode == LUA_ERRERR) {
				msg += "There was an error with the attached debugger:\n";
			} else {
				msg += "There was a memory or garbage collection error:\n";
			}
			msg += lua_tostring(mState_, -1);
			throw mapgen_exception(msg);
		}
	}
	return luaW_checkconfig(mState_, -1);
}
