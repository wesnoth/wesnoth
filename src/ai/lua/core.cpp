/*
   Copyright (C) 2010 - 2016 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Provides core classes for the Lua AI.
 *
 */

#include <cassert>
#include <cstring>

#include "core.hpp"
#include "../../scripting/game_lua_kernel.hpp"
#include "../../scripting/lua_api.hpp"
#include "../../scripting/push_check.hpp"
#include "lua_object.hpp" // (Nephro)

#include "../../attack_prediction.hpp"
#include "../../game_display.hpp"
#include "../../log.hpp"
#include "../../map.hpp"
#include "../../pathfind/pathfind.hpp"
#include "../../play_controller.hpp"
#include "../../resources.hpp"
#include "../../terrain_translation.hpp"
#include "../../terrain_filter.hpp"
#include "../../unit.hpp"
#include "../actions.hpp"
#include "../composite/engine_lua.hpp"
#include "../composite/contexts.hpp"

#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/llimits.h"

static lg::log_domain log_ai_engine_lua("ai/engine/lua");
#define LOG_LUA LOG_STREAM(info, log_ai_engine_lua)
#define ERR_LUA LOG_STREAM(err, log_ai_engine_lua)

static char const aisKey     = 0;

namespace ai {

static void push_attack_analysis(lua_State *L, attack_analysis&);

void lua_ai_context::init(lua_State *L)
{
	// Create the ai elements table.
	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));
	lua_newtable(L);
	lua_rawset(L, LUA_REGISTRYINDEX);
}

void lua_ai_context::get_persistent_data(config &cfg) const
{
	int top = lua_gettop(L);

	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, -1, num_);

	lua_getfield(L, -1, "data");
	luaW_toconfig(L, -1, cfg);

	lua_settop(L, top);
}

void lua_ai_context::set_persistent_data(const config &cfg)
{
	int top = lua_gettop(L);

	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_rawgeti(L, -1, num_);

	luaW_pushconfig(L, cfg);
	lua_setfield(L, -2, "data");

	lua_settop(L, top);
}
static ai::engine_lua &get_engine(lua_State *L)
{
	return *(static_cast<ai::engine_lua*>(
			lua_touserdata(L, lua_upvalueindex(1))));
}

static ai::readonly_context &get_readonly_context(lua_State *L)
{
	return get_engine(L).get_readonly_context();
}

static int transform_ai_action(lua_State *L, ai::action_result_ptr action_result)
{
	lua_newtable(L);
	lua_pushboolean(L,action_result->is_ok());
	lua_setfield(L, -2, "ok");
	lua_pushboolean(L,action_result->is_gamestate_changed());
	lua_setfield(L, -2, "gamestate_changed");
	lua_pushinteger(L,action_result->get_status());
	lua_setfield(L, -2, "status");
	return 1;
}

static bool to_map_location(lua_State *L, int &index, map_location &res)
{
	if (lua_isuserdata(L, index))
	{
		const unit* u = luaW_tounit(L, index);
		if (!u) return false;
		res = u->get_location();
		++index;
	}
	else
	{
		if (!lua_isnumber(L, index)) return false;
		res.x = lua_tointeger(L, index) - 1;
		++index;
		if (!lua_isnumber(L, index)) return false;
		res.y = lua_tointeger(L, index) - 1;
		++index;
	}

	return true;
}

static int cfun_ai_get_suitable_keep(lua_State *L)
{
	int index = 1;

	ai::readonly_context &context = get_readonly_context(L);
	unit* leader = NULL;
	if (lua_isuserdata(L, index))
	{
		leader = luaW_tounit(L, index);
		if (!leader) return luaL_argerror(L, 1, "unknown unit");
	}
	else return luaL_typerror(L, 1, "unit");
	const map_location loc = leader->get_location();
	const pathfind::paths leader_paths(*leader, false, true, context.current_team());
	const map_location &res = context.suitable_keep(loc,leader_paths);
	if (!res.valid()) {
		return 0;
	}
	else {
		lua_pushnumber(L, res.x+1);
		lua_pushnumber(L, res.y+1);
		return 2;
	}
}

