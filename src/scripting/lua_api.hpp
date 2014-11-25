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
#include "unit_ptr.hpp"

struct lua_State;

/**
 * Converts a Lua value to a unit pointer.
 */
unit_ptr luaW_tounit(lua_State *L, int index, bool only_on_map = false);

/**
 * Displays a message in the chat window.
 */
void chat_message(std::string const &caption, std::string const &msg);

/**
 * Calls a Lua function stored below its @a nArgs arguments at the top of the stack.
 * @param nRets LUA_MULTRET for unbounded return values.
 * @return true if the call was successful and @a nRets return values are available.
 */
bool luaW_pcall(lua_State *L, int nArgs, int nRets, bool allow_wml_error = false);

/**
 * Converts a Lua value to a unit pointer.
 */
unit_ptr luaW_checkunit(lua_State *L, int index, bool only_on_map = false);

struct map_location;

/**
 * Storage for a unit, either owned by the Lua code (#ptr != 0), on a
 * recall list (#side != 0), or on the map. Shared units are represented
 * by their underlying ID (#uid).
 */
class lua_unit
{
	size_t uid;
	unit_ptr ptr;
	int side;
	lua_unit(lua_unit const &);

public:
	lua_unit(size_t u): uid(u), ptr(), side(0) {}
	lua_unit(unit_ptr u): uid(0), ptr(u), side(0) {}
	lua_unit(int s, size_t u): uid(u), ptr(), side(s) {}
	~lua_unit();
	bool on_map() const { return !ptr && side == 0; }
	int on_recall_list() const { return side; }
	unit_ptr get();

	// Clobbers loc
	bool put_map(const map_location &loc);
};

#endif
