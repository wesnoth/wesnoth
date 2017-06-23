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

class unit_race;
struct lua_State;

#include <string>

/**
 * This namespace contains bindings for lua to hold a pointer to a race,
 * and to access and modify it.
 */
namespace lua_race {
	std::string register_metatable(lua_State *);
} //end namespace lua_team

// Create a lua reference to the race.
void luaW_pushrace(lua_State *, const unit_race &);
void luaW_pushracetable(lua_State *);
