/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
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

#pragma once

struct lua_State;
class t_string;
class vconfig;

#include "config.hpp"
#include "variable_info.hpp"
#include "map/location.hpp"
#include <vector>
#include <string>

namespace lua_common {
	int intf_textdomain(lua_State *L);
	int intf_tovconfig(lua_State* L);

	std::string register_gettext_metatable(lua_State *L);
	std::string register_tstring_metatable(lua_State *L);
	std::string register_vconfig_metatable(lua_State *L);

}

void* operator new(size_t sz, lua_State *L);
void operator delete(void* p, lua_State *L);

/**
 * Like luaL_getmetafield, but returns false if key is an empty string
 * or begins with two underscores.
 */
bool luaW_getmetafield(lua_State *L, int idx, const char* key);

/**
 * Pushes a vconfig on the top of the stack.
 */
void luaW_pushvconfig(lua_State *L, const vconfig& cfg);

/**
 * Pushes a t_string on the top of the stack.
 */
void luaW_pushtstring(lua_State *L, const t_string& v);

/**
 * Converts an attribute value into a Lua object pushed at the top of the stack.
 */
void luaW_pushscalar(lua_State *L, const config::attribute_value& v);

/**
 * Converts the value at the top of the stack to an attribute value
 */
bool luaW_toscalar(lua_State *L, int index, config::attribute_value& v);

/**
 * Converts a scalar to a translatable string.
 */
bool luaW_totstring(lua_State *L, int index, t_string &str);

/**
 * Converts a scalar to a translatable string.
 */
t_string luaW_checktstring(lua_State *L, int index);

/*
 * Test if a scalar is either a plain or translatable string.
 * Also returns true if it's a number since that's convertible to string.
 */
bool luaW_isstring(lua_State* L, int index);

/**
 * Converts a config object to a Lua table.
 * The destination table should be at the top of the stack on entry. It is
 * still at the top on exit.
 */
void luaW_filltable(lua_State *L, const config& cfg);

/**
 * Converts a map location object to a Lua table pushed at the top of the stack.
 */
void luaW_pushlocation(lua_State *L, const map_location& loc);

/**
 * Converts an optional table or pair of integers to a map location object.
 * @param index stack position of the table or first integer.
 * @return false if a map location couldn't be matched.
 */
bool luaW_tolocation(lua_State *L, int index, map_location &loc);

/**
 * Converts an optional table or pair of integers to a map location object.
 * @note If a pair of integers was found, the first one will be removed
 *       from the stack when the function returns.
 */
map_location luaW_checklocation(lua_State *L, int index);

/**
 * Converts a config object to a Lua table pushed at the top of the stack.
 */
void luaW_pushconfig(lua_State *L, const config& cfg);

/**
 * Converts an optional table or vconfig to a config object.
 * @param index stack position of the table.
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
 * Like the two-argument version, but if it was a vconfig, also
 * returns a pointer to that vconfig.
 */
config luaW_checkconfig(lua_State *L, int index, const vconfig*& vcfg);

/**
 * Pushes the value found by following the variadic names (char *), if the
 * value is not nil.
 * @return true if an element was pushed.
 */
bool luaW_getglobal(lua_State *L, const std::vector<std::string>& path);

/**
 * Pushes the value found by following the variadic names (char *), if the
 * value is not nil.
 * @return true if an element was pushed.
 */
template<typename... T>
bool luaW_getglobal(lua_State *L, T... path) {
	return luaW_getglobal(L, std::vector<std::string> {path...} );
}

bool luaW_toboolean(lua_State *L, int n);


bool luaW_pushvariable(lua_State *L, variable_access_const& v);

bool luaW_checkvariable(lua_State *L, variable_access_create& v, int n);

/**
 * Displays a message in the chat window.
 */
void chat_message(const std::string& caption, const std::string& msg);

/**
 * Calls a Lua function stored below its @a nArgs arguments at the top of the stack.
 * @param nRets LUA_MULTRET for unbounded return values.
 * @return true if the call was successful and @a nRets return values are available.
 */