static int ai_move(lua_State *L, bool exec, bool remove_movement)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	int side = get_readonly_context(L).get_side();
	map_location from, to;
	if (!to_map_location(L, index, from)) goto error_call_destructors;
	if (!to_map_location(L, index, to)) goto error_call_destructors;
	bool unreach_is_ok = false;
	if (lua_isboolean(L, index)) {
		unreach_is_ok = luaW_toboolean(L, index);
	}
	ai::move_result_ptr move_result = ai::actions::execute_move_action(side,exec,from,to,remove_movement, unreach_is_ok);
	return transform_ai_action(L,move_result);
}

static int cfun_ai_execute_move_full(lua_State *L)
{
	return ai_move(L, true, true);
}

static int cfun_ai_execute_move_partial(lua_State *L)
{
	return ai_move(L, true, false);
}

static int cfun_ai_check_move(lua_State *L)
{
	return ai_move(L, false, false);
}

static int ai_attack(lua_State *L, bool exec)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	ai::readonly_context &context = get_readonly_context(L);

	int side = context.get_side();
	map_location attacker, defender;
	if (!to_map_location(L, index, attacker)) goto error_call_destructors;
	if (!to_map_location(L, index, defender)) goto error_call_destructors;

	int attacker_weapon = -1;//-1 means 'select what is best'
	double aggression = context.get_aggression();//use the aggression from the context

	if (!lua_isnoneornil(L, index)) {
		attacker_weapon = lua_tointeger(L, index);
		if (attacker_weapon != -1) {
			attacker_weapon--;	// Done for consistency of the Lua style
		}
	}

	//TODO: Right now, aggression is used by the attack execution functions to determine the weapon to be used.
	// If a decision is made to expand the function that determines the weapon, this block must be refactored
	// to parse aggression if a single int is on the stack, or create a table of parameters, if a table is on the
	// stack.
	if (!lua_isnoneornil(L, index + 1) && lua_isnumber(L,index + 1)) {
		aggression = lua_tonumber(L, index + 1);
	}

	unit_advancements_aspect advancements = context.get_advancements();
	ai::attack_result_ptr attack_result = ai::actions::execute_attack_action(side,exec,attacker,defender,attacker_weapon,aggression,advancements);
	return transform_ai_action(L,attack_result);
}

static int cfun_ai_execute_attack(lua_State *L)
{
	return ai_attack(L, true);
}

static int cfun_ai_check_attack(lua_State *L)
{
	return ai_attack(L, false);
}

static int ai_stopunit_select(lua_State *L, bool exec, bool remove_movement, bool remove_attacks)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	int side = get_readonly_context(L).get_side();
	map_location loc;
	if (!to_map_location(L, index, loc)) goto error_call_destructors;

	ai::stopunit_result_ptr stopunit_result = ai::actions::execute_stopunit_action(side,exec,loc,remove_movement,remove_attacks);
	return transform_ai_action(L,stopunit_result);
}

static int cfun_ai_execute_stopunit_moves(lua_State *L)
{
	return ai_stopunit_select(L, true, true, false);
}

static int cfun_ai_execute_stopunit_attacks(lua_State *L)
{
	return ai_stopunit_select(L, true, false, true);
}

static int cfun_ai_execute_stopunit_all(lua_State *L)
{
	return ai_stopunit_select(L, true, true, true);
}

static int cfun_ai_check_stopunit(lua_State *L)
{
	return ai_stopunit_select(L, false, true, true);
}

static int ai_synced_command(lua_State *L, bool exec)
{
	const char *lua_code = luaL_checkstring(L, 1);
	int side = get_readonly_context(L).get_side();
	map_location location;
	if (!lua_isnoneornil(L, 2)) {
		location.x = lua_tonumber(L, 2);
		location.y = lua_tonumber(L, 3);
	}

	ai::synced_command_result_ptr synced_command_result = ai::actions::execute_synced_command_action(side,exec,std::string(lua_code),location);
	return transform_ai_action(L,synced_command_result);
}

static int cfun_ai_execute_synced_command(lua_State *L)
{
	return ai_synced_command(L, true);
}

static int cfun_ai_check_synced_command(lua_State *L)
{
	return ai_synced_command(L, false);
}

