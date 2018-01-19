/*
   Copyright (C) 2009 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <cstddef>
#include <string>
#include "scripting/lua_common.hpp"
#include "units/ptr.hpp"

struct lua_State;
class lua_unit;
struct map_location;

/**
 * Test if a Lua value is a unit
 */
bool luaW_isunit(lua_State *, int index);

/**
 * Converts a Lua value to a unit pointer.
 */
unit* luaW_tounit(lua_State *L, int index, bool only_on_map = false);

/**
 * Converts a Lua value to a unit pointer.
 */
unit& luaW_checkunit(lua_State *L, int index, bool only_on_map = false);

/**
 * Pushes a private unit on the stack.
 */
lua_unit* luaW_pushlocalunit(lua_State *L, unit& u);

/**
 * Similar to luaW_tounit but returns a unit_ptr; use this instead of
 * luaW_tounit when using an api that needs unit_ptr.
 */
unit_ptr luaW_tounit_ptr(lua_State *L, int index, bool only_on_map);

/**
 * Similar to luaW_checkunit but returns a unit_ptr; use this instead of
 * luaW_checkunit when using an api that needs unit_ptr.
 */
unit_ptr luaW_checkunit_ptr(lua_State *L, int index, bool only_on_map);

/**
 * Similar to luaW_tounit but returns a lua_unit; use this if you need
 * to handle map and recall units differently, for example.
 *
 * Note that this only returns null if the element on the stack was not a unit,
 * so it may be an invalid unit.
 */
lua_unit* luaW_tounit_ref(lua_State *L, int index);

/**
 * Similar to luaW_checkunit but returns a lua_unit; use this if you need
 * to handle map and recall units differently, for example.
 */
lua_unit* luaW_checkunit_ref(lua_State *L, int index);


/**
 * Storage for a unit, either owned by the Lua code (#ptr != 0), a
 * local variable unit (c_ptr != 0), on a recall list (#side != 0), or on the map.
 * Shared units are represented by their underlying ID (#uid).
 */
class lua_unit
{
	size_t uid;
	unit_ptr ptr;
	int side;
	unit* c_ptr;
	lua_unit(lua_unit const &) = delete;
	lua_unit& operator=(const lua_unit&) = delete;

	template<typename... Args>
	friend lua_unit* luaW_pushunit(lua_State *L, Args... args);
	friend lua_unit* luaW_pushlocalunit(lua_State *L, unit& u);
	static void setmetatable(lua_State *L);
public:
	lua_unit(size_t u): uid(u), ptr(), side(0), c_ptr() {}
	lua_unit(unit_ptr u): uid(0), ptr(u), side(0), c_ptr() {}
	lua_unit(int s, size_t u): uid(u), ptr(), side(s), c_ptr() {}
	lua_unit(unit& u): uid(0), ptr(), side(0), c_ptr(&u) {}
	~lua_unit();

	bool on_map() const { return !ptr && side == 0; }
	int on_recall_list() const { return side; }

	unit* get();
	unit_ptr get_shared();

	unit* operator->() {return get();}
	unit& operator*() {return *get();}

	void clear_ref() { uid = 0; ptr = unit_ptr(); side = 0; c_ptr = nullptr; }
	// Clobbers loc
	bool put_map(const map_location &loc);
};

template<typename... Args>
inline lua_unit* luaW_pushunit(lua_State *L, Args... args) {
	lua_unit* lu = new(L) lua_unit(args...);
	lua_unit::setmetatable(L);
	return lu;
}

namespace lua_units {
	std::string register_metatables(lua_State *L);
}
