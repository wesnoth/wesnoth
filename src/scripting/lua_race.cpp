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

#include "scripting/lua_race.hpp"

#include "units/race.hpp"
#include "scripting/lua_common.hpp"
#include "units/types.hpp"

#include <string>
#include <cstring>

#include "lua/lua.h"
#include "lua/lauxlib.h"

/**
 * Implementation for a lua reference to a race,
 * used by the wesnoth in-game races table.
 */

// Registry key
static const char * Race = "race";
static const char * Gen = "name generator";

/**
 * Gets some data on a race (__index metamethod).
 * - Arg 1: table containing an "id" field.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_race_get(lua_State* L)
{
	char const* m = luaL_checkstring(L, 2);
	lua_pushstring(L, "id");
	lua_rawget(L, 1);
	const unit_race* raceptr = unit_types.find_race(lua_tostring(L, -1));
	if(!raceptr) return luaL_argerror(L, 1, "unknown race");
	unit_race const &race = *raceptr;

	return_tstring_attrib("description", race.description());
	return_tstring_attrib("name", race.name());
	return_int_attrib("num_traits", race.num_traits());
	return_tstring_attrib("plural_name", race.plural_name());
	return_bool_attrib("ignore_global_traits", !race.uses_global_traits());
	return_string_attrib("undead_variation", race.undead_variation());
	return_cfgref_attrib("__cfg", race.get_cfg());
	if (strcmp(m, "traits") == 0) {
		lua_newtable(L);
		if (race.uses_global_traits()) {
			for (const config& trait : unit_types.traits()) {
				const std::string& id = trait["id"];
				lua_pushlstring(L, id.c_str(), id.length());
				luaW_pushconfig(L, trait);
				lua_rawset(L, -3);
			}
		}
		for (const config& trait : race.additional_traits()) {
			const std::string& id = trait["id"];
			lua_pushlstring(L, id.c_str(), id.length());
			luaW_pushconfig(L, trait);
			lua_rawset(L, -3);
		}
		return 1;
	}
	if (strcmp(m, "male_name_gen") == 0) {
		new(L) proxy_name_generator(race.generator(unit_gender::MALE));
		luaL_getmetatable(L, Gen);
		lua_setmetatable(L, -2);
		return 1;
	}
	if (strcmp(m, "female_name_gen") == 0) {
		new(L) proxy_name_generator(race.generator(unit_gender::FEMALE));
		luaL_getmetatable(L, Gen);
		lua_setmetatable(L, -2);
		return 1;
	}

	return 0;
}

namespace lua_race {

	std::string register_metatable(lua_State * L)
	{
		luaL_newmetatable(L, Race);

		static luaL_Reg const callbacks[] {
			{ "__index", 	    &impl_race_get},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, callbacks, 0);

		lua_pushstring(L, "race");
		lua_setfield(L, -2, "__metatable");

		return "Adding getrace metatable...\n";
	}
}

void luaW_pushrace(lua_State *L, const unit_race & race)
{
	lua_createtable(L, 0, 1);
	lua_pushstring(L, race.id().c_str());
	lua_setfield(L, -2, "id");
	luaL_setmetatable(L, Race);
}

void luaW_pushracetable(lua_State *L)
{
	const race_map& races = unit_types.races();
	lua_createtable(L, 0, races.size());

	for (const race_map::value_type &race : races)
	{
		assert(race.first == race.second.id());
		luaW_pushrace(L, race.second);
		lua_setfield(L, -2, race.first.c_str());
	}
}
