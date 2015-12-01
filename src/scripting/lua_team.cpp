/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "team.hpp"

#include <boost/foreach.hpp>
#include <string>

#include "lua/lua.h"
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

/**
 * Gets some data on a side (__index metamethod).
 * - Arg 1: full userdata containing the team.
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
static int impl_side_get(lua_State *L)
{
	// Hidden metamethod, so arg1 has to be a pointer to a team.
	team &t = **static_cast<team **>(luaL_checkudata(L, 1, Team));
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("side", t.side());
	return_string_attrib("save_id", t.save_id());
	return_int_attrib("gold", t.gold());
	return_tstring_attrib("objectives", t.objectives());
	return_int_attrib("village_gold", t.village_gold());
	return_int_attrib("village_support", t.village_support());
	return_int_attrib("recall_cost", t.recall_cost());
	return_int_attrib("base_income", t.base_income());
	return_int_attrib("total_income", t.total_income());
	return_bool_attrib("objectives_changed", t.objectives_changed());
	return_bool_attrib("fog", t.uses_fog());
	return_bool_attrib("shroud", t.uses_shroud());
	return_bool_attrib("hidden", t.hidden());
	return_bool_attrib("scroll_to_leader", t.get_scroll_to_leader());
	return_string_attrib("flag", t.flag());
	return_string_attrib("flag_icon", t.flag_icon());
	return_tstring_attrib("team_name", t.team_name());
	return_string_attrib("team_id", t.team_id());
	return_string_attrib("name", t.name());
	return_string_attrib("color", t.color());
	return_cstring_attrib("controller", t.controller().to_string().c_str());
	return_string_attrib("defeat_condition", t.defeat_condition().to_string());
	return_string_attrib("share_vision", t.share_vision().to_string());
	return_float_attrib("carryover_bonus", t.carryover_bonus());
	return_int_attrib("carryover_percentage", t.carryover_percentage());
	return_bool_attrib("carryover_add", t.carryover_add());
	return_bool_attrib("lost", t.lost());

	if (strcmp(m, "recruit") == 0) {
		std::set<std::string> const &recruits = t.recruits();
		lua_createtable(L, recruits.size(), 0);
		int i = 1;
		BOOST_FOREACH(std::string const &r, t.recruits()) {
			lua_pushstring(L, r.c_str());
			lua_rawseti(L, -2, i++);
		}
		return 1;
	}

	return_cfg_attrib("__cfg", t.write(cfg));
	return 0;
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
	team &t = **static_cast<team **>(luaL_checkudata(L, 1, Team));
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	modify_int_attrib("gold", t.set_gold(value));
	modify_tstring_attrib("objectives", t.set_objectives(value, true));
	modify_int_attrib("village_gold", t.set_village_gold(value));
	modify_int_attrib("village_support", t.set_village_support(value));
	modify_int_attrib("recall_cost", t.set_recall_cost(value));
	modify_int_attrib("base_income", t.set_base_income(value));
	modify_bool_attrib("objectives_changed", t.set_objectives_changed(value));
	modify_bool_attrib("hidden", t.set_hidden(value));
	modify_bool_attrib("scroll_to_leader", t.set_scroll_to_leader(value));
	modify_tstring_attrib("team_name", t.change_team(t.team_id(), value));
	modify_string_attrib("team_id", t.change_team(value, t.team_name()));
	modify_string_attrib("controller", t.change_controller_by_wml(value));
	modify_string_attrib("color", t.set_color(value));
	modify_string_attrib("defeat_condition", t.set_defeat_condition_string(value));
	modify_int_attrib("carryover_percentage", t.set_carryover_percentage(value));
	modify_bool_attrib("carryover_add", t.set_carryover_add(value));
	modify_bool_attrib("lost", t.set_lost(value));

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

namespace lua_team {

	std::string register_metatable(lua_State * L)
	{
		luaL_newmetatable(L, Team);

		static luaL_Reg const callbacks[] = {
			{ "__index", 	    &impl_side_get},
			{ "__newindex",	    &impl_side_set},
			{ NULL, NULL }
		};
		luaL_setfuncs(L, callbacks, 0);

		lua_pushstring(L, "side");
		lua_setfield(L, -2, "__metatable");

		return "Adding getside metatable...\n";
	}
}

void luaW_pushteam(lua_State *L, team & tm)
{
	team** t = static_cast<team**>(lua_newuserdata(L, sizeof(team*)));
	*t = &tm;
	luaL_setmetatable(L, Team);
}
