/*
   Copyright (C) 2009 - 2017 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "scripting/lua_unit_attacks.hpp"

#include "scripting/lua_common.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_type.hpp"
#include "units/unit.hpp"
#include "units/attack_type.hpp"
#include "utils/const_clone.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"                    // for lua_State, lua_settop, etc

#include <type_traits>

static const char uattacksKey[] = "unit attacks table";
static const char uattackKey[] = "unit attack";

struct attack_ref {
	attack_ptr attack;
	const_attack_ptr cattack;
	attack_ref(attack_ptr atk) : attack(atk), cattack(atk) {}
	attack_ref(const_attack_ptr atk) : cattack(atk) {}
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

void luaW_pushweapon(lua_State* L, attack_ptr weapon)
{
	new(L) attack_ref(weapon);
	luaL_setmetatable(L, uattackKey);
}

void luaW_pushweapon(lua_State* L, const_attack_ptr weapon)
{
	new(L) attack_ref(weapon);
	luaL_setmetatable(L, uattackKey);
}

static attack_ref& luaW_checkweapon_ref(lua_State* L, int idx)
{
	return *static_cast<attack_ref*>(luaL_checkudata(L, idx, uattackKey));
}

const_attack_ptr luaW_toweapon(lua_State* L, int idx)
{
	if(void* p = luaL_testudata(L, idx, uattackKey)) {
		return static_cast<attack_ref*>(p)->cattack;
	}
	return nullptr;
}

attack_type& luaW_checkweapon(lua_State* L, int idx)
{
	attack_ref& atk = luaW_checkweapon_ref(L, idx);
	if(!atk.attack) {
		luaL_argerror(L, idx, "attack is read-only");
	}
	return *atk.attack;
}

template<typename T>
using attack_ptr_in = std::shared_ptr<typename utils::const_clone<attack_type, typename std::remove_pointer<T>::type>::type>;

// Note that these two templates are designed on the assumption that T is either unit or unit_type
template<typename T>
auto find_attack(T* u, const std::string& id) -> attack_ptr_in<T>
{
	auto attacks = u->attacks();
	for(auto at = attacks.begin(); at != attacks.end(); ++at) {
		if(at->id() == id) {
			return *at.base();
		}
	}
	return nullptr;
}

template<typename T>
auto find_attack(T* u, size_t i) -> attack_ptr_in<T>
{
	auto attacks = u->attacks();
	if(i < static_cast<size_t>(attacks.size())) {
		auto iter = attacks.begin();
		iter += i;
		return *iter.base();
	}
	return nullptr;
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
		return luaW_type_error(L, 1, "unit attacks");
	}
	lua_rawgeti(L, 1, 0);
	lua_unit* lu = luaW_tounit_ref(L, -1);
	const unit_type* ut = luaW_tounittype(L, -1);
	if(lu && lu->get()) {
		unit* u = lu->get();
		attack_ptr atk = lua_isnumber(L, 2) ? find_attack(u, luaL_checkinteger(L, 2) - 1) : find_attack(u, luaL_checkstring(L, 2));
		luaW_pushweapon(L, atk);
	} else if(ut) {
		const_attack_ptr atk = lua_isnumber(L, 2) ? find_attack(ut, luaL_checkinteger(L, 2) - 1) : find_attack(ut, luaL_checkstring(L, 2));
		luaW_pushweapon(L, atk);
	} else {
		return luaL_argerror(L, 1, "unit not found");
	}
	return 1;
}

static attack_itors::iterator get_attack_iter(unit& u, attack_ptr atk)
{
	// This is slightly inefficient since it walks the attack list a second time...
	return std::find_if(u.attacks().begin(), u.attacks().end(), [&atk](const attack_type& atk2) {
		return &atk2 == atk.get();
	});
}

static int impl_unit_attacks_set(lua_State* L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit attacks");
	}
	lua_rawgeti(L, 1, 0);
	const unit_type* ut = luaW_tounittype(L, -1);
	if(ut) {
		return luaL_argerror(L, 1, "unit type attack table is immutable");
	}

	unit& u = luaW_checkunit(L, -1);
	attack_ptr atk = lua_isnumber(L, 2) ? find_attack(&u, luaL_checkinteger(L, 2) - 1) : find_attack(&u, luaL_checkstring(L, 2));
	if(lua_isnumber(L, 2) && lua_tonumber(L, 2) - 1 > u.attacks().size()) {
		return luaL_argerror(L, 2, "attack can only be added at the end of the list");
	}

	if(lua_isnil(L, 3)) {
		// Delete the attack
		u.remove_attack(atk);
		return 0;
	}

	auto iter = get_attack_iter(u, atk), end = u.attacks().end();
	if(const_attack_ptr atk2 = luaW_toweapon(L, 3)) {
		if(iter == end) {
			atk = u.add_attack(end, *atk2);
		} else {
			iter.base()->reset(new attack_type(*atk2));
			atk = *iter.base();
		}
	} else {
		config cfg = luaW_checkconfig(L, 3);
		if(iter == end) {
			atk = u.add_attack(end, cfg);
		} else {
			iter.base()->reset(new attack_type(cfg));
			atk = *iter.base();
		}
	}
	if(!lua_isnumber(L, 2)) {
		atk->set_id(lua_tostring(L, 2));
	}
	return 0;
}

/**
 * Counts the attacks of a unit (__len metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Ret 1: size of unit attacks vector.
 */
