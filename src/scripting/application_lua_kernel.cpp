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

/**
 * @file
 * Provides a Lua interpreter, to drive the game_controller.
 *
 * @note Naming conventions:
 *   - intf_ functions are exported in the wesnoth domain,
 *   - impl_ functions are hidden inside metatables,
 *   - cfun_ functions are closures,
 *   - luaW_ functions are helpers in Lua style.
 */

#include "scripting/application_lua_kernel.hpp"

#include "global.hpp"

#include "log.hpp"
#include "lua/lua.h"
#include "scripting/lua_api.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_game_launcher.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_types.hpp"

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

#include <ostream>
#include <string>


static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

application_lua_kernel::application_lua_kernel()
 : lua_kernel_base()
{}

bool application_lua_kernel::initialize(game_launcher * gl)
{

	cmd_log_ << "Adding game_launcher...\n"; 

	//if (resources::app_lua_kernel && resources::app_lua_kernel != this) {
	//	throw "you appear to have multiple application lua kernels, this is bad";
	//}

	lua_State * L = mState;

	lua_game_launcher::define_metatable(L);
	lua_game_launcher::add_table(L, gl);

	return true;
}

/**
 * Sets the current lua script for the wesnoth application
 * - Arg 1: A lua function. This should be a closure, essentially an input iterator
 *   which takes as input tables corresponding to configs. Whenever an event occurs
 *   this function will be called with the config of that event passed as its argument.
 */
int application_lua_kernel::intf_set_script(lua_State *L) {
	lua_pushlightuserdata(L	, currentscriptKey); // stack is now [fcn], [key]
	lua_insert(L,-2); // stack is now [key], [fcn]
	lua_rawset(L, LUA_REGISTRYINDEX); // pair is stored in registry
	return 0;
}

/**
 * Calls the current script, with given config translated to a table and passed as arg.
 */
void application_lua_kernel::call_script(const config & event_cfg) {
	lua_State * L = mState;

	lua_pushlightuserdata(L	, currentscriptKey);
	lua_rawget(L, LUA_REGISTRYINDEX); //get the script from the registry, on the top of the stack

	if (lua_type(L, -1) != LUA_TFUNCTION) {
		WRN_LUA << "Tried to execute script from registry, but did not retrieve a function. Aborting." << std::endl;
		return ;
	}

	luaW_pushconfig(L, event_cfg); //push the config as an argument

	if (!luaW_pcall(L, 1, 0)) //call the script from protected mode, there is one argument and we expect no return values.
	{
		WRN_LUA << "Got an error when executing script:\n" << lua_tostring(L,-1) << std::endl;
	}
}
