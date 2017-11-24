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

#include "scripting/lua_unit.hpp"

#include "game_board.hpp"
#include "log.hpp"
#include "map/location.hpp"             // for map_location
#include "resources.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/push_check.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "units/unit.hpp"
#include "units/map.hpp"
#include "units/animation_component.hpp"

#include "lua/lauxlib.h"
#include "lua/lua.h"                    // for lua_State, lua_settop, etc

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static const char getunitKey[] = "unit";
static const char ustatusKey[] = "unit status";
static const char unitvarKey[] = "unit variables";

lua_unit::~lua_unit()
{
}

unit* lua_unit::get()
{
	if (ptr) return ptr.get();
	if (c_ptr) return c_ptr;
	if (side) {
		return resources::gameboard->get_team(side).recall_list().find_if_matches_underlying_id(uid).get();
	}
	unit_map::unit_iterator ui = resources::gameboard->units().find(uid);
	if (!ui.valid()) return nullptr;
	return ui.get_shared_ptr().get(); //&*ui would not be legal, must get new shared_ptr by copy ctor because the unit_map itself is holding a boost shared pointer.
}
unit_ptr lua_unit::get_shared()
{
	if (ptr) return ptr;
	if (side) {
		return resources::gameboard->get_team(side).recall_list().find_if_matches_underlying_id(uid);
	}
	unit_map::unit_iterator ui = resources::gameboard->units().find(uid);
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
		std::pair<unit_map::unit_iterator, bool> res = resources::gameboard->units().replace(loc, ptr);
		if (res.second) {
			ptr.reset();
			uid = res.first->underlying_id();
		} else {
			ERR_LUA << "Could not move unit " << ptr->underlying_id() << " onto map location " << loc << '\n';
			return false;
		}
	} else if (side) { // recall list
		unit_ptr it = resources::gameboard->get_team(side).recall_list().extract_if_matches_underlying_id(uid);
		if (it) {
			side = 0;
			// uid may be changed by unit_map on insertion
			uid = resources::gameboard->units().replace(loc, it).first->underlying_id();
		} else {
			ERR_LUA << "Could not find unit " << uid << " on recall list of side " << side << '\n';
			return false;
		}
	} else { // on map
		unit_map::unit_iterator ui = resources::gameboard->units().find(uid);
		if (ui != resources::gameboard->units().end()) {
			map_location from = ui->get_location();
			if (from != loc) { // This check is redundant in current usage
				resources::gameboard->units().erase(loc);
				resources::gameboard->units().move(from, loc);
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
	return luaL_testudata(L, index,getunitKey) != nullptr;
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
			luaW_type_error(L, index, "unit");
			break;
		case LU_NOT_VALID:
			luaL_argerror(L, index, "unit not found");
			break;
		case LU_NOT_ON_MAP:
			luaL_argerror(L, index, "unit not found on map");
			break;
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
	luaL_setmetatable(L, getunitKey);
}

lua_unit* luaW_pushlocalunit(lua_State *L, unit& u)
{
	lua_unit* res = new(L) lua_unit(u);
	lua_unit::setmetatable(L);
	return res;
}

/**
 * Destroys a unit object before it is collected (__gc metamethod).
 */
static int impl_unit_collect(lua_State *L)
{
	lua_unit *u = static_cast<lua_unit *>(lua_touserdata(L, 1));
	u->lua_unit::~lua_unit();
	return 0;
}

/**
 * Checks two lua proxy units for equality. (__eq metamethod)
 */
static int impl_unit_equality(lua_State* L)
{
	unit& left = luaW_checkunit(L, 1);
	unit& right = luaW_checkunit(L, 2);
	const bool equal = left.underlying_id() == right.underlying_id();
	lua_pushboolean(L, equal);
	return 1;
}

/**
 * Gets some data on a unit (__index metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_unit_get(lua_State *L)
{
	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
	char const *m = luaL_checkstring(L, 2);
	const unit* pu = lu->get();

	if(strcmp(m, "valid") == 0) {
		if(!pu) {
			return 0;
		}
		if(lu->on_map()) {
			lua_pushstring(L, "map");
		} else if(lu->on_recall_list()) {
			lua_pushstring(L, "recall");
		} else {
			lua_pushstring(L, "private");
		}
		return 1;
	}

	if(!pu) {
		return luaL_argerror(L, 1, "unknown unit");
	}

	unit const &u = *pu;

	// Find the corresponding attribute.
	return_int_attrib("x", u.get_location().wml_x());
	return_int_attrib("y", u.get_location().wml_y());
	if(strcmp(m, "loc") == 0) {
		lua_pushinteger(L, u.get_location().wml_x());
		lua_pushinteger(L, u.get_location().wml_y());
		return 2;
	}
	return_int_attrib("side", u.side());
	return_string_attrib("id", u.id());
	return_string_attrib("type", u.type_id());
	return_string_attrib("image_mods", u.effect_image_mods());
	return_string_attrib("usage", u.usage());
	return_int_attrib("hitpoints", u.hitpoints());
	return_int_attrib("max_hitpoints", u.max_hitpoints());
	return_int_attrib("experience", u.experience());
	return_int_attrib("max_experience", u.max_experience());
	return_int_attrib("recall_cost", u.recall_cost());
	return_int_attrib("moves", u.movement_left());
	return_int_attrib("max_moves", u.total_movement());
	return_int_attrib("max_attacks", u.max_attacks());
	return_int_attrib("attacks_left", u.attacks_left());
	return_tstring_attrib("name", u.name());
	return_bool_attrib("canrecruit", u.can_recruit());
	return_int_attrib("level", u.level());
	return_int_attrib("cost", u.cost());

	return_vector_string_attrib("extra_recruit", u.recruits());
	return_vector_string_attrib("advances_to", u.advances_to());

	if(strcmp(m, "alignment") == 0) {
		lua_push(L, u.alignment());
		return 1;
	}

	if(strcmp(m, "upkeep") == 0) {
		unit::upkeep_t upkeep = u.upkeep_raw();

		// Need to keep these seperate in order to ensure an int value is always used if applicable.
		if(int* v = boost::get<int>(&upkeep)) {
			lua_push(L, *v);
		} else {
			const std::string type = boost::apply_visitor(unit::upkeep_type_visitor(), upkeep);
			lua_push(L, type);
		}

		return 1;
	}
	if(strcmp(m, "advancements") == 0) {
		lua_push(L, u.modification_advancements());
		return 1;
	}
	if(strcmp(m, "overlays") == 0) {
		lua_push(L, u.overlays());
		return 1;
	}
	if(strcmp(m, "traits") == 0) {
		lua_push(L, u.get_traits_list());
		return 1;
	}
	if(strcmp(m, "abilities") == 0) {
		lua_push(L, u.get_ability_list());
		return 1;
	}
	if(strcmp(m, "status") == 0) {
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, 1);
		lua_rawseti(L, -2, 1);
		luaL_setmetatable(L, ustatusKey);
		return 1;
	}
	if(strcmp(m, "variables") == 0) {
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, 1);
		lua_rawseti(L, -2, 1);
		luaL_setmetatable(L, unitvarKey);
		return 1;
	}
	if(strcmp(m, "attacks") == 0) {
		push_unit_attacks_table(L, 1);
		return 1;
	}
	return_vector_string_attrib("animations", u.anim_comp().get_flags());
	return_cfg_attrib("recall_filter", cfg = u.recall_filter());
	return_bool_attrib("hidden", u.get_hidden());
	return_bool_attrib("petrified", u.incapacitated());
	return_bool_attrib("resting", u.resting());
	return_string_attrib("role", u.get_role());
	return_string_attrib("race", u.race()->id());
	return_string_attrib("gender", u.gender().str());
	return_string_attrib("variation", u.variation());
	return_bool_attrib("zoc", u.get_emit_zoc());
	return_string_attrib("facing", map_location::write_direction(u.facing()));
	return_string_attrib("portrait", u.big_profile() == u.absolute_image() ? u.absolute_image() + u.image_mods() : u.big_profile());
	return_cfg_attrib("__cfg", u.write(cfg); u.get_location().write(cfg));

	if(luaW_getmetafield(L, 1, m)) {
		return 1;
	}
	return 0;
}

/**
 * Sets some data on a unit (__newindex metamethod).
 * - Arg 1: full userdata containing the unit id.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_unit_set(lua_State *L)
{
	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
	char const *m = luaL_checkstring(L, 2);
	unit* pu = lu->get();
	if (!pu) return luaL_argerror(L, 1, "unknown unit");
	unit &u = *pu;

	// Find the corresponding attribute.
	//modify_int_attrib_check_range("side", u.set_side(value), 1, static_cast<int>(teams().size())); TODO: Figure out if this is a good idea, to refer to teams() and make this depend on having a gamestate
	modify_int_attrib("side", u.set_side(value));
	modify_int_attrib("moves", u.set_movement(value));
	modify_int_attrib("hitpoints", u.set_hitpoints(value));
	modify_int_attrib("experience", u.set_experience(value));
	modify_int_attrib("recall_cost", u.set_recall_cost(value));
	modify_int_attrib("attacks_left", u.set_attacks(value));
	modify_int_attrib("level", u.set_level(value));
	modify_bool_attrib("resting", u.set_resting(value));
	modify_tstring_attrib("name", u.set_name(value));
	modify_string_attrib("role", u.set_role(value));
	modify_string_attrib("facing", u.set_facing(map_location::parse_direction(value)));
	modify_bool_attrib("hidden", u.set_hidden(value));
	modify_bool_attrib("zoc", u.set_emit_zoc(value));
	modify_bool_attrib("canrecruit", u.set_can_recruit(value));

	modify_vector_string_attrib("extra_recruit", u.set_recruits(value));
	modify_vector_string_attrib("advances_to", u.set_advances_to(value));
	if(strcmp(m, "alignment") == 0) {
		u.set_alignment(lua_check<unit_type::ALIGNMENT>(L, 3));
		return 0;
	}

	if(strcmp(m, "advancements") == 0) {
		u.set_advancements(lua_check<std::vector<config> >(L, 3));
		return 0;
	}

	if(strcmp(m, "upkeep") == 0) {
		if(lua_isnumber(L, 3)) {
			u.set_upkeep(luaL_checkinteger(L, 3));
			return 0;
		}
		const char* v = luaL_checkstring(L, 3);
		if((strcmp(v, "loyal") == 0) || (strcmp(v, "free") == 0)) {
			u.set_upkeep(unit::upkeep_loyal());
		} else if(strcmp(v, "full") == 0) {
			u.set_upkeep(unit::upkeep_full());
		} else {
			std::string err_msg = "unknown upkeep value of unit: ";
			err_msg += v;
			return luaL_argerror(L, 2, err_msg.c_str());
		}
		return 0;
	}
	if(!lu->on_map()) {
		map_location loc = u.get_location();
		modify_int_attrib("x", loc.set_wml_x(value); u.set_location(loc));
		modify_int_attrib("y", loc.set_wml_y(value); u.set_location(loc));
	}

	std::string err_msg = "unknown modifiable property of unit: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

/**
 * Gets the status of a unit (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Ret 1: boolean.
 */
static int impl_unit_status_get(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit status");
	}
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	lua_pushboolean(L, u->get_state(m));
	return 1;
}

/**
 * Sets the status of a unit (__newindex metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Arg 3: boolean.
 */
static int impl_unit_status_set(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit status");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	u->set_state(m, luaW_toboolean(L, 3));
	return 0;
}

/**
 * Gets the variable of a unit (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Ret 1: boolean.
 */
static int impl_unit_variables_get(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit variables");
	}
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 2, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	return_cfgref_attrib("__cfg", u->variables());

	variable_access_const v(m, u->variables());
	return luaW_pushvariable(L, v) ? 1 : 0;
}

