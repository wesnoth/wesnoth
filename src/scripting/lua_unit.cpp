/*
   Copyright (C) 2009 - 2016 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "lua_unit.hpp"
#include "lua_types.hpp"

#include "game_board.hpp"
#include "log.hpp"
#include "map/location.hpp"             // for map_location
#include "resources.hpp"
#include "scripting/lua_common.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"                    // for lua_State, lua_settop, etc

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

lua_unit::~lua_unit()
{
}

unit* lua_unit::get()
{
	if (ptr) return ptr.get();
	if (c_ptr) return c_ptr;
	if (side) {
		return resources::gameboard->teams()[side - 1].recall_list().find_if_matches_underlying_id(uid).get();
	}
	unit_map::unit_iterator ui = resources::units->find(uid);
	if (!ui.valid()) return nullptr;
	return ui.get_shared_ptr().get(); //&*ui would not be legal, must get new shared_ptr by copy ctor because the unit_map itself is holding a boost shared pointer.
}
unit_ptr lua_unit::get_shared()
{
	if (ptr) return ptr;
	if (side) {
		return resources::gameboard->teams()[side - 1].recall_list().find_if_matches_underlying_id(uid);
	}
	unit_map::unit_iterator ui = resources::units->find(uid);
	if (!ui.valid()) return unit_ptr();
	return ui.get_shared_ptr(); //&*ui would not be legal, must get new shared_ptr by copy ctor because the unit_map itself is holding a boost shared pointer.
}

// Having this function here not only simplifies other code, it allows us to move
// pointers around from one structure to another.
// This makes bare pointer->map in particular about 2 orders of magnitude faster,
// as benchmarked from Lua code.
bool lua_unit::put_map(const map_location &loc)
{
	if (ptr) {
		ptr->set_location(loc);
		resources::units->erase(loc);
		std::pair<unit_map::unit_iterator, bool> res = resources::units->insert(ptr);
		if (res.second) {
			ptr.reset();
			uid = res.first->underlying_id();
		} else {
			ERR_LUA << "Could not move unit " << ptr->underlying_id() << " onto map location " << loc << '\n';
			return false;
		}
	} else if (side) { // recall list
		unit_ptr it = resources::gameboard->teams()[side - 1].recall_list().extract_if_matches_underlying_id(uid);
		if (it) {
			side = 0;
			// uid may be changed by unit_map on insertion
			uid = resources::units->replace(loc, *it).first->underlying_id();
		} else {
			ERR_LUA << "Could not find unit " << uid << " on recall list of side " << side << '\n';
			return false;
		}
	} else { // on map
		unit_map::unit_iterator ui = resources::units->find(uid);
		if (ui != resources::units->end()) {
			map_location from = ui->get_location();
			if (from != loc) { // This check is redundant in current usage
				resources::units->erase(loc);
				resources::units->move(from, loc);
			}
			// No need to change our contents
		} else {
			ERR_LUA << "Could not find unit " << uid << " on the map" << std::endl;
			return false;
		}
	}
	return true;
}

bool luaW_isunit(lua_State* L, int index)
{
	return luaW_hasmetatable(L, index,getunitKey);
}

enum {
	LU_OK,
	LU_NOT_UNIT,
	LU_NOT_ON_MAP,
	LU_NOT_VALID,
};

static lua_unit* internal_get_unit(lua_State *L, int index, bool only_on_map, int& error)
{
	error = LU_OK;
	if(!luaW_isunit(L, index)) {
		error = LU_NOT_UNIT;
		return nullptr;
	}
	lua_unit* lu = static_cast<lua_unit*>(lua_touserdata(L, index));
	if(only_on_map && !lu->on_map()) {
		error = LU_NOT_ON_MAP;
	}
	if(!lu->get()) {
		error = LU_NOT_VALID;
	}
	return lu;
}

unit* luaW_tounit(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	if(error != LU_OK) {
		return nullptr;
	}
	return lu->get();
}

unit_ptr luaW_tounit_ptr(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	if(error != LU_OK) {
		return nullptr;
	}
	return lu->get_shared();
}

lua_unit* luaW_tounit_ref(lua_State *L, int index)
{
	int error;
	return internal_get_unit(L, index, false, error);
}

static void unit_show_error(lua_State *L, int index, int error)
{
	switch(error) {
		case LU_NOT_UNIT:
			luaL_typerror(L, index, "unit");
		case LU_NOT_VALID:
			luaL_argerror(L, index, "unit not found");
		case LU_NOT_ON_MAP:
			luaL_argerror(L, index, "unit not found on map");
	}
}

unit_ptr luaW_checkunit_ptr(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	unit_show_error(L, index, error);
	return lu->get_shared();
}

unit& luaW_checkunit(lua_State *L, int index, bool only_on_map)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, only_on_map, error);
	unit_show_error(L, index, error);
	return *lu->get();
}

lua_unit* luaW_checkunit_ref(lua_State *L, int index)
{
	int error;
	lua_unit* lu = internal_get_unit(L, index, false, error);
	unit_show_error(L, index, error);
	return lu;
}

void lua_unit::setmetatable(lua_State *L)
{
	lua_pushlightuserdata(L, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
}

lua_unit* luaW_pushlocalunit(lua_State *L, unit& u)
{
	lua_unit* res = new(L) lua_unit(u);
	lua_unit::setmetatable(L);
	return res;
}
