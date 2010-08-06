/* $Id$ */
/*
   Copyright (C) 2010 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Provides core classes for the Lua AI.
 *
 */

#include "lua/lualib.h"
#include "lua/lauxlib.h"

#include <cassert>
#include <cstring>

#include "core.hpp"
#include "../../scripting/lua.hpp"
#include "../../scripting/lua_api.hpp"

#include "../../actions.hpp"
#include "../../attack_prediction.hpp"
#include "../../filesystem.hpp"
#include "../../foreach.hpp"
#include "../../game_display.hpp"
#include "../../gamestatus.hpp"
#include "../../log.hpp"
#include "../../map.hpp"
#include "../../pathfind/pathfind.hpp"
#include "../../play_controller.hpp"
#include "../../resources.hpp"
#include "../../terrain_translation.hpp"
#include "../../unit.hpp"
#include "../actions.hpp"
#include "../composite/engine_lua.hpp"

static lg::log_domain log_ai_engine_lua("ai/engine/lua");
#define LOG_LUA LOG_STREAM(info, log_ai_engine_lua)
#define ERR_LUA LOG_STREAM(err, log_ai_engine_lua)

static char const aisKey     = 0;

namespace ai {

void lua_ai_context::init(lua_State *L)
{
	// Create the ai elements table.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_newtable(L);
	lua_rawset(L, LUA_REGISTRYINDEX);
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
		unit const *u = luaW_tounit(L, index);
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

static int ai_execute_move(lua_State *L, bool remove_movement)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location from, to;
	if (!to_map_location(L, index, from)) goto error_call_destructors;
	if (!to_map_location(L, index, to)) goto error_call_destructors;
	ai::move_result_ptr move_result = ai::actions::execute_move_action(side,true,from,to,remove_movement);
	return transform_ai_action(L,move_result);
}

static int cfun_ai_execute_move_full(lua_State *L)
{
	return ai_execute_move(L, true);
}

static int cfun_ai_execute_move_partial(lua_State *L)
{
	return ai_execute_move(L, false);
}

static int cfun_ai_execute_attack(lua_State *L)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	ai::readonly_context &context = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context();

	int side = context.get_side();
	map_location attacker, defender;
	if (!to_map_location(L, index, attacker)) goto error_call_destructors;
	if (!to_map_location(L, index, defender)) goto error_call_destructors;

	int attacker_weapon = -1;//-1 means 'select what is best'
	double aggression = context.get_aggression();//use the aggression from the context

	if (!lua_isnoneornil(L, index+1) && lua_isnumber(L,index+1)) {
		aggression = lua_tonumber(L, index+1);
	}

	if (!lua_isnoneornil(L, index)) {
		attacker_weapon = lua_tointeger(L, index);
	}

	ai::attack_result_ptr attack_result = ai::actions::execute_attack_action(side,true,attacker,defender,attacker_weapon,aggression);
	return transform_ai_action(L,attack_result);
}

static int ai_execute_stopunit_select(lua_State *L, bool remove_movement, bool remove_attacks)
{
	int index = 1;
	if (false) {
		error_call_destructors:
		return luaL_typerror(L, index, "location (unit/integers)");
	}

	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location loc;
	if (!to_map_location(L, index, loc)) goto error_call_destructors;

	ai::stopunit_result_ptr stopunit_result = ai::actions::execute_stopunit_action(side,true,loc,remove_movement,remove_attacks);
	return transform_ai_action(L,stopunit_result);
}

static int cfun_ai_execute_stopunit_moves(lua_State *L)
{
	return ai_execute_stopunit_select(L, true, false);
}

static int cfun_ai_execute_stopunit_attacks(lua_State *L)
{
	return ai_execute_stopunit_select(L, false, true);
}

static int cfun_ai_execute_stopunit_all(lua_State *L)
{
	return ai_execute_stopunit_select(L, true, true);
}

static int cfun_ai_execute_recruit(lua_State *L)
{
	const char *unit_name = luaL_checkstring(L, 1);
	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location where;
	if (!lua_isnoneornil(L, 2)) {
		where.x = lua_tonumber(L, 2) - 1;
		where.y = lua_tonumber(L, 3) - 1;
	}

	ai::recruit_result_ptr recruit_result = ai::actions::execute_recruit_action(side,true,std::string(unit_name),where);
	return transform_ai_action(L,recruit_result);
}

