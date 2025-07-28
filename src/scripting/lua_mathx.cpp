/*
	Copyright (C) 2014 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "scripting/lua_mathx.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/push_check.hpp"
#include "random.hpp"


#include <cmath>

namespace lua_mathx {

/**
* Returns a random number, same interface as math.random.
*/
static int intf_random(lua_State* L)
{
	if (lua_isnoneornil(L, 1)) {
		double r = static_cast<double>(randomness::generator->next_random());
		double r_max = static_cast<double>(std::numeric_limits<uint32_t>::max());
		lua_push(L, r / (r_max + 1));
		return 1;
	}
	else {
		int32_t min;
		int32_t max;
		if (lua_isnumber(L, 2)) {
			min = lua_check<int32_t>(L, 1);
			max = lua_check<int32_t>(L, 2);
		}
		else {
			min = 1;
			max = lua_check<int32_t>(L, 1);
		}
		if (min > max) {
			return luaL_argerror(L, 1, "min > max");
		}
		lua_push(L, randomness::generator->get_random_int(min, max));
		return 1;
	}
}

static int intf_round(lua_State* L) {
	double n = lua_tonumber(L, 1);
	lua_pushinteger(L, std::round(n));
	return 1;
}

int luaW_open(lua_State* L) {
	auto& lk = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L);
	lk.add_log("Adding mathx module...\n");
	static luaL_Reg const math_callbacks[] = {
		{ "random",                      &intf_random                   },
		{ "round",                       &intf_round                    },
		{ nullptr, nullptr },
	};
	lua_newtable(L);
	luaL_setfuncs(L, math_callbacks, 0);
	// Set the mathx metatable to index the math module
	lua_createtable(L, 0, 1);
	lua_getglobal(L, "math");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	return 1;
}

}
