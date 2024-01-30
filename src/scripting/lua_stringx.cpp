/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_stringx.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/push_check.hpp"

#include "formula/string_utils.hpp"
#include "variable.hpp" // for config_variable_set

#include <boost/algorithm/string/trim.hpp>

#include "lua/wrapper_lauxlib.h"

namespace lua_stringx {

/**
* Formats a message by interpolating WML variable syntax
* Arg 1: (optional) Logger
* Arg 2: Message
*/
static int intf_format(lua_State* L)
{
	config cfg = luaW_checkconfig(L, 2);
	config_variable_set variables(cfg);
	if(lua_isstring(L, 1)) {
		std::string str = lua_tostring(L, 1);
		lua_push(L, utils::interpolate_variables_into_string(str, variables));
		return 1;
	}
	t_string str = luaW_checktstring(L, 1);
	lua_push(L, utils::interpolate_variables_into_tstring(str, variables));
	return 1;
}

/**
* Formats a list into human-readable format
* Arg 1: default value, used if the list is empty
* Arg 2: list of strings
*/
template<bool conjunct>
static int intf_format_list(lua_State* L)
{
	const t_string empty = luaW_checktstring(L, 1);
	auto values = lua_check<std::vector<t_string>>(L, 2);
	lua_push(L, (conjunct ? utils::format_conjunct_list : utils::format_disjunct_list)(empty, values));
	return 1;
}

/**
* Enables indexing a string by an integer, while also treating the stringx module as its metatable.__index
*/
static int impl_str_index(lua_State* L)
{
	if(lua_type(L, 2) == LUA_TSTRING) {
		// return stringx[key]
		lua_getglobal(L, "stringx");
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
		return 1;
	} else if(lua_type(L, 2) == LUA_TNUMBER) {
		// get the string length and the index
		int len = lua_rawlen(L, 1);
		int i = luaL_checkinteger(L, 2);
		// In order to not break ipairs, an out-of-bounds access needs to return nil
		if(i == 0 || abs(i) > len) {
			lua_pushnil(L);
			return 1;
		}
		// return string.sub(str, key, key)
		luaW_getglobal(L, "string", "sub");
		lua_pushvalue(L, 1);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 2);
		lua_call(L, 3, 1);
		return 1;
	}
	return 0;
}

/**
* Splits a string into parts according to options
* Arg 1: String to split
* Arg 2: Separator
* Arg 3: Options table
*/
static int intf_str_split(lua_State* L)
{
	enum {BASIC, ESCAPED, PAREN, ANIM} type = BASIC;
	const std::string& str = luaL_checkstring(L, 1);
	const std::string& sep = luaL_optstring(L, 2, ",");
	std::string left, right;
	int flags = utils::REMOVE_EMPTY | utils::STRIP_SPACES;
	if(lua_istable(L, 3)) {
		flags = 0;
		if(luaW_table_get_def(L, 3, "remove_empty", true)) {
			flags |= utils::REMOVE_EMPTY;
		}
		if(luaW_table_get_def(L, 3, "strip_spaces", true)) {
			flags |= utils::STRIP_SPACES;
		}
		bool anim = luaW_table_get_def(L, 3, "expand_anim", false);
		if(luaW_tableget(L, 3, "escape")) {
			if(anim) {
				return luaL_error(L, "escape and expand_anim options are incompatible!");
			}
			type = ESCAPED;
			left = luaL_checkstring(L, -1);
			if(left.size() != 1) {
				return luaL_error(L, "escape must be a single character");
			}
		} else if(luaW_tableget(L, 3, "quote")) {
			left = right = luaL_checkstring(L, -1);
			if(anim) {
				type = ANIM;
				left.push_back('[');
				right.push_back(']');
			} else type = PAREN;
		} else if(luaW_tableget(L, 3, "quote_left") && luaW_tableget(L, 3, "quote_right")) {
			left = luaL_checkstring(L, -2);
			right = luaL_checkstring(L, -1);
			if(anim) {
				if(left.find_first_of("[]") != std::string::npos || right.find_first_of("[]") != std::string::npos) {
					return luaL_error(L, "left and right cannot include square brackets [] if expand_anim is enabled");
				}
				type = ANIM;
				left.push_back('[');
				right.push_back(']');
			} else type = PAREN;
		} else if(anim) {
			type = ANIM;
			left = "([";
			right = ")]";
		}
		if(type != ESCAPED && left.size() != right.size()) {
			return luaL_error(L, "left and right need to be strings of the same length");
		}
	}
	switch(type) {
		case BASIC:
			lua_push(L, utils::split(str, sep[0], flags));
			break;
		case ESCAPED:
			lua_push(L, utils::quoted_split(str, sep[0], flags, left[0]));
			break;
		case PAREN:
			lua_push(L, utils::parenthetical_split(str, sep[0], left, right, flags));
			break;
		case ANIM:
			lua_push(L, utils::square_parenthetical_split(str, sep[0], left, right, flags));
			break;
	}
	return 1;
}