static int impl_unit_attacks_len(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit attacks");
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

static int impl_unit_attacks_next(lua_State *L)
{
	lua_len(L, 1);
	int n = luaL_checknumber(L, 2) + 1;
	int max_n = luaL_checknumber(L, -1);
	if(n > max_n) {
		return 0;
	}
	lua_pushnumber(L, n);
	lua_pushvalue(L, -1);
	lua_gettable(L, 1);
	return 2;
}

static int impl_unit_attacks_iter(lua_State* L)
{
	lua_pushcfunction(L, impl_unit_attacks_next);
	lua_pushvalue(L, 1);
	lua_pushnumber(L, 0);
	return 3;
}

/**
 * Gets a property of a units attack (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id. and a string identifying the attack.
 * - Arg 2: string
 * - Ret 1:
 */
static int impl_unit_attack_get(lua_State *L)
{
	attack_ref& atk_ref = luaW_checkweapon_ref(L, 1);
	const attack_type& attack = *atk_ref.cattack;
	char const *m = luaL_checkstring(L, 2);
	return_bool_attrib("read_only", atk_ref.attack == nullptr);
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
	if(luaW_getmetafield(L, 1, m)) {
		return 1;
	}
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
	attack_type& attack = luaW_checkweapon(L, 1);
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
	const_attack_ptr ut1 = luaW_toweapon(L, 1);
	const_attack_ptr ut2 = luaW_toweapon(L, 2);
	lua_pushboolean(L, ut1 == ut2);
	return 1;
}

static int impl_unit_attack_match(lua_State* L)
{
	const_attack_ptr atk = luaW_toweapon(L, 1);
	config cfg = luaW_checkconfig(L, 2);
	lua_pushboolean(L, atk->matches_filter(cfg));
	return 1;
}

static int impl_unit_attack_collect(lua_State* L)
{
	attack_ref* atk = static_cast<attack_ref*>(luaL_checkudata(L, 1, uattackKey));
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
		lua_pushcfunction(L, impl_unit_attacks_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, impl_unit_attacks_len);
		lua_setfield(L, -2, "__len");
		lua_pushcfunction(L, impl_unit_attacks_iter);
		lua_setfield(L, -2, "__ipairs");
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
		lua_pushcfunction(L, impl_unit_attack_match);
		lua_setfield(L, -2, "matches");

		return cmd_out.str();
	}
}
