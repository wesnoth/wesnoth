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

#include "scripting/lua_kernel_base.hpp"

#include "global.hpp"

#include "log.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "game_errors.hpp"

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include "scripting/lua_api.hpp"

#include <cstring>
#include <string>
#include <boost/bind.hpp>

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

lua_kernel_base::lua_kernel_base()
 : mState(luaL_newstate())
{
	lua_State *L = mState;

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

	// Store the error handler.
	lua_pushlightuserdata(L
			, executeKey);
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	lua_rawset(L, LUA_REGISTRYINDEX);

	lua_settop(L, 0);
}

lua_kernel_base::~lua_kernel_base()
{
	lua_close(mState);
}

void lua_kernel_base::log_error(char const * msg, char const * context)
{
	ERR_LUA << context << ": " << msg;
}

void lua_kernel_base::throw_exception(char const * msg, char const * context)
{
	throw game::lua_error(msg, context);
}

bool lua_kernel_base::protected_call(int nArgs, int nRets)
{
	error_handler eh = boost::bind(&lua_kernel_base::log_error, this, _1, _2 );
	return protected_call(nArgs, nRets, eh);
}

bool lua_kernel_base::load_string(char const * prog)
{
	error_handler eh = boost::bind(&lua_kernel_base::log_error, this, _1, _2 );
	return load_string(prog, eh);
}

bool lua_kernel_base::protected_call(int nArgs, int nRets, error_handler e_h)
{
	lua_State *L = mState;

	// Load the error handler before the function and its arguments.
	lua_pushlightuserdata(L
			, executeKey);

	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_insert(L, -2 - nArgs);

	int error_handler_index = lua_gettop(L) - nArgs - 1;

	// Call the function.
	int errcode = lua_pcall(L, nArgs, nRets, -2 - nArgs);
	tlua_jailbreak_exception::rethrow();

	// Remove the error handler.
	lua_remove(L, error_handler_index);

	if (errcode != LUA_OK) {
		std::string message = lua_tostring(L, -1);

		std::string context = "When executing, ";
		if (errcode == LUA_ERRRUN) {
			context += "Lua runtime error: ";
		} else if (errcode == LUA_ERRERR) {
			context += "Lua error in attached debugger: ";
		} else if (errcode == LUA_ERRMEM) {
			context += "Lua out of memory error: ";
		} else if (errcode == LUA_ERRGCMM) {
			context += "Lua error in garbage collection metamethod: ";
		} else {
			context += "unknown lua error: ";
		}

		e_h(message.c_str(), context.c_str());

		return false;
	}

	return true;
}

bool lua_kernel_base::load_string(char const * prog, error_handler e_h)
{
	int errcode = luaL_loadstring(mState, prog);
	if (errcode != LUA_OK) {
		std::string msg = lua_tostring(mState, -1);

		std::string context = "When parsing a string to lua, ";

		if (errcode == LUA_ERRSYNTAX) {
			msg += " a syntax error: ";
		} else if(errcode == LUA_ERRMEM){
			msg += " a memory error: ";
		} else if(errcode == LUA_ERRGCMM) {
			msg += " an error in garbage collection metamethod: ";
		} else {
			msg += " an unknown error: ";
		}

		e_h(msg.c_str(), context.c_str());

		return false;
	}
	return true;
}

// Call load_string and protected call. Make them throw exceptions, and if we catch one, reformat it with signature for this function and log it.
void lua_kernel_base::run(const char * prog) {
	try {
		error_handler eh = boost::bind(&lua_kernel_base::throw_exception, this, _1, _2 );
		load_string(prog, eh);
		protected_call(0, 0, eh);
	} catch (game::lua_error & e) {
		lua_kernel_base::log_error(e.what(), "In function lua_kernel::run()");
	}
}


/**
 * Loads the "package" package into the Lua environment.
 * This action is inherently unsafe, as Lua scripts will now be able to
 * load C libraries on their own, hence granting them the same privileges
 * as the Wesnoth binary itsef.
 */
void lua_kernel_base::load_package()
{
	lua_State *L = mState;
	lua_pushcfunction(L, luaopen_package);
	lua_pushstring(L, "package");
	lua_call(L, 1, 0);
}