/**
* Splits a string into parenthesized portions and portions between parenthesized portions
* Arg 1: String to split
* Arg 2: Possible left parentheses
* Arg 3: Matching right parentheses
*/
static int intf_str_paren_split(lua_State* L)
{
	const std::string& str = luaL_checkstring(L, 1);
	const std::string& left = luaL_optstring(L, 2, "(");
	const std::string& right = luaL_optstring(L, 3, ")");
	if(left.size() != right.size()) {
		return luaL_error(L, "left and right need to be strings of the same length");
	}
	bool strip_spaces = luaL_opt(L, luaW_toboolean, 4, true);
	lua_push(L, utils::parenthetical_split(str, 0, left, right, strip_spaces ? utils::STRIP_SPACES : 0));
	return 1;
}

/**
* Splits a string into a map
* Arg 1: string to split
* Arg 2: Separator for items
* Arg 3: Separator for key and value
*/
static int intf_str_map_split(lua_State* L)
{
	const std::string& str = luaL_checkstring(L, 1);
	const std::string& sep = luaL_optstring(L, 2, ",");
	const std::string& kv = luaL_optstring(L, 3, ":");
	std::string dflt;
	if(sep.size() != 1) {
		return luaL_error(L, "separator must be a single character");
	}
	if(kv.size() != 1) {
		return luaL_error(L, "key_value_separator must be a single character");
	}
	int flags = utils::REMOVE_EMPTY | utils::STRIP_SPACES;
	if(lua_istable(L, 4)) {
		flags = 0;
		if(luaW_table_get_def(L, 4, "remove_empty", true)) {
			flags |= utils::REMOVE_EMPTY;
		}
		if(luaW_table_get_def(L, 4, "strip_spaces", true)) {
			flags |= utils::STRIP_SPACES;
		}
		if(luaW_tableget(L, 4, "default")) {
			dflt = luaL_checkstring(L, -1);
		}
	}
	lua_push(L, utils::map_split(str, sep[0], kv[0], flags, dflt));
	return 1;
}

/**
* Joins a list into a string; calls __tostring and __index metamethods
* Arg 1: list to join
* Arg 2: separator
* (arguments can be swapped)
*/
static int intf_str_join(lua_State* L) {
	// Support both join(list, [sep]) and join(sep, list)
	// The latter form means sep:join(list) also works.
	std::string sep;
	int list_idx;
	if(lua_istable(L, 1)) {
		list_idx = 1;
		sep = luaL_optstring(L, 2, ",");
	} else if(lua_istable(L, 2)) {
		sep = luaL_checkstring(L, 1);
		list_idx = 2;
	} else return luaL_error(L, "invalid arguments to join, should have map and separator");
	std::vector<std::string> pieces;
	for(int i = 1; i <= luaL_len(L, list_idx); i++) {
		lua_getglobal(L, "tostring");
		lua_geti(L, list_idx, i);
		lua_call(L, 1, 1);
		pieces.push_back(luaL_checkstring(L, -1));
	}
	lua_push(L, utils::join(pieces, sep));
	return 1;
}

