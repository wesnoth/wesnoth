/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCRIPTING_LUA_API_HPP
#define SCRIPTING_LUA_API_HPP

#include <cstddef>

struct lua_State;
class config;
class unit;

bool luaW_pcall(lua_State *L , int nArgs, int nRets, bool allow_wml_error = false);
unit *luaW_tounit(lua_State *L, int index, bool only_on_map = false);
void luaW_pushconfig(lua_State *L, config const &cfg);
bool luaW_toconfig(lua_State *L, int index, config &cfg, int tstring_meta = 0);

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
};

#endif
