/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "scripting/push_check.hpp"

#include "map/location.hpp"
#include "pathutils.hpp"

#include <string>
#include <utility>

static bool luaW_tocubeloc(lua_State* L, int idx, cubic_location& out) {
	if(!lua_istable(L, idx)) {
		return false;
	}
	int n = lua_absindex(L, -1);
	if(!luaW_tableget(L, n, "q")) {
		return false;
	}
	out.q = luaL_checkinteger(L, -1);
	if(!luaW_tableget(L, n, "r")) {
		return false;
	}
	out.r = luaL_checkinteger(L, -1);
	if(luaW_tableget(L, n, "s")) {
		out.s = luaL_checkinteger(L, -1);
	} else {
		out.s = -out.q - out.r;
	}
	if(out.q + out.s + out.r != 0) {
		return false;
	}
	return true;
}

static cubic_location luaW_checkcubeloc(lua_State* L, int idx) {
	cubic_location loc;
	if(!luaW_tocubeloc(L, idx, loc)) {
		luaL_argerror(L, idx, "expected cubic location");
	}
	return loc;
}

static void luaW_pushcubeloc(lua_State* L, cubic_location loc) {
	luaW_push_namedtuple(L, {"q", "r", "s"});
	lua_pushinteger(L, loc.q);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, loc.r);
	lua_rawseti(L, -2, 2);
	lua_pushinteger(L, loc.s);
	lua_rawseti(L, -2, 3);
}

namespace lua_map_location {

/**
 * Expose map_location::get_direction function to lua
 * Arg 1: a location
 * Arg 2: a direction
 * Arg 3: (optional) number of steps
 */
int intf_get_direction(lua_State* L)
{
	map_location l;
	if(!luaW_tolocation(L, 1, l)) {
		return luaL_argerror(L, 1, "get_direction: first argument(S) must be a location");
	}
	int nargs = lua_gettop(L);
	if (nargs < 2) {
		luaL_error(L, "get_direction: not missing direction argument");
		return 0;
	}

	int n = 1;
	if (nargs == 3) {
		n = luaL_checkinteger(L, -1);
		lua_pop(L,1);
	}

	map_location::direction d;
	if (lua_isstring(L, -1)) {
		d = map_location::parse_direction(luaL_checkstring(L,-1));
		lua_pop(L,1);
	} else {
		std::string msg("get_direction: second argument should be a direction string, instead found a ");
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
 * Expose map_location::vector_difference to lua
 */
int intf_vector_diff(lua_State* L)
{
	map_location l1, l2;
	if(!luaW_tolocation(L, 1, l1) || !luaW_tolocation(L, 2, l2)) {
		lua_pushstring(L, "vector_diff: requires two locations");
		return lua_error(L);
	}

	luaW_pushlocation(L, l1.vector_difference_assign(l2));
	return 1;
}

/**
 * Expose map_location::vector_negation to lua
 * - Arg 1: Location
 * - Ret: Negated vector
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
 * - Args 1, 2: Two locations
 * - Ret: True if the locations are adjacent
 */
int intf_tiles_adjacent(lua_State* L)
{
	map_location l1, l2;
	if(!luaW_tolocation(L, 1, l1) || !luaW_tolocation(L, 2, l2)) {
		lua_pushstring(L, "tiles_adjacent: requires two locations");
		return lua_error(L);
	}

	lua_pushboolean(L, tiles_adjacent(l1,l2));
	return 1;
}

/**
 * Expose map_location get_adjacent_tiles
 * - Arg 1: A location
 * - Ret 1 - 6: The adjacent locations
 */
int intf_get_adjacent_tiles(lua_State* L)
{
	map_location l1;
	if(!luaW_tolocation(L, 1, l1)) {
		return luaL_argerror(L, 1, "expected a location");
	}

	for(const map_location& adj : get_adjacent_tiles(l1)) {
		luaW_pushlocation(L, adj);
	}

	return 6;
}

/**
 * Expose map_location get_tile_ring
 * - Arg 1: A location
 * - Arg 2: A radius
 * - Ret: The locations
 */
int intf_get_tile_ring(lua_State* L)
{
	map_location l1;
	if(!luaW_tolocation(L, 1, l1)) {
		return luaL_argerror(L, 1, "expected a location");
	}
	int radius = luaL_checkinteger(L, 2);

	std::vector<map_location> locs;
	get_tile_ring(l1, radius, locs);
	lua_push(L, locs);

	return 1;
}

/**
* Expose map_location get_tiles_in_radius
* - Arg 1: A location
* - Arg 2: A radius
* - Ret: The locations
*/
int intf_get_tiles_in_radius(lua_State* L)
{
	map_location l1;
	if(!luaW_tolocation(L, 1, l1)) {
		return luaL_argerror(L, 1, "expected a location");
	}
	int radius = luaL_checkinteger(L, 2);

	std::vector<map_location> locs;
	get_tiles_in_radius(l1, radius, locs);
	lua_push(L, locs);

	return 1;
}

/**
 * Expose map_location distance_between
 * - Args 1, 2: Two locations
 * - Ret: The distance between the two locations
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
 * Expose map_location to_cubic
 * - Arg: Location
 * - Ret: Location in cubic coordinates
 */
int intf_get_in_cubic(lua_State* L)
{
	map_location l1;
	if(!luaW_tolocation(L, 1, l1)) {
		return luaL_argerror(L, 1, "expected a location");
	}
	cubic_location h = l1.to_cubic();
	// Due to the way that locations in Lua have are 1 more on each coordinate
	// compared to the C++ coordinates, the output here also needs to be adjusted.
	h.q++;
	h.s--;
	luaW_pushcubeloc(L, h);
	return 1;
}

/**
 * Expose map_location from_cubic
 * - Arg: Location in cubic coordinates
 * - Ret: Location
 */
int intf_get_from_cubic(lua_State* L)
{
	cubic_location h = luaW_checkcubeloc(L, 1);
	// Adjust it from the WML location system
	h.q--;
	h.s++;
	map_location l = map_location::from_cubic(h);
	luaW_pushlocation(L, l);
	return 1;
}

/**
 * Expose map_location get_relative_dir
 * - Args 1, 2: Two locations
 * - Ret: The direction of location 2 from location 1
 */
int intf_get_relative_dir(lua_State* L)
{
	map_location l1, l2;
	if(!luaW_tolocation(L, 1, l1) || !luaW_tolocation(L, 2, l2)) {
		lua_pushstring(L, "get_relative_dir: requires two locations");
		return lua_error(L);
	}

	const std::string dir = map_location::write_direction(l1.get_relative_dir(l2));
	lua_pushlstring(L, dir.c_str(), dir.length());
	return 1;
}

} // end namespace lua_map_location