static int ai_recruit(lua_State *L, bool exec)
{
	const char *unit_name = luaL_checkstring(L, 1);
	int side = get_readonly_context(L).get_side();
	map_location where;
	if (!lua_isnoneornil(L, 2)) {
		where.x = lua_tonumber(L, 2) - 1;
		where.y = lua_tonumber(L, 3) - 1;
	}
	//TODO fendrin: talk to Crab about the from argument.
	map_location from = map_location::null_location();
	ai::recruit_result_ptr recruit_result = ai::actions::execute_recruit_action(side,exec,std::string(unit_name),where,from);
	return transform_ai_action(L,recruit_result);
}

static int cfun_ai_execute_recruit(lua_State *L)
{
	return ai_recruit(L, true);
}

static int cfun_ai_check_recruit(lua_State *L)
{
	return ai_recruit(L, false);
}

static int ai_recall(lua_State *L, bool exec)
{
	const char *unit_id = luaL_checkstring(L, 1);
	int side = get_readonly_context(L).get_side();
	map_location where;
	if (!lua_isnoneornil(L, 2)) {
		where.x = lua_tonumber(L, 2) - 1;
		where.y = lua_tonumber(L, 3) - 1;
	}
	//TODO fendrin: talk to Crab about the from argument.
	map_location from = map_location::null_location();
	ai::recall_result_ptr recall_result = ai::actions::execute_recall_action(side,exec,std::string(unit_id),where,from);
	return transform_ai_action(L,recall_result);
}

static int cfun_ai_execute_recall(lua_State *L)
{
	return ai_recall(L, true);
}

static int cfun_ai_check_recall(lua_State *L)
{
	return ai_recall(L, false);
}

// Goals and targets


static int cfun_ai_get_targets(lua_State *L)
{
	move_map enemy_dst_src = get_readonly_context(L).get_enemy_dstsrc();
	std::vector<target> targets = get_engine(L).get_ai_context()->find_targets(enemy_dst_src);
	int i = 1;

	lua_createtable(L, 0, 0);
	for (std::vector<target>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		lua_pushinteger(L, i);

		//to factor out
		lua_createtable(L, 3, 0);


		lua_pushstring(L, "type");
		lua_pushnumber(L, it->type);
		lua_rawset(L, -3);

		lua_pushstring(L, "loc");
		luaW_pushlocation(L, it->loc);
		lua_rawset(L, -3);

		lua_pushstring(L, "value");
		lua_pushnumber(L, it->value);
		lua_rawset(L, -3);

		lua_rawset(L, -3);
		++i;
	}
	return 1;
}

// Aspect section
static int cfun_ai_get_aggression(lua_State *L)
{
	double aggression = get_readonly_context(L).get_aggression();
	lua_pushnumber(L, aggression);
	return 1;
}

static int cfun_ai_get_attack_depth(lua_State *L)
{
	int attack_depth = get_readonly_context(L).get_attack_depth();
	lua_pushnumber(L, attack_depth);
	return 1;
}

static int cfun_ai_get_attacks(lua_State *L)
{
	ai::attacks_vector attacks = get_readonly_context(L).get_attacks();
	lua_createtable(L, attacks.size(), 0);
	int table_index = lua_gettop(L);

	ai::attacks_vector::iterator it = attacks.begin();
	for (int i = 1; it != attacks.end(); ++it, ++i)
	{
		push_attack_analysis(L, *it);

		lua_rawseti(L, table_index, i);
	}
	return 1;
}

static int cfun_ai_get_avoid(lua_State *L)
{
	std::set<map_location> locs;
	terrain_filter avoid = get_readonly_context(L).get_avoid();
	avoid.get_locations(locs);
	lua_push(L, locs);

	return 1;
}

static int cfun_ai_get_caution(lua_State *L)
{
	double caution = get_readonly_context(L).get_caution();
	lua_pushnumber(L, caution);
	return 1;
}

static int cfun_ai_get_grouping(lua_State *L)
{
	std::string grouping = get_readonly_context(L).get_grouping();
	lua_pushstring(L, grouping.c_str());
	return 1;
}

