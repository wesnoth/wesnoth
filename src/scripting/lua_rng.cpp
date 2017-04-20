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

#include "scripting/lua_rng.hpp"

#include "log.hpp"
#include "mt_rng.hpp"
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_common.hpp" // for new(L)

#include <new>
#include <string>

#include "lua/lua.h"
#include "lua/lauxlib.h"

static lg::log_domain log_lua("scripting/lua");
#define ERR_LUA LOG_STREAM(err, log_lua)

// Begin lua rng bindings
namespace lua_rng {

using rand_rng::mt_rng;

static const char * Rng = "Rng";

int impl_rng_create(lua_State* L)
{
	uint32_t seed = lua_kernel_base::get_lua_kernel<lua_kernel_base>(L).get_random_seed();
	new(L) mt_rng(seed);
	luaL_setmetatable(L, Rng);

	return 1;
}

int impl_rng_destroy(lua_State* L)
{
	mt_rng * d = static_cast< mt_rng *> (luaL_testudata(L, 1, Rng));
	if (d == nullptr) {
		ERR_LUA << "rng_destroy called on data of type: " << lua_typename( L, lua_type( L, 1 ) ) << std::endl;
		ERR_LUA << "This may indicate a memory leak, please report at bugs.wesnoth.org" << std::endl;
		lua_pushstring(L, "Rng object garbage collection failure");
		lua_error(L);
	} else {
		d->~mt_rng();
	}
	return 0;
}

int impl_rng_seed(lua_State* L)
{
	mt_rng * rng = static_cast<mt_rng *>(luaL_checkudata(L, 1, Rng));
	std::string seed = luaL_checkstring(L, 2);

	rng->seed_random(seed);
	return 0;
}

int impl_rng_draw(lua_State* L)
{
	mt_rng * rng = static_cast<mt_rng *>(luaL_checkudata(L, 1, Rng));

	lua_pushnumber(L, rng->get_next_random());
	return 1;
}

// End Lua Rng bindings

void load_tables(lua_State* L)
{
	luaL_newmetatable(L, Rng);

	static luaL_Reg const callbacks[] {
		{ "create",         &impl_rng_create},
		{ "__gc",           &impl_rng_destroy},
		{ "seed", 	    &impl_rng_seed},
		{ "draw",	    &impl_rng_draw},
		{ nullptr, nullptr }
	};
	luaL_setfuncs(L, callbacks, 0);

	lua_pushvalue(L, -1); //make a copy of this table, set it to be its own __index table
	lua_setfield(L, -2, "__index");

	lua_setglobal(L, Rng);
}

} // end namespace lua_map_rng
