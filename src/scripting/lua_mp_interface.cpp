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
 * Provides a lua proxy table for the game launcher object.
 *
 * @note Naming conventions:
 *   - intf_ functions are exported in the wesnoth domain,
 *   - impl_ functions are hidden inside metatables,
 *   - cfun_ functions are closures,
 *   - luaW_ functions are helpers in Lua style.
 */

#include "scripting/lua_mp_interface.hpp"

#include "global.hpp"

#include "config.hpp"
#include "log.hpp"
#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "game_initialization/multiplayer_ui.hpp"
#include "resources.hpp"
#include "scripting/application_lua_kernel.hpp"
#include "scripting/lua_api.hpp"
#include "scripting/lua_common.hpp"

#include <string>

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char * metatable_name = "mp";

static mp::ui* screen()
{
	if (!resources::lua_mp) return NULL;
	return resources::lua_mp->current_screen();
}

const char * modes[] = { "NONE", "LOBBY" }; //, "CONNECT", "WAIT", "GAME" };

static const char * current_mode()
{
	if (screen() == NULL) {
		return modes[0];
	}
	return modes[1];
}

/**
 * Get info from multiplayer (_index metamethod)*
 * - Arg 1: table corresponding to multiplayer interface.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
int lua_mp_interface::impl_get(lua_State* L)
{
	char const *m = luaL_checkstring(L, -1);

	// If we got here, we need to dereference the multiplayer pointer.
	if (!resources::lua_mp) return 0;

	mp::ui * s = screen();
	return_cstring_attrib("mode", current_mode());
	return_cfgref_attrib("gamelist", s ? s->gamelist() : config());
	return_cfgref_attrib("game_config", s ? s->game_config() : config());
	return 0;
}

/**
 * Set fields of multiplayer (_newindex metamethod)
 */
int lua_mp_interface::impl_set(lua_State* L)
{
	ERR_LUA << "you cannot write to the fields of multiplayer" << std::endl;
	lua_error(L);
	return 0;
}

/**
 * Send a chat message from this client, in whatever context we are currently in.
 */
int lua_mp_interface::intf_send_chat(lua_State* L)
{
	if (!resources::lua_mp) return 0;
	if (!screen()) return 0;

	char const *m = luaL_checkstring(L, 2);

	config ret;
	ret.add_child("chat");
	ret.child("chat")["message"] = m;

	resources::lua_mp->push_request(ret);
	return 0;
}

/**
 * Exit the current mp screen
 */
int lua_mp_interface::intf_exit(lua_State* /*L*/)
{
	if (!resources::lua_mp) return 0;
	if (!screen()) return 0;

	config ret;
	ret.add_child("exit");

	resources::lua_mp->push_request(ret);
	return 0;
}

/**
 * Get the next request from front of request queue, if there is one.
 */
/*
const boost::optional<config> lua_mp_interface::next_request()
{
	boost::optional<config> ret;

	if (requests.size() > 0) {
		ret = requests.pop_front();
	}

	return ret;
}*/

/**
 * Notify lua that there is a new screen, and update the records in this object
 */
void lua_mp_interface::notify_new_screen(mp::ui* screen)
{
	if (!resources::app_lua_kernel) return;

	if (screen != screens.top()) screens.push(screen);

	config notification;
	config & c = notification.add_child("multiplayer");
	if (screen != NULL) {
		c.add_child("lobby"); // for now assume it is the lobby since there are no functions that permit us to leave it yet, besides exit
	}

	resources::app_lua_kernel->call_script(notification);
}

static void push_helper_metatable(lua_State* L)
{
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, lua_mp_interface::impl_get);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "multiplayer helper");
	lua_setfield(L, -2, "__metatable");
}

void lua_mp_interface::define_metatable(lua_State* L)
{
	// Create the gamelauncher metatable.
	luaL_newmetatable(L, metatable_name);

	//Put some callback functions in the scripting environment.
	static luaL_Reg const callbacks[] = {
		{ "send_chat",		&lua_mp_interface::intf_send_chat},
		{ "exit",		&lua_mp_interface::intf_exit},
		{ "__newindex",		&lua_mp_interface::impl_set},
		{ NULL, NULL }
	};
	luaL_setfuncs(L, callbacks, 0);

	lua_pushvalue(L, -1); //make a copy of this table, set it to be its own __index table
	lua_setfield(L, -2, "__index");

	push_helper_metatable(L);
	lua_setmetatable(L, -2);

	lua_pop(L,1);
}

/* Adds an instance of multiplayer proxy object to the lua environment */
void lua_mp_interface::add_table(lua_State* L)
{
	if (resources::lua_mp && resources::lua_mp != this) {
		WRN_LUA << "overwriting lua_mp pointer";
	}

	resources::lua_mp = this;

	lua_createtable(L, 0, 1);

	luaL_getmetatable(L,metatable_name);
	lua_setmetatable(L,-2);

	lua_setglobal(L, "multiplayer");
}
