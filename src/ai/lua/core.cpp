/*
   Copyright (C) 2010 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "ai/lua/core.hpp"
#include "ai/composite/aspect.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/push_check.hpp"
#include "ai/lua/lua_object.hpp" // (Nephro)

#include "attack_prediction.hpp"
#include "game_display.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "pathfind/pathfind.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "terrain/translation.hpp"
#include "terrain/filter.hpp"
#include "units/unit.hpp"
#include "ai/actions.hpp"
#include "ai/lua/engine_lua.hpp"
#include "ai/composite/contexts.hpp"
#include "ai/default/aspect_attacks.hpp"
#include "deprecation.hpp"

#include "lua/lualib.h"
#include "lua/lauxlib.h"
#include "lua/llimits.h"

static lg::log_domain log_ai_engine_lua("ai/engine/lua");
#define LOG_LUA LOG_STREAM(info, log_ai_engine_lua)
#define WRN_LUA LOG_STREAM(warn, log_ai_engine_lua)
#define ERR_LUA LOG_STREAM(err, log_ai_engine_lua)

static char const aisKey[] = "ai contexts";

namespace ai {

static void push_attack_analysis(lua_State *L, const attack_analysis&);

void lua_ai_context::init(lua_State *L)
{
	// Create the ai elements table.
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, aisKey);
}

void lua_ai_context::get_arguments(config &cfg) const
{
	int top = lua_gettop(L);

	lua_getfield(L, LUA_REGISTRYINDEX, aisKey);
	lua_rawgeti(L, -1, num_);

	lua_getfield(L, -1, "params");
	luaW_toconfig(L, -1, cfg);

	lua_settop(L, top);
}

void lua_ai_context::set_arguments(const config &cfg)
{
	int top = lua_gettop(L);

	lua_getfield(L, LUA_REGISTRYINDEX, aisKey);
	lua_rawgeti(L, -1, num_);

	luaW_pushconfig(L, cfg);
	lua_setfield(L, -2, "params");

	lua_settop(L, top);
}

void lua_ai_context::get_persistent_data(config &cfg) const
{
	int top = lua_gettop(L);

	lua_getfield(L, LUA_REGISTRYINDEX, aisKey);
	lua_rawgeti(L, -1, num_);

	lua_getfield(L, -1, "data");
	luaW_toconfig(L, -1, cfg);

	lua_settop(L, top);
}

void lua_ai_context::set_persistent_data(const config &cfg)
{
	int top = lua_gettop(L);

	lua_getfield(L, LUA_REGISTRYINDEX, aisKey);
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

void lua_ai_context::push_ai_table()
{
	lua_ai_load ctx(*this, false);
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
	lua_pushstring(L, actions::get_error_name(action_result->get_status()).c_str());
	lua_setfield(L, -2, "result");
	return 1;
}

static int cfun_ai_get_suitable_keep(lua_State *L)
{
	int index = 1;

	ai::readonly_context &context = get_readonly_context(L);
	unit* leader = nullptr;
	if (lua_isuserdata(L, index))
	{
		leader = luaW_tounit(L, index);
		if (!leader) return luaL_argerror(L, 1, "unknown unit");
	}
	else return luaW_type_error(L, 1, "unit");
	const map_location loc = leader->get_location();
	const pathfind::paths leader_paths(*leader, false, true, context.current_team());
	const map_location &res = context.suitable_keep(loc,leader_paths);
	if (!res.valid()) {
		return 0;
	}
	else {
		lua_pushnumber(L, res.wml_x());
		lua_pushnumber(L, res.wml_y());
		return 2;
	}
}

static int ai_move(lua_State *L, bool exec, bool remove_movement)
{
	int side = get_readonly_context(L).get_side();
	map_location from = luaW_checklocation(L, 1);
	map_location to = luaW_checklocation(L, 2);
	bool unreach_is_ok = false;
	if (lua_isboolean(L, 3)) {
		unreach_is_ok = luaW_toboolean(L, 3);
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
	ai::readonly_context &context = get_readonly_context(L);

	int side = context.get_side();
	map_location attacker = luaW_checklocation(L, 1);
	map_location defender = luaW_checklocation(L, 2);

	int attacker_weapon = -1;//-1 means 'select what is best'
	double aggression = context.get_aggression();//use the aggression from the context

	if (!lua_isnoneornil(L, 3)) {
		attacker_weapon = lua_tointeger(L, 3);
		if (attacker_weapon != -1) {
			attacker_weapon--;	// Done for consistency of the Lua style
		}
	}

	//TODO: Right now, aggression is used by the attack execution functions to determine the weapon to be used.
	// If a decision is made to expand the function that determines the weapon, this block must be refactored
	// to parse aggression if a single int is on the stack, or create a table of parameters, if a table is on the
	// stack.
	if (!lua_isnoneornil(L, 4) && lua_isnumber(L,4)) {
		aggression = lua_tonumber(L, 4);
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
	int side = get_readonly_context(L).get_side();
	map_location loc = luaW_checklocation(L, 1);

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
		location.set_wml_x(lua_tonumber(L, 2));
		location.set_wml_y(lua_tonumber(L, 3));
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
		where.set_wml_x(lua_tonumber(L, 2));
		where.set_wml_y(lua_tonumber(L, 3));
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
		where.set_wml_x(lua_tonumber(L, 2));
		where.set_wml_y(lua_tonumber(L, 3));
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

static int cfun_ai_fallback_human(lua_State*)
{
	throw fallback_ai_to_human_exception();
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
		lua_pushstring(L, it->type.to_string().c_str());
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

// Note: If adding new uses of this macro, it will be necessary to either remove the old ones
// (and the things so deprecated) OR add a version parameter to the macro.
// Also note that the name MUST be a string literal.
#define DEPRECATED_ASPECT_MESSAGE(name) \
	deprecated_message("ai.get_" name, DEP_LEVEL::PREEMPTIVE, {1, 15, 0}, "Use ai.aspects." name " instead")

// Aspect section
static int cfun_ai_get_aggression(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("aggression");
	double aggression = get_readonly_context(L).get_aggression();
	lua_pushnumber(L, aggression);
	return 1;
}

static int cfun_ai_get_attack_depth(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("attack_depth");
	int attack_depth = get_readonly_context(L).get_attack_depth();
	lua_pushnumber(L, attack_depth);
	return 1;
}

static int cfun_ai_get_attacks(lua_State *L)
{
	// Unlike the other aspect fetchers, this one is not deprecated!
	// This is because ai.aspects.attacks returns the viable units but this returns a full attack analysis
	const ai::attacks_vector& attacks = get_readonly_context(L).get_attacks();
	lua_createtable(L, attacks.size(), 0);
	int table_index = lua_gettop(L);

	ai::attacks_vector::const_iterator it = attacks.begin();
	for (int i = 1; it != attacks.end(); ++it, ++i)
	{
		push_attack_analysis(L, *it);

		lua_rawseti(L, table_index, i);
	}
	return 1;
}

static int cfun_ai_get_avoid(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("avoid");
	std::set<map_location> locs;
	terrain_filter avoid = get_readonly_context(L).get_avoid();
	avoid.get_locations(locs);
	lua_push(L, locs);

	return 1;
}

static int cfun_ai_get_caution(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("caution");
	double caution = get_readonly_context(L).get_caution();
	lua_pushnumber(L, caution);
	return 1;
}

static int cfun_ai_get_grouping(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("grouping");
	std::string grouping = get_readonly_context(L).get_grouping();
	lua_pushstring(L, grouping.c_str());
	return 1;
}

static int cfun_ai_get_leader_aggression(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("leader_aggression");
	double leader_aggression = get_readonly_context(L).get_leader_aggression();
	lua_pushnumber(L, leader_aggression);
	return 1;
}

static int cfun_ai_get_leader_goal(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("leader_goal");
	config goal = get_readonly_context(L).get_leader_goal();
	luaW_pushconfig(L, goal);
	return 1;
}

static int cfun_ai_get_leader_ignores_keep(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("leader_ignores_keep");
	bool leader_ignores_keep = get_readonly_context(L).get_leader_ignores_keep();
	lua_pushboolean(L, leader_ignores_keep);
	return 1;
}

static int cfun_ai_get_leader_value(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("leader_value");
	double leader_value = get_readonly_context(L).get_leader_value();
	lua_pushnumber(L, leader_value);
	return 1;
}

static int cfun_ai_get_passive_leader(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("passive_leader");
	bool passive_leader = get_readonly_context(L).get_passive_leader();
	lua_pushboolean(L, passive_leader);
	return 1;
}

static int cfun_ai_get_passive_leader_shares_keep(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("passive_leader_shares_keep");
	bool passive_leader_shares_keep = get_readonly_context(L).get_passive_leader_shares_keep();
	lua_pushboolean(L, passive_leader_shares_keep);
	return 1;
}

static int cfun_ai_get_recruitment_pattern(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("recruitment_pattern");
	std::vector<std::string> recruiting = get_readonly_context(L).get_recruitment_pattern();
	int size = recruiting.size();
	lua_createtable(L, size, 0); // create an empty table with predefined size
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
	DEPRECATED_ASPECT_MESSAGE("scout_village_targeting");
	double scout_village_targeting = get_readonly_context(L).get_scout_village_targeting();
	lua_pushnumber(L, scout_village_targeting);
	return 1;
}

static int cfun_ai_get_simple_targeting(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("simple_targeting");
	bool simple_targeting = get_readonly_context(L).get_simple_targeting();
	lua_pushboolean(L, simple_targeting);
	return 1;
}

static int cfun_ai_get_support_villages(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("support_villages");
	bool support_villages = get_readonly_context(L).get_support_villages();
	lua_pushboolean(L, support_villages);
	return 1;
}

static int cfun_ai_get_village_value(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("village_value");
	double village_value = get_readonly_context(L).get_village_value();
	lua_pushnumber(L, village_value);
	return 1;
}

static int cfun_ai_get_villages_per_scout(lua_State *L)
{
	DEPRECATED_ASPECT_MESSAGE("villages_per_scout");
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
	const attack_analysis* aa_ptr = static_cast< attack_analysis * >(lua_touserdata(L, -1));

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

static void push_attack_analysis(lua_State *L, const attack_analysis& aa)
{
	lua_newtable(L);

	// Pushing a pointer to the current object
	lua_pushstring(L, "att_ptr");
	lua_pushlightuserdata(L, const_cast<attack_analysis*>(&aa));
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

	std::hash<map_location> lhash;

	do
	{
		map_location key = it->first;
		lua_pushinteger(L, lhash(key));

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

template<typename T>
typesafe_aspect<T>* try_aspect_as(aspect_ptr p)
{
	return std::dynamic_pointer_cast<typesafe_aspect<T> >(p).get();
}

static int impl_ai_aspect_get(lua_State* L)
{
	const aspect_map& aspects = get_engine(L).get_readonly_context().get_aspects();
	aspect_map::const_iterator iter = aspects.find(luaL_checkstring(L, 2));
	if(iter == aspects.end()) {
		return 0;
	}

	typedef std::vector<std::string> string_list;
	if(typesafe_aspect<bool>* aspect_as_bool = try_aspect_as<bool>(iter->second)) {
		lua_pushboolean(L, aspect_as_bool->get());
	} else if(typesafe_aspect<int>* aspect_as_int = try_aspect_as<int>(iter->second)) {
		lua_pushinteger(L, aspect_as_int->get());
	} else if(typesafe_aspect<double>* aspect_as_double = try_aspect_as<double>(iter->second)) {
		lua_pushnumber(L, aspect_as_double->get());
	} else if(typesafe_aspect<config>* aspect_as_config = try_aspect_as<config>(iter->second)) {
		luaW_pushconfig(L, aspect_as_config->get());
	} else if(typesafe_aspect<string_list>* aspect_as_string_list = try_aspect_as<string_list>(iter->second)) {
		lua_push(L, aspect_as_string_list->get());
	} else if(typesafe_aspect<terrain_filter>* aspect_as_terrain_filter = try_aspect_as<terrain_filter>(iter->second)) {
		std::set<map_location> result;
		aspect_as_terrain_filter->get().get_locations(result);
		lua_push(L, result);
	} else if(typesafe_aspect<attacks_vector>* aspect_as_attacks_vector = try_aspect_as<attacks_vector>(iter->second)) {
		using ai_default_rca::aspect_attacks_base;
		aspect_attacks_base* real_aspect = dynamic_cast<aspect_attacks_base*>(aspect_as_attacks_vector);
		while(real_aspect == nullptr) {
			// It's probably a composite aspect, so find the active facet
			composite_aspect<attacks_vector>& composite = dynamic_cast<composite_aspect<attacks_vector>&>(*aspect_as_attacks_vector);
			aspect_as_attacks_vector = &dynamic_cast<typesafe_aspect<attacks_vector>&>(composite.find_active());
			real_aspect = dynamic_cast<aspect_attacks_base*>(aspect_as_attacks_vector);
		}
		int my_side = get_engine(L).get_readonly_context().get_side();
		std::vector<unit_const_ptr> attackers, enemies;
		for(unit_map::const_iterator u = resources::gameboard->units().begin(); u != resources::gameboard->units().end(); ++u) {
			if(!u.valid()) {
				continue;
			}
			if(u->side() == my_side && real_aspect->is_allowed_attacker(*u)) {
				attackers.push_back(u.get_shared_ptr());
			} else if(u->side() != my_side && real_aspect->is_allowed_enemy(*u)) {
				enemies.push_back(u.get_shared_ptr());
			}
		}
		lua_createtable(L, 0, 2);
		lua_createtable(L, attackers.size(), 0);
		for(size_t i = 0; i < attackers.size(); i++) {
			luaW_pushunit(L, attackers[i]->underlying_id());
			lua_rawseti(L, -2, i + 1);
		}
		lua_setfield(L, -2, "own");
		lua_createtable(L, enemies.size(), 0);
		for(size_t i = 0; i < enemies.size(); i++) {
			luaW_pushunit(L, enemies[i]->underlying_id());
			lua_rawseti(L, -2, i + 1);
		}
		lua_setfield(L, -2, "enemy");
		return 1;
	} else if(typesafe_aspect<unit_advancements_aspect>* aspect_as_unit_advancements_aspects = try_aspect_as<unit_advancements_aspect>(iter->second)) {
		const unit_advancements_aspect& val = aspect_as_unit_advancements_aspects->get();
		int my_side = get_engine(L).get_readonly_context().get_side();
		lua_newtable(L);
		std::hash<map_location> lhash;
		for (unit_map::const_iterator u = resources::gameboard->units().begin(); u != resources::gameboard->units().end(); ++u) {
			if (!u.valid() || u->side() != my_side) {
				continue;
			}
			lua_pushinteger(L, lhash(u->get_location()));
			lua_push(L, val.get_advancements(u));
			lua_settable(L, -3);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int impl_ai_aspect_set(lua_State* L)
{
	lua_pushstring(L, "attempted to write to the ai.aspects table, which is read-only");
	return lua_error(L);
}

static int impl_ai_get(lua_State* L)
{
	if(!lua_isstring(L,2)) {
		return 0;
	}
	ai::engine_lua& engine = get_engine(L);
	std::string m = lua_tostring(L,2);
	if(m == "side") {
		lua_pushinteger(L, engine.get_readonly_context().get_side());
		return 1;
	}
	if(m == "aspects") {
		lua_newtable(L); // [-1: Aspects table]
		lua_newtable(L); // [-1: Aspects metatable  -2: Aspects table]
		lua_pushlightuserdata(L, &engine); // [-1: Engine  -2: Aspects mt  -3: Aspects table]
		lua_pushcclosure(L, &impl_ai_aspect_get, 1); // [-1: Metafunction  -2: Aspects mt  -3: Aspects table]
		lua_setfield(L, -2, "__index"); // [-1: Aspects metatable  -2: Aspects table]
		lua_pushcfunction(L, &impl_ai_aspect_set); // [-1: Metafunction  -2: Aspects mt  -3: Aspects table]
		lua_setfield(L, -2, "__newindex"); // [-1: Aspects metatable  -2: Aspects table]
		lua_setmetatable(L, -2); // [-1: Aspects table]
		return 1;
	}
	static luaL_Reg const callbacks[] = {
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
			{ "get_passive_leader", &cfun_ai_get_passive_leader },
			{ "get_passive_leader_shares_keep", &cfun_ai_get_passive_leader_shares_keep },
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
			{ "suitable_keep", &cfun_ai_get_suitable_keep },
			{ "check_recall", &cfun_ai_check_recall },
			{ "check_move", &cfun_ai_check_move },
			{ "check_stopunit", &cfun_ai_check_stopunit },
			{ "check_synced_command", &cfun_ai_check_synced_command },
			{ "check_attack", &cfun_ai_check_attack },
			{ "check_recruit", &cfun_ai_check_recruit },
			//{ "",},
			//{ "",},
			{ nullptr, nullptr } };
	for (const luaL_Reg* p = callbacks; p->name; ++p) {
		if(m == p->name) {
			lua_pushlightuserdata(L, &engine); // [-1: engine  ...]
			lua_pushcclosure(L, p->func, 1); // [-1: function  ...]
			// Store the function so that __index doesn't need to be called next time
			lua_pushstring(L, p->name); // [-1: name  -2: function  ...]
			lua_pushvalue(L, -2); // [-1: function  -2: name  -3: function ...]
			lua_rawset(L, 1); // [-1: function  ...]
			return 1;
		}
	}
	lua_pushstring(L, "read_only");
	lua_rawget(L, 1);
	bool read_only = luaW_toboolean(L, -1);
	lua_pop(L, 1);
	if(read_only) {
		return 0;
	}
	static luaL_Reg const mutating_callbacks[] = {
			{ "attack", &cfun_ai_execute_attack },
			{ "move", &cfun_ai_execute_move_partial },
			{ "move_full", &cfun_ai_execute_move_full },
			{ "recall", &cfun_ai_execute_recall },
			{ "recruit", &cfun_ai_execute_recruit },
			{ "stopunit_all", &cfun_ai_execute_stopunit_all },
			{ "stopunit_attacks", &cfun_ai_execute_stopunit_attacks },
			{ "stopunit_moves", &cfun_ai_execute_stopunit_moves },
			{ "synced_command", &cfun_ai_execute_synced_command },
			{ "fallback_human", &cfun_ai_fallback_human},
			{ nullptr, nullptr } };
	for (const luaL_Reg* p = mutating_callbacks; p->name; ++p) {
		if(m == p->name) {
			lua_pushlightuserdata(L, &engine);
			lua_pushcclosure(L, p->func, 1);
			return 1;
		}
	}
	return 0;
}

static void generate_and_push_ai_table(lua_State* L, ai::engine_lua* engine) {
	//push data table here
	lua_newtable(L); // [-1: ai table]
	lua_newtable(L); // [-1: metatable  -2: ai table]
	lua_pushlightuserdata(L, engine); // [-1: engine  -2: metatable  -3: ai table]
	lua_pushcclosure(L, &impl_ai_get, 1); // [-1: metafunc  -2: metatable  -3: ai table]
	lua_setfield(L, -2, "__index"); // [-1: metatable  -2: ai table]
	lua_setmetatable(L, -2); // [-1: ai table]
}

static size_t generate_and_push_ai_state(lua_State* L, ai::engine_lua* engine)
{
	// Retrieve the ai elements table from the registry.
	lua_getfield(L, LUA_REGISTRYINDEX, aisKey); // [-1: AIs registry table]
	size_t length_ai = lua_rawlen(L, -1); // length of table
	lua_newtable(L); // [-1: AI state table  -2: AIs registry table]
	generate_and_push_ai_table(L, engine); // [-1: AI routines  -2: AI state  -3: AIs registry]
	lua_setfield(L, -2, "ai"); // [-1: AI state  -2: AIs registry]
	lua_pushvalue(L, -1); // [-1: AI state  -2: AI state  -3: AIs registry]
	lua_rawseti(L, -3, length_ai + 1); // [-1: AI state  -2: AIs registry]
	lua_remove(L, -2); // [-1: AI state table]
	return length_ai + 1;
}

lua_ai_context* lua_ai_context::create(lua_State *L, char const *code, ai::engine_lua *engine)
{
	int res_ai = luaL_loadstring(L, code); // [-1: AI code]
	if (res_ai != 0)
	{

		char const *m = lua_tostring(L, -1);
		ERR_LUA << "error while initializing ai:  " <<m << '\n';
		lua_pop(L, 2);//return with stack size 0 []
		return nullptr;
	}
	//push data table here
	size_t idx = generate_and_push_ai_state(L, engine); // [-1: AI state  -2: AI code]
	lua_pushvalue(L, -2); // [-1: AI code  -2: AI state  -3: AI code]
	lua_setfield(L, -2, "update_self"); // [-1: AI state  -2: AI code]
	lua_pushlightuserdata(L, engine);
	lua_setfield(L, -2, "engine"); // [-1: AI state  -2: AI code]
	lua_pop(L, 2);
	return new lua_ai_context(L, idx, engine->get_readonly_context().get_side());
}

void lua_ai_context::update_state()
{
	lua_ai_load ctx(*this, true); // [-1: AI state table]

	// Load the AI code and arguments
	lua_getfield(L, -1, "update_self"); // [-1: AI code  -2: AI state]
	lua_getfield(L, -2, "params"); // [-1: Arguments  -2: AI code  -3: AI state]
	lua_getfield(L, -3, "data"); // [-1: Persistent data  -2: Arguments  -3: AI code  -4: AI state]

	// Call the function
	if (!luaW_pcall(L, 2, 1, true)) { // [-1: Result  -2: AI state]
		return; // return with stack size 0 []
	}

	// Store the state for use by components
	lua_setfield(L, -2, "self"); // [-1: AI state]

	// And return with empty stack.
	lua_pop(L, 1);
}

lua_ai_action_handler* lua_ai_action_handler::create(lua_State *L, char const *code, lua_ai_context &context)
{
	int res = luaL_loadstring(L, code);//stack size is now 1 [ -1: f]
	if (res)
	{
		char const *m = lua_tostring(L, -1);
		ERR_LUA << "error while creating ai function:  " <<m << '\n';
		lua_pop(L, 2);//return with stack size 0 []
		return nullptr;
	}

	// Retrieve the ai elements table from the registry.
	lua_getfield(L, LUA_REGISTRYINDEX, aisKey);   //stack size is now 2  [-1: ais_table -2: f]
	// Push the function in the table so that it is not collected.
	size_t length = lua_rawlen(L, -1);//length of ais_table
	lua_pushvalue(L, -2); //stack size is now 3: [-1: f  -2: ais_table  -3: f]
	lua_rawseti(L, -2, length + 1);// ais_table[length+1]=f.  stack size is now 2 [-1: ais_table  -2: f]
	lua_remove(L, -1);//stack size is now 1 [-1: f]
	lua_remove(L, -1);//stack size is now 0 []
	// Create the proxy C++ action handler.
	return new lua_ai_action_handler(L, context, length + 1);
}


int lua_ai_load::refcount = 0;

lua_ai_load::lua_ai_load(lua_ai_context& ctx, bool read_only) : L(ctx.L), was_readonly(false)
{
	refcount++;
	// Check if the AI table is already loaded. If so, we have less work to do.
	lua_getglobal(L, "ai");
	if(!lua_isnoneornil(L, -1)) {
		// Save the previous read-only state
		lua_getfield(L, -1, "read_only");
		was_readonly = luaW_toboolean(L, -1);
		lua_pop(L, 1);
		// Update the read-only state
		lua_pushstring(L, "read_only");
		lua_pushboolean(L, read_only);
		lua_rawset(L, -3);
		return; // Leave the AI table on the stack, as requested
	}
	lua_pop(L, 1); // Pop the nil value off the stack
	lua_getfield(L, LUA_REGISTRYINDEX, aisKey); // [-1: AI registry]
	lua_rawgeti(L, -1, ctx.num_); // [-1: AI state  -2: AI registry]
	lua_remove(L,-2); // [-1: AI state]

	// Load the AI functions table into global scope
	lua_getfield(L, -1, "ai"); // [-1: AI functions  -2: AI state]
	lua_pushstring(L, "read_only"); // [-1: key  -2: AI functions  -3: AI state]
	lua_pushboolean(L, read_only); // [-1: value  -2: key  -3: AI functions  -4: AI state]
	lua_rawset(L, -3); // [-1: AI functions  -2: AI state]
	lua_setglobal(L, "ai"); // [-1: AI state]
}

lua_ai_load::~lua_ai_load()
{
	refcount--;
	if (refcount == 0) {
		// Remove the AI functions from the global scope
		lua_pushnil(L);
		lua_setglobal(L, "ai");
	} else {
		// Restore the read-only state
		lua_getglobal(L, "ai");
		lua_pushstring(L, "read_only");
		lua_pushboolean(L, was_readonly);
		lua_rawset(L, -3);
		lua_pop(L, 1);
	}
}

lua_ai_context::~lua_ai_context()
{
	// Remove the ai context from the registry, so that it can be collected.
	lua_getfield(L, LUA_REGISTRYINDEX, aisKey);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}

void lua_ai_action_handler::handle(const config &cfg, bool read_only, lua_object_ptr l_obj)
{
	int initial_top = lua_gettop(L);//get the old stack size

	// Load the context
	lua_ai_load ctx(context_, read_only); // [-1: AI state table]

	// Load the user function from the registry.
	lua_getfield(L, LUA_REGISTRYINDEX, aisKey); // [-1: AI registry  -2: AI state]
	lua_rawgeti(L, -1, num_); // [-1: AI action  -2: AI registry  -3: AI state]
	lua_remove(L, -2); // [-1: AI action  -2: AI state]

	// Load the arguments
	int iState = lua_absindex(L, -2);
	lua_getfield(L, iState, "self");
	luaW_pushconfig(L, cfg);
	lua_getfield(L, iState, "data");

	// Call the function
	luaW_pcall(L, 3, l_obj ? 1 : 0, true);
	if (l_obj) {
		l_obj->store(L, -1);
	}

	lua_settop(L, initial_top);//empty stack
}

lua_ai_action_handler::~lua_ai_action_handler()
{
	// Remove the function from the registry, so that it can be collected.
	lua_getfield(L, LUA_REGISTRYINDEX, aisKey);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}

} // of namespace ai
