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

#include "scripting/lua_team.hpp"

#include "scripting/lua_attributes.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/push_check.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "team.hpp"
#include "resources.hpp" // for gameboard
#include "game_board.hpp"
#include "game_display.hpp"
#include "map/map.hpp"

#include <string>


/**
 * Implementation for a lua reference to a team,
 * used by the wesnoth in-game sides table.
 *
 * (The userdata has type team** because lua holds
 * only a pointer to a team, not a full-size team.
 * If it were a full object then we would cast to
 * type team *, since checkudata returns a pointer
 * to the type corresponding to the sizeof expr
 * used when we allocated the userdata.)
 */

// Registry key
static const char * Team = "side";
static const char teamVar[] = "side variables";

#define SIDE_GETTER(name, type) LATTR_GETTER(name, type, team, t)
#define SIDE_SETTER(name, type) LATTR_SETTER(name, type, team, t)
luaW_Registry sideReg{"wesnoth", "sides", Team};

template<> struct lua_object_traits<team> {
	inline static auto metatable = Team;
	inline static team& get(lua_State* L, int n) {
		return luaW_checkteam(L, n);
	}
};

SIDE_GETTER("side", int) {
	return t.side();
}

SIDE_GETTER("save_id", std::string) {
	return t.save_id();
}
//maybe add a setter for save_id too?

SIDE_GETTER("gold", int) {
	return t.gold();
}

SIDE_SETTER("gold", int) {
	t.set_gold(value);
}

SIDE_GETTER("objectives", t_string) {
	return t.objectives();
}

SIDE_SETTER("objectives", t_string) {
	t.set_objectives(value, true);
}

SIDE_GETTER("village_gold", int) {
	return t.village_gold();
}

SIDE_SETTER("village_gold", int) {
	t.set_village_support(value);
}

SIDE_GETTER("village_support", int) {
	return t.village_support();
}

SIDE_SETTER("village_support", int) {
	t.set_village_support(value);
}

SIDE_GETTER("num_villages", int) {
	return t.villages().size();
}

SIDE_GETTER("recall_cost", int) {
	return t.recall_cost();
}

SIDE_SETTER("recall_cost", int) {
	t.set_recall_cost(value);
}

SIDE_GETTER("base_income", int) {
	return t.base_income();
}

SIDE_SETTER("base_income", int) {
	t.set_base_income(value);
}

SIDE_GETTER("total_income", int) {
	return t.total_income();
}

SIDE_GETTER("objectives_changed", bool) {
	return t.objectives_changed();
}

SIDE_SETTER("objectives_changed", bool) {
	t.set_objectives_changed(value);
}

SIDE_GETTER("fog", bool) {
	return t.uses_fog();
}

SIDE_SETTER("fog", bool) {
	t.set_fog(value);
}

SIDE_GETTER("shroud", bool) {
	return t.uses_shroud();
}

SIDE_SETTER("shroud", bool) {
	t.set_shroud(value);
}

SIDE_GETTER("hidden", bool) {
	return t.hidden();
}

SIDE_SETTER("hidden", bool) {
	t.set_hidden(value);
}

SIDE_GETTER("scroll_to_leader", bool) {
	return t.get_scroll_to_leader();
}

SIDE_SETTER("scroll_to_leader", bool) {
	t.set_scroll_to_leader(value);
}

static void reinit_flag_for_team(lua_State* L, const team& t) {
   auto* disp = lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).get_display();
   if(disp) {
	   disp->reinit_flags_for_team(t);
   }
}

SIDE_GETTER("color", std::string) {
	return t.color();
}

SIDE_SETTER("color", std::string) {
	t.set_color(value);
	reinit_flag_for_team(L, t);
}

SIDE_GETTER("flag", std::string) {
	return t.flag().empty() ? game_config::images::flag : t.flag();
}

SIDE_SETTER("flag", std::string) {
	t.set_flag(value);
	reinit_flag_for_team(L, t);
}

SIDE_GETTER("flag_icon", std::string) {
	return t.flag_icon().empty() ? game_config::images::flag_icon : t.flag_icon();
}

SIDE_SETTER("flag_icon", std::string) {
	t.set_flag_icon(value);
}

SIDE_GETTER("user_team_name", t_string) {
	return t.user_team_name();
}

SIDE_SETTER("user_team_name", t_string) {
	t.change_team(t.team_name(), value);
}

SIDE_GETTER("team_name", std::string) {
	return t.team_name();
}

SIDE_SETTER("team_name", std::string) {
	t.change_team(value, t.user_team_name());
}

