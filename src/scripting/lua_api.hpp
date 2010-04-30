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

#include "game_events.hpp"

struct lua_State;


namespace lua {

bool luaW_pcall(lua_State *L , int nArgs, int nRets, bool allow_wml_error = false);

unit *luaW_tounit(lua_State *L, int index, bool only_on_map = false);

void table_of_wml_config(lua_State *L, config const &cfg);

bool luaW_toconfig(lua_State *L, int index, config &cfg, int tstring_meta = 0);
/**
 * Storage for a unit, either one on the map, or one owned by the Lua code.
 */
class lua_unit
{
	size_t uid;
	unit *ptr;
	lua_unit(lua_unit const &);

public:
	lua_unit(size_t u): uid(u), ptr(NULL) {}
	lua_unit(unit *u): uid(0), ptr(u) {}
	~lua_unit();
	bool on_map() const { return !ptr; }
	void reload();
	unit *get();
};


} //of namespace lua

#endif