static int cfun_ai_get_leader_aggression(lua_State *L)
{
	double leader_aggression = get_readonly_context(L).get_leader_aggression();
	lua_pushnumber(L, leader_aggression);
	return 1;
}

static int cfun_ai_get_leader_goal(lua_State *L)
{
	config goal = get_readonly_context(L).get_leader_goal();
	luaW_pushconfig(L, goal);
	return 1;
}

static int cfun_ai_get_leader_ignores_keep(lua_State *L)
{
	bool leader_ignores_keep = get_readonly_context(L).get_leader_ignores_keep();
	lua_pushboolean(L, leader_ignores_keep);
	return 1;
}

static int cfun_ai_get_leader_value(lua_State *L)
{
	double leader_value = get_readonly_context(L).get_leader_value();
	lua_pushnumber(L, leader_value);
	return 1;
}

static int cfun_ai_get_passive_leader(lua_State *L)
{
	bool passive_leader = get_readonly_context(L).get_passive_leader();
	lua_pushboolean(L, passive_leader);
	return 1;
}

static int cfun_ai_get_passive_leader_shares_keep(lua_State *L)
{
	bool passive_leader_shares_keep = get_readonly_context(L).get_passive_leader_shares_keep();
	lua_pushboolean(L, passive_leader_shares_keep);
	return 1;
}

static int cfun_ai_get_number_of_possible_recruits_to_force_recruit(lua_State *L)
{
	double noprtfr = get_readonly_context(L).get_number_of_possible_recruits_to_force_recruit(); // @note: abbreviation
	lua_pushnumber(L, noprtfr);
	return 1;
}

static int cfun_ai_get_recruitment_ignore_bad_combat(lua_State *L)
{
	bool recruitment_ignore_bad_combat = get_readonly_context(L).get_recruitment_ignore_bad_combat();
	lua_pushboolean(L, recruitment_ignore_bad_combat);
	return 1;
}

static int cfun_ai_get_recruitment_ignore_bad_movement(lua_State *L)
{
	bool recruitment_ignore_bad_movement = get_readonly_context(L).get_recruitment_ignore_bad_movement();
	lua_pushboolean(L, recruitment_ignore_bad_movement);
	return 1;
}

static int cfun_ai_get_recruitment_pattern(lua_State *L)
{
	std::vector<std::string> recruiting = get_readonly_context(L).get_recruitment_pattern();
	int size = recruiting.size();
	lua_createtable(L, size, 0); // create an exmpty table with predefined size
	for (int i = 0; i < size; ++i)
	{
		lua_pushinteger(L, i + 1); // Indexing in Lua starts from 1
		lua_pushstring(L, recruiting[i].c_str());
		lua_settable(L, -3);
	}
	return 1;
}

static int cfun_ai_get_scout_village_targeting(lua_State *L)
{
	double scout_village_targeting = get_readonly_context(L).get_scout_village_targeting();
	lua_pushnumber(L, scout_village_targeting);
	return 1;
}

static int cfun_ai_get_simple_targeting(lua_State *L)
{
	bool simple_targeting = get_readonly_context(L).get_simple_targeting();
	lua_pushboolean(L, simple_targeting);
	return 1;
}

static int cfun_ai_get_support_villages(lua_State *L)
{
	bool support_villages = get_readonly_context(L).get_support_villages();
	lua_pushboolean(L, support_villages);
	return 1;
}

static int cfun_ai_get_village_value(lua_State *L)
{
	double village_value = get_readonly_context(L).get_village_value();
	lua_pushnumber(L, village_value);
	return 1;
}

static int cfun_ai_get_villages_per_scout(lua_State *L)
{
	int villages_per_scout = get_readonly_context(L).get_villages_per_scout();
	lua_pushnumber(L, villages_per_scout);
	return 1;
}
// End of aspect section

