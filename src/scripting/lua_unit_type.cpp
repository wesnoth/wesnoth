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

#include "log.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/push_check.hpp"
#include "units/types.hpp"

#include <chrono>
#include <cstring>
#include <string>
#include <vector>

#include "lua/lua.h"
#include "lua/lauxlib.h"

/**
 * Implementation for a Lua table of unit types.
 */

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)

// Registry key
static const char UnitType[] = "unit type";
static const char UnitTypeTable[] = "unit types";

// Forward declarations
void push_unit_type(lua_State* L, const unit_type& ut, const std::string& id);

static void push_string_vec(lua_State* L, const std::vector<std::string>& vec, const std::string& key)
{
	lua_newtable(L);

	for(unsigned int i = 0; i < vec.size(); ++i) {
		lua_pushlstring(L, vec[i].data(), vec[i].length());
		lua_seti(L, -2, i);
	}

	lua_setfield(L, -2, key.c_str());
}

static void push_traits(lua_State* L, const unit_type& ut)
{
	lua_newtable(L);

	for(const config& trait : ut.possible_traits()) {
		const std::string& id = trait["id"];
		lua_pushlstring(L, id.c_str(), id.length());
		luaW_pushconfig(L, trait);
		lua_rawset(L, -3);
	}

	lua_setfield(L, -2, "traits");
}

// Requires that the unit type table is at the top of the Lua stack.
static void push_variations_and_genders(lua_State* L, const unit_type& ut)
{
	lua_newtable(L);

	for(const auto& v : ut.variation_types()) {
		push_unit_type(L, v.second, v.first);
	}

	for(unit_race::GENDER g : ut.genders()) {
		const unit_type& gender_unit_type = ut.get_gender_unit_type(g);
		if(&gender_unit_type == &ut) {
			// Push a reference to the unit type table.
			lua_pushvalue(L, -2);
			lua_setfield(L, -2, gender_string(g).c_str());
		} else {
			push_unit_type(L, gender_unit_type, gender_string(g));
		}
	}

	lua_setfield(L, -2, "variations");
}

static void push_unit_type(lua_State* L, const unit_type& ut, const std::string& id)
{
	lua_newtable(L);

	luaW_pushtstring(L, ut.type_name());
	lua_setfield(L, -2, "name");
	lua_pushlstring(L, ut.id().data(), ut.id().length());
	lua_setfield(L, -2, "id");
	lua_pushstring(L, ut.alignment().to_cstring());
	lua_setfield(L, -2, "alignment");
	lua_pushlstring(L, ut.race_id().data(), ut.race_id().length());
	lua_setfield(L, -2, "race");
	lua_pushlstring(L, ut.image().data(), ut.image().length());
	lua_setfield(L, -2, "image");
	lua_pushlstring(L, ut.icon().data(), ut.icon().length());
	lua_setfield(L, -2, "icon");
	lua_pushinteger(L, ut.hitpoints());
	lua_setfield(L, -2, "max_hitpoints");
	lua_pushinteger(L, ut.movement());
	lua_setfield(L, -2, "max_moves");
	lua_pushinteger(L, ut.experience_needed());
	lua_setfield(L, -2, "max_experience");
	lua_pushinteger(L, ut.cost());
	lua_setfield(L, -2, "cost");
	lua_pushinteger(L, ut.level());
	lua_setfield(L, -2, "level");
	lua_pushinteger(L, ut.recall_cost());
	lua_setfield(L, -2, "recall_cost");
	push_string_vec(L, ut.advances_to(), "advances_to");
	push_string_vec(L, ut.advances_from(), "advances_from");
	luaW_pushconfig(L, ut.get_cfg());
	lua_setfield(L, -2, "__cfg");
	push_traits(L, ut);
	push_string_vec(L, ut.get_ability_list(), "abilities");
	push_unit_attacks_table(L, -1);
	lua_setfield(L, -2, "attacks");
	push_variations_and_genders(L, ut);

	// TODO: write-protect the tables

	lua_setfield(L, -2, id.c_str());
}

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
	auto unit_map = base ? base->variation_types() : unit_types.types();
	decltype(unit_map)::const_iterator it = unit_map.end();
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

	void register_table(lua_State* L)
	{
		lua_getglobal(L, "wesnoth");

		lua_newtable(L);

		std::chrono::high_resolution_clock::time_point start, end;
		start = std::chrono::high_resolution_clock::now();

		for(const auto& ut : unit_types.types()) {
			push_unit_type(L, ut.second, ut.first);
		}

		end = std::chrono::high_resolution_clock::now();

		LOG_LUA << "wesnoth.unit_types constructed in " <<
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";

		lua_setfield(L, -2, "unit_types");
		lua_pop(L, 1);
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
	return **static_cast<const unit_type**>(luaL_checkudata(L, idx, UnitType));;
}