/**
* Joins a map into a string; calls __tostring metamethods (on both key and value) but not __index
* Arg 1: list to join
* Arg 2: separator for items
* Arg 3: separator for key and value
* (list argument can be swapped to any position)
*/
static int intf_str_join_map(lua_State* L) {
	// Support join_map(map, [sep], [kv_sep]), join_map(sep, map, [kv_sep]), and join_map(sep, kv_sep, map)
	// The latter forms mean sep:join_map(kv_sep, map) and sep:join_map(map) also work.
	// If only one separator is given in the first form, it will be sep, not kv_sep
	std::string sep, kv;
	int map_idx;
	if(lua_istable(L, 1)) {
		map_idx = 1;
		sep = luaL_optstring(L, 2, ",");
		kv = luaL_optstring(L, 3, ":");
	} else if(lua_istable(L, 2)) {
		sep = luaL_checkstring(L, 1);
		map_idx = 2;
		kv = luaL_optstring(L, 3, ":");
	} else if(lua_istable(L, 3)) {
		sep = luaL_checkstring(L, 1);
		kv = luaL_checkstring(L, 2);
		map_idx = 3;
	} else return luaL_error(L, "invalid arguments to join_map, should have map, separator, and key_value_separator");
	std::map<std::string, std::string> pieces;
	for(lua_pushnil(L); lua_next(L, map_idx); /*pop in loop body*/) {
		int key_idx = lua_absindex(L, -2), val_idx = lua_absindex(L, -1);
		lua_getglobal(L, "tostring");
		lua_pushvalue(L, key_idx);
		lua_call(L, 1, 1);
		std::string& val = pieces[luaL_checkstring(L, -1)];
		lua_getglobal(L, "tostring");
		lua_pushvalue(L, val_idx);
		lua_call(L, 1, 1);
		val = luaL_checkstring(L, -1);
		lua_settop(L, key_idx);
	}
	lua_push(L, utils::join_map(pieces, sep, kv));
	return 1;
}

/**
 * Trims whitespace from the beginning and end of a string
 */
static int intf_str_trim(lua_State* L)
{
	std::string str = luaL_checkstring(L, 1);
	boost::trim(str);
	lua_pushlstring(L, str.c_str(), str.size());
	return 1;
}

// Override string.format to coerce the format to a string
static int intf_str_format(lua_State* L)
{
	int nargs = lua_gettop(L);
	if(luaW_iststring(L, 1)) {
		// get the tostring() function and call it on the first argument
		lua_getglobal(L, "tostring");
		lua_pushvalue(L, 1);
		lua_call(L, 1, 1);
		// replace the first argument with the coerced value
		lua_replace(L, 1);
	}
	// grab the original string.format function from the closure...
	lua_pushvalue(L, lua_upvalueindex(1));
	// ...move it to the bottom of the stack...
	lua_insert(L, 1);
	// ...and finally pass along all the arguments to it.
	lua_call(L, nargs, 1);
	return 1;
}

/**
 * Parses a range string of the form a-b into an interval pair
 * Accepts the string "infinity" as representing a Very Large Number
 * Arg 2: (optional) If true, parse as real numbers instead of integers
 */
static int intf_parse_range(lua_State* L)
{
	const std::string str = luaL_checkstring(L, 1);
	if(luaL_opt(L, lua_toboolean, 2, false)) {
		auto interval = utils::parse_range_real(str);
		lua_pushnumber(L, interval.first);
		lua_pushnumber(L, interval.second);
	} else {
		auto interval = utils::parse_range(str);
		lua_pushinteger(L, interval.first);
		lua_pushinteger(L, interval.second);
	}
	return 2;
}

int luaW_open(lua_State* L) {
	auto& lk = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
	lk.add_log("Adding stringx module...\n");
	static luaL_Reg const str_callbacks[] = {
		{ "split",               &intf_str_split },
		{ "parenthetical_split", &intf_str_paren_split },
		{ "map_split",           &intf_str_map_split },
		{ "join",                &intf_str_join },
		{ "join_map",            &intf_str_join_map },
		{ "trim",                &intf_str_trim },
		{ "parse_range",         &intf_parse_range },
		{ "vformat",                  &intf_format                   },
		{ "format_conjunct_list",     &intf_format_list<true>        },
		{ "format_disjunct_list",     &intf_format_list<false>       },
		{ nullptr, nullptr },
	};
	lua_newtable(L);
	luaL_setfuncs(L, str_callbacks, 0);
	// Set the stringx metatable to index the string module
	lua_createtable(L, 0, 1);
	lua_getglobal(L, "string");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	// Set the metatable of strings to index the stringx module instead of the string module
	lua_pushliteral(L, "");
	lua_getmetatable(L, -1);
	lua_pushcfunction(L, &impl_str_index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

	// Override string.format so it can accept a t_string
	lua_getglobal(L, "string");
	lua_getfield(L, -1, "format");
	lua_pushcclosure(L, &intf_str_format, 1);
	lua_setfield(L, -2, "format");
	lua_pop(L, 1);
	return 1;
}

}