bool luaW_pcall(lua_State *L, int nArgs, int nRets, bool allow_wml_error = false);

// Don't use these directly
void push_error_handler(lua_State *L);
int luaW_pcall_internal(lua_State *L, int nArgs, int nRets);

int luaW_type_error(lua_State *L, int narg, const char *tname);
int luaW_type_error(lua_State *L, int narg, const char* kpath, const char *tname);

#define return_tstring_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		luaW_pushtstring(L, (accessor)); \
		return 1; \
	} \
} while(false)

#define return_cstring_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		lua_pushstring(L, (accessor)); \
		return 1; \
	} \
} while(false)

#define return_string_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		const std::string& str = (accessor); \
		lua_pushlstring(L, str.c_str(), str.length()); \
		return 1; \
	} \
} while(false)

#define return_int_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		lua_pushinteger(L, (accessor)); \
		return 1; \
	} \
} while(false)

#define return_float_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		lua_pushnumber(L, (accessor)); \
		return 1; \
	} \
} while(false)

#define return_bool_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		lua_pushboolean(L, (accessor)); \
		return 1; \
	} \
} while(false)

#define return_cfg_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		config cfg; \
		{accessor;} \
		luaW_pushconfig(L, cfg); \
		return 1; \
	} \
} while(false)

#define return_cfgref_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		luaW_pushconfig(L, (accessor)); \
		return 1; \
	} \
} while(false)

#define return_vector_string_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		const std::vector<std::string>& vector = (accessor); \
		lua_createtable(L, vector.size(), 0); \
		int i = 1; \
		for (const std::string& s : vector) { \
			lua_pushlstring(L, s.c_str(), s.length()); \
			lua_rawseti(L, -2, i); \
			++i; \
		} \
		return 1; \
	} \
} while(false)

#define modify_tstring_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		t_string value = luaW_checktstring(L, 3); \
		{accessor;} \
		return 0; \
	} \
} while(false)

#define modify_string_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		const char *value = luaL_checkstring(L, 3); \
		{accessor;} \
		return 0; \
	} \
} while(false)

#define modify_int_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		int value = static_cast<int>(luaL_checknumber(L, 3)); \
		{accessor;} \
		return 0; \
	} \
} while(false)

#define modify_int_attrib_check_range(name, accessor, allowed_min, allowed_max) \
do { \
	if (strcmp(m, (name)) == 0) { \
		int value = static_cast<int>(luaL_checknumber(L, 3)); \
		if (value < (allowed_min) || (allowed_max) < value) return luaL_argerror(L, 3, "out of bounds"); \
		{accessor;} \
		return 0; \
	} \
} while(false)

#define modify_float_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		lua_Number value = luaL_checknumber(L, 3); \
		{accessor;} \
		return 0; \
	} \
} while(false)

#define modify_float_attrib_check_range(name, accessor, allowed_min, allowed_max) \
do { \
	if (strcmp(m, (name)) == 0) { \
		lua_Number value = luaL_checknumber(L, 3); \
		if (value < (allowed_min) || (allowed_max) < value) return luaL_argerror(L, 3, "out of bounds"); \
		{accessor;} \
		return 0; \
	} \
} while(false)

#define modify_bool_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		bool value = luaW_toboolean(L, 3); \
		{accessor;} \
		return 0; \
	} \
} while(false)

#define modify_vector_string_attrib(name, accessor) \
do { \
	if (strcmp(m, (name)) == 0) { \
		std::vector<std::string> value; \
		char const* message = "table with unnamed indices holding strings expected"; \
		if (!lua_istable(L, 3)) return luaL_argerror(L, 3, message); \
		unsigned length = lua_rawlen(L, 3); \
		for (unsigned i = 1; i <= length; ++i) { \
			lua_rawgeti(L, 3, i); \
			char const* string = lua_tostring(L, 4); \
			if(!string) return luaL_argerror(L, 2 + i, message); \
			value.push_back(string); \
			lua_pop(L, 1); \
		} \
		{accessor;} \
		return 0; \
	} \
} while(false)
