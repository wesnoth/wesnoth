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

#include "scripting/lua_map_location_ops.hpp"
#include "scripting/lua_common.hpp"

#include "map/location.hpp"

#include <string>
#include <utility>
#include <ciso646>   // for and

#include "lua/lua.h"
#include "lua/lauxlib.h"

namespace lua_map_location {

/**
 * Expose map_location::get_direction function to lua
 * Arg 1: a location
 * Arg 2: a direction
 * Arg 3: number of steps
 */
int intf_get_direction(lua_State* L)
{
	map_location l;
	if(!luaW_tolocation(L, 1, l)) {
		return luaL_argerror(L, 1, "get_direction: first argument(S) must be a location");
	}
	int nargs = lua_gettop(L);
	if (nargs != 2 and nargs != 3) {
		std::string msg("get_direction: must pass 2 or 3 args, found ");
		msg += std::to_string(nargs);
		luaL_error(L, msg.c_str());
		return 0;
	}

	int n = 1;
	if (nargs == 3) {
		n = luaL_checkinteger(L, -1);
		lua_pop(L,1);
	}

	map_location::DIRECTION d;
	if (lua_isnumber(L, -1)) {
		d = map_location::rotate_right(map_location::NORTH, static_cast<int>(luaL_checkinteger(L, -1))); //easiest way to correctly convert int to direction
		lua_pop(L,1);
	} else if (lua_isstring(L, -1)) {
		d = map_location::parse_direction(luaL_checkstring(L,-1));
		lua_pop(L,1);
	} else {
		std::string msg("get_direction: second argument should be a direction, either a string or an integer, instead found a ");
		msg += lua_typename(L, lua_type(L, -1));
		return luaL_argerror(L, -1, msg.c_str());
	}

	map_location result = l.get_direction(d, n);
	luaW_pushlocation(L, result);
	return 1;
}

/**
 * Expose map_location::vector_sum to lua
 */
int intf_vector_sum(lua_State* L)
{
	map_location l1, l2;
	if(!luaW_tolocation(L, 1, l1) || !luaW_tolocation(L, 2, l2)) {
		lua_pushstring(L, "vector_sum: requires two locations");
		return lua_error(L);
	}

	luaW_pushlocation(L, l1.vector_sum_assign(l2));
	return 1;
}

/**
 * Expose map_location::vector_negation to lua
 */
int intf_vector_negation(lua_State* L)
{
	map_location l1;
	if(!luaW_tolocation(L, 1, l1)) {
		return luaL_argerror(L, 1, "expected a location");
	}

	luaW_pushlocation(L, l1.vector_negation());
	return 1;
}

/**
 * Expose map_location::ZERO to lua
 */
int intf_vector_zero(lua_State* L)
{
	luaW_pushlocation(L, map_location::ZERO());
	return 1;
}

/**
 * Expose map_location::rotate_right_around_center to lua
 */
int intf_rotate_right_around_center(lua_State* L)
{
	int k = luaL_checkinteger(L, -1);
	lua_pop(L,1);
	map_location center, loc;
	if(!luaW_tolocation(L, 1, loc) || !luaW_tolocation(L, 2, center)) {
		lua_pushstring(L, "rotate_right_around_center: requires two locations");
		return lua_error(L);
	}

	luaW_pushlocation(L, loc.rotate_right_around_center(center, k));
	return 1;
}

/**
 * Expose map_location tiles_adjacent
 */
int intf_tiles_adjacent(lua_State* L)
{
	map_location l1, l2;
	if(!luaW_tolocation(L, 1, l1) || !luaW_tolocation(L, 2, l2)) {
		lua_pushstring(L, "vector_sum: requires two locations");
		return lua_error(L);
	}

	lua_pushboolean(L, tiles_adjacent(l1,l2));
	return 1;
}

/**
 * Expose map_location get_adjacent_tiles
 */
int intf_get_adjacent_tiles(lua_State* L)
{
	map_location l1;
	if(!luaW_tolocation(L, 1, l1)) {
		return luaL_argerror(L, 1, "expected a location");
	}

	map_location locs[6];
	get_adjacent_tiles(l1, locs);

	for (int i = 0; i < 6; ++i) {
		luaW_pushlocation(L, locs[i]);
	}

	return 6;
}

/**
 * Expose map_location distance_between
 */
int intf_distance_between(lua_State* L)
{
	map_location l1, l2;
	if(!luaW_tolocation(L, 1, l1) || !luaW_tolocation(L, 2, l2)) {
		lua_pushstring(L, "distance_between: requires two locations");
		return lua_error(L);
	}

	lua_pushinteger(L, distance_between(l1,l2));
	return 1;
}

/**
 * Expose map_location get_in_basis_N_NE
 */
int intf_get_in_basis_N_NE(lua_State* L)
{
	map_location l1;
	if(!luaW_tolocation(L, 1, l1)) {
		return luaL_argerror(L, 1, "expected a location");
	}

	std::pair<int, int> r = l1.get_in_basis_N_NE();
	lua_pushinteger(L, r.first);
	lua_pushinteger(L, r.second);
	return 2;
}

/**
 * Expose map_location get_relative_dir
 */
int intf_get_relative_dir(lua_State* L)
{
	map_location l1, l2;
	if(!luaW_tolocation(L, 1, l1) || !luaW_tolocation(L, 2, l2)) {
		lua_pushstring(L, "get_relative_dir: requires two locations");
		return lua_error(L);
	}

	lua_pushinteger(L, l1.get_relative_dir(l2));
	return 1;
}

/**
 * Expose map_location parse_direction
 */
int intf_parse_direction(lua_State* L)
{
	std::string str = luaL_checkstring(L, -1);
	map_location::DIRECTION d = map_location::parse_direction(str);
	if (d == map_location::NDIRECTIONS) {
		luaL_argerror(L, -1, "error: not a direction string");
		return 0;
	} else {
		lua_pushinteger(L, d);
		return 1;
	}
}

/**
 * Expose map_location write_direction
 */
int intf_write_direction(lua_State* L)
{
	int d = luaL_checkinteger(L, -1);
	if (d >= 0 && d < map_location::NDIRECTIONS) {
		lua_pushstring(L, map_location::write_direction(static_cast<map_location::DIRECTION>(d)).c_str());
		return 1;
	} else {
		luaL_argerror(L, -1, "error: must be an integer from 0 to 5");
		return 0;
	}
}

} // end namespace lua_map_location