static int cfun_attack_rating(lua_State *L)
{
	int top = lua_gettop(L);
	// the attack_analysis table should be on top of the stack
	lua_getfield(L, -1, "att_ptr"); // [-2: attack_analysis; -1: pointer to attack_analysis object in c++]
	// now the pointer to our attack_analysis C++ object is on top
	attack_analysis* aa_ptr = static_cast< attack_analysis * >(lua_touserdata(L, -1));

	//[-2: attack_analysis; -1: pointer to attack_analysis object in c++]

	double aggression = get_readonly_context(L).get_aggression();

	double rating = aa_ptr->rating(aggression, get_readonly_context(L));

	lua_settop(L, top);

	lua_pushnumber(L, rating);
	return 1;
}

static void push_movements(lua_State *L, const std::vector< std::pair < map_location, map_location > > & moves)
{
	lua_createtable(L, moves.size(), 0);

	int table_index = lua_gettop(L);

	std::vector< std::pair < map_location, map_location > >::const_iterator move = moves.begin();

	for (int i = 1; move != moves.end(); ++move, ++i)
	{
		lua_createtable(L, 2, 0); // Creating a table for a pair of map_location's

		lua_pushstring(L, "src");
		luaW_pushlocation(L, move->first);
		lua_rawset(L, -3);

		lua_pushstring(L, "dst");
		luaW_pushlocation(L, move->second);
		lua_rawset(L, -3);

		lua_rawseti(L, table_index, i); // setting  the pair as an element of the movements table
	}


}

static void push_attack_analysis(lua_State *L, attack_analysis& aa)
{
	lua_newtable(L);

	// Pushing a pointer to the current object
	lua_pushstring(L, "att_ptr");
	lua_pushlightuserdata(L, &aa);
	lua_rawset(L, -3);

	// Registering callback function for the rating method
	lua_pushstring(L, "rating");
	lua_pushlightuserdata(L, &get_engine(L));
	lua_pushcclosure(L, &cfun_attack_rating, 1);
	lua_rawset(L, -3);

	lua_pushstring(L, "movements");
	push_movements(L, aa.movements);
	lua_rawset(L, -3);

	lua_pushstring(L, "target");
	luaW_pushlocation(L, aa.target);
	lua_rawset(L, -3);

	lua_pushstring(L, "target_value");
	lua_pushnumber(L, aa.target_value);
	lua_rawset(L, -3);

	lua_pushstring(L, "avg_losses");
	lua_pushnumber(L, aa.avg_losses);
	lua_rawset(L, -3);

	lua_pushstring(L, "chance_to_kill");
	lua_pushnumber(L, aa.chance_to_kill);
	lua_rawset(L, -3);

	lua_pushstring(L, "avg_damage_inflicted");
	lua_pushnumber(L, aa.avg_damage_inflicted);
	lua_rawset(L, -3);

	lua_pushstring(L, "target_starting_damage");
	lua_pushinteger(L, aa.target_starting_damage);
	lua_rawset(L, -3);

	lua_pushstring(L, "avg_damage_taken");
	lua_pushnumber(L, aa.avg_damage_taken);
	lua_rawset(L, -3);

	lua_pushstring(L, "resources_used");
	lua_pushnumber(L, aa.resources_used);
	lua_rawset(L, -3);

	lua_pushstring(L, "terrain_quality");
	lua_pushnumber(L, aa.alternative_terrain_quality);
	lua_rawset(L, -3);

	lua_pushstring(L, "alternative_terrain_quality");
	lua_pushnumber(L, aa.alternative_terrain_quality);
	lua_rawset(L, -3);

	lua_pushstring(L, "vulnerability");
	lua_pushnumber(L, aa.vulnerability);
	lua_rawset(L, -3);

	lua_pushstring(L, "support");
	lua_pushnumber(L, aa.support);
	lua_rawset(L, -3);

	lua_pushstring(L, "leader_threat");
	lua_pushboolean(L, aa.leader_threat);
	lua_rawset(L, -3);

	lua_pushstring(L, "uses_leader");
	lua_pushboolean(L, aa.uses_leader);
	lua_rawset(L, -3);

	lua_pushstring(L, "is_surrounded");
	lua_pushboolean(L, aa.is_surrounded);
	lua_rawset(L, -3);
}

