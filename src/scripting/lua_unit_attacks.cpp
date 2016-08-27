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
#include "scripting/lua_unit_type.hpp"
#include "units/unit.hpp"
#include "units/attack_type.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"                    // for lua_State, lua_settop, etc

static const char uattacksKey[] = "unit attacks table";
static const char uattackKey[] = "unit attack";

struct attack_ref {
	bool read_only; // true if it's a unit type attack
	int owner_ref;
	attack_type* attack;
	lua_unit* owner(lua_State* L) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, owner_ref);
		lua_unit* u = luaW_tounit_ref(L, -1);
		lua_pop(L, 1);
		return u;
	}
};

void push_unit_attacks_table(lua_State* L, int idx)
{
	idx = lua_absindex(L, idx);
	lua_createtable(L, 1, 0);
	lua_pushvalue(L, idx);
	// hack: store the unit_type at 0 because we want positive indices to refer to the attacks.
	lua_rawseti(L, -2, 0);
	luaL_setmetatable(L, uattacksKey);
}

void luaW_pushweapon(lua_State* L, const attack_type& weapon, int owner_idx)
{
	owner_idx = lua_absindex(L, owner_idx);
	attack_ref* atk = new(L) attack_ref {!owner_idx, 0, const_cast<attack_type*>(&weapon)};
	luaL_setmetatable(L, uattackKey);
	if(owner_idx) {
		lua_pushvalue(L, owner_idx);
		atk->owner_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
}

static attack_ref& luaW_checkweapon_ref(lua_State* L, int idx)
{
	return *static_cast<attack_ref*>(luaL_checkudata(L, idx, uattackKey));
}

attack_type* luaW_toweapon(lua_State* L, int idx)
{
	if(void* p = luaL_testudata(L, idx, uattackKey)) {
		return static_cast<attack_ref*>(p)->attack;
	}
	return nullptr;
}

attack_type& luaW_checkweapon(lua_State* L, int idx)
{
	return *luaW_checkweapon_ref(L, idx).attack;
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
	lua_rawgeti(L, 1, 0);
	lua_unit* u = luaW_tounit_ref(L, -1);
	const unit_type* ut = luaW_tounittype(L, -1);
	if((!u || !u->get()) && !ut) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	const attack_type* attack = nullptr;
	const std::vector<attack_type>& attacks = u ? u->get()->attacks() : ut->attacks();
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

	luaW_pushweapon(L, *attack, u ? -1 : 0);
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
	lua_rawgeti(L, 1, 0);
	const unit* u = luaW_tounit(L, -1);
	const unit_type* ut = luaW_tounittype(L, -1);
	if(!u && !ut) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	lua_pushinteger(L, (u ? u->attacks() : ut->attacks()).size());
	return 1;
}

/**
 * Gets a property of a units attack (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id. and a string identyfying the attack.
 * - Arg 2: string
 * - Ret 1:
 */
static int impl_unit_attack_get(lua_State *L)
{
	attack_ref& atk_ref = luaW_checkweapon_ref(L, 1);
	lua_unit* owner = atk_ref.owner(L);
	if(owner && !owner->get()) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	attack_type& attack = *atk_ref.attack;
	char const *m = luaL_checkstring(L, 2);
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

/**
 * Gets a property of a units attack (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id. and a string identyfying the attack.
 * - Arg 2: string
 * - Ret 1:
 */
static int impl_unit_attack_set(lua_State *L)
{
	attack_ref& atk_ref = luaW_checkweapon_ref(L, 1);
	lua_unit* owner = atk_ref.owner(L);
	if(owner && !owner->get()) {
		return luaL_argerror(L, 1, "unknown unit");
	} else if(!owner) {
		return luaL_argerror(L, 1, "unit type attacks are immutable");
	}
	unit* u = owner ? owner->get() : nullptr;
	(void)u;
	attack_type& attack = *atk_ref.attack;
	char const *m = luaL_checkstring(L, 2);
	modify_tstring_attrib("description", attack.set_name(value));
	modify_string_attrib("name", attack.set_id(value));
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

	std::string err_msg = "unknown modifiable property of attack: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

static int impl_unit_attack_equal(lua_State* L)
{
	const attack_type& ut1 = luaW_checkweapon(L, 1);
	if(const attack_type* ut2 = luaW_toweapon(L, 2)) {
		lua_pushboolean(L, &ut1 == ut2);
	} else {
		lua_pushboolean(L, false);
	}
	return 1;
}

static int impl_unit_attack_collect(lua_State* L)
{
	attack_ref* atk = static_cast<attack_ref*>(luaL_checkudata(L, 1, uattackKey));
	if(!atk->read_only) {
		luaL_unref(L, LUA_REGISTRYINDEX, atk->owner_ref);
	}
	atk->~attack_ref();
	return 0;
}

namespace lua_units {
	std::string register_attacks_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the unit attacks metatable.
		cmd_out << "Adding unit attacks metatable...\n";

		luaL_newmetatable(L, uattacksKey);
		lua_pushcfunction(L, impl_unit_attacks_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_attacks_len);
		lua_setfield(L, -2, "__len");
		lua_pushstring(L, uattacksKey);
		lua_setfield(L, -2, "__metatable");

		// Create the unit attack metatable
		luaL_newmetatable(L, uattackKey);
		lua_pushcfunction(L, impl_unit_attack_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_attack_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_unit_attack_equal);
		lua_setfield(L, -2, "__eq");
		lua_pushcfunction(L, impl_unit_attack_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushstring(L, uattackKey);
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}
