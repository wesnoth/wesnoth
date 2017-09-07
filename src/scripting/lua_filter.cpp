/*
   Copyright (C) 2017
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_team.hpp"

#include "scripting/lua_common.hpp"
#include "team.hpp"
#include "resources.hpp" // for gameboard
#include "game_board.hpp"

#include <string>

#include "lua/lua.h"
#include "lua/lauxlib.h"


// Registry keys
static const char * key_location_filter = "location_filter";
static const char * key_unit_filter = "unit_filter";

/**
 *  - Arg 1: filter userdata.
 *  - Arg 2: unit.
 *  - Arg 3: table for optional argeuments.
 *  - Ret 1: boolean.
*/
static int impl_unit_filter_call(lua_State *L)
{
	unit_filter& filt = luaW_check_unit_filter(L, 1);
	unit* pu = luaW_checkunit_ref(L, 2)->get();
	
	
	if(!pu) {
		return luaL_argerror(L, 1, "unknown unit");
	}

	if(lua_istable(L, 3)) {
		lua_pushstring(L, "loc");
		lua_rawget(L, 3);
		map_location src = luaW_checklocation(L, -1);
		lua_pop(L, 1);

		lua_push(filt.matches(*pu, src));
	}
	else {
		lua_push(filt.matches(*pu));
	}
	return 1;
}

/**
 *  - Arg 1: filter userdata.
 *  - Arg 2: location.
 *  - Ret 1: boolean.
*/
static int impl_location_filter_call(lua_State *L)
{
	location_filter& filt = luaW_check_location_filter(L, 1);
	map_locaton loc = luaW_checklocation(L, 2);

	lua_push(filt.match(loc));
	return 1;
}

static int impl_unit_filter_get(lua_State *L)
{
	unit_filter& filt = luaW_check_unit_filter(L, 1);
	char const *m = luaL_checkstring(L, 2);

	return_cfg_attrib("__cfg", cfg = filt.to_config());
	if(luaW_getmetafield(L, 1, m)) {
		return 1;
	}
	return 0;
}

static int impl_unit_filter_set(lua_State *L)
{
	std::string err_msg = "unknown modifiable property of unit filters: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

static int impl_locaton_filter_get(lua_State *L)
{
	unit_filter& filt = luaW_check_unit_filter(L, 1);
	char const *m = luaL_checkstring(L, 2);

	return_cfg_attrib("__cfg", cg = filt.to_config());
	if(luaW_getmetafield(L, 1, m)) {
		return 1;
	}
	return 0;
}

static int impl_location_filter_set(lua_State *L)
{
	std::string err_msg = "unknown modifiable property of location filters: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

namespace lua_filter {
	std::string register_metatable(lua_State* L)
	{
		luaL_newmetatable(L, key_unit_filter);

		//TODO: add a 'all_matches' function that that returns all loctions/units that match that filter.
		static luaL_Reg const callbacks[] {
			{ "__call", 	    &impl_unit_filter_call},
			{ "__index", 	    &impl_unit_filter_get},
			{ "__newindex",	    &impl_unit_filter_set},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, callbacks, 0);

		lua_pushstring(L, key_unit_filter);
		lua_setfield(L, -2, "__metatable");
		
		
		luaL_newmetatable(L, key_location_filter);

		static luaL_Reg const callbacks[] {
			{ "__call", 	    &impl_location_filter_call},
			{ "__index", 	    &impl_location_filter_get},
			{ "__newindex",	    &impl_location_filter_set},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, callbacks, 0);

		lua_pushstring(L, key_location_filter);
		lua_setfield(L, -2, "__metatable");

		return "Adding filters table...";
	}
}


void luaW_push_unit_filter(lua_State *, config&& cfg)
{
	unit_filter* res = new(L) unit_filter(cfg);
	luaL_setmetatable(L, key_unit_filter);
	//return res;
}

void luaW_push_location_filter(lua_State *, config&& cfg)
{
	location_filter* res = new(L) location_filter(cfg);
	luaL_setmetatable(L, key_location_filter);
	//return res;
}

location_filter& luaW_check_location_filter(lua_State* L, int idx)
{
	return **static_cast<location_filter **>(luaL_checkudata(L, idx, key_location_filter));
}

location_filter* luaW_to_location_filter(lua_State* L, int idx)
{
	if(void* p = luaL_testudata(L, idx, key_location_filter)) {
		return *static_cast<location_filter **>(p);
	}
	return nullptr;
}

unit_filter& luaW_check_unit_filter(lua_State* L, int idx)
{
	return **static_cast<unit_filter **>(luaL_checkudata(L, idx, key_unit_filter));
}

unit_filter* luaW_to_unit_filter(lua_State* L, int idx)
{
	if(void* p = luaL_testudata(L, idx, key_unit_filter)) {
		return *static_cast<unit_filter **>(p);
	}
	return nullptr;
}
