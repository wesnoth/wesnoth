/*
   Copyright (C) 2009 - 2016 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SCRIPTING_LUA_UNIT_ATTACKS_HPP
#define SCRIPTING_LUA_UNIT_ATTACKS_HPP

#include <string>

struct lua_State;

void push_unit_attacks_table(lua_State* L, int idx);

namespace lua_units {
	std::string register_attacks_metatables(lua_State* L);
}

#endif
