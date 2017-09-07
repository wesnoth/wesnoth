/*
   Copyright (C) 2017
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

class team;
struct lua_State;

#include <string>

/**
 * This namespace contains bindings for lua to hold a pointer to a team,
 * and to access and modify it.
 */
namespace lua_filter {
	std::string register_metatable(lua_State *);
} //end namespace lua_team

/// Create a full userdata containing a compiled unit filter.
void luaW_push_unit_filter(lua_State *, config&& cfg);

/// Create a full userdata containing a compiled unit filter.
void luaW_push_location_filter(lua_State *, config&& cfg);


location_filter& luaW_check_location_filter(lua_State* L, int idx);

location_filter* luaW_to_location_filter(lua_State* L, int idx);

unit_filter& luaW_check_unit_filter(lua_State* L, int idx);

unit_filter* luaW_to_unit_filter(lua_State* L, int idx);
