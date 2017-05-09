/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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

struct lua_State;
class unit_type;

#include <string>

/**
 * This namespace contains bindings for lua to hold a reference to a
 * unit type and access its stats.
 */
namespace lua_unit_type {
	std::string register_metatable(lua_State *);
	std::string register_table(lua_State *);
} //end namespace lua_team

/// Create a lua object containing a reference to a unittype, and a
/// metatable to access the properties.
void luaW_pushunittype(lua_State *, const unit_type&);

/// Test if a stack element is a unit type, and return it if so
const unit_type* luaW_tounittype(lua_State*, int);

/// Test if a stack element is a unit type, and return it if so
const unit_type& luaW_checkunittype(lua_State*, int);