SIDE_GETTER("faction", std::string) {
	return t.faction();
}

SIDE_GETTER("faction_name", t_string) {
	return t.faction_name();
}

SIDE_GETTER("controller", std::string) {
	return side_controller::get_string(t.controller());
}

SIDE_SETTER("controller", std::string) {
	t.change_controller_by_wml(value);
}

SIDE_GETTER("is_local", bool) {
	return t.is_local();
}

SIDE_GETTER("defeat_condition", std::string) {
	return defeat_condition::get_string(t.defeat_cond());
}

SIDE_SETTER("defeat_condition", std::string) {
	t.set_defeat_condition_string(value);
}

SIDE_GETTER("share_vision", std::string) {
	return team_shared_vision::get_string(t.share_vision());
}

SIDE_SETTER("share_vision", std::string) {
	auto v = team_shared_vision::get_enum(value);
	if(v) {
		t.set_share_vision(*v);
	} else {
		throw luaL_argerror(L, 3, "Invalid share_vision value (should be 'all', 'none', or 'shroud')");
	}
}

SIDE_GETTER("carryover_bonus", double) {
	return t.carryover_bonus();
}

SIDE_SETTER("carryover_bonus", double) {
	t.set_carryover_bonus(value);
}

SIDE_GETTER("carryover_percentage", int) {
	return t.carryover_percentage();
}

SIDE_SETTER("carryover_percentage", int) {
	t.set_carryover_percentage(value);
}

SIDE_GETTER("carryover_gold", int) {
	return t.carryover_gold();
}

SIDE_SETTER("carryover_gold", int) {
	t.set_carryover_gold(value);
}

SIDE_GETTER("carryover_add", bool) {
	return t.carryover_add();
}

SIDE_SETTER("carryover_add", bool) {
	t.set_carryover_add(value);
}

SIDE_GETTER("lost", bool) {
	return t.lost();
}

SIDE_SETTER("lost", bool) {
	t.set_lost(value);
}

SIDE_GETTER("persistent", bool) {
	return t.persistent();
}

SIDE_SETTER("persistent", bool) {
	t.set_persistent(value);
}

SIDE_GETTER("suppress_end_turn_confirmation", bool) {
	return t.no_turn_confirmation();
}

SIDE_SETTER("suppress_end_turn_confirmation", bool) {
	t.set_no_turn_confirmation(value);
}

SIDE_GETTER("share_maps", bool) {
	return t.share_maps();
}

SIDE_GETTER("share_view", bool) {
	return t.share_view();
}

SIDE_GETTER("chose_random", bool) {
	return t.chose_random();
}

SIDE_GETTER("side_name", t_string) {
	return t.side_name_tstr();
}

SIDE_SETTER("side_name", t_string) {
	t.set_side_name(value);
}

SIDE_GETTER("shroud_data", std::string) {
	return t.shroud_data();
}

SIDE_SETTER("shroud_data", std::string) {
	t.reshroud();
	t.merge_shroud_map_data(value);
}

SIDE_GETTER("recruit", std::set<std::string>) {
	return t.recruits();
}

SIDE_SETTER("recruit", std::set<std::string>) {
	t.set_recruits(value);
}

SIDE_GETTER("variables", lua_index_raw) {
	(void)t;
	lua_createtable(L, 1, 0);
	lua_pushvalue(L, 1);
	lua_rawseti(L, -2, 1);
	luaL_setmetatable(L, teamVar);
	return lua_index_raw(L);
}

SIDE_GETTER("starting_location", utils::optional<map_location>) {
	const map_location& starting_pos = resources::gameboard->map().starting_position(t.side());
	if(!resources::gameboard->map().on_board(starting_pos)) return utils::nullopt;
	return starting_pos;
}

SIDE_GETTER("num_units", int) {
	return team_data(*resources::gameboard, t).units;
}

SIDE_GETTER("total_upkeep", int) {
	return team_data(*resources::gameboard, t).upkeep;
}

SIDE_GETTER("expenses", int) {
	return team_data(*resources::gameboard, t).expenses;
}

SIDE_GETTER("net_income", int) {
	return team_data(*resources::gameboard, t).net_income;
}

SIDE_GETTER("__cfg", config) {
	config cfg;
	t.write(cfg);
	return cfg;
}

/**
 * Gets some data on a side (__index metamethod).
 * - Arg 1: full userdata containing the team.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_side_get(lua_State *L)
{
	return sideReg.get(L);
}

/**
 * Gets a list of data on a side (__dir metamethod).
 * - Arg 1: full userdata containing the team.
 * - Ret 1: a list of attributes.
 */
