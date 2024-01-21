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

#include "scripting/lua_common.hpp"
#include "scripting/push_check.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "team.hpp"
#include "resources.hpp" // for gameboard
#include "game_board.hpp"
#include "game_display.hpp"
#include "map/map.hpp"

#include <string>

#include "lua/lauxlib.h"

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

/**
 * Gets some data on a side (__index metamethod).
 * - Arg 1: full userdata containing the team.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_side_get(lua_State *L)
{
	// Hidden metamethod, so arg1 has to be a pointer to a team.
	team &t = luaW_checkteam(L, 1);
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("side", t.side());
	return_string_attrib("save_id", t.save_id());
	return_int_attrib("gold", t.gold());
	return_tstring_attrib("objectives", t.objectives());
	return_int_attrib("village_gold", t.village_gold());
	return_int_attrib("village_support", t.village_support());
	return_int_attrib("num_villages", t.villages().size());
	return_int_attrib("recall_cost", t.recall_cost());
	return_int_attrib("base_income", t.base_income());
	return_int_attrib("total_income", t.total_income());
	return_bool_attrib("objectives_changed", t.objectives_changed());
	return_bool_attrib("fog", t.uses_fog());
	return_bool_attrib("shroud", t.uses_shroud());
	return_bool_attrib("hidden", t.hidden());
	return_bool_attrib("scroll_to_leader", t.get_scroll_to_leader());
	return_string_attrib("flag", t.flag().empty() ? game_config::images::flag : t.flag());
	return_string_attrib("flag_icon", t.flag_icon().empty() ? game_config::images::flag_icon : t.flag_icon());
	return_tstring_attrib("user_team_name", t.user_team_name());
	return_string_attrib("team_name", t.team_name());
	return_string_attrib("faction", t.faction());
	return_tstring_attrib("faction_name", t.faction_name());
	return_string_attrib("color", t.color());
	return_string_attrib("controller", side_controller::get_string(t.controller()));
	return_bool_attrib("is_local", t.is_local());
	return_string_attrib("defeat_condition", defeat_condition::get_string(t.defeat_cond()));
	return_string_attrib("share_vision", team_shared_vision::get_string(t.share_vision()));
	return_float_attrib("carryover_bonus", t.carryover_bonus());
	return_int_attrib("carryover_percentage", t.carryover_percentage());
	return_bool_attrib("carryover_add", t.carryover_add());
	return_bool_attrib("lost", t.lost());
	return_bool_attrib("persistent", t.persistent());
	return_bool_attrib("suppress_end_turn_confirmation", t.no_turn_confirmation());
	return_bool_attrib("share_maps", t.share_maps());
	return_bool_attrib("share_view", t.share_view());
	return_bool_attrib("chose_random", t.chose_random());
	return_tstring_attrib("side_name", t.side_name_tstr());
	return_string_attrib("shroud_data", t.shroud_data());

	if (strcmp(m, "recruit") == 0) {
		const std::set<std::string>& recruits = t.recruits();
		lua_createtable(L, recruits.size(), 0);
		int i = 1;
		for (const std::string& r : t.recruits()) {
			lua_pushstring(L, r.c_str());
			lua_rawseti(L, -2, i++);
		}
		return 1;
	}
	if(strcmp(m, "variables") == 0) {
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, 1);
		lua_rawseti(L, -2, 1);
		luaL_setmetatable(L, teamVar);
		return 1;
	}
	if(strcmp(m, "starting_location") == 0) {
		const map_location& starting_pos = resources::gameboard->map().starting_position(t.side());
		if(!resources::gameboard->map().on_board(starting_pos)) return 0;

		luaW_pushlocation(L, starting_pos);
		return 1;
	}

	// These are blocked together because they are all part of the team_data struct.
	// Some of these values involve iterating over the units map to calculate them.
	auto d = [&](){ return team_data(*resources::gameboard, t); };
	return_int_attrib("num_units", d().units);
	return_int_attrib("total_upkeep", d().upkeep);
	return_int_attrib("expenses", d().expenses);
	return_int_attrib("net_income", d().net_income);

	return_cfg_attrib("__cfg", t.write(cfg));
	if(luaW_getglobal(L, "wesnoth", "sides", m)) {
		return 1;
	}
	return 0;
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
	// Hidden metamethod, so arg1 has to be a pointer to a team.
	team &t = luaW_checkteam(L, 1);
	char const *m = luaL_checkstring(L, 2);

	const auto& reinit_flag_for_team = [&L] (const team& t) -> void {
		auto* disp = lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).get_display();
		if(disp) {
			disp->reinit_flags_for_team(t);
		}
	};
	// Find the corresponding attribute.
	modify_int_attrib("gold", t.set_gold(value));
	modify_tstring_attrib("objectives", t.set_objectives(value, true));
	//maybe add a setter for save_id too?
	modify_int_attrib("village_gold", t.set_village_gold(value));
	modify_int_attrib("village_support", t.set_village_support(value));
	modify_int_attrib("recall_cost", t.set_recall_cost(value));
	modify_int_attrib("base_income", t.set_base_income(value));
	modify_bool_attrib("objectives_changed", t.set_objectives_changed(value));
	modify_bool_attrib("hidden", t.set_hidden(value));
	modify_bool_attrib("scroll_to_leader", t.set_scroll_to_leader(value));
	modify_string_attrib("flag", {
		t.set_flag(value);
		reinit_flag_for_team(t);
	});
	modify_string_attrib("flag_icon", t.set_flag_icon(value));
	modify_tstring_attrib("user_team_name", t.change_team(t.team_name(), value));
	modify_string_attrib("team_name", t.change_team(value, t.user_team_name()));
	modify_string_attrib("controller", t.change_controller_by_wml(value));
	modify_string_attrib("color", {
		t.set_color(value);
		reinit_flag_for_team(t);
	});
	modify_string_attrib("defeat_condition", t.set_defeat_condition_string(value));
	modify_int_attrib("carryover_percentage", t.set_carryover_percentage(value));
	modify_bool_attrib("carryover_add", t.set_carryover_add(value));
	modify_bool_attrib("lost", t.set_lost(value));
	modify_bool_attrib("persistent", t.set_persistent(value));
	modify_bool_attrib("suppress_end_turn_confirmation", t.set_no_turn_confirmation(value));
	modify_bool_attrib("shroud", t.set_shroud(value));
	modify_bool_attrib("fog", t.set_fog(value));
	modify_string_attrib("flag_icon", t.set_flag_icon(value));
	modify_tstring_attrib("side_name", t.set_side_name(value));
	modify_string_attrib("shroud_data", t.reshroud(); t.merge_shroud_map_data(value));
	modify_string_attrib("share_vision", {
		auto v = team_shared_vision::get_enum(value);
		if(v) {
			t.set_share_vision(*v);
		} else {
			return luaL_argerror(L, 3, "Invalid share_vision value (should be 'all', 'none', or 'shroud')");
		}
	});

	if (strcmp(m, "carryover_bonus") == 0) {
		t.set_carryover_bonus(luaL_checknumber(L, 3));
		return 0;
	}

	if (strcmp(m, "recruit") == 0) {
		t.set_recruits(std::set<std::string>());
		if (!lua_istable(L, 3)) return 0;
		for (int i = 1;; ++i) {
			lua_rawgeti(L, 3, i);
			if (lua_isnil(L, -1)) break;
			t.add_recruit(lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		return 0;
	}

	std::string err_msg = "unknown modifiable property of side: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
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