static void push_move_map(lua_State *L, const move_map& m)
{
	lua_createtable(L, 0, 0); // the main table

	if (m.empty())
	{
		return;
	}

	move_map::const_iterator it = m.begin();

	int index = 1;



	do
	{
		map_location key = it->first;

		//push_map_location(L, key); // deprecated

		// This should be factored out. The same function is defined in data/lua/location_set.lua
		// At this point, it is not clear, where this(hashing) function can be placed
		// Implemented it this way, to test the new version of the data structure
		// as requested from the users of LuaAI <Nephro>
		int hashed_index = (key.x + 1) * 16384 + (key.y + 1) + 2000;
		lua_pushinteger(L, hashed_index);

		lua_createtable(L, 0, 0);

		while (key == it->first) {

			luaW_pushlocation(L, it->second);
			lua_rawseti(L, -2, index);

			++index;
			++it;

		}

		lua_settable(L, -3);

		index = 1;

	} while (it != m.end());
}

static int cfun_ai_get_dstsrc(lua_State *L)
{
	move_map dst_src = get_readonly_context(L).get_dstsrc();
	get_readonly_context(L).set_dst_src_valid_lua();
	push_move_map(L, dst_src);
	return 1;
}

static int cfun_ai_get_srcdst(lua_State *L)
{
	move_map src_dst = get_readonly_context(L).get_srcdst();
	get_readonly_context(L).set_src_dst_valid_lua();
	push_move_map(L, src_dst);
	return 1;
}

static int cfun_ai_get_enemy_dstsrc(lua_State *L)
{
	move_map enemy_dst_src = get_readonly_context(L).get_enemy_dstsrc();
	get_readonly_context(L).set_dst_src_enemy_valid_lua();
	push_move_map(L, enemy_dst_src);
	return 1;
}

static int cfun_ai_get_enemy_srcdst(lua_State *L)
{
	move_map enemy_src_dst = get_readonly_context(L).get_enemy_srcdst();
	get_readonly_context(L).set_src_dst_enemy_valid_lua();
	push_move_map(L, enemy_src_dst);
	return 1;
}

static int cfun_ai_is_dst_src_valid(lua_State *L)
{
	bool valid = get_readonly_context(L).is_dst_src_valid_lua();
	lua_pushboolean(L, valid);
	return 1;
}

static int cfun_ai_is_dst_src_enemy_valid(lua_State *L)
{
	bool valid = get_readonly_context(L).is_dst_src_enemy_valid_lua();
	lua_pushboolean(L, valid);
	return 1;
}

static int cfun_ai_is_src_dst_valid(lua_State *L)
{
	bool valid = get_readonly_context(L).is_src_dst_valid_lua();
	lua_pushboolean(L, valid);
	return 1;
}

static int cfun_ai_is_src_dst_enemy_valid(lua_State *L)
{
	bool valid = get_readonly_context(L).is_src_dst_enemy_valid_lua();
	lua_pushboolean(L, valid);
	return 1;
}

static int cfun_ai_recalculate_move_maps(lua_State *L)
{
	get_readonly_context(L).recalculate_move_maps();
	return 1;
}

static int cfun_ai_recalculate_move_maps_enemy(lua_State *L)
{
	get_readonly_context(L).recalculate_move_maps_enemy();
	return 1;
}

