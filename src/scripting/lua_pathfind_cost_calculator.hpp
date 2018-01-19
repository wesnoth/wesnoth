/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once
#include "lua/lua.h"
#include "map/location.hpp"
#include "pathfind/pathfind.hpp"

/**
 * Cost function object relying on a Lua function.
 * @note The stack index of the Lua function must be valid each time the cost is computed.
 */
struct lua_pathfind_cost_calculator : pathfind::cost_calculator
{
	lua_State *L;
	int index;
	/// @param i the stack position of the lua function to calculate the cost.
	lua_pathfind_cost_calculator(lua_State *L_, int i): L(L_), index(i) {}
	double cost(const map_location &loc, const double so_far) const
	{
		// Copy the user function and push the location and current cost.
		lua_pushvalue(L, index);
		lua_pushinteger(L, loc.wml_x());
		lua_pushinteger(L, loc.wml_y());
		lua_pushnumber(L, so_far);
		// Execute the user function.
		if (!luaW_pcall(L, 3, 1)) {
			return 1.;
		}
		// Return a cost of at least 1 mp to avoid issues in pathfinder.
		// (Condition is inverted to detect NaNs.)
		double cost = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return !(cost >= 1.) ? 1. : cost;
	}
};
