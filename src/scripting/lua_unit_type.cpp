/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_unit_type.hpp"

#include "scripting/lua_common.hpp"
#include "unit_types.hpp"

#include <boost/foreach.hpp>
#include <string>

#include "lua/lua.h"
#include "lua/lauxlib.h"

/**
 * Implementation for a lua reference to a unit_type.
 */

// Registry key
static const char * UnitType = "unit type";

/**
 * Gets some data on a unit type (__index metamethod).
 * - Arg 1: table containing an "id" field.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_unit_type_get(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	lua_pushstring(L, "id");
	lua_rawget(L, 1);
	const unit_type *utp = unit_types.find(lua_tostring(L, -1));
	if (!utp) return luaL_argerror(L, 1, "unknown unit type");
	unit_type const &ut = *utp;

	// Find the corresponding attribute.
	return_tstring_attrib("name", ut.type_name());
	return_int_attrib("max_hitpoints", ut.hitpoints());
	return_int_attrib("max_moves", ut.movement());
	return_int_attrib("max_experience", ut.experience_needed());
	return_int_attrib("cost", ut.cost());
	return_int_attrib("level", ut.level());
	return_int_attrib("recall_cost", ut.recall_cost());
	return_cfgref_attrib("__cfg", ut.get_cfg());
	return 0;
}

namespace lua_unit_type {
	std::string register_metatable(lua_State * L)
	{
		luaL_newmetatable(L, UnitType);

		lua_pushcfunction(L, impl_unit_type_get);
		lua_setfield(L, -2, "__index");
		lua_pushstring(L, "unit type");
		lua_setfield(L, -2, "__metatable");

		return "Adding unit type metatable...\n";
	}
}

void luaW_pushunittype(lua_State *L, const std::string & id)
{
	lua_createtable(L, 0, 1);
	lua_pushstring(L, id.c_str());
	lua_setfield(L, -2, "id");
	luaL_setmetatable(L, UnitType);
}
