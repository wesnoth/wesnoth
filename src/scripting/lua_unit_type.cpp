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

#include "scripting/lua_unit_type.hpp"

#include "scripting/lua_attributes.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/push_check.hpp"
#include "units/types.hpp"

#include <string>
#include <cstring>


/**
 * Implementation for a lua reference to a unit_type.
 */

// Registry key
static const char UnitType[] = "unit type";
static const char UnitTypeTable[] = "unit types";

#define UNIT_TYPE_GETTER(name, type) LATTR_GETTER(name, type, unit_type, ut)
#define UNIT_TYPE_VALID(name) LATTR_VALID(name, unit_type, ut)
luaW_Registry unitTypeReg{UnitType};

template<> struct lua_object_traits<unit_type> {
	inline static auto metatable = UnitType;
	inline static const unit_type& get(lua_State* L, int n) {
		return luaW_checkunittype(L, n);
	}
};

UNIT_TYPE_GETTER("name", t_string) {
	return ut.type_name();
}

UNIT_TYPE_GETTER("id", std::string) {
	return ut.id();
}

UNIT_TYPE_GETTER("alignment", std::string) {
	return unit_alignments::get_string(ut.alignment());
}

UNIT_TYPE_GETTER("race", std::string) {
	return ut.race_id();
}

UNIT_TYPE_GETTER("image", std::string) {
	return ut.image();
}

UNIT_TYPE_GETTER("icon", std::string) {
	return ut.icon();
}

UNIT_TYPE_GETTER("profile", std::string) {
	return ut.big_profile();
}

UNIT_TYPE_GETTER("small_profile", std::string) {
	return ut.small_profile();
}

UNIT_TYPE_GETTER("max_hitpoints", int) {
	return ut.hitpoints();
}

UNIT_TYPE_GETTER("max_moves", int) {
	return ut.movement();
}

UNIT_TYPE_GETTER("max_experience", int) {
	return ut.experience_needed();
}

UNIT_TYPE_GETTER("cost", int) {
	return ut.cost();
}

UNIT_TYPE_GETTER("level", int) {
	return ut.level();
}

UNIT_TYPE_GETTER("recall_cost", int) {
	return ut.recall_cost();
}

UNIT_TYPE_GETTER("advances_to", std::vector<std::string>) {
	return ut.advances_to();
}

UNIT_TYPE_GETTER("advances_from", std::vector<std::string>) {
	return ut.advances_from();
}

UNIT_TYPE_GETTER("__cfg", config) {
	return ut.get_cfg();
}

using traits_map = std::map<std::string,config>;
UNIT_TYPE_GETTER("traits", traits_map) {
	traits_map traits;
	for (const config& trait : ut.possible_traits()) {
		traits.emplace(trait["id"], trait);
	}
	return traits;
}

UNIT_TYPE_GETTER("abilities", std::vector<std::string>) {
	return ut.get_ability_list();
}

UNIT_TYPE_GETTER("attacks", lua_index_raw) {
	(void)ut;
	push_unit_attacks_table(L, 1);
	return lua_index_raw(L);
}

UNIT_TYPE_VALID("variations") {
	return ut.variation_id().empty();
}

UNIT_TYPE_GETTER("variations", lua_index_raw) {
	// TODO: Should this only exist for base units?
	*new(L) const unit_type* = &ut;
	luaL_setmetatable(L, UnitTypeTable);
	return lua_index_raw(L);
}

/**
 * Gets some data on a unit type (__index metamethod).
 * - Arg 1: table containing an "id" field.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_unit_type_get(lua_State *L)
{
	return unitTypeReg.get(L);
}

/**
 * Gets a list of data on a unit type (__dir metamethod).
 * - Ret 1: a list of attributes.
 */
static int impl_unit_type_dir(lua_State *L)
{
	return unitTypeReg.dir(L);
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

static int impl_unit_type_list(lua_State* L) {
	std::vector<std::string> keys;
	if(const unit_type* base = *static_cast<const unit_type**>(luaL_testudata(L, 1, UnitTypeTable))) {
		keys = base->variations();
		if(base->has_gender_variation(unit_race::MALE)) {
			keys.push_back("male");
		}
		if(base->has_gender_variation(unit_race::FEMALE)) {
			keys.push_back("female");
		}
	} else {
		keys.reserve(unit_types.types().size());
		for(const auto& p : unit_types.types()) {
			keys.push_back(p.first);
		}
	}
	lua_push(L, keys);
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
	if(!base) {
		// Make sure the unit is built.
		unit_types.build_unit_type(it->second, unit_type::FULL);
	}
	return 2;
}

static int impl_unit_type_pairs(lua_State* L) {
	lua_pushcfunction(L, &impl_unit_type_next);
	lua_pushvalue(L, -2);
	lua_pushnil(L);
	return 3;
}

/**
 * Turns a lua proxy unit type to string. (__tostring metamethod)
 */
static int impl_unit_type_tostring(lua_State* L)
{
	const unit_type& ut = luaW_checkunittype(L, 1);
	std::ostringstream str;

	str << "unit type: <" << ut.id() << '>';

	lua_push(L, str.str());
	return 1;
}

namespace lua_unit_type {
	std::string register_metatable(lua_State * L)
	{
		luaL_newmetatable(L, UnitType);

		lua_pushcfunction(L, impl_unit_type_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_type_dir);
		lua_setfield(L, -2, "__dir");
		lua_pushcfunction(L, impl_unit_type_tostring);
		lua_setfield(L, -2, "__tostring");
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
		lua_pushcfunction(L, impl_unit_type_list);
		lua_setfield(L, -2, "__dir");
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
	*static_cast<const unit_type**>(lua_newuserdatauv(L, sizeof(unit_type*), 0)) = &ut;
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
