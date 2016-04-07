#pragma once
#include "lua/lua.h"
#include "map/location.hpp"
#include "scripting/lua_api.hpp"
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
		lua_pushinteger(L, loc.x + 1);
		lua_pushinteger(L, loc.y + 1);
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