/**
 * Sets the variable of a unit (__newindex metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Arg 3: scalar.
 */
static int impl_unit_variables_set(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "unit variables");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if(!u) {
		return luaL_argerror(L, 2, "unknown unit");
	}
	char const *m = luaL_checkstring(L, 2);
	if(strcmp(m, "__cfg") == 0) {
		u->variables() = luaW_checkconfig(L, 3);
		return 0;
	}
	config& vars = u->variables();
	if(lua_isnoneornil(L, 3)) {
		try {
			variable_access_throw(m, vars).clear(false);
		} catch(const invalid_variablename_exception&) {
		}
		return 0;
	}
	variable_access_create v(m, vars);
	luaW_checkvariable(L, v, 3);
	return 0;
}

namespace lua_units {
	std::string register_metatables(lua_State* L)
	{
		std::ostringstream cmd_out;

		// Create the getunit metatable.
		cmd_out << "Adding getunit metatable...\n";

		luaL_newmetatable(L, getunitKey);
		lua_pushcfunction(L, impl_unit_collect);
		lua_setfield(L, -2, "__gc");
		lua_pushcfunction(L, impl_unit_equality);
		lua_setfield(L, -2, "__eq");
		lua_pushcfunction(L, impl_unit_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "unit");
		lua_setfield(L, -2, "__metatable");
		// Unit methods
		luaW_getglobal(L, "wesnoth", "match_unit");
		lua_setfield(L, -2, "matches");
		luaW_getglobal(L, "wesnoth", "put_recall_unit");
		lua_setfield(L, -2, "to_recall");
		luaW_getglobal(L, "wesnoth", "put_unit");
		lua_setfield(L, -2, "to_map");
		luaW_getglobal(L, "wesnoth", "erase_unit");
		lua_setfield(L, -2, "erase");
		luaW_getglobal(L, "wesnoth", "copy_unit");
		lua_setfield(L, -2, "clone");
		luaW_getglobal(L, "wesnoth", "extract_unit");
		lua_setfield(L, -2, "extract");
		luaW_getglobal(L, "wesnoth", "advance_unit");
		lua_setfield(L, -2, "advance");
		luaW_getglobal(L, "wesnoth", "add_modification");
		lua_setfield(L, -2, "add_modification");
		luaW_getglobal(L, "wesnoth", "unit_resistance");
		lua_setfield(L, -2, "resistance");
		luaW_getglobal(L, "wesnoth", "remove_modifications");
		lua_setfield(L, -2, "remove_modifications");
		luaW_getglobal(L, "wesnoth", "unit_defense");
		lua_setfield(L, -2, "defense");
		luaW_getglobal(L, "wesnoth", "unit_movement_cost");
		lua_setfield(L, -2, "movement");
		luaW_getglobal(L, "wesnoth", "unit_vision_cost");
		lua_setfield(L, -2, "vision");
		luaW_getglobal(L, "wesnoth", "unit_jamming_cost");
		lua_setfield(L, -2, "jamming");
		luaW_getglobal(L, "wesnoth", "unit_ability");
		lua_setfield(L, -2, "ability");
		luaW_getglobal(L, "wesnoth", "transform_unit");
		lua_setfield(L, -2, "transform");
		luaW_getglobal(L, "wesnoth", "select_unit");
		lua_setfield(L, -2, "select");

		// Create the unit status metatable.
		cmd_out << "Adding unit status metatable...\n";

		luaL_newmetatable(L, ustatusKey);
		lua_pushcfunction(L, impl_unit_status_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_status_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "unit status");
		lua_setfield(L, -2, "__metatable");

		// Create the unit variables metatable.
		cmd_out << "Adding unit variables metatable...\n";

		luaL_newmetatable(L, unitvarKey);
		lua_pushcfunction(L, impl_unit_variables_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_unit_variables_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "unit variables");
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}