static int impl_side_dir(lua_State *L)
{
	return sideReg.dir(L);
}

/**
 * Turns a lua proxy side to string. (__tostring metamethod)
 */
static int impl_side_tostring(lua_State* L)
{
	const team& team = luaW_checkteam(L, 1);
	std::ostringstream str;

	str << "side: <" << team.side();
	if(!team.side_name().empty()) {
		str << " " << team.side_name();
	}
	str << '>';

	lua_push(L, str.str());
	return 1;
}

/**
 * Sets some data on a side (__newindex metamethod).
 * - Arg 1: full userdata containing the team.
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
static int impl_side_set(lua_State *L)
{
	return sideReg.set(L);
}

static int impl_side_equal(lua_State *L)
{
	// Hidden metamethod, so arg1 has to be a pointer to a team.
	team &t1 = luaW_checkteam(L, 1);
	if(team* t2 = luaW_toteam(L, 2)) {
		lua_pushboolean(L, t1.side() == t2->side());
	} else {
		lua_pushboolean(L, false);
	}
	return 1;
}

/**
 * Gets the variable of a side (__index metamethod).
 * - Arg 1: table containing the userdata containing the side id.
 * - Arg 2: string containing the name of the status.
 * - Ret 1: boolean.
 */
static int impl_side_variables_get(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "side variables");
	}
	lua_rawgeti(L, 1, 1);
	const team& side = luaW_checkteam(L, -1);

	char const *m = luaL_checkstring(L, 2);
	return_cfgref_attrib("__cfg", side.variables());

	variable_access_const v(m, side.variables());
	return luaW_pushvariable(L, v) ? 1 : 0;
}

/**
 * Sets the variable of a side (__newindex metamethod).
 * - Arg 1: table containing the userdata containing the side id.
 * - Arg 2: string containing the name of the status.
 * - Arg 3: scalar.
 */
static int impl_side_variables_set(lua_State *L)
{
	if(!lua_istable(L, 1)) {
		return luaW_type_error(L, 1, "side variables");
	}
	lua_rawgeti(L, 1, 1);
	team& side = luaW_checkteam(L, -1);

	char const *m = luaL_checkstring(L, 2);
	if(strcmp(m, "__cfg") == 0) {
		side.variables() = luaW_checkconfig(L, 3);
		return 0;
	}
	config& vars = side.variables();
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

namespace lua_team {

	std::string register_metatable(lua_State * L)
	{
		std::ostringstream cmd_out;

		cmd_out << "Adding getside metatable...\n";

		luaL_newmetatable(L, Team);

		static luaL_Reg const callbacks[] {
			{ "__index", 	    &impl_side_get},
			{ "__newindex",	    &impl_side_set},
			{ "__dir",          &impl_side_dir},
			{ "__eq",	        &impl_side_equal},
			{ "__tostring",     &impl_side_tostring},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, callbacks, 0);

		lua_pushstring(L, Team);
		lua_setfield(L, -2, "__metatable");

		// Create the side variables metatable.
		cmd_out << "Adding side variables metatable...\n";

		luaL_newmetatable(L, teamVar);
		lua_pushcfunction(L, impl_side_variables_get);
		lua_setfield(L, -2, "__index");
		lua_pushcfunction(L, impl_side_variables_set);
		lua_setfield(L, -2, "__newindex");
		lua_pushstring(L, "side variables");
		lua_setfield(L, -2, "__metatable");

		return cmd_out.str();
	}
}

void luaW_pushteam(lua_State *L, team & tm)
{
	team** t = static_cast<team**>(lua_newuserdatauv(L, sizeof(team*), 0));
	*t = &tm;
	luaL_setmetatable(L, Team);
}

team& luaW_checkteam(lua_State* L, int idx)
{
	return **static_cast<team **>(luaL_checkudata(L, idx, Team));
}

team& luaW_checkteam(lua_State* L, int idx, game_board& board)
{
	if(lua_isinteger(L, idx)) {
		int side = lua_tointeger(L, idx);
		if(!board.has_team(side)) {
			std::string error = "side " + std::to_string(side) + " does not exist";
			luaL_argerror(L, 1, error.c_str());
			// Unreachable
		}
		return board.get_team(side);
	}
	return **static_cast<team **>(luaL_checkudata(L, idx, Team));
}

team* luaW_toteam(lua_State* L, int idx)
{
	if(void* p = luaL_testudata(L, idx, Team)) {
		return *static_cast<team **>(p);
	}
	return nullptr;
}
