/*
	Copyright (C) 2020 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "color_range.hpp"
#include "scripting/lua_color.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/push_check.hpp"
#include "log.hpp"
#include "game_config.hpp"

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char colorKey[] = "color range";

static bool luaW_iscolor(lua_State* L, int index)
{
	return luaL_testudata(L, index, colorKey) != nullptr;
}

static color_range& LuaW_checkcolor(lua_State *L, int index)
{
	if(!luaW_iscolor(L, index)) {
		luaW_type_error(L, index, "color");
		throw "luaW_type_error returned";
	}
	return *static_cast<color_range*>(lua_touserdata(L, index));
}


static color_range* luaW_pushcolor(lua_State *L, const color_range& color)
{
	color_range* res = new(L) color_range(color);
	luaL_setmetatable(L, colorKey);
	return res;
}

static int luaW_pushsinglecolor(lua_State *L, const color_t& color)
{
	lua_createtable(L, 0, 4);
	luaW_table_set(L, -1, "r", color.r);
	luaW_table_set(L, -1, "g", color.g);
	luaW_table_set(L, -1, "b", color.b);
	luaW_table_set(L, -1, "a", color.a);
	return 1;
}

static int impl_color_collect(lua_State *L)
{
	color_range *c = static_cast<color_range *>(lua_touserdata(L, 1));
	c->color_range::~color_range();
	return 0;
}

/**
 * Checks two color units for equality. (__eq metamethod)
 */
static int impl_color_equality(lua_State* L)
{
	color_range& left = LuaW_checkcolor(L, 1);
	color_range& right = LuaW_checkcolor(L, 2);
	const bool equal = left == right;
	lua_pushboolean(L, equal);
	return 1;
}

/**
 * Turns a lua color to string. (__tostring metamethod)
 */
static int impl_color_tostring(lua_State* L)
{
	color_range& c = LuaW_checkcolor(L, 1);
	//TODO: is this the best way to call tostring?
	lua_push(L, c.debug());
	return 1;
}


/**
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the color.
 * - Ret 1: color_range containing the color.
 */
static int impl_get_color(lua_State *L)
{
	std::string color_id = luaL_checkstring(L, 2);
	luaW_pushcolor(L, game_config::color_info(color_id));
	return 1;
}

static int impl_color_get(lua_State *L)
{
	color_range& c = LuaW_checkcolor(L, 1);
	char const *m = luaL_checkstring(L, 2);

	if(strcmp(m, "min") == 0) {
		return luaW_pushsinglecolor(L, c.min());
	}
	if(strcmp(m, "max") == 0) {
		return luaW_pushsinglecolor(L, c.max());
	}
	if(strcmp(m, "mid") == 0) {
		return luaW_pushsinglecolor(L, c.mid());
	}
	if(strcmp(m, "minimap") == 0) {
		return luaW_pushsinglecolor(L, c.rep());
	}
	// returns a string which can be used in Pango's foreground= attribute
	if(strcmp(m, "pango_color") == 0) {
		lua_push(L, c.mid().to_hex_string());
		return 1;
	}
	return 0;
}

static int impl_color_dir(lua_State* L)
{
	static const std::vector<std::string> keys{"min", "max", "mid", "minimap", "pango_color"};
	lua_push(L, keys);
	return 1;
}

static int impl_color_set(lua_State *L)
{
	return luaL_argerror(L, 2, "color objects canot be modified");
}

static int impl_colors_table_dir(lua_State* L)
{
	std::vector<std::string> all_colours;
	for(const auto& [key, value] : game_config::team_rgb_range) {
		if(std::all_of(key.begin(), key.end(), [](char c) {return isdigit(c);})) {
			// These colors are deprecated, don't show them
			continue;
		}
		all_colours.push_back(key);
	}
	lua_push(L, all_colours);
	return 1;
}

namespace lua_colors {
	std::string register_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the color metatable.
		cmd_out << "Adding color metatable...\n";

		luaL_newmetatable(L, colorKey);
		lua_pushcfunction(L, impl_color_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_color_equality);
		lua_setfield(L, -2, "__eq");
		lua_pushcfunction(L, impl_color_tostring);
		lua_setfield(L, -2, "__tostring");
		lua_pushcfunction(L, impl_color_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_color_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_color_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushstring(L, "color range");
		lua_setfield(L, -2, "__metatable");


		// Create the colors variable with its metatable.
		cmd_out << "Adding wesnoth.colors table...\n";

		lua_getglobal(L, "wesnoth");
		lua_newuserdatauv(L, 0, 0);
		lua_createtable(L, 0, 2);
		lua_pushcfunction(L, impl_get_color);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_colors_table_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushstring(L, "colors table");
		lua_setfield(L, -2, "__metatable");
		lua_pushboolean(L, true);
		lua_setfield(L, -2, "__dir_tablelike");
		lua_setmetatable(L, -2);
		lua_setfield(L, -2, "colors");
		lua_pop(L, 1);

		return cmd_out.str();
	}
}