static int cfun_ai_execute_recall(lua_State *L)
{
	const char *unit_id = luaL_checkstring(L, 1);
	int side = ((ai::engine_lua*)lua_touserdata(L,lua_upvalueindex(1)))->get_readonly_context().get_side();
	map_location where;
	if (!lua_isnoneornil(L, 2)) {
		where.x = lua_tonumber(L, 2) - 1;
		where.y = lua_tonumber(L, 3) - 1;
	}

	ai::recall_result_ptr recall_result = ai::actions::execute_recall_action(side,true,std::string(unit_id),where);
	return transform_ai_action(L,recall_result);
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
	lua_newtable(L);// stack size is 2 [ -1: new table, -2: ai as string ]
	lua_pushinteger(L, engine->get_readonly_context().get_side());

	lua_setfield(L, -2, "side");//stack size is 2 [- 1: new table; -2 ai as string]

	static luaL_reg const callbacks[] = {
		{ "attack",           &cfun_ai_execute_attack           },
		{ "move",             &cfun_ai_execute_move_partial     },
		{ "move_full",        &cfun_ai_execute_move_full        },
		{ "recall",           &cfun_ai_execute_recall           },
		{ "recruit",          &cfun_ai_execute_recruit          },
		{ "stopunit_all",     &cfun_ai_execute_stopunit_all     },
		{ "stopunit_attacks", &cfun_ai_execute_stopunit_attacks },
		{ "stopunit_moves",   &cfun_ai_execute_stopunit_moves   },
		{ NULL, NULL }
	};

	for (const luaL_reg *p = callbacks; p->name; ++p) {
		lua_pushlightuserdata(L, engine);
		lua_pushcclosure(L, p->func, 1);
		lua_setfield(L, -2, p->name);
	}

	//compile the ai as a closure
	if (!luaW_pcall(L, 1, 1, true)) {
		return NULL;//return with stack size 0 []
	}

	// Retrieve the ai elements table from the registry.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);   //stack size is now 2  [-1: ais_table -2: f]
	// Push the function in the table so that it is not collected.
	size_t length_ai = lua_objlen(L, -1);//length of ais_table
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
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);   //stack size is now 2  [-1: ais_table -2: f]
	// Push the function in the table so that it is not collected.
	size_t length = lua_objlen(L, -1);//length of ais_table
	lua_pushvalue(L, -2); //stack size is now 3: [-1: f  -2: ais_table  -3: f]
	lua_rawseti(L, -2, length + 1);// ais_table[length+1]=f.  stack size is now 2 [-1: ais_table  -2: f]
	lua_remove(L, -1);//stack size is now 1 [-1: f]
	lua_remove(L, -1);//stack size is now 0 []
	// Create the proxy C++ action handler.
	return new lua_ai_action_handler(L, context, length + 1);
}


void lua_ai_context::load()
{
	lua_pushlightuserdata(L, (void *)&aisKey);//stack size is now 1 [-1: ais_table key]
	lua_rawget(L, LUA_REGISTRYINDEX);//stack size is still 1 [-1: ais_table]
	lua_rawgeti(L, -1, num_);//stack size is 2 [-1: ai_context -2: ais_table]
	lua_remove(L,-2);
}

lua_ai_context::~lua_ai_context()
{
	// Remove the ai context from the registry, so that it can be collected.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}

void lua_ai_action_handler::handle(config &cfg, bool configOut)
{
	int initial_top = lua_gettop(L);//get the old stack size

	// Load the user function from the registry.
	lua_pushlightuserdata(L, (void *)&aisKey);//stack size is now 1 [-1: ais_table key]
	lua_rawget(L, LUA_REGISTRYINDEX);//stack size is still 1 [-1: ais_table]
	lua_rawgeti(L, -1, num_);//stack size is 2 [-1: ai_action  -2: ais_table]
	lua_remove(L, -2);//stack size is 1 [-1: ai_action]
	//load the lua ai context as a parameter
	context_.load();//stack size is 2 [-1: ai_context -2: ai_action]

	if (!configOut)
	{
		luaW_pushconfig(L, cfg);
		luaW_pcall(L, 2, LUA_MULTRET, true);
	}
	else if (lua_gettop(L) > initial_top)
	{
		if (luaW_pcall(L, 1, LUA_MULTRET, true)) {
			int score = lua_tonumber(L, initial_top + 1);//get score

			if (lua_gettop(L) >= initial_top + 2) {//check if we also have config
				luaW_toconfig(L, initial_top + 2, cfg);//get config
			}

			cfg["score"] = score; // write score to the config
		}
	}

	lua_settop(L, initial_top);//empty stack
}

lua_ai_action_handler::~lua_ai_action_handler()
{
	// Remove the function from the registry, so that it can be collected.
	lua_pushlightuserdata(L, (void *)&aisKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	lua_rawseti(L, -2, num_);
	lua_pop(L, 1);
}

} // of namespace ai
