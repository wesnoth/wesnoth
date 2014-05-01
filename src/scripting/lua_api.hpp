/*
   Copyright (C) 2009 - 2014 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCRIPTING_LUA_API_HPP
#define SCRIPTING_LUA_API_HPP

#include <cstddef>
#include <string>
#include "config.hpp" // forward declaration of the nested type config::attribute_value is not possible
#include "lua_types.hpp" // the luatype typedef
struct lua_State;
class t_string;
class vconfig;
class unit;


/**
 * Converts a Lua value to a unit pointer.
 */
unit *luaW_tounit(lua_State *L, int index, bool only_on_map = false);

/**
 * Displays a message in the chat window.
 */
void chat_message(std::string const &caption, std::string const &msg);

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
 * @param tstring_meta absolute stack position of t_string's metatable, or 0 if none.
 * @return false if some attributes had not the proper type.
 * @note If the table has holes in the integer keys or floating-point keys,
 *       some keys will be ignored and the error will go undetected.
 */
bool luaW_toconfig(lua_State *L, int index, config &cfg, int tstring_meta = 0);

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
 * Calls a Lua function stored below its @a nArgs arguments at the top of the stack.
 * @param nRets LUA_MULTRET for unbounded return values.
 * @return true if the call was successful and @a nRets return values are available.
 */
bool luaW_pcall(lua_State *L, int nArgs, int nRets, bool allow_wml_error = false);


/**
 * Pushes the value found by following the variadic names (char *), if the
 * value is not nil.
 * @return true if an element was pushed.
 */
bool luaW_getglobal(lua_State *L, ...);

/**
 * Converts a Lua value to a unit pointer.
 */
unit *luaW_checkunit(lua_State *L, int index, bool only_on_map = false);

bool luaW_toboolean(lua_State *L, int n);

struct map_location;

/**
 * Storage for a unit, either owned by the Lua code (#ptr != 0), on a
 * recall list (#side != 0), or on the map. Shared units are represented
 * by their underlying ID (#uid).
 */
class lua_unit
{
	size_t uid;
	unit *ptr;
	int side;
	lua_unit(lua_unit const &);

public:
	lua_unit(size_t u): uid(u), ptr(NULL), side(0) {}
	lua_unit(unit *u): uid(0), ptr(u), side(0) {}
	lua_unit(int s, size_t u): uid(u), ptr(NULL), side(s) {}
	~lua_unit();
	bool on_map() const { return !ptr && side == 0; }
	int on_recall_list() const { return side; }
	unit *get();

	// Clobbers loc
	bool put_map(const map_location &loc);
};

#endif
