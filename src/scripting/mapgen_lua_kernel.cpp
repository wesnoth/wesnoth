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

#include "scripting/mapgen_lua_kernel.hpp"

#include "config.hpp"
#include "game_errors.hpp"
#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_rng.hpp"
#include "scripting/lua_pathfind_cost_calculator.hpp"

#include <ostream>
#include <string>
#include "utils/functional.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"
#include "scripting/push_check.hpp"

static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)
#define DBG_NG LOG_STREAM(debug, log_mapgen)

struct lua_State;



/**
 * Returns a random numer, same interface as math.random.
 */
static int intf_random(lua_State *L)
{
	std::mt19937& rng = lua_kernel_base::get_lua_kernel<mapgen_lua_kernel>(L).get_default_rng();
	if(lua_isnoneornil(L, 1)) {
		double r = double (rng());
		double r_max = double (rng.max());
		lua_push(L, r / (r_max + 1));
		return 1;
	}
	else {
		int32_t min;
		int32_t max;
		if(lua_isnumber(L, 2)) {
			min = lua_check<int32_t>(L, 1);
			max = lua_check<int32_t>(L, 2);
		}
		else {
			min = 1;
			max = lua_check<int32_t>(L, 1);
		}
		if(min > max) {
			return luaL_argerror(L, 1, "min > max");
		}
		lua_push(L, min + static_cast<int>(rng() % (max - min + 1)));
		return 1;
	}
}

/**
 * Finds a path between two locations.
 * - Args 1,2: source location.
 * - Args 3,4: destination.
 * - Arg 5: cost function
 * - Args 6,7 size of map.
 * - Arg 8 include border.
 * - Ret 1: array of pairs containing path steps.
 * - Ret 2: path cost.
 */
static int intf_find_path(lua_State *L)
{
	int arg = 1;
	map_location src, dst;
	src.set_wml_x(luaL_checkinteger(L, 1));
	src.set_wml_y(luaL_checkinteger(L, 2));
	dst.set_wml_x(luaL_checkinteger(L, 3));
	dst.set_wml_y(luaL_checkinteger(L, 4));
	if(lua_isfunction(L, arg)) {
		const char *msg = lua_pushfstring(L, "%s expected, got %s", lua_typename(L, LUA_TFUNCTION), luaL_typename(L, 5));
		return luaL_argerror(L, 5, msg);
	}
	lua_pathfind_cost_calculator calc(L, 5);
	int width = luaL_checkinteger(L, 6);
	int height = luaL_checkinteger(L, 7);
	bool border = false;
	if(lua_isboolean(L, 8)) {
		border = luaW_toboolean(L, 8);
	}
	pathfind::plain_route res = pathfind::a_star_search(src, dst, 10000, calc, width, height, nullptr, border);

	int nb = res.steps.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, res.steps[i].wml_x());
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, res.steps[i].wml_y());
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, i + 1);
	}
	lua_pushinteger(L, res.move_cost);

	return 2;
}


mapgen_lua_kernel::mapgen_lua_kernel()
	: lua_kernel_base()
	, random_seed_()
	, default_rng_()
{
	lua_State *L = mState;

	// Unset wesnoth.random. This guarantees that the mapgen_lua_kernel version
	// of wesnoth.random overrides the lua_kernel_base version.
	lua_getglobal(L, "wesnoth");
	lua_pushnil(L);
	lua_setfield(L, -2, "random");

	lua_settop(L, 0);

	static luaL_Reg const callbacks[] {
		{ "find_path",           &intf_find_path           },
		{ "random",              &intf_random              },
		{ nullptr, nullptr }
	};

	lua_getglobal(L, "wesnoth");
	assert(lua_istable(L,-1));
	luaL_setfuncs(L, callbacks, 0);
	lua_pop(L, 1);
	assert(lua_gettop(L) == 0);
}

void mapgen_lua_kernel::run_generator(const char * prog, const config & generator)
{
	load_string(prog, std::bind(&lua_kernel_base::throw_exception, this, _1, _2));
	luaW_pushconfig(mState, generator);
	protected_call(1, 1, std::bind(&lua_kernel_base::throw_exception, this, _1, _2));
}

void mapgen_lua_kernel::user_config(const char * prog, const config & generator)
{
	run_generator(prog, generator);
}

std::string mapgen_lua_kernel::create_map(const char * prog, const config & generator, boost::optional<uint32_t> seed) // throws game::lua_error
{
	random_seed_ = seed;
	run_generator(prog, generator);

	if (!lua_isstring(mState,-1)) {
		std::string msg = "expected a string, found a ";
		msg += lua_typename(mState, lua_type(mState, -1));
		lua_pop(mState, 1);
		throw game::lua_error(msg.c_str(),"bad return value");
	}

	return lua_tostring(mState, -1);
}

config mapgen_lua_kernel::create_scenario(const char * prog, const config & generator, boost::optional<uint32_t> seed) // throws game::lua_error
{
	random_seed_ = seed;
	run_generator(prog, generator);

	if (!lua_istable(mState, -1)) {
		std::string msg = "expected a config (table), found a ";
		msg += lua_typename(mState, lua_type(mState, -1));
		lua_pop(mState, 1);
		throw game::lua_error(msg.c_str(),"bad return value");
	}
	config result;
	if (!luaW_toconfig(mState, -1, result)) {
		std::string msg = "expected a config, but it is malformed ";
		lua_pop(mState, 1);
		throw game::lua_error(msg.c_str(),"bad return value");
	}
	return result;
}
uint32_t mapgen_lua_kernel::get_random_seed()
{
	if(uint32_t* pint = random_seed_.get_ptr()) {
		return (*pint)++;
	}
	else {
		return lua_kernel_base::get_random_seed();
	}
}

std::mt19937& mapgen_lua_kernel::get_default_rng()
{
	if(!default_rng_) {
		default_rng_ = std::mt19937(get_random_seed());
	}
	return *default_rng_;
}
