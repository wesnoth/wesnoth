/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
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
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/push_check.hpp"
#include "units/types.hpp"

#include <string>
#include <cstring>

#include "lua/lua.h"
#include "lua/lauxlib.h"

/**
 * Implementation for a lua reference to a unit_type.
 */

// Registry key
static const char UnitType[] = "unit type";
static const char UnitTypeTable[] = "unit types";

/**
 * Gets some data on a unit type (__index metamethod).
 * - Arg 1: table containing an "id" field.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_unit_type_get(lua_State *L)
{
	const unit_type& ut = luaW_checkunittype(L, 1);
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_tstring_attrib("name", ut.type_name());
	return_string_attrib("id", ut.id());
	return_string_attrib("alignment", ut.alignment().to_string());
	return_string_attrib("race", ut.race_id());
	return_string_attrib("image", ut.image());
	return_string_attrib("icon", ut.icon());
	return_int_attrib("max_hitpoints", ut.hitpoints());
	return_int_attrib("max_moves", ut.movement());
	return_int_attrib("max_experience", ut.experience_needed());
	return_int_attrib("cost", ut.cost());
	return_int_attrib("level", ut.level());
	return_int_attrib("recall_cost", ut.recall_cost());
	return_vector_string_attrib("advances_to", ut.advances_to());
	return_vector_string_attrib("advances_from", ut.advances_from());
	return_cfgref_attrib("__cfg", ut.get_cfg());
	if (strcmp(m, "traits") == 0) {
		lua_newtable(L);
		for (const config& trait : ut.possible_traits()) {
			const std::string& id = trait["id"];
			lua_pushlstring(L, id.c_str(), id.length());
			luaW_pushconfig(L, trait);
			lua_rawset(L, -3);
		}
		return 1;
	}
	if (strcmp(m, "abilities") == 0) {
		lua_push(L, ut.get_ability_list());
		return 1;
	}
	if (strcmp(m, "attacks") == 0) {
		push_unit_attacks_table(L, 1);
		return 1;
	}
	// TODO: Should this only exist for base units?
	if(strcmp(m, "variations") == 0) {
		*new(L) const unit_type* = &ut;
		luaL_setmetatable(L, UnitTypeTable);
		return 1;
	}
	return 0;
}

static int impl_unit_type_equal(lua_State* L)
{
	const unit_type& ut1 = luaW_checkunittype(L, 1);
	if(const unit_type* ut2 = luaW_tounittype(L, 2)) {
		lua_pushboolean(L, &ut1 == ut2);
	} else {
		lua_pushboolean(L, false);
	}
	return 1;
}

static int impl_unit_type_lookup(lua_State* L)
{
	std::string id = luaL_checkstring(L, 2);
	const unit_type* ut;
	if(const unit_type* base = *static_cast<const unit_type**>(luaL_testudata(L, 1, UnitTypeTable))) {
		if(id == "male" || id == "female") {
			ut = &base->get_gender_unit_type(id);
		} else {
			ut = &base->get_variation(id);
		}
	} else {
		ut = unit_types.find(id);
	}
	if(ut) {
		luaW_pushunittype(L, *ut);
		return 1;
	}
	return 0;
}

static int impl_unit_type_new(lua_State* L)
{
	// This could someday become a hook to construct new unit types on the fly?
	// For now though, it's just an error
	lua_pushstring(L, "unit_types table is read-only");
	return lua_error(L);
}

static int impl_unit_type_count(lua_State* L)
{
	lua_pushnumber(L, unit_types.types().size());
	return 1;
}

static int impl_unit_type_next(lua_State* L)
{
	const unit_type* base = *static_cast<const unit_type**>(luaL_checkudata(L, 1, UnitTypeTable));
	const auto& unit_map = base ? base->variation_types() : unit_types.types();
	auto it = unit_map.end();
	if(lua_isnoneornil(L, 2)) {
		if(base) {
			if(base->has_gender_variation(unit_race::MALE)) {
				lua_pushstring(L, "male");
				luaW_pushunittype(L, base->get_gender_unit_type(unit_race::MALE));
				return 2;
			} else if(base->has_gender_variation(unit_race::FEMALE)) {
				lua_pushstring(L, "female");
				luaW_pushunittype(L, base->get_gender_unit_type(unit_race::FEMALE));
				return 2;
			}
		}
		it = unit_map.begin();
	} else {
		const std::string id = luaL_checkstring(L, 2);
		if(base) {
			if(id == "male" && base->has_gender_variation(unit_race::FEMALE)) {
				lua_pushstring(L, "female");
				luaW_pushunittype(L, base->get_gender_unit_type(unit_race::FEMALE));
				return 2;
			} else if(id == "male" || id == "female") {
				it = unit_map.begin();
			}
		}
		if(it == unit_map.end()) {
			it = unit_map.find(id);
		}
		if(it == unit_map.end()) {
			return 0;
		}
		++it;
	}
	if (it == unit_map.end()) {
		return 0;
	}
	lua_pushlstring(L, it->first.c_str(), it->first.size());
	luaW_pushunittype(L, it->second);
	return 2;
}

static int impl_unit_type_pairs(lua_State* L) {
	lua_pushcfunction(L, &impl_unit_type_next);
	lua_pushvalue(L, -2);
	lua_pushnil(L);
	return 3;
}

namespace lua_unit_type {
	std::string register_metatable(lua_State * L)
	{
		luaL_newmetatable(L, UnitType);

		lua_pushcfunction(L, impl_unit_type_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_type_equal);
		lua_setfield(L, -2, "__eq");
		lua_pushstring(L, UnitType);
		lua_setfield(L, -2, "__metatable");

		return "Adding unit type metatable...\n";
	}

	std::string register_table(lua_State* L)
	{
		lua_getglobal(L, "wesnoth");
		*new(L) unit_type* = nullptr;
		luaL_newmetatable(L, UnitTypeTable);
		lua_pushcfunction(L, impl_unit_type_lookup);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_type_new);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_unit_type_count);
		lua_setfield(L, -2, "__len");
		lua_pushcfunction(L, impl_unit_type_pairs);
		lua_setfield(L, -2, "__pairs");
		lua_pushstring(L, UnitTypeTable);
		lua_setfield(L, -2, "__metatable");
		lua_setmetatable(L, -2);
		lua_setfield(L, -2, "unit_types");
		lua_pop(L, 1);

		return "Adding unit_types table...\n";
	}
}

void luaW_pushunittype(lua_State *L, const unit_type& ut)
{
	*static_cast<const unit_type**>(lua_newuserdata(L, sizeof(unit_type*))) = &ut;
	luaL_setmetatable(L, UnitType);
}

const unit_type* luaW_tounittype(lua_State* L, int idx)
{
	if(void* p = luaL_testudata(L, idx, UnitType)) {
		return *static_cast<const unit_type**>(p);
	}
	return nullptr;
}

const unit_type& luaW_checkunittype(lua_State* L, int idx)
{
	return **static_cast<const unit_type**>(luaL_checkudata(L, idx, UnitType));
}