static void generate_and_push_ai_table(lua_State* L, ai::engine_lua* engine) {
	//push data table here
	lua_newtable(L);
	lua_pushinteger(L, engine->get_readonly_context().get_side());
	lua_setfield(L, -2, "side"); //stack size is 2 [- 1: new table; -2 ai as string]
	static luaL_Reg const callbacks[] = {
			{ "attack", &cfun_ai_execute_attack },
			// Move maps
			{ "get_new_dst_src", &cfun_ai_get_dstsrc },
			{ "get_new_src_dst", &cfun_ai_get_srcdst },
			{ "get_new_enemy_dst_src", &cfun_ai_get_enemy_dstsrc },
			{ "get_new_enemy_src_dst", &cfun_ai_get_enemy_srcdst },
			{ "recalculate_move_maps", &cfun_ai_recalculate_move_maps },
			{ "recalculate_enemy_move_maps", &cfun_ai_recalculate_move_maps_enemy },
			// End of move maps
			// Goals and targets
			{ "get_targets", &cfun_ai_get_targets },
			// End of G & T
			// Aspects
			{ "get_aggression", &cfun_ai_get_aggression },
			{ "get_avoid", &cfun_ai_get_avoid },
			{ "get_attack_depth", &cfun_ai_get_attack_depth },
			{ "get_attacks", &cfun_ai_get_attacks },
			{ "get_caution", &cfun_ai_get_caution },
			{ "get_grouping", &cfun_ai_get_grouping },
			{ "get_leader_aggression", &cfun_ai_get_leader_aggression },
			{ "get_leader_goal", &cfun_ai_get_leader_goal },
			{ "get_leader_ignores_keep", &cfun_ai_get_leader_ignores_keep },
			{ "get_leader_value", &cfun_ai_get_leader_value },
			{ "get_number_of_possible_recruits_to_force_recruit", &cfun_ai_get_number_of_possible_recruits_to_force_recruit },
			{ "get_passive_leader", &cfun_ai_get_passive_leader },
			{ "get_passive_leader_shares_keep", &cfun_ai_get_passive_leader_shares_keep },
			{ "get_recruitment_ignore_bad_combat", &cfun_ai_get_recruitment_ignore_bad_combat },
			{ "get_recruitment_ignore_bad_movement", &cfun_ai_get_recruitment_ignore_bad_movement },
			{ "get_recruitment_pattern", &cfun_ai_get_recruitment_pattern },
			{ "get_scout_village_targeting", &cfun_ai_get_scout_village_targeting },
			{ "get_simple_targeting", &cfun_ai_get_simple_targeting },
			{ "get_support_villages", &cfun_ai_get_support_villages },
			{ "get_village_value", &cfun_ai_get_village_value },
			{ "get_villages_per_scout", &cfun_ai_get_villages_per_scout },
			// End of aspects
			// Validation/cache functions
			{ "is_dst_src_valid", &cfun_ai_is_dst_src_valid },
			{ "is_enemy_dst_src_valid", &cfun_ai_is_dst_src_enemy_valid },
			{ "is_src_dst_valid", &cfun_ai_is_src_dst_valid },
			{ "is_enemy_src_dst_valid", &cfun_ai_is_src_dst_enemy_valid },
			// End of validation functions
			{ "move", &cfun_ai_execute_move_partial },
			{ "move_full", &cfun_ai_execute_move_full },
			{ "recall", &cfun_ai_execute_recall },
			{ "recruit", &cfun_ai_execute_recruit },
			{ "stopunit_all", &cfun_ai_execute_stopunit_all },
			{ "stopunit_attacks", &cfun_ai_execute_stopunit_attacks },
			{ "stopunit_moves", &cfun_ai_execute_stopunit_moves },
			{ "synced_command", &cfun_ai_execute_synced_command },
			{ "suitable_keep", &cfun_ai_get_suitable_keep },
			{ "check_recall", &cfun_ai_check_recall },
			{ "check_move", &cfun_ai_check_move },
			{ "check_stopunit", &cfun_ai_check_stopunit },
			{ "check_synced_command", &cfun_ai_check_synced_command },
			{ "check_attack", &cfun_ai_check_attack },
			{ "check_recruit", &cfun_ai_check_recruit },
			//{ "",},
			//{ "",},
			{ NULL, NULL } };
	for (const luaL_Reg* p = callbacks; p->name; ++p) {
		lua_pushlightuserdata(L, engine);
		lua_pushcclosure(L, p->func, 1);
		lua_setfield(L, -2, p->name);
	}
}

