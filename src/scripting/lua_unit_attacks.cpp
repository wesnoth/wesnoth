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

#include "lua_unit_attacks.hpp"

#include "scripting/lua_common.hpp"
#include "scripting/lua_unit.hpp"
#include "units/unit.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"                    // for lua_State, lua_settop, etc

// These are arrays so we can use sizeof
static const char uattacksKey[] = "unit attacks table";
static const char uattackKey[] = "unit attack";

void push_unit_attacks_table(lua_State* L, int idx)
{
	lua_createtable(L, 1, 0);
	lua_pushvalue(L, idx);
	// hack: store the unit_type at -1 because we want positive indices to refer to the attacks.
	lua_rawseti(L, -2, -1);
	lua_pushlstring(L, uattacksKey, sizeof(uattacksKey));
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
}

/**
 * Gets the attacks of a unit or unit type (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit or unit type.
 * - Arg 2: index (int) or id (string) identifying a particular attack.
 * - Ret 1: the unit's attacks.
 */
static int impl_unit_attacks_get(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attacks");
	}
	lua_rawgeti(L, 1, -1);
	const unit* u = luaW_tounit(L, -1);
	const unit_type* ut = static_cast<const unit_type*>(luaL_testudata(L, -1, "unit type"));
	if(!u && !ut) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	const attack_type* attack = nullptr;
	const std::vector<attack_type>& attacks = u ? u->attacks() : ut->attacks();
	if(!lua_isnumber(L,2)) {
		std::string attack_id = luaL_checkstring(L, 2);
		for(const attack_type& at : attacks) {
			if(at.id() == attack_id) {
				attack = &at;
				break;
			}
		}
		if(attack == nullptr) {
			//return nil on invalid index, just like lua tables do.
			return 0;
		}
	} else {
		size_t index = luaL_checkinteger(L, 2) - 1;
		if(index >= attacks.size()) {
			//return nil on invalid index, just like lua tables do.
			return 0;
		}
		attack = &attacks[index];
	}

	// stack { lua_unit }, id/index, lua_unit
	lua_createtable(L, 2, 0);
	// stack { lua_unit }, id/index, lua_unit, table
	lua_pushvalue(L, -2);
	// stack { lua_unit }, id/index, lua_unit, table, lua_unit
	lua_rawseti(L, -2, 1);
	// stack { lua_unit }, id/index, lua_unit, table
	lua_pushstring(L, attack->id().c_str());
	// stack { lua_unit }, id/index, lua_unit, table, attack id
	lua_rawseti(L, -2, 2);
	// stack { lua_unit }, id/index, lua_unit, table
	lua_pushlstring(L, uattackKey, sizeof(uattackKey));
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Counts the attacks of a unit (__len metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Ret 1: size of unit attacks vector.
 */
static int impl_unit_attacks_len(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attacks");
	}
	lua_rawgeti(L, 1, -1);
	const unit* u = luaW_tounit(L, -1);
	const unit_type* ut = static_cast<const unit_type*>(luaL_testudata(L, -1, "unit type"));
	if(!u && !ut) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	lua_pushinteger(L, (u ? u->attacks() : ut->attacks()).size());
	return 1;
}

/**
 * Gets a propoerty of a units attack (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id. and a string identyfying the attack.
 * - Arg 2: string
 * - Ret 1:
 */
static int impl_unit_attack_get(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attack");
	}
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	const unit_type* ut = static_cast<const unit_type*>(luaL_testudata(L, -1, "unit type"));
	if(!u && !ut) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	lua_rawgeti(L, 1, 2);
	std::string attack_id = luaL_checkstring(L, -1);
	char const *m = luaL_checkstring(L, 2);
	for(const attack_type& attack : u ? u->attacks() : ut->attacks()) {
		if(attack.id() == attack_id) {
			return_string_attrib("description", attack.name());
			return_string_attrib("name", attack.id());
			return_string_attrib("type", attack.type());
			return_string_attrib("icon", attack.icon());
			return_string_attrib("range", attack.range());
			return_int_attrib("damage", attack.damage());
			return_int_attrib("number", attack.num_attacks());
			return_int_attrib("attack_weight", attack.attack_weight());
			return_int_attrib("defense_weight", attack.defense_weight());
			return_int_attrib("accuracy", attack.accuracy());
			return_int_attrib("movement_used", attack.movement_used());
			return_int_attrib("parry", attack.parry());
			return_cfgref_attrib("specials", attack.specials());
			return_cfgref_attrib("__cfg", attack.to_config());
			std::string err_msg = "unknown property of attack: ";
			err_msg += m;
			return luaL_argerror(L, 2, err_msg.c_str());
		}
	}
	return luaL_argerror(L, 1, "invalid attack id");
}

/**
 * Gets a propoerty of a units attack (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id. and a string identyfying the attack.
 * - Arg 2: string
 * - Ret 1:
 */
static int impl_unit_attack_set(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attack");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	const unit_type* ut = static_cast<const unit_type*>(luaL_testudata(L, -1, "unit type"));
	if(!u && !ut) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	lua_rawgeti(L, 1, 2);
	std::string attack_id = luaL_checkstring(L, -1);
	char const *m = luaL_checkstring(L, 2);
	for(attack_type& attack : u ? u->attacks() : ut->attacks()) {
		if(attack.id() == attack_id) {
			modify_tstring_attrib("description", attack.set_name(value));
			// modify_string_attrib("name", attack.set_id(value));
			modify_string_attrib("type", attack.set_type(value));
			modify_string_attrib("icon", attack.set_icon(value));
			modify_string_attrib("range", attack.set_range(value));
			modify_int_attrib("damage", attack.set_damage(value));
			modify_int_attrib("number", attack.set_num_attacks(value));
			modify_int_attrib("attack_weight", attack.set_attack_weight(value));
			modify_int_attrib("defense_weight", attack.set_defense_weight(value));
			modify_int_attrib("accuracy", attack.set_accuracy(value));
			modify_int_attrib("movement_used", attack.set_movement_used(value));
			modify_int_attrib("parry", attack.set_parry(value));

			if(strcmp(m, "specials") == 0) {
				attack.set_specials(luaW_checkconfig(L, 3));
				return 0;
			}
			return_cfgref_attrib("specials", attack.specials());
			std::string err_msg = "unknown modifyable property of attack: ";
			err_msg += m;
			return luaL_argerror(L, 2, err_msg.c_str());
		}
	}
	return luaL_argerror(L, 1, "invalid attack id");
}

namespace lua_units {
	std::string register_attacks_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the unit attacks metatable.
		cmd_out << "Adding unit attacks metatable...\n";

		lua_pushlstring(L, uattacksKey, sizeof(uattacksKey));
		lua_createtable(L, 0, 3);
		lua_pushcfunction(L, impl_unit_attacks_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_attacks_len);
		lua_setfield(L, -2, "__len");
		lua_pushstring(L, "unit attacks");
		lua_setfield(L, -2, "__metatable");
		lua_rawset(L, LUA_REGISTRYINDEX);

		// Create the unit attack metatable
		lua_pushlstring(L, uattackKey, sizeof(uattackKey));
		lua_createtable(L, 0, 3);
		lua_pushcfunction(L, impl_unit_attack_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_attack_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "unit attack");
		lua_setfield(L, -2, "__metatable");
		lua_rawset(L, LUA_REGISTRYINDEX);

		return cmd_out.str();
	}
}
