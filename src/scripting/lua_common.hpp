/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
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
 * Common callbacks and functions to manipulate config, vconfig, tstring
 * in lua, and macros to get them from the stack.
 */

#ifndef LUA_COMMON_HPP_INCLUDED
#define LUA_COMMON_HPP_INCLUDED

struct lua_State;
class t_string;
class vconfig;

#include "config.hpp"
#include "scripting/lua_types.hpp"

namespace lua_common {
	int intf_textdomain(lua_State *L);
	int intf_tovconfig(lua_State* L);

	std::string register_gettext_metatable(lua_State *L);
	std::string register_tstring_metatable(lua_State *L);
	std::string register_vconfig_metatable(lua_State *L);

}
extern const char * tstringKey;

/**
 * Pushes a vconfig on the top of the stack.
 */
void luaW_pushvconfig(lua_State *L, vconfig const &cfg);

/**
 * Pushes a t_string on the top of the stack.
 */
void luaW_pushtstring(lua_State *L, t_string const &v);

/**
 * Converts a string into a Lua object pushed at the top of the stack.
 */
void luaW_pushscalar(lua_State *L, config::attribute_value const &v);


/**
 * Returns true if the metatable of the object is the one found in the registry.
 */
bool luaW_hasmetatable(lua_State *L, int index, luatypekey key);

/**
 * Converts a scalar to a translatable string.
 */
bool luaW_totstring(lua_State *L, int index, t_string &str);

/**
 * Converts a scalar to a translatable string.
 */
t_string luaW_checktstring(lua_State *L, int index);

/**
 * Converts a config object to a Lua table.
 * The destination table should be at the top of the stack on entry. It is
 * still at the top on exit.
 */
void luaW_filltable(lua_State *L, config const &cfg);

/**
 * Converts a config object to a Lua table pushed at the top of the stack.
 */
void luaW_pushconfig(lua_State *L, config const &cfg);

/**
 * Converts an optional table or vconfig to a config object.
 * @param index absolute stack position of t_string's metatable, or 0 if none.
 * @return false if some attributes had not the proper type.
 * @note If the table has holes in the integer keys or floating-point keys,
 *       some keys will be ignored and the error will go undetected.
 */
bool luaW_toconfig(lua_State *L, int index, config &cfg);

/**
 * Converts an optional table or vconfig to a config object.
 */
config luaW_checkconfig(lua_State *L, int index);

/**
 * Gets an optional vconfig from either a table or a userdata.
 * @return false in case of failure.
 */
bool luaW_tovconfig(lua_State *L, int index, vconfig &vcfg);

/**
 * Gets an optional vconfig from either a table or a userdata.
 * @param allow_missing true if missing values are allowed; the function
 *        then returns an unconstructed vconfig.
 */
vconfig luaW_checkvconfig(lua_State *L, int index, bool allow_missing = false);

/**
 * Pushes the value found by following the variadic names (char *), if the
 * value is not nil.
 * @return true if an element was pushed.
 */
bool luaW_getglobal(lua_State *L, ...);

bool luaW_toboolean(lua_State *L, int n);

#define return_tstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		luaW_pushtstring(L, accessor); \
		return 1; \
	}

#define return_cstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushstring(L, accessor); \
		return 1; \
	}

#define return_string_attrib(name, accessor) \
	return_cstring_attrib(name, accessor.c_str())

#define return_int_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushinteger(L, accessor); \
		return 1; \
	}

#define return_float_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushnumber(L, accessor); \
		return 1; \
	}

#define return_bool_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		lua_pushboolean(L, accessor); \
		return 1; \
	}

#define return_cfg_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		config cfg; \
		accessor; \
		luaW_pushconfig(L, cfg); \
		return 1; \
	}

#define return_cfgref_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		luaW_pushconfig(L, accessor); \
		return 1; \
	}

#define return_vector_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		const std::vector<std::string>& vector = accessor; \
		lua_createtable(L, vector.size(), 0); \
		int i = 1; \
		BOOST_FOREACH(const std::string& s, vector) { \
			lua_pushstring(L, s.c_str()); \
			lua_rawseti(L, -2, i); \
			++i; \
		} \
		return 1; \
	}

#define modify_tstring_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		t_string value = luaW_checktstring(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		const char *value = luaL_checkstring(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_int_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		int value = luaL_checkinteger(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_int_attrib_check_range(name, accessor, allowed_min, allowed_max) \
	if (strcmp(m, name) == 0) { \
		int value = luaL_checkinteger(L, 3); \
		if (value < allowed_min || allowed_max < value) return luaL_argerror(L, 3, "out of bounds"); \
		accessor; \
		return 0; \
	}

#define modify_bool_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		bool value = luaW_toboolean(L, 3); \
		accessor; \
		return 0; \
	}

#define modify_vector_string_attrib(name, accessor) \
	if (strcmp(m, name) == 0) { \
		std::vector<std::string> vector; \
		char const* message = "table with unnamed indices holding strings expected"; \
		if (!lua_istable(L, 3)) return luaL_argerror(L, 3, message); \
		unsigned length = lua_rawlen(L, 3); \
		for (unsigned i = 1; i <= length; ++i) { \
			lua_rawgeti(L, 3, i); \
			char const* string = lua_tostring(L, 4); \
			if(!string) return luaL_argerror(L, 2 + i, message); \
			vector.push_back(string); \
			lua_pop(L, 1); \
		} \
		accessor; \
		return 0; \
	}

#endif
