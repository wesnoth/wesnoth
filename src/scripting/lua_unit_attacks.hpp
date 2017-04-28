/*
   Copyright (C) 2009 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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

#include "units/attack_type.hpp"

struct lua_State;
class lua_unit;

void push_unit_attacks_table(lua_State* L, int idx);

namespace lua_units {
	std::string register_attacks_metatables(lua_State* L);
}

void luaW_pushweapon(lua_State* L, attack_ptr weapon);
void luaW_pushweapon(lua_State* L, const_attack_ptr weapon);
const_attack_ptr luaW_toweapon(lua_State* L, int idx);
attack_type& luaW_checkweapon(lua_State* L, int idx);

#endif
