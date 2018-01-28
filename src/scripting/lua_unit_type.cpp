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
#include "scripting/lua_kernel_base.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/push_check.hpp"
#include "units/types.hpp"

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "lua/lua.h"
#include "lua/lauxlib.h"

/**
 * Implementation for a Lua table of unit types.
 */

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)

// Forward declarations
static void push_unit_type(lua_State* L, const unit_type& ut, const std::string& id);

static void push_string_vec(lua_State* L, const std::vector<std::string>& vec, const std::string& key)
{
	lua_newtable(L);

	for(unsigned int i = 0; i < vec.size(); ++i) {
		lua_pushlstring(L, vec[i].data(), vec[i].length());
		lua_seti(L, -2, i);
	}

	lua_getfield(L, LUA_REGISTRYINDEX, lua_kernel_base::read_only);
	lua_setmetatable(L, -2);

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

	lua_getfield(L, LUA_REGISTRYINDEX, lua_kernel_base::read_only);
	lua_setmetatable(L, -2);

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

	lua_getfield(L, LUA_REGISTRYINDEX, lua_kernel_base::read_only);
	lua_setmetatable(L, -2);

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

	lua_getfield(L, LUA_REGISTRYINDEX, lua_kernel_base::read_only);
	lua_setmetatable(L, -2);

	lua_setfield(L, -2, id.c_str());
}

namespace lua_unit_type {
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

		lua_getfield(L, LUA_REGISTRYINDEX, lua_kernel_base::read_only);
		lua_setmetatable(L, -2);

		lua_setfield(L, -2, "unit_types");
		lua_pop(L, 1);
	}
}

const unit_type* luaW_tounittype(lua_State* L, int idx)
{
	lua_getfield(L, idx, "id");
	if(lua_isstring(L, -1)) {
		std::string id = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		auto ut = unit_types.types().find(id);
		if(ut != unit_types.types().end()) {
			return &ut->second;
		}
	} else {
		lua_pop(L, 1);
	}

	return nullptr;
}

const unit_type& luaW_checkunittype(lua_State* L, int idx)
{
	const unit_type* ut = luaW_tounittype(L, idx);
	if(ut != nullptr) {
		return *ut;
	} else {
		throw std::invalid_argument("Not a unit type");
	}
}