lua_ai_context* lua_ai_context::create(lua_State *L, char const *code, ai::engine_lua *engine)
{
	int res_ai = luaL_loadstring(L, code);//stack size is now 1 [ -1: ai_context]
	if (res_ai)
	{

		char const *m = lua_tostring(L, -1);
		ERR_LUA << "error while initializing ai:  " <<m << '\n';
		lua_pop(L, 2);//return with stack size 0 []
		return NULL;
	}
	//push data table here
	generate_and_push_ai_table(L, engine);

	//compile the ai as a closure
	if (!luaW_pcall(L, 1, 1, true)) {
		return NULL;//return with stack size 0 []
	}

	// Retrieve the ai elements table from the registry.
	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));
	lua_rawget(L, LUA_REGISTRYINDEX);   //stack size is now 2  [-1: ais_table -2: f]
	// Push the function in the table so that it is not collected.
	size_t length_ai = lua_rawlen(L, -1);//length of ais_table
	lua_pushvalue(L, -2); //stack size is now 3: [-1: ai_context  -2: ais_table  -3: ai_context]
	lua_rawseti(L, -2, length_ai + 1);// ais_table[length+1]=ai_context.  stack size is now 2 [-1: ais_table  -2: ai_context]
	lua_pop(L, 2);
	return new lua_ai_context(L, length_ai + 1, engine->get_readonly_context().get_side());
}

lua_ai_action_handler* lua_ai_action_handler::create(lua_State *L, char const *code, lua_ai_context &context)
{
	int res = luaL_loadstring(L, code);//stack size is now 1 [ -1: f]
	if (res)
	{
		char const *m = lua_tostring(L, -1);
		ERR_LUA << "error while creating ai function:  " <<m << '\n';
		lua_pop(L, 2);//return with stack size 0 []
		return NULL;
	}


	// Retrieve the ai elements table from the registry.
	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));
	lua_rawget(L, LUA_REGISTRYINDEX);   //stack size is now 2  [-1: ais_table -2: f]
	// Push the function in the table so that it is not collected.
	size_t length = lua_rawlen(L, -1);//length of ais_table
	lua_pushvalue(L, -2); //stack size is now 3: [-1: f  -2: ais_table  -3: f]
	lua_rawseti(L, -2, length + 1);// ais_table[length+1]=f.  stack size is now 2 [-1: ais_table  -2: f]
	lua_remove(L, -1);//stack size is now 1 [-1: f]
	lua_remove(L, -1);//stack size is now 0 []
	// Create the proxy C++ action handler.
	return new lua_ai_action_handler(L, context, length + 1);
}


void lua_ai_context::load()
{
	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));//stack size is now 1 [-1: ais_table key]
	lua_rawget(L, LUA_REGISTRYINDEX);//stack size is still 1 [-1: ais_table]
	lua_rawgeti(L, -1, num_);//stack size is 2 [-1: ai_context -2: ais_table]
	lua_remove(L,-2);
}

void lua_ai_context::load_and_inject_ai_table(ai::engine_lua* engine)
{
	load(); //stack size is 1 [-1: ai_context]
	generate_and_push_ai_table(L, engine); //stack size is 2 [-1: ai_table -2: ai_context]
	lua_setfield(L, -2, "ai"); //stack size is 1 [-1: ai_context]
}

lua_ai_context::~lua_ai_context()
{
	// Remove the ai context from the registry, so that it can be collected.
	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}

void lua_ai_action_handler::handle(config &cfg, bool configOut, lua_object_ptr l_obj)
{
	int initial_top = lua_gettop(L);//get the old stack size

	// Load the user function from the registry.
	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));//stack size is now 1 [-1: ais_table key]
	lua_rawget(L, LUA_REGISTRYINDEX);//stack size is still 1 [-1: ais_table]
	lua_rawgeti(L, -1, num_);//stack size is 2 [-1: ai_action  -2: ais_table]
	lua_remove(L, -2);//stack size is 1 [-1: ai_action]
	//load the lua ai context as a parameter
	context_.load();//stack size is 2 [-1: ai_context -2: ai_action]

	if (!configOut)
	{
		luaW_pushconfig(L, cfg);
		luaW_pcall(L, 2, 0, true);
	}
	else if (luaW_pcall(L, 1, 5, true)) // @note for Crab: how much nrets should we actually have here
	{				    // there were 2 initially, but aspects like recruitment pattern
		l_obj->store(L, initial_top + 1); // return a lot of results
	}

	lua_settop(L, initial_top);//empty stack
}

lua_ai_action_handler::~lua_ai_action_handler()
{
	// Remove the function from the registry, so that it can be collected.
	lua_pushlightuserdata(L, static_cast<void *>(const_cast<char *>(&aisKey)));
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}

} // of namespace ai
