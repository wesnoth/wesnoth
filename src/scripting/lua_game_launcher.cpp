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
 * Provides a lua proxy for the game launcher object.
 *
 * @note Naming conventions:
 *   - intf_ functions are exported in the wesnoth domain,
 *   - impl_ functions are hidden inside metatables,
 *   - cfun_ functions are closures,
 *   - luaW_ functions are helpers in Lua style.
 *
 * This lua object is arranged as follows:
 *
 * - Type is an empty table
 * - metatable set to the gamelauncher metatable, which holds all of the callbacks,
 *   and the __newindex method. It is set to be it's own __index, so the callbacks are accessible.
 * - gamelauncher metatable has as its metatable a single "helper" metatable, whose __index is
 *   a C function defined below.
 *
 * The game_launcher object is not light user data and does not itself hold a pointer to the
 * underlying C object. There is a static pointer in this file that points to the most recent
 * game launcher. This slightly eases the syntax when using the lua gamelauncher object -- it's
 * never necessary to use a : when calling member functions, and still it allows us to have all of the
 * callbacks and metamethods have access to the pointer when they are called.
 */

#include "scripting/lua_game_launcher.hpp"

#include "global.hpp"

#include "commandline_options.hpp"
#include "game_launcher.hpp"
#include "log.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "scripting/application_lua_kernel.hpp"
#include "scripting/lua_api.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_types.hpp"

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

namespace lua_game_launcher {

static const char* metatable_name = "gamelauncher";

static game_launcher * gl_ = NULL;

/**
 * Get info from game launcher (_index metamethod)*
 * - Arg 1: table corresponding to gamelauncher.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_gamelauncher_get(lua_State* L) {
	if (!gl_) return 0;

	char const *m = luaL_checkstring(L, -1);

	return_string_attrib("version", game_config::version);

	return_cfgref_attrib("commandline_opts", gl_->opts().to_config());
	return 0;
}

static int impl_gamelauncher_set(lua_State* L) {
	ERR_LUA << "you cannot write to the fields of gamelauncher" << std::endl;
	lua_error(L);
	return 0;
}

/* Open a server connection */
static int intf_play_multiplayer(lua_State* /*L*/) {
	if (!gl_) return 0;
	gl_->play_multiplayer();
	return 0;
}

static void push_helper_metatable(lua_State* L)
{
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, impl_gamelauncher_get);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "game launcher helper");
	lua_setfield(L, -2, "__metatable");
}

void define_metatable(lua_State* L)
{
	// Create the gamelauncher metatable.
	luaL_newmetatable(L, metatable_name);

	//Put some callback functions in the scripting environment.
	static luaL_Reg const callbacks[] = {
		{ "play_multiplayer",           &intf_play_multiplayer},
		{ "set_script", 		&application_lua_kernel::intf_set_script},
		{ "__newindex",			&impl_gamelauncher_set},
		{ NULL, NULL }
	};
	luaL_setfuncs(L, callbacks, 0);

	lua_pushvalue(L, -1); //make a copy of this table, set it to be its own __index table
	lua_setfield(L, -2, "__index");

	push_helper_metatable(L);
	lua_setmetatable(L, -2);

	lua_pop(L,1);
}

/* Adds an instance of game launcher to the lua environment */
void add_table(lua_State* L, game_launcher * gl)
{
	gl_ = gl;

	//Set gl as the _pointer field
	lua_createtable(L, 0, 1);

	luaL_getmetatable(L,metatable_name);
	lua_setmetatable(L,-2);

	//if (!lua_islightuserdata(L, -1)) {
	//	ERR_LUA << "pointer was not light userdata! this indicates something wrong with lua api, pushlightuserdata or islightuserdata" << std::endl;
	//}

	lua_setglobal(L, "game_launcher");
}

} // end namespace lua_game_launcher

