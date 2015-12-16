/*
   Copyright (C) 2009 - 2015 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
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
 * Provides a Lua interpreter, to be embedded in WML.
 *
 * @note Naming conventions:
 *   - intf_ functions are exported in the wesnoth domain,
 *   - impl_ functions are hidden inside metatables,
 *   - cfun_ functions are closures,
 *   - luaW_ functions are helpers in Lua style.
 */

#include "scripting/game_lua_kernel.hpp"

#include "global.hpp"

#include "actions/attack.hpp"           // for battle_context_unit_stats, etc
#include "actions/vision.hpp"		// for clear_shroud
#include "ai/composite/ai.hpp"          // for ai_composite
#include "ai/composite/component.hpp"   // for component, etc
#include "ai/composite/contexts.hpp"    // for ai_context
#include "ai/composite/engine_lua.hpp"  // for engine_lua
#include "ai/composite/rca.hpp"  // for candidate_action
#include "ai/composite/stage.hpp"  // for stage
#include "ai/configuration.hpp"         // for configuration
#include "ai/lua/core.hpp"              // for lua_ai_context, etc
#include "ai/manager.hpp"               // for manager, holder
#include "attack_prediction.hpp"        // for combatant
#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"                   // for config, etc
#include "display_chat_manager.hpp"	// for clear_chat_messages
#include "utils/foreach.tpp"
#include "formatter.hpp"
#include "game_board.hpp"               // for game_board
#include "game_classification.hpp"      // for game_classification, etc
#include "game_config.hpp"              // for debug, base_income, etc
#include "game_config_manager.hpp"      // for game_config_manager
#include "game_data.hpp"               // for game_data, etc
#include "game_display.hpp"             // for game_display
#include "game_errors.hpp"              // for game_error
#include "game_events/conditional_wml.hpp"  // for conditional_passed
#include "game_events/entity_location.hpp"
#include "game_events/manager.hpp"	// for add_event_handler
#include "game_events/pump.hpp"         // for queued_event
#include "game_preferences.hpp"         // for encountered_units
#include "help/help.hpp"
#include "image.hpp"                    // for get_image, locator
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "make_enum.hpp"                // for operator<<
#include "map.hpp"                      // for gamemap
#include "map_label.hpp"
#include "map_location.hpp"             // for map_location
#include "mouse_events.hpp"             // for mouse_handler
#include "mp_game_settings.hpp"         // for mp_game_settings
#include "pathfind/pathfind.hpp"        // for full_cost_map, plain_route, etc
#include "pathfind/teleport.hpp"        // for get_teleport_locations, etc
#include "play_controller.hpp"          // for play_controller
#include "recall_list_manager.hpp"      // for recall_list_manager
#include "replay.hpp"                   // for get_user_choice, etc
#include "reports.hpp"                  // for register_generator, etc
#include "scripting/lua_api.hpp"        // for luaW_toboolean, etc
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_gui2.hpp"	// for show_gamestate_inspector
#include "scripting/lua_race.hpp"
#include "scripting/lua_team.hpp"
#include "scripting/lua_types.hpp"      // for getunitKey, dlgclbkKey, etc
#include "scripting/lua_unit_type.hpp"
#include "scripting/push_check.hpp"
#include "sdl/utils.hpp"                // for surface
#include "side_filter.hpp"              // for side_filter
#include "sound.hpp"                    // for commit_music_changes, etc
#include "synced_context.hpp"           // for synced_context, etc
#include "synced_user_choice.hpp"
#include "team.hpp"                     // for team, village_owner
#include "terrain.hpp"                  // for terrain_type
#include "terrain_filter.hpp"           // for terrain_filter
#include "terrain_translation.hpp"      // for read_terrain_code, etc
#include "terrain_type_data.hpp"
#include "time_of_day.hpp"              // for time_of_day, tod_color
#include "tod_manager.hpp"              // for tod_manager
#include "tstring.hpp"                  // for t_string, operator+
#include "unit.hpp"                     // for unit, intrusive_ptr_add_ref, etc
#include "unit_animation_component.hpp"  // for unit_animation_component
#include "unit_display.hpp"
#include "unit_filter.hpp"
#include "unit_map.hpp"  // for unit_map, etc
#include "unit_ptr.hpp"                 // for unit_const_ptr, unit_ptr
#include "unit_types.hpp"    // for unit_type_data, unit_types, etc
#include "util.hpp"                     // for lexical_cast
#include "variable.hpp"                 // for vconfig, etc
#include "variable_info.hpp"
#include "wml_exception.hpp"

#include <boost/bind.hpp>               // for bind_t, bind
#include <boost/foreach.hpp>            // for auto_any_base, etc
#include <boost/intrusive_ptr.hpp>      // for intrusive_ptr
#include <boost/optional.hpp>
#include <boost/range/algorithm/copy.hpp>    // boost::copy
#include <boost/range/adaptors.hpp>     // boost::adaptors::filtered
#include <boost/tuple/tuple.hpp>        // for tuple
#include <cassert>                      // for assert
#include <cstring>                      // for strcmp, NULL
#include <iterator>                     // for distance, advance
#include <map>                          // for map, map<>::value_type, etc
#include <new>                          // for operator new
#include <set>                          // for set
#include <sstream>                      // for operator<<, basic_ostream, etc
#include <utility>                      // for pair
#include <vector>                       // for vector, etc
#include "SDL_timer.h"                  // for SDL_GetTicks
#include "SDL_video.h"                  // for SDL_Color, SDL_Surface
#include "lua/lauxlib.h"                // for luaL_checkinteger, etc
#include "lua/lua.h"                    // for lua_setfield, etc

#include "resources.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"

class CVideo;

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

// Suppress uninitialized variables warnings, because of boost::optional constructors in this file which apparently confuses gcc
#if defined(__GNUC__) && !defined(__clang__) // we shouldn't need this for clang, but for gcc and tdm-gcc we probably do
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6 ) // "GCC diagnostic ignored" is apparently not available at version 4.5.2
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#endif

static lg::log_domain log_scripting_lua("scripting/lua");
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

std::vector<config> game_lua_kernel::preload_scripts;
config game_lua_kernel::preload_config;

void game_lua_kernel::extract_preload_scripts(config const &game_config)
{
	game_lua_kernel::preload_scripts.clear();
	BOOST_FOREACH(config const &cfg, game_config.child_range("lua")) {
		game_lua_kernel::preload_scripts.push_back(cfg);
	}
	game_lua_kernel::preload_config = game_config.child("game_config");
}

void game_lua_kernel::log_error(char const * msg, char const * context)
{
	lua_kernel_base::log_error(msg, context);
	lua_chat(context, msg);
}

void game_lua_kernel::lua_chat(std::string const &caption, std::string const &msg)
{
	if (game_display_) {
		game_display_->get_chat_manager().add_chat_message(time(NULL), caption, 0, msg,
			events::chat_handler::MESSAGE_PUBLIC, false);
	}
}

/**
 * Gets a vector of sides from side= attribute in a given config node.
 * Promotes consistent behavior.
 */
std::vector<int> game_lua_kernel::get_sides_vector(const vconfig& cfg)
{
	const config::attribute_value sides = cfg["side"];
	const vconfig &ssf = cfg.child("filter_side");

	if (!ssf.null()) {
		if(!sides.empty()) { WRN_LUA << "ignoring duplicate side filter information (inline side=)" << std::endl; }
		side_filter filter(ssf, &game_state_);
		return filter.get_teams();
	}

	side_filter filter(sides.str(), &game_state_);
	return filter.get_teams();
}


namespace {
	/**
	 * Temporary entry to a queued_event stack
	 */
	struct queued_event_context
	{
		typedef game_events::queued_event qe;
		std::stack<qe const *> & stack_;

		queued_event_context(qe const *new_qe, std::stack<qe const*> & stack)
			: stack_(stack)
		{
			stack_.push(new_qe);
		}

		~queued_event_context()
		{
			stack_.pop();
		}
	};
}//unnamed namespace for queued_event_context

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

	if (strcmp(m, "valid") == 0)
	{
		if (!pu) return 0;
		if (lu->on_map())
			lua_pushstring(L, "map");
		else if (lu->on_recall_list())
			lua_pushstring(L, "recall");
		else
			lua_pushstring(L, "private");
		return 1;
	}

	if (!pu) return luaL_argerror(L, 1, "unknown unit");
	unit const &u = *pu;

	// Find the corresponding attribute.
	return_int_attrib("x", u.get_location().x + 1);
	return_int_attrib("y", u.get_location().y + 1);
	if (strcmp(m, "loc") == 0) {
		lua_pushinteger(L, u.get_location().x + 1);
		lua_pushinteger(L, u.get_location().y + 1);
		return 2;
	}
	return_int_attrib("side", u.side());
	return_string_attrib("id", u.id());
	return_string_attrib("type", u.type_id());
	return_string_attrib("image_mods", u.effect_image_mods());
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

	return_vector_string_attrib("extra_recruit", u.recruits());
	return_vector_string_attrib("advances_to", u.advances_to());

	if (strcmp(m, "upkeep") == 0) {
		const config::attribute_value& upkeep = u.upkeep_raw();
		if(upkeep == "full"){
			lua_pushstring(L, "full");
		}
		else {
			lua_push(L, upkeep.to_int());
		}
		return 1;
	}
	if (strcmp(m, "advancements") == 0) {
		lua_push(L, boost::iterator_range<config::const_child_iterator>(u.modification_advancements()));
		return 1;
	}
	if (strcmp(m, "overlays") == 0) {
		lua_push(L, u.overlays());
		return 1;
	}
	if (strcmp(m, "status") == 0) {
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, 1);
		lua_rawseti(L, -2, 1);
		lua_pushlightuserdata(L
				, ustatusKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_setmetatable(L, -2);
		return 1;
	}
	if (strcmp(m, "variables") == 0) {
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, 1);
		lua_rawseti(L, -2, 1);
		lua_pushlightuserdata(L
				, unitvarKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_setmetatable(L, -2);
		return 1;
	}
	if (strcmp(m, "attacks") == 0) {
		lua_createtable(L, 1, 0);
		lua_pushvalue(L, 1);
		// hack: store the unit at -1 becasue we want positive indexes to refers to the attacks.
		lua_rawseti(L, -2, -1);
		lua_pushlightuserdata(L, uattacksKey);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_setmetatable(L, -2);
		return 1;
	}
	return_bool_attrib("hidden", u.get_hidden());
	return_bool_attrib("petrified", u.incapacitated());
	return_bool_attrib("resting", u.resting());
	return_string_attrib("role", u.get_role());
	return_string_attrib("race", u.race()->id());
	return_string_attrib("gender", gender_string(u.gender()));
	return_string_attrib("variation", u.variation());
	return_bool_attrib("zoc", u.get_emit_zoc());
	return_string_attrib("facing", map_location::write_direction(u.facing()));
	return_cfg_attrib("__cfg", u.write(cfg); u.get_location().write(cfg));

	return lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).return_unit_method(L, m);
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
	modify_bool_attrib("resting", u.set_resting(value));
	modify_tstring_attrib("name", u.set_name(value));
	modify_string_attrib("role", u.set_role(value));
	modify_string_attrib("facing", u.set_facing(map_location::parse_direction(value)));
	modify_bool_attrib("hidden", u.set_hidden(value));
	modify_bool_attrib("zoc", u.set_emit_zoc(value));
	modify_bool_attrib("canrecruit", u.set_can_recruit(value));

	modify_vector_string_attrib("extra_recruit", u.set_recruits(vector));
	modify_vector_string_attrib("advances_to", u.set_advances_to(vector));

	if (strcmp(m, "advancements") == 0) {
		u.set_advancements(lua_check<std::vector<config> >(L, 3));
		return 0;
	}

	if (!lu->on_map()) {
		map_location loc = u.get_location();
		modify_int_attrib("x", loc.x = value - 1; u.set_location(loc));
		modify_int_attrib("y", loc.y = value - 1; u.set_location(loc));
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
	if (!lua_istable(L, 1))
		return luaL_typerror(L, 1, "unit status");
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	if (!u) return luaL_argerror(L, 1, "unknown unit");
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
	if (!lua_istable(L, 1))
		return luaL_typerror(L, 1, "unit status");
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if (!u) return luaL_argerror(L, 1, "unknown unit");
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
	if (!lua_istable(L, 1))
		return luaL_typerror(L, 1, "unit variables");
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	if (!u) return luaL_argerror(L, 1, "unknown unit");
	char const *m = luaL_checkstring(L, 2);
	return_cfgref_attrib("__cfg", u->variables());

	variable_access_const v(m, u->variables());
	return LuaW_pushvariable(L, v) ? 1 : 0;
}
/**
 * Gets the attacks of a unit (__index metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: index (int) or id (string) identifying a particular attack.
 * - Ret 1: the unit's attacks.
 */
static int impl_unit_attacks_get(lua_State *L)
{
	if (!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attacks");
	}
	lua_rawgeti(L, 1, -1);
	const unit* u = luaW_tounit(L, -1);
	if (!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	const attack_type* attack = NULL;
	const std::vector<attack_type>& attacks = u->attacks();
	if(!lua_isnumber(L,2)) {
		std::string attack_id = luaL_checkstring(L, 2);
		BOOST_FOREACH(const attack_type& at, attacks) {
			if(at.id() == attack_id) {
				attack = &at;
				break;
			}
		}
		if (attack == NULL) {
			//return nil on invalid index, just like lua tables do.
			return 0;
		}
	}
	else
	{
		//
		size_t index = luaL_checkinteger(L, 2) - 1;
		if (index >= attacks.size()) {
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
	lua_pushlightuserdata(L, uattackKey);
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
	if (!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attacks");
	}
	lua_rawgeti(L, 1, -1);
	const unit* u = luaW_tounit(L, -1);
	if (!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	lua_pushinteger(L, u->attacks().size());
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
	if (!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attack");
	}
	lua_rawgeti(L, 1, 1);
	const unit* u = luaW_tounit(L, -1);
	if (!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	lua_rawgeti(L, 1, 2);
	std::string attack_id = luaL_checkstring(L, -1);
	char const *m = luaL_checkstring(L, 2);
	BOOST_FOREACH(const attack_type& attack, u->attacks())
	{
		if(attack.id() == attack_id)
		{

			return_string_attrib("description", attack.name());
			return_string_attrib("name", attack.id());
			return_string_attrib("type", attack.type());
			return_string_attrib("icon", attack.icon());
			return_string_attrib("range", attack.range());
			// "min_range"
			// "max_range"
			return_int_attrib("damage", attack.damage());
			return_int_attrib("number", attack.num_attacks());
			return_int_attrib("attack_weight", attack.attack_weight());
			return_int_attrib("defense_weight", attack.defense_weight());
			return_int_attrib("accuracy", attack.accuracy());
			return_int_attrib("movement_used", attack.movement_used());
			return_int_attrib("parry", attack.parry());
			return_cfgref_attrib("specials", attack.specials());
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
	if (!lua_istable(L, 1)) {
		return luaL_typerror(L, 1, "unit attack");
	}
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if (!u) {
		return luaL_argerror(L, 1, "unknown unit");
	}
	lua_rawgeti(L, 1, 2);
	std::string attack_id = luaL_checkstring(L, -1);
	char const *m = luaL_checkstring(L, 2);
	BOOST_FOREACH(attack_type& attack, u->attacks())
	{
		if(attack.id() == attack_id)
		{

			modify_tstring_attrib("description", attack.set_name(value));
			// modify_string_attrib("name", attack.set_id(value));
			modify_string_attrib("type", attack.set_type(value));
			modify_string_attrib("icon", attack.set_icon(value));
			modify_string_attrib("range", attack.set_range(value));
			// "min_range"
			// "max_range"
			modify_int_attrib("damage", attack.set_damage(value));
			modify_int_attrib("number", attack.set_num_attacks(value));
			modify_int_attrib("attack_weight", attack.set_attack_weight(value));
			modify_int_attrib("defense_weight", attack.set_defense_weight(value));
			modify_int_attrib("accuracy", attack.set_accuracy(value));
			modify_int_attrib("movement_used", attack.set_movement_used(value));
			modify_int_attrib("parry", attack.set_parry(value));

			if (strcmp(m, "specials") == 0) { \
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

/**
 * Sets the variable of a unit (__newindex metamethod).
 * - Arg 1: table containing the userdata containing the unit id.
 * - Arg 2: string containing the name of the status.
 * - Arg 3: scalar.
 */
static int impl_unit_variables_set(lua_State *L)
{
	if (!lua_istable(L, 1))
		return luaL_typerror(L, 1, "unit variables");
	lua_rawgeti(L, 1, 1);
	unit* u = luaW_tounit(L, -1);
	if (!u) return luaL_argerror(L, 1, "unknown unit");
	char const *m = luaL_checkstring(L, 2);
	if (strcmp(m, "__cfg") == 0) {
		u->variables() = luaW_checkconfig(L, 3);
		return 0;
	}
	variable_access_create v(m, u->variables());
	LuaW_checkvariable(L, v, 3);
	return 0;
}

/**
 * Gets currently viewing side.
 * - Ret 1: integer specifying the currently viewing side
 * - Ret 2: Bool whether the vision is not limited to that team, this can for example be true during replays.
 */
static int intf_get_viewing_side(lua_State *L)
{
	if(const display* disp = display::get_singleton()) {
		lua_pushinteger(L, disp->viewing_side());
		lua_pushboolean(L, disp->show_everything());
		return 2;
	}
	else {
		return 0;
	}
}

int game_lua_kernel::intf_animate_unit(lua_State *L)
{
	// if (game_display_)
	{
		events::command_disabler disable_commands;
		unit_display::wml_animation(luaW_checkvconfig(L, 1), get_event_info().loc1);
	}
	return 0;
}

int game_lua_kernel::intf_gamestate_inspector(lua_State *L)
{
	if (game_display_) {
		return lua_gui2::show_gamestate_inspector(game_display_->video(), luaW_checkvconfig(L, 1));
	}
	return 0;
}

/**
 * Gets the unit at the given location or with the given id.
 * - Arg 1: table containing location either at indices 1,2 or at keys x,y.
 * OR
 * - Arg 1: string ID
 * OR
 * - Args 1, 2: location.
 * - Ret 1: full userdata with __index pointing to impl_unit_get and
 *          __newindex pointing to impl_unit_set.
 */
int game_lua_kernel::intf_get_unit(lua_State *L)
{
	int x, y;
	if (lua_isnoneornil(L, 2)) {
		if (lua_istable(L, 1)) {
			lua_rawgeti(L, 1, 1);
			if (lua_isnoneornil(L, -1)) {
				lua_getfield(L, 1, "x");
				x = luaL_checkinteger(L, -1) - 1;
				lua_getfield(L, 1, "y");
				y = luaL_checkinteger(L, -1) - 1;
			} else {
				x = luaL_checkinteger(L, -1) - 1;
				lua_rawgeti(L, 1, 2);
				y = luaL_checkinteger(L, -1) - 1;
			}
		} else if(lua_isstring(L, 1)) {
			// Forward to wesnoth.get_units and return the first result
			luaW_getglobal(L, "wesnoth", "get_units", NULL);
			lua_newtable(L);
			lua_pushstring(L, "id");
			lua_pushvalue(L, 1);
			lua_rawset(L, -3);
			lua_call(L, 1, 1);
			lua_rawgeti(L, -1, 1);
			return 1;
		} else {
			return luaL_argerror(L, 1, "expected string, location table, or integer");
		}
	} else {
		x = luaL_checkinteger(L, 1) - 1;
		y = luaL_checkinteger(L, 2) - 1;
	}
	unit_map::const_iterator ui = units().find(map_location(x, y));

	if (!ui.valid()) return 0;

	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->underlying_id());
	lua_pushlightuserdata(L
			, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Gets the unit displayed in the sidebar.
 * - Ret 1: full userdata with __index pointing to impl_unit_get and
 *          __newindex pointing to impl_unit_set.
 */
int game_lua_kernel::intf_get_displayed_unit(lua_State *L)
{
	if (!game_display_) {
		return 0;
	}

	unit_map::const_iterator ui = board().find_visible_unit(
		game_display_->displayed_unit_hex(),
		teams()[game_display_->viewing_team()],
		game_display_->show_everything());
	if (!ui.valid()) return 0;

	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->underlying_id());
	lua_pushlightuserdata(L
			, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Gets all the units matching a given filter.
 * - Arg 1: optional table containing a filter
 * - Ret 1: table containing full userdata with __index pointing to
 *          impl_unit_get and __newindex pointing to impl_unit_set.
 */
int game_lua_kernel::intf_get_units(lua_State *L)
{
	vconfig filter = luaW_checkvconfig(L, 1, true);

	// Go through all the units while keeping the following stack:
	// 1: metatable, 2: return table, 3: userdata, 4: metatable copy
	lua_settop(L, 0);
	lua_pushlightuserdata(L
			, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_newtable(L);
	int i = 1;

	// note that if filter is null, this yields a null filter matching everything (and doing no work)
	filter_context & fc = game_state_;
	BOOST_FOREACH ( const unit * ui, unit_filter(filter, &fc).all_matches_on_map()) {
		new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->underlying_id());
		lua_pushvalue(L, 1);
		lua_setmetatable(L, 3);
		lua_rawseti(L, 2, i);
		++i;
	}
	return 1;
}

/**
 * Matches a unit against the given filter.
 * - Arg 1: full userdata.
 * - Arg 2: table containing a filter
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_match_unit(lua_State *L)
{
	if (!luaW_hasmetatable(L, 1, getunitKey))
		return luaL_typerror(L, 1, "unit");

	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
	unit* u = lu->get();
	if (!u) return luaL_argerror(L, 1, "unit not found");

	vconfig filter = luaW_checkvconfig(L, 2, true);

	if (filter.null()) {
		lua_pushboolean(L, true);
		return 1;
	}

	filter_context & fc = game_state_;
	if (int side = lu->on_recall_list()) {
		if (!lua_isnoneornil(L, 3)) {
			WRN_LUA << "wesnoth.match_unit called with 3rd argument, but unit to match was on recall list. ";
			WRN_LUA << "Thus the 3rd argument is ignored.\n";
		}
		team &t = (teams())[side - 1];
		scoped_recall_unit auto_store("this_unit",
			t.save_id(), t.recall_list().find_index(u->id()));
		lua_pushboolean(L, unit_filter(filter, &fc).matches(*u, map_location()));
		return 1;
	}

	if (!lua_isnoneornil(L, 3)) {
		lua_unit *lu_adj = static_cast<lua_unit *>(lua_touserdata(L, 1));
		unit* u_adj = lu_adj->get();
		if (!u_adj) {
			return luaL_argerror(L, 3, "unit not found");
		}
		lua_pushboolean(L, unit_filter(filter, &fc).matches(*u, *u_adj));
	} else {
		lua_pushboolean(L, unit_filter(filter, &fc).matches(*u));
	}
	return 1;
}

/**
 * Gets the numeric ids of all the units matching a given filter on the recall lists.
 * - Arg 1: optional table containing a filter
 * - Ret 1: table containing full userdata with __index pointing to
 *          impl_unit_get and __newindex pointing to impl_unit_set.
 */
int game_lua_kernel::intf_get_recall_units(lua_State *L)
{
	vconfig filter = luaW_checkvconfig(L, 1, true);

	// Go through all the units while keeping the following stack:
	// 1: metatable, 2: return table, 3: userdata, 4: metatable copy
	lua_settop(L, 0);
	lua_pushlightuserdata(L
			, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_newtable(L);
	int i = 1, s = 1;
	filter_context & fc = game_state_;
	const unit_filter ufilt(filter, &fc);
	BOOST_FOREACH(team &t, teams())
	{
		BOOST_FOREACH(unit_ptr & u, t.recall_list())
		{
			if (!filter.null()) {
				scoped_recall_unit auto_store("this_unit",
					t.save_id(), t.recall_list().find_index(u->id()));
				if (!ufilt( *u, map_location() ))
					continue;
			}
			new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(s, u->underlying_id());
			lua_pushvalue(L, 1);
			lua_setmetatable(L, 3);
			lua_rawseti(L, 2, i);
			++i;
		}
		++s;
	}
	return 1;
}

/**
 * Fires an event.
 * - Arg 1: string containing the event name.
 * - Arg 2,3: optional first location.
 * - Arg 4,5: optional second location.
 * - Arg 6: optional WML table used as the [weapon] tag.
 * - Arg 7: optional WML table used as the [second_weapon] tag.
 * - Ret 1: boolean indicating whether the event was processed or not.
 */
int game_lua_kernel::intf_fire_event(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);

	int pos = 2;
	map_location l1, l2;
	config data;

	if (lua_isnumber(L, 2)) {
		l1.x = lua_tointeger(L, 2) - 1;
		l1.y = luaL_checkinteger(L, 3) - 1;
		if (lua_isnumber(L, 4)) {
			l2.x = lua_tointeger(L, 4) - 1;
			l2.y = luaL_checkinteger(L, 5) - 1;
			pos = 6;
		} else pos = 4;
	}

	if (!lua_isnoneornil(L, pos)) {
		data.add_child("first", luaW_checkconfig(L, pos));
	}
	++pos;
	if (!lua_isnoneornil(L, pos)) {
		data.add_child("second", luaW_checkconfig(L, pos));
	}

	bool b = play_controller_.pump().fire(m, l1, l2, data);
	lua_pushboolean(L, b);
	return 1;
}

/**
 * Fires a wml menu item.
 * - Arg 1: id of the item. it is not possible to fire items that don't have ids with this function.
 * - Arg 2,3: optional first location.
 * - Ret 1: boolean, true indicating that the event was fired successfully
 *
 * NOTE: This is not an "official" feature, it may currently cause assertion failures if used with
 * menu items which have "needs_select". It is not supported right now to use it this way.
 * The purpose of this function right now is to make it possible to have automated sanity tests for
 * the wml menu items system.
 */
int game_lua_kernel::intf_fire_wml_menu_item(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);

	map_location l1;

	l1.x = luaL_checkinteger(L, 2) - 1;
	l1.y = luaL_checkinteger(L, 3) - 1;

	bool b = game_state_.get_wml_menu_items().fire_item(m, l1, gamedata(), game_state_, units());
	lua_pushboolean(L, b);
	return 1;
}

/**
 * Gets a WML variable.
 * - Arg 1: string containing the variable name.
 * - Arg 2: optional bool indicating if tables for containers should be left empty.
 * - Ret 1: value of the variable, if any.
 */
int game_lua_kernel::intf_get_variable(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	variable_access_const v = gamedata().get_variable_access_read(m);
	return LuaW_pushvariable(L, v) ? 1 : 0;
}

/**
 * Gets a side specific WML variable.
 * - Arg 1: integer side number.
 * - Arg 2: string containing the variable name.
 * - Ret 1: value of the variable, if any.
 */
int game_lua_kernel::intf_get_side_variable(lua_State *L)
{

	unsigned side_index = luaL_checkinteger(L, 1) - 1;
	if(side_index >= teams().size()) {
		return luaL_argerror(L, 1, "invalid side number");
	}
	char const *m = luaL_checkstring(L, 1);
	variable_access_const v(m, teams()[side_index].variables());
	return LuaW_pushvariable(L, v) ? 1 : 0;
}

/**
 * Gets a side specific WML variable.
 * - Arg 1: integer side number.
 * - Arg 2: string containing the variable name.
 * - Arg 3: boolean/integer/string/table containing the value.
 */
int game_lua_kernel::intf_set_side_variable(lua_State *L)
{
	unsigned side_index = luaL_checkinteger(L, 1) - 1;
	if(side_index >= teams().size()) {
		return luaL_argerror(L, 1, "invalid side number");
	}
	char const *m = luaL_checkstring(L, 1);
	//TODO: maybe support removing values with an empty arg3.
	variable_access_create v(m, teams()[side_index].variables());
	LuaW_checkvariable(L, v, 2);
	return 0;
}

/**
 * Sets a WML variable.
 * - Arg 1: string containing the variable name.
 * - Arg 2: boolean/integer/string/table containing the value.
 */
int game_lua_kernel::intf_set_variable(lua_State *L)
{
	const std::string m = luaL_checkstring(L, 1);
	if(m.empty()) return luaL_argerror(L, 1, "empty variable name");
	if (lua_isnoneornil(L, 2)) {
		gamedata().clear_variable(m);
		return 0;
	}
	variable_access_create v = gamedata().get_variable_access_write(m);
	LuaW_checkvariable(L, v, 2);
	return 0;
}

/**
 * Returns a random numer, same interface as math.random.
 */
int game_lua_kernel::intf_random(lua_State *L)
{
	if(lua_isnoneornil(L, 1)) {
		double r = double (random_new::generator->next_random());
		double r_max = double (std::numeric_limits<uint32_t>::max());
		lua_push(L, r / (r_max + 1));
		return 1;
	}
	else {
		int32_t min;
		int32_t max;
		if(lua_isnumber(L, 2)) {
			min = lua_check<int32_t>(L, 1);
			max = lua_check<int32_t>(L, 2);
		}
		else {
			min = 1;
			max = lua_check<int32_t>(L, 1);
		}
		if(min > max) {
			return luaL_argerror(L, 1, "min > max");
		}
		lua_push(L, random_new::generator->get_random_int(min, max));
		return 1;
	}
}

int game_lua_kernel::intf_set_menu_item(lua_State *L)
{
	game_state_.get_wml_menu_items().set_item(luaL_checkstring(L, 1), luaW_checkvconfig(L,2));
	return 0;
}

int game_lua_kernel::intf_clear_menu_item(lua_State *L)
{
	std::string ids(luaL_checkstring(L, 1));
	BOOST_FOREACH(const std::string& id, utils::split(ids, ',', utils::STRIP_SPACES)) {
		if(id.empty()) {
			WRN_LUA << "[clear_menu_item] has been given an empty id=, ignoring" << std::endl;
			continue;
		}
		game_state_.get_wml_menu_items().erase(id);
	}
	return 0;
}

int game_lua_kernel::intf_set_end_campaign_credits(lua_State *L)
{
	game_classification &classification = const_cast<game_classification &> (play_controller_.get_classification());
	classification.end_credits = luaW_toboolean(L, 1);
	return 0;
}

int game_lua_kernel::intf_set_end_campaign_text(lua_State *L)
{
	game_classification &classification = const_cast<game_classification &> (play_controller_.get_classification());
	classification.end_text = luaL_checkstring(L, 1);
	if (lua_isnumber(L, 2)) {
		classification.end_text_duration = static_cast<int> (lua_tonumber(L, 2));
	}

	return 0;
}

int game_lua_kernel::intf_set_next_scenario(lua_State *L)
{
	gamedata().set_next_scenario(luaL_checkstring(L, 1));
	return 0;
}

int game_lua_kernel::intf_shroud_op(lua_State *L, bool place_shroud)
{
	vconfig cfg = luaW_checkvconfig(L, 1);

	// Filter the sides.
	std::vector<int> sides = get_sides_vector(cfg);
	size_t index;

	// Filter the locations.
	std::set<map_location> locs;
	const terrain_filter filter(cfg, &game_state_);
	filter.get_locations(locs, true);

	BOOST_FOREACH(const int &side_num, sides)
	{
		index = side_num - 1;
		team &t = teams()[index];

		BOOST_FOREACH(map_location const &loc, locs)
		{
			if (place_shroud) {
				t.place_shroud(loc);
			} else {
				t.clear_shroud(loc);
			}
		}
	}

	game_display_->labels().recalculate_shroud();
	game_display_->recalculate_minimap();
	game_display_->invalidate_all();

	return 0;
}


/**
 * Highlights the given location on the map.
 * - Args 1,2: location.
 */
int game_lua_kernel::intf_highlight_hex(lua_State *L)
{
	ERR_LUA << "wesnoth.highlight_hex is deprecated, use wesnoth.select_hex" << std::endl;

	if (!game_display_) {
		return 0;
	}

	int x = luaL_checkinteger(L, 1) - 1;
	int y = luaL_checkinteger(L, 2) - 1;
	const map_location loc(x, y);
	game_display_->highlight_hex(loc);
	game_display_->display_unit_hex(loc);

	unit_map::const_unit_iterator i = board().units().find(loc);
	if(i != board().units().end()) {
		game_display_->highlight_reach(pathfind::paths(*i, false,
			true, teams().front()));
			/// @todo: teams().front() is not always the correct
			///        choice for the viewing team.
	}

	return 0;
}

/**
 * Returns whether the first side is an enemy of the second one.
 * - Args 1,2: side numbers.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_is_enemy(lua_State *L)
{
	unsigned side_1 = luaL_checkint(L, 1) - 1;
	unsigned side_2 = luaL_checkint(L, 2) - 1;
	if (side_1 >= teams().size() || side_2 >= teams().size()) return 0;
	lua_pushboolean(L, teams()[side_1].is_enemy(side_2 + 1));
	return 1;
}

/**
 * Gets whether gamemap scrolling is disabled for the user.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_view_locked(lua_State *L)
{
	if (!game_display_) {
		return 0;
	}

	lua_pushboolean(L, game_display_->view_locked());
	return 1;
}

/**
 * Sets whether gamemap scrolling is disabled for the user.
 * - Arg 1: boolean, specifying the new locked/unlocked status.
 */
int game_lua_kernel::intf_lock_view(lua_State *L)
{
	bool lock = luaW_toboolean(L, 1);
	if (game_display_) {
		game_display_->set_view_locked(lock);
	}
	return 0;
}

/**
 * Gets a terrain code.
 * - Args 1,2: map location.
 * - Ret 1: string.
 */
int game_lua_kernel::intf_get_terrain(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);

	t_translation::t_terrain const &t = board().map().
		get_terrain(map_location(x - 1, y - 1));
	lua_pushstring(L, t_translation::write_terrain_code(t).c_str());
	return 1;
}

/**
 * Sets a terrain code.
 * - Args 1,2: map location.
 * - Arg 3: terrain code string.
 * - Arg 4: layer: (overlay|base|both, default=both)
 * - Arg 5: replace_if_failed, default = no
 */
int game_lua_kernel::intf_set_terrain(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	std::string t_str(luaL_checkstring(L, 3));

	std::string mode_str = "both";
	bool replace_if_failed = false;
	if (!lua_isnone(L, 4)) {
		if (!lua_isnil(L, 4)) {
			mode_str = luaL_checkstring(L, 4);
		}

		if(!lua_isnoneornil(L, 5)) {
			replace_if_failed = luaW_toboolean(L, 5);
		}
	}

	bool result = board().change_terrain(map_location(x - 1, y - 1), t_str, mode_str, replace_if_failed);

	if (game_display_) {
		game_display_->needs_rebuild(result);
	}

	return 0;
}

/**
 * Gets details about a terrain.
 * - Arg 1: terrain code string.
 * - Ret 1: table.
 */
int game_lua_kernel::intf_get_terrain_info(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	if (t == t_translation::NONE_TERRAIN) return 0;
	terrain_type const &info = board().map().tdata()->get_terrain_info(t);

	lua_newtable(L);
	lua_pushstring(L, info.id().c_str());
	lua_setfield(L, -2, "id");
	luaW_pushtstring(L, info.name());
	lua_setfield(L, -2, "name");
	luaW_pushtstring(L, info.editor_name());
	lua_setfield(L, -2, "editor_name");
	luaW_pushtstring(L, info.description());
	lua_setfield(L, -2, "description");
	lua_pushboolean(L, info.is_village());
	lua_setfield(L, -2, "village");
	lua_pushboolean(L, info.is_castle());
	lua_setfield(L, -2, "castle");
	lua_pushboolean(L, info.is_keep());
	lua_setfield(L, -2, "keep");
	lua_pushinteger(L, info.gives_healing());
	lua_setfield(L, -2, "healing");

	return 1;
}

/**
 * Gets time of day information.
 * - Arg 1: optional turn number
 * - Arg 2: optional table
 * - Ret 1: table.
 */
int game_lua_kernel::intf_get_time_of_day(lua_State *L)
{
	unsigned arg = 1;

	int for_turn = tod_man().turn();
	map_location loc = map_location();
	bool consider_illuminates = false;

	if(lua_isnumber(L, arg)) {
		++arg;
		for_turn = luaL_checkint(L, 1);
		int number_of_turns = tod_man().number_of_turns();
		if(for_turn < 1 || (number_of_turns != -1 && for_turn > number_of_turns)) {
			return luaL_argerror(L, 1, "turn number out of range");
		}
	}
	else if(lua_isnil(L, arg)) ++arg;

	if(lua_istable(L, arg)) {
		lua_rawgeti(L, arg, 1);
		lua_rawgeti(L, arg, 2);
		loc = map_location(luaL_checkinteger(L, -2) - 1, luaL_checkinteger(L, -1) - 1);
		if(!board().map().on_board(loc)) return luaL_argerror(L, arg, "coordinates are not on board");
		lua_pop(L, 2);

		lua_rawgeti(L, arg, 3);
		consider_illuminates = luaW_toboolean(L, -1);
		lua_pop(L, 1);
	}

	const time_of_day& tod = consider_illuminates ?
		tod_man().get_illuminated_time_of_day(board().units(), board().map(), loc, for_turn) :
		tod_man().get_time_of_day(loc, for_turn);

	lua_newtable(L);
	lua_pushstring(L, tod.id.c_str());
	lua_setfield(L, -2, "id");
	lua_pushinteger(L, tod.lawful_bonus);
	lua_setfield(L, -2, "lawful_bonus");
	lua_pushinteger(L, tod.bonus_modified);
	lua_setfield(L, -2, "bonus_modified");
	lua_pushstring(L, tod.image.c_str());
	lua_setfield(L, -2, "image");
	luaW_pushtstring(L, tod.name);
	lua_setfield(L, -2, "name");

	lua_pushinteger(L, tod.color.r);
	lua_setfield(L, -2, "red");
	lua_pushinteger(L, tod.color.g);
	lua_setfield(L, -2, "green");
	lua_pushinteger(L, tod.color.b);
	lua_setfield(L, -2, "blue");

	return 1;
}

/**
 * Gets the side of a village owner.
 * - Args 1,2: map location.
 * - Ret 1: integer.
 */
int game_lua_kernel::intf_get_village_owner(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);

	map_location loc(x - 1, y - 1);
	if (!board().map().is_village(loc))
		return 0;

	int side = board().village_owner(loc) + 1;
	if (!side) return 0;
	lua_pushinteger(L, side);
	return 1;
}

/**
 * Sets the owner of a village.
 * - Args 1,2: map location.
 * - Arg 3: integer for the side or empty to remove ownership.
 */
int game_lua_kernel::intf_set_village_owner(lua_State *L)
{
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int new_side = lua_isnoneornil(L, 3) ? 0 : luaL_checkint(L, 3);

	map_location loc(x - 1, y - 1);
	if (!board().map().is_village(loc))
		return 0;

	int old_side = board().village_owner(loc) + 1;

	if (new_side == old_side || new_side < 0 || new_side > static_cast<int>(teams().size()) || board().team_is_defeated(teams()[new_side - 1])) {
		return 0;
	}

	if (old_side) {
		teams()[old_side - 1].lose_village(loc);
	}
	if (new_side) {
		teams()[new_side - 1].get_village(loc, old_side, (luaW_toboolean(L, 4) ? &gamedata() : NULL) );
	}
	return 0;
}


/**
 * Returns the map size.
 * - Ret 1: width.
 * - Ret 2: height.
 * - Ret 3: border size.
 */
int game_lua_kernel::intf_get_map_size(lua_State *L)
{
	const gamemap &map = board().map();
	lua_pushinteger(L, map.w());
	lua_pushinteger(L, map.h());
	lua_pushinteger(L, map.border_size());
	return 3;
}

/**
 * Returns the currently overed tile.
 * - Ret 1: x.
 * - Ret 2: y.
 */
int game_lua_kernel::intf_get_mouseover_tile(lua_State *L)
{
	if (!game_display_) {
		return 0;
	}

	const map_location &loc = game_display_->mouseover_hex();
	if (!board().map().on_board(loc)) return 0;
	lua_pushinteger(L, loc.x + 1);
	lua_pushinteger(L, loc.y + 1);
	return 2;
}

/**
 * Returns the currently selected tile.
 * - Ret 1: x.
 * - Ret 2: y.
 */
int game_lua_kernel::intf_get_selected_tile(lua_State *L)
{
	if (!game_display_) {
		return 0;
	}

	const map_location &loc = game_display_->selected_hex();
	if (!board().map().on_board(loc)) return 0;
	lua_pushinteger(L, loc.x + 1);
	lua_pushinteger(L, loc.y + 1);
	return 2;
}

/**
 * Returns the starting position of a side.
 * Arg 1: side number
 * Ret 1: table with unnamed indices holding wml coordinates x and y
*/
int game_lua_kernel::intf_get_starting_location(lua_State* L)
{
	const int side = luaL_checkint(L, 1);
	if(side < 1 || static_cast<int>(teams().size()) < side)
		return luaL_argerror(L, 1, "out of bounds");
	const map_location& starting_pos = board().map().starting_position(side);
	if(!board().map().on_board(starting_pos)) return 0;

	lua_createtable(L, 2, 0);
	lua_pushinteger(L, starting_pos.x + 1);
	lua_rawseti(L, -2, 1);
	lua_pushinteger(L, starting_pos.y + 1);
	lua_rawseti(L, -2, 2);

	return 1;
}

/**
 * Gets a table for an era tag.
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing id of the desired era
 * - Ret 1: config for the era
 */
static int intf_get_era(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	luaW_pushconfig(L, game_config_manager::get()->game_config().find_child("era","id",m));
	return 1;
}

/**
 * Gets some game_config data (__index metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
int game_lua_kernel::impl_game_config_get(lua_State *L)
{
	LOG_LUA << "impl_game_config_get\n";
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("base_income", game_config::base_income);
	return_int_attrib("village_income", game_config::village_income);
	return_int_attrib("village_support", game_config::village_support);
	return_int_attrib("poison_amount", game_config::poison_amount);
	return_int_attrib("rest_heal_amount", game_config::rest_heal_amount);
	return_int_attrib("recall_cost", game_config::recall_cost);
	return_int_attrib("kill_experience", game_config::kill_experience);
	return_int_attrib("last_turn", tod_man().number_of_turns());
	return_string_attrib("version", game_config::version);
	return_bool_attrib("debug", game_config::debug);
	return_bool_attrib("debug_lua", game_config::debug_lua);
	return_bool_attrib("mp_debug", game_config::mp_debug);

	const mp_game_settings& mp_settings = play_controller_.get_mp_settings();
	const game_classification & classification = play_controller_.get_classification();

	return_string_attrib("campaign_type", lexical_cast<std::string>(classification.campaign_type));
	if(classification.campaign_type==game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
		return_cfgref_attrib("mp_settings", mp_settings.to_config());
		return_cfgref_attrib("era", game_config_manager::get()->game_config().find_child("era","id",mp_settings.mp_era));
		//^ finds the era with name matching mp_era, and creates a lua reference from the config of that era.

		//This code for SigurdFD, not the cleanest implementation but seems to work just fine.
		config::const_child_itors its = game_config_manager::get()->game_config().child_range("era");
		std::string eras_list((*(its.first))["id"]);
		++its.first;
		for(; its.first != its.second; ++its.first) {
			eras_list = eras_list + "," + (*(its.first))["id"];
		}
		return_string_attrib("eras", eras_list);
	}
	return 0;
}

/**
 * Sets some game_config data (__newindex metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
int game_lua_kernel::impl_game_config_set(lua_State *L)
{
	LOG_LUA << "impl_game_config_set\n";
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	modify_int_attrib("base_income", game_config::base_income = value);
	modify_int_attrib("village_income", game_config::village_income = value);
	modify_int_attrib("village_support", game_config::village_support = value);
	modify_int_attrib("poison_amount", game_config::poison_amount = value);
	modify_int_attrib("rest_heal_amount", game_config::rest_heal_amount = value);
	modify_int_attrib("recall_cost", game_config::recall_cost = value);
	modify_int_attrib("kill_experience", game_config::kill_experience = value);
	modify_int_attrib("last_turn", tod_man().set_number_of_turns_by_wml(value));

	std::string err_msg = "unknown modifiable property of game_config: ";
	err_msg += m;
	return luaL_argerror(L, 2, err_msg.c_str());
}

/**
	converts synced_context::get_synced_state() to a string.
*/
std::string game_lua_kernel::synced_state()
{
	//maybe return "initial" for game_data::INITIAL?
	if(gamedata().phase() == game_data::PRELOAD || gamedata().phase() == game_data::INITIAL)
	{
		return "preload";
	}
	switch(synced_context::get_synced_state())
	{
	case synced_context::LOCAL_CHOICE:
		return "local_choice";
	case synced_context::SYNCED:
		return "synced";
	case synced_context::UNSYNCED:
		return "unsynced";
	default:
		throw game::game_error("Found corrupt synced_context::synced_state");
	}
}


/**
 * Gets some data about current point of game (__index metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
int game_lua_kernel::impl_current_get(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);

	// Find the corresponding attribute.
	return_int_attrib("side", play_controller_.current_side());
	return_int_attrib("turn", play_controller_.turn());
	return_string_attrib("synced_state", synced_state());

	if (strcmp(m, "event_context") == 0)
	{
		const game_events::queued_event &ev = get_event_info();
		config cfg;
		cfg["name"] = ev.name;
		if (const config &weapon = ev.data.child("first")) {
			cfg.add_child("weapon", weapon);
		}
		if (const config &weapon = ev.data.child("second")) {
			cfg.add_child("second_weapon", weapon);
		}
		if (ev.loc1.valid()) {
			cfg["x1"] = ev.loc1.filter_x() + 1;
			cfg["y1"] = ev.loc1.filter_y() + 1;
			// The position of the unit involved in this event, currently the only case where this is different from x1/y1 are enter/exit_hex events
			cfg["unit_x"] = ev.loc1.x + 1;
			cfg["unit_y"] = ev.loc1.y + 1;
		}
		if (ev.loc2.valid()) {
			cfg["x2"] = ev.loc2.filter_x() + 1;
			cfg["y2"] = ev.loc2.filter_y() + 1;
		}
		luaW_pushconfig(L, cfg);
		return 1;
	}

	return 0;
}

/**
 * Displays a message in the chat window and in the logs.
 * - Arg 1: optional message header.
 * - Arg 2 (or 1): message.
 */
int game_lua_kernel::intf_message(lua_State *L)
{
	t_string m = luaW_checktstring(L, 1);
	t_string h = m;
	if (lua_isnone(L, 2)) {
		h = "Lua";
	} else {
		m = luaW_checktstring(L, 2);
	}
	lua_chat(h, m);
	LOG_LUA << "Script says: \"" << m << "\"\n";
	return 0;
}

int game_lua_kernel::intf_open_help(lua_State *L)
{
	if (game_display_) {
		help::show_help(*game_display_, luaL_checkstring(L, 1));
	}
	return 0;
}

/**
 * Dumps a wml table or userdata wml object into a pretty string.
 * - Arg 1: wml table or vconfig userdata
 * - Ret 1: string
 */
static int intf_debug(lua_State* L)
{
	const config& arg = luaW_checkconfig(L, 1);
	const std::string& result = arg.debug();
	lua_pushstring(L, result.c_str());
	return 1;
}

int game_lua_kernel::intf_check_end_level_disabled(lua_State * L)
{
	lua_pushboolean(L, play_controller_.is_regular_game_end());
	return 1;
}

/**
 * Removes all messages from the chat window.
 */
int game_lua_kernel::intf_clear_messages(lua_State*)
{
	if (game_display_) {
		game_display_->get_chat_manager().clear_chat_messages();
	}
	return 0;
}

int game_lua_kernel::intf_end_level(lua_State *L)
{
	vconfig cfg(luaW_checkvconfig(L, 1));


	if (play_controller_.is_regular_game_end()) {
		return 0;
	}
	end_level_data data;

	data.proceed_to_next_level = cfg["proceed_to_next_level"].to_bool(true);
	data.transient.custom_endlevel_music = cfg["music"].str();
	data.transient.carryover_report = cfg["carryover_report"].to_bool(true);
	data.prescenario_save = cfg["save"].to_bool(true);
	data.replay_save = cfg["replay_save"].to_bool(true);
	data.transient.linger_mode = cfg["linger_mode"].to_bool(true) && !teams().empty();
	data.transient.reveal_map = cfg["reveal_map"].to_bool(true);
	data.is_victory = cfg["result"] == "victory";
	play_controller_.set_end_level_data(data);
	return 0;
}

int game_lua_kernel::intf_end_turn(lua_State*)
{
	play_controller_.force_end_turn();
	return 0;
}

/**
 * Evaluates a boolean WML conditional.
 * - Arg 1: WML table.
 * - Ret 1: boolean.
 */
static int intf_eval_conditional(lua_State *L)
{
	vconfig cond = luaW_checkvconfig(L, 1);
	bool b = game_events::conditional_passed(cond);
	lua_pushboolean(L, b);
	return 1;
}

namespace {
	/**
	 * Cost function object relying on a Lua function.
	 * @note The stack index of the Lua function must be valid each time the cost is computed.
	 */
	struct lua_calculator : pathfind::cost_calculator
	{
		lua_State *L;
		int index;

		lua_calculator(lua_State *L_, int i): L(L_), index(i) {}
		double cost(const map_location &loc, const double so_far) const;
	};

	double lua_calculator::cost(const map_location &loc, const double so_far) const
	{
		// Copy the user function and push the location and current cost.
		lua_pushvalue(L, index);
		lua_pushinteger(L, loc.x + 1);
		lua_pushinteger(L, loc.y + 1);
		lua_pushnumber(L, so_far);

		// Execute the user function.
		if (!luaW_pcall(L, 3, 1)) return 1.;

		// Return a cost of at least 1 mp to avoid issues in pathfinder.
		// (Condition is inverted to detect NaNs.)
		double cost = lua_tonumber(L, -1);
		lua_pop(L, 1);
		return !(cost >= 1.) ? 1. : cost;
	}
}//unnamed namespace for lua_calculator

/**
 * Finds a path between two locations.
 * - Args 1,2: source location. (Or Arg 1: unit.)
 * - Args 3,4: destination.
 * - Arg 5: optional cost function or
 *          table (optional fields: ignore_units, ignore_teleport, max_cost, viewing_side).
 * - Ret 1: array of pairs containing path steps.
 * - Ret 2: path cost.
 */
int game_lua_kernel::intf_find_path(lua_State *L)
{
	int arg = 1;
	map_location src, dst;
	const unit* u = NULL;

	if (lua_isuserdata(L, arg))
	{
		u = &luaW_checkunit(L, 1);
		src = u->get_location();
		++arg;
	}
	else
	{
		src.x = luaL_checkinteger(L, arg) - 1;
		++arg;
		src.y = luaL_checkinteger(L, arg) - 1;
		unit_map::const_unit_iterator ui = units().find(src);
		if (ui.valid()) {
			u = ui.get_shared_ptr().get();
		}
		++arg;
	}

	dst.x = luaL_checkinteger(L, arg) - 1;
	++arg;
	dst.y = luaL_checkinteger(L, arg) - 1;
	++arg;

	if (!board().map().on_board(src))
		return luaL_argerror(L, 1, "invalid location");
	if (!board().map().on_board(dst))
		return luaL_argerror(L, arg - 2, "invalid location");

	const gamemap &map = board().map();
	int viewing_side = 0;
	bool ignore_units = false, see_all = false, ignore_teleport = false;
	double stop_at = 10000;
	pathfind::cost_calculator *calc = NULL;

	if (lua_istable(L, arg))
	{
		lua_pushstring(L, "ignore_units");
		lua_rawget(L, arg);
		ignore_units = luaW_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "ignore_teleport");
		lua_rawget(L, arg);
		ignore_teleport = luaW_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "max_cost");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1))
			stop_at = luaL_checknumber(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "viewing_side");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1)) {
			int i = luaL_checkinteger(L, -1);
			if (i >= 1 && i <= int(teams().size())) viewing_side = i;
			else see_all = true;
		}
		lua_pop(L, 1);
	}
	else if (lua_isfunction(L, arg))
	{
		calc = new lua_calculator(L, arg);
	}

	pathfind::teleport_map teleport_locations;

	if (!calc) {
		if (!u) return luaL_argerror(L, 1, "unit not found");

		const team &viewing_team = board().teams()[(viewing_side ? viewing_side : u->side()) - 1];
		if (!ignore_teleport) {
			teleport_locations = pathfind::get_teleport_locations(
				*u, viewing_team, see_all, ignore_units);
		}
		calc = new pathfind::shortest_path_calculator(*u, viewing_team,
			teams(), map, ignore_units, false, see_all);
	}

	pathfind::plain_route res = pathfind::a_star_search(src, dst, stop_at, calc, map.w(), map.h(),
		&teleport_locations);
	delete calc;

	int nb = res.steps.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, res.steps[i].x + 1);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, res.steps[i].y + 1);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, i + 1);
	}
	lua_pushinteger(L, res.move_cost);

	return 2;
}

/**
 * Finds all the locations reachable by a unit.
 * - Args 1,2: source location. (Or Arg 1: unit.)
 * - Arg 3: optional table (optional fields: ignore_units, ignore_teleport, additional_turns, viewing_side).
 * - Ret 1: array of triples (coordinates + remaining movement).
 */
int game_lua_kernel::intf_find_reach(lua_State *L)
{
	int arg = 1;
	const unit* u = NULL;

	if (lua_isuserdata(L, arg))
	{
		u = &luaW_checkunit(L, 1);
		++arg;
	}
	else
	{
		map_location src;
		src.x = luaL_checkinteger(L, arg) - 1;
		++arg;
		src.y = luaL_checkinteger(L, arg) - 1;
		unit_map::const_unit_iterator ui = units().find(src);
		if (!ui.valid())
			return luaL_argerror(L, 1, "unit not found");
		u = ui.get_shared_ptr().get();
		++arg;
	}

	int viewing_side = 0;
	bool ignore_units = false, see_all = false, ignore_teleport = false;
	int additional_turns = 0;

	if (lua_istable(L, arg))
	{
		lua_pushstring(L, "ignore_units");
		lua_rawget(L, arg);
		ignore_units = luaW_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "ignore_teleport");
		lua_rawget(L, arg);
		ignore_teleport = luaW_toboolean(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "additional_turns");
		lua_rawget(L, arg);
		additional_turns = lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_pushstring(L, "viewing_side");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1)) {
			int i = luaL_checkinteger(L, -1);
			if (i >= 1 && i <= int(teams().size())) viewing_side = i;
			else see_all = true;
		}
		lua_pop(L, 1);
	}

	const team &viewing_team = board().teams()[(viewing_side ? viewing_side : u->side()) - 1];
	pathfind::paths res(*u, ignore_units, !ignore_teleport,
		viewing_team, additional_turns, see_all, ignore_units);

	int nb = res.destinations.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		pathfind::paths::step &s = res.destinations[i];
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, s.curr.x + 1);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, s.curr.y + 1);
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, s.move_left);
		lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

static bool intf_find_cost_map_helper(const unit * ptr) {
	return ptr->get_location().valid();
}

/**
 * Is called with one or more units and builds a cost map.
 * - Args 1,2: source location. (Or Arg 1: unit. Or Arg 1: table containing a filter)
 * - Arg 3: optional array of tables with 4 elements (coordinates + side + unit type string)
 * - Arg 4: optional table (optional fields: ignore_units, ignore_teleport, viewing_side, debug).
 * - Arg 5: optional table: standard location filter.
 * - Ret 1: array of triples (coordinates + array of tuples(summed cost + reach counter)).
 */
int game_lua_kernel::intf_find_cost_map(lua_State *L)
{
	int arg = 1;
	unit* u = luaW_tounit(L, arg, true);
	vconfig filter = vconfig::unconstructed_vconfig();
	luaW_tovconfig(L, arg, filter);

	std::vector<const unit*> real_units;
	typedef std::vector<boost::tuple<map_location, int, std::string> > unit_type_vector;
	unit_type_vector fake_units;


	if (u)  // 1. arg - unit
	{
		real_units.push_back(u);
	}
	else if (!filter.null())  // 1. arg - filter
	{
		filter_context & fc = game_state_;
		boost::copy(unit_filter(filter, &fc).all_matches_on_map() | boost::adaptors::filtered(&intf_find_cost_map_helper), std::back_inserter(real_units));
	}
	else  // 1. + 2. arg - coordinates
	{
		map_location src;
		src.x = luaL_checkinteger(L, arg) - 1;
		++arg;
		src.y = luaL_checkinteger(L, arg) - 1;
		unit_map::const_unit_iterator ui = units().find(src);
		if (ui.valid())
		{
			real_units.push_back(&(*ui));
		}
	}
	++arg;

	if (lua_istable(L, arg))  // 2. arg - optional types
	{
		for (int i = 1, i_end = lua_rawlen(L, arg); i <= i_end; ++i)
		{
			map_location src;
			lua_rawgeti(L, arg, i);
			if (!lua_istable(L, -1)) return luaL_argerror(L, 1, "unit type table missformed");

			lua_rawgeti(L, -1, 1);
			if (!lua_isnumber(L, -1)) return luaL_argerror(L, 1, "unit type table missformed");
			src.x = lua_tointeger(L, -1) - 1;
			lua_rawgeti(L, -2, 2);
			if (!lua_isnumber(L, -1)) return luaL_argerror(L, 1, "unit type table missformed");
			src.y = lua_tointeger(L, -1) - 1;

			lua_rawgeti(L, -3, 3);
			if (!lua_isnumber(L, -1)) return luaL_argerror(L, 1, "unit type table missformed");
			int side = lua_tointeger(L, -1);

			lua_rawgeti(L, -4, 4);
			if (!lua_isstring(L, -1)) return luaL_argerror(L, 1, "unit type table missformed");
			std::string unit_type = lua_tostring(L, -1);

			boost::tuple<map_location, int, std::string> tuple(src, side, unit_type);
			fake_units.push_back(tuple);

			lua_pop(L, 5);
		}
		++arg;
	}

	if(real_units.empty() && fake_units.empty())
	{
		return luaL_argerror(L, 1, "unit(s) not found");
	}

	int viewing_side = 0;
	bool ignore_units = true, see_all = true, ignore_teleport = false, debug = false, use_max_moves = false;

	if (lua_istable(L, arg))  // 4. arg - options
	{
		lua_pushstring(L, "ignore_units");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1))
		{
			ignore_units = luaW_toboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_pushstring(L, "ignore_teleport");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1))
		{
			ignore_teleport = luaW_toboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_pushstring(L, "viewing_side");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1))
		{
			int i = luaL_checkinteger(L, -1);
			if (i >= 1 && i <= int(teams().size()))
			{
				viewing_side = i;
				see_all = false;
			}
		}

		lua_pushstring(L, "debug");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1))
		{
			debug = luaW_toboolean(L, -1);
		}
		lua_pop(L, 1);

		lua_pushstring(L, "use_max_moves");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1))
		{
			use_max_moves = luaW_toboolean(L, -1);
		}
		lua_pop(L, 1);
		++arg;
	}

	// 5. arg - location filter
	filter = vconfig::unconstructed_vconfig();
	std::set<map_location> location_set;
	luaW_tovconfig(L, arg, filter);
	if (filter.null())
	{
		filter = vconfig(config(), true);
	}
	filter_context & fc = game_state_;
	const terrain_filter t_filter(filter, &fc);
	t_filter.get_locations(location_set, true);
	++arg;

	// build cost_map
	const team &viewing_team = board().teams()[(viewing_side ? viewing_side : 1) - 1];
	pathfind::full_cost_map cost_map(
			ignore_units, !ignore_teleport, viewing_team, see_all, ignore_units);

	BOOST_FOREACH(const unit* const u, real_units)
	{
		cost_map.add_unit(*u, use_max_moves);
	}
	BOOST_FOREACH(const unit_type_vector::value_type& fu, fake_units)
	{
		const unit_type* ut = unit_types.find(fu.get<2>());
		cost_map.add_unit(fu.get<0>(), ut, fu.get<1>());
	}

	if (debug)
	{
		if (game_display_) {
			game_display_->labels().clear_all();
			BOOST_FOREACH(const map_location& loc, location_set)
			{
				std::stringstream s;
				s << cost_map.get_pair_at(loc.x, loc.y).first;
				s << " / ";
				s << cost_map.get_pair_at(loc.x, loc.y).second;
				game_display_->labels().set_label(loc, s.str());
			}
		}
	}

	// create return value
	lua_createtable(L, location_set.size(), 0);
	int counter = 1;
	BOOST_FOREACH(const map_location& loc, location_set)
	{
		lua_createtable(L, 4, 0);

		lua_pushinteger(L, loc.x + 1);
		lua_rawseti(L, -2, 1);

		lua_pushinteger(L, loc.y + 1);
		lua_rawseti(L, -2, 2);

		lua_pushinteger(L, cost_map.get_pair_at(loc.x, loc.y).first);
		lua_rawseti(L, -2, 3);

		lua_pushinteger(L, cost_map.get_pair_at(loc.x, loc.y).second);
		lua_rawseti(L, -2, 4);

		lua_rawseti(L, -2, counter);
		++counter;
	}
	return 1;
}

int game_lua_kernel::intf_heal_unit(lua_State *L)
{
	vconfig cfg(luaW_checkvconfig(L, 1));

	const game_events::queued_event &event_info = get_event_info();

	unit_map & temp = units();
	unit_map* units = & temp;

	const vconfig & healers_filter = cfg.child("filter_second");
	std::vector<unit*> healers;
	if (!healers_filter.null()) {
		const unit_filter ufilt(healers_filter, &game_state_);
		BOOST_FOREACH(unit& u, *units) {
			if ( ufilt(u) && u.has_ability_type("heals") ) {
				healers.push_back(&u);
			}
		}
	}

	const config::attribute_value amount = cfg["amount"];
	const config::attribute_value moves = cfg["moves"];
	const bool restore_attacks = cfg["restore_attacks"].to_bool(false);
	const bool restore_statuses = cfg["restore_statuses"].to_bool(true);
	const bool animate = cfg["animate"].to_bool(false);

	const vconfig & healed_filter = cfg.child("filter");
	bool only_unit_at_loc1 = healed_filter.null();
	bool heal_amount_to_set = true;

	const unit_filter ufilt(healed_filter, &game_state_);
	for(unit_map::unit_iterator u  = units->begin(); u != units->end(); ++u) {
		if (only_unit_at_loc1)
		{
			u = units->find(event_info.loc1);
			if(!u.valid()) return 0;
		}
		else if ( !ufilt(*u) ) continue;

		int heal_amount = u->max_hitpoints() - u->hitpoints();
		if(amount.blank() || amount == "full") u->set_hitpoints(u->max_hitpoints());
		else {
			heal_amount = lexical_cast_default<int, config::attribute_value> (amount, heal_amount);
			const int new_hitpoints = std::max(1, std::min(u->max_hitpoints(), u->hitpoints() + heal_amount));
			heal_amount = new_hitpoints - u->hitpoints();
			u->set_hitpoints(new_hitpoints);
		}

		if(!moves.blank()) {
			if(moves == "full") u->set_movement(u->total_movement());
			else {
				// set_movement doesn't set below 0
				u->set_movement(std::min<int>(
					u->total_movement(),
					u->movement_left() + lexical_cast_default<int, config::attribute_value> (moves, 0)
					));
			}
		}

		if(restore_attacks) u->set_attacks(u->max_attacks());

		if(restore_statuses)
		{
			u->set_state(unit::STATE_POISONED, false);
			u->set_state(unit::STATE_SLOWED, false);
			u->set_state(unit::STATE_PETRIFIED, false);
			u->set_state(unit::STATE_UNHEALABLE, false);
			u->anim_comp().set_standing();
		}

		if (heal_amount_to_set)
		{
			heal_amount_to_set = false;
			gamedata().get_variable("heal_amount") = heal_amount;
		}

		if(animate) unit_display::unit_healing(*u, healers, heal_amount);
		if(only_unit_at_loc1) return 0;
	}
	return 0;
}

int game_lua_kernel::intf_print(lua_State *L) {
	vconfig cfg(luaW_checkvconfig(L, 1));

	// Remove any old message.
	static int floating_label = 0;
	if (floating_label)
		font::remove_floating_label(floating_label);

	// Display a message on-screen
	std::string text = cfg["text"];
	if(text.empty() || !game_display_)
		return 0;

	int size = cfg["size"].to_int(font::SIZE_SMALL);
	int lifetime = cfg["duration"].to_int(50);

	SDL_Color color = font::LABEL_COLOR;

	if(!cfg["color"].empty()) {
		color = string_to_color(cfg["color"]);
	} else if(cfg.has_attribute("red") || cfg.has_attribute("green") || cfg.has_attribute("blue")) {
		color = create_color(cfg["red"], cfg["green"], cfg["blue"]);
	} 

	const SDL_Rect& rect = game_display_->map_outside_area();

	font::floating_label flabel(text);
	flabel.set_font_size(size);
	flabel.set_color(color);
	flabel.set_position(rect.w/2,rect.h/2);
	flabel.set_lifetime(lifetime);
	flabel.set_clip_rect(rect);

	floating_label = font::add_floating_label(flabel);

	return 0;
}

/**
 * Places a unit on the map.
 * - Args 1,2: (optional) location.
 * - Arg 3: Unit (WML table or proxy), or nothing/nil to delete.
 * OR
 * - Arg 1: Unit (WML table or proxy)
 * - Args 2,3: (optional) location
 */
int game_lua_kernel::intf_put_unit(lua_State *L)
{
	int unit_arg = 1;

	lua_unit *lu = NULL;
	unit_ptr u = unit_ptr();
	map_location loc;
	if (lua_isnumber(L, 1)) {
		unit_arg = 3;
		loc.x = lua_tointeger(L, 1) - 1;
		loc.y = luaL_checkinteger(L, 2) - 1;
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 1, "invalid location");
		}
	} else if (lua_isnumber(L, 2)) {
		loc.x = lua_tointeger(L, 2) - 1;
		loc.y = luaL_checkinteger(L, 3) - 1;
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 1, "invalid location");
		}
	}

	if (luaW_hasmetatable(L, unit_arg, getunitKey))
	{
		lu = static_cast<lua_unit *>(lua_touserdata(L, unit_arg));
		u = lu->get_shared();
		if (!u) return luaL_argerror(L, unit_arg, "unit not found");
		if (lu->on_map() && (unit_arg == 1 || u->get_location() == loc)) {
			return 0;
		}
		if (!loc.valid()) {
			loc = u->get_location();
			if (!map().on_board(loc))
				return luaL_argerror(L, 1, "invalid location");
		} else {
			WRN_LUA << "wesnoth.put_unit(x, y, unit) is deprecated. Use wesnoth.put_unit(unit, x, y) instead\n";
		}
	}
	else if (!lua_isnoneornil(L, unit_arg))
	{
		config cfg = luaW_checkconfig(L, unit_arg);
		if (unit_arg == 1 && !map().on_board(loc)) {
			loc.x = cfg["x"] - 1;
			loc.y = cfg["y"] - 1;
			if (!map().on_board(loc))
				return luaL_argerror(L, 2, "invalid location");
		} else if (unit_arg != 1) {
			WRN_LUA << "wesnoth.put_unit(x, y, unit) is deprecated. Use wesnoth.put_unit(unit, x, y) instead\n";
		}
		u = unit_ptr (new unit(cfg, true));
	}

	if (game_display_) {
		game_display_->invalidate(loc);
	}

	if (!u) {
		if (unit_arg == 3) {
			WRN_LUA << "wesnoth.put_unit(x, y) is deprecated. Use wesnoth.erase_unit(x, y) instead\n";
			units().erase(loc);
		}
		return 0;
	}
	units().erase(loc);

	if (lu) {
		lu->put_map(loc);
		lu->get_shared()->anim_comp().set_standing();
	} else {
		u->set_location(loc);
		units().insert(u);
	}

	return 0;
}

/**
 * Erases a unit from the map
 * - Arg 1: Unit to erase
 * OR
 * - Args 1,2: Location to erase unit
 */
int game_lua_kernel::intf_erase_unit(lua_State *L)
{
	map_location loc;

	if (lua_isnumber(L, 1)) {
		loc.x = lua_tointeger(L, 2) - 1;
		loc.y = luaL_checkinteger(L, 3) - 1;
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 1, "invalid location");
		}
	} else if (luaW_hasmetatable(L, 1, getunitKey)) {
		lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
		unit_ptr u = lu->get_shared();
		if (!lu->get()) {
			return luaL_argerror(L, 1, "unit not found");
		}
		if (lu->on_map()) {
			loc = u->get_location();
			if (!map().on_board(loc)) {
				return luaL_argerror(L, 1, "invalid location");
			}
		} else if (int side = lu->on_recall_list()) {
			team &t = teams()[side - 1];
			// Should it use underlying ID instead?
			t.recall_list().erase_if_matches_id(u->id());
		} else {
			return luaL_argerror(L, 1, "can't erase private units");
		}
	} else if (!lua_isnoneornil(L, 1)) {
		config cfg = luaW_checkconfig(L, 1);
		loc.x = cfg["x"] - 1;
		loc.y = cfg["y"] - 1;
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 1, "invalid location");
		}
	} else {
		return luaL_argerror(L, 1, "expected unit or integer");
	}

	units().erase(loc);
	return 0;
}

/**
 * Puts a unit on a recall list.
 * - Arg 1: WML table or unit.
 * - Arg 2: (optional) side.
 */
int game_lua_kernel::intf_put_recall_unit(lua_State *L)
{
	lua_unit *lu = NULL;
	unit_ptr u = unit_ptr();
	int side = lua_tointeger(L, 2);
	if (unsigned(side) > teams().size()) side = 0;

	if (luaW_hasmetatable(L, 1, getunitKey))
	{
		lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
		u = lu->get_shared();
		if (!u || lu->on_recall_list())
			return luaL_argerror(L, 1, "unit not found");
	}
	else
	{
		config cfg = luaW_checkconfig(L, 1);
		u = unit_ptr(new unit(cfg, true));
	}

	if (!side) side = u->side();
	team &t = teams()[side - 1];
	// Avoid duplicates in the recall list.
	size_t uid = u->underlying_id();
	t.recall_list().erase_by_underlying_id(uid);
	t.recall_list().add(u);
	if (lu) {
		if (lu->on_map())
			units().erase(u->get_location());
		lu->lua_unit::~lua_unit();
		new(lu) lua_unit(side, uid);
	}

	return 0;
}

/**
 * Extracts a unit from the map or a recall list and gives it to Lua.
 * - Arg 1: unit userdata.
 */
int game_lua_kernel::intf_extract_unit(lua_State *L)
{
	if (!luaW_hasmetatable(L, 1, getunitKey))
		return luaL_typerror(L, 1, "unit");
	lua_unit *lu = static_cast<lua_unit *>(lua_touserdata(L, 1));
	unit_ptr u = lu->get_shared();
	if (!u) return luaL_argerror(L, 1, "unit not found");

	if (lu->on_map()) {
		u = units().extract(u->get_location());
		assert(u);
		u->anim_comp().clear_haloes();
	} else if (int side = lu->on_recall_list()) {
		team &t = teams()[side - 1];
		unit_ptr v = unit_ptr(new unit(*u));
		t.recall_list().erase_if_matches_id(u->id());
		u = v;
	} else {
		return 0;
	}

	lu->lua_unit::~lua_unit();
	new(lu) lua_unit(u);
	return 0;
}

/**
 * Finds a vacant tile.
 * - Args 1,2: location.
 * - Arg 3: optional unit for checking movement type.
 * - Rets 1,2: location.
 */
int game_lua_kernel::intf_find_vacant_tile(lua_State *L)
{
	int x = luaL_checkint(L, 1) - 1, y = luaL_checkint(L, 2) - 1;

	unit_ptr u = unit_ptr();
	if (!lua_isnoneornil(L, 3)) {
		if (luaW_hasmetatable(L, 3, getunitKey)) {
			u = static_cast<lua_unit *>(lua_touserdata(L, 3))->get_shared();
		} else {
			config cfg = luaW_checkconfig(L, 3);
			u.reset(new unit(cfg, false));
		}
	}

	map_location res = find_vacant_tile(map_location(x, y),
	                                    pathfind::VACANT_ANY, u.get());

	if (!res.valid()) return 0;
	lua_pushinteger(L, res.x + 1);
	lua_pushinteger(L, res.y + 1);
	return 2;
}

/**
 * Floats some text on the map.
 * - Args 1,2: location.
 * - Arg 3: string.
 */
int game_lua_kernel::intf_float_label(lua_State *L)
{
	map_location loc;
	loc.x = luaL_checkinteger(L, 1) - 1;
	loc.y = luaL_checkinteger(L, 2) - 1;

	t_string text = luaW_checktstring(L, 3);
	if (game_display_) {
		game_display_->float_label(loc, text, font::LABEL_COLOR);
	}
	return 0;
}

/**
 * Creates a unit from its WML description.
 * - Arg 1: WML table.
 * - Ret 1: unit userdata.
 */
static int intf_create_unit(lua_State *L)
{
	config cfg = luaW_checkconfig(L, 1);
	unit_ptr u = unit_ptr(new unit(cfg, true));
	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(u);
	lua_pushlightuserdata(L
			, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Copies a unit.
 * - Arg 1: unit userdata.
 * - Ret 1: unit userdata.
 */
static int intf_copy_unit(lua_State *L)
{
	unit& u = luaW_checkunit(L, 1);
	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(unit_ptr(new unit(u)));
	lua_pushlightuserdata(L
			, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);
	return 1;
}

/**
 * Returns unit resistance against a given attack type.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the attack type.
 * - Arg 3: boolean indicating if attacker.
 * - Args 4,5: optional location.
 * - Ret 1: integer.
 */
static int intf_unit_resistance(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	bool a = luaW_toboolean(L, 3);

	map_location loc = u.get_location();
	if (!lua_isnoneornil(L, 4)) {
		loc.x = luaL_checkinteger(L, 4) - 1;
		loc.y = luaL_checkinteger(L, 5) - 1;
	}

	lua_pushinteger(L, u.resistance_against(m, a, loc));
	return 1;
}

/**
 * Returns unit movement cost on a given terrain.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the terrain type.
 * - Ret 1: integer.
 */
static int intf_unit_movement_cost(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	lua_pushinteger(L, u.movement_cost(t));
	return 1;
}

/**
 * Returns unit vision cost on a given terrain.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the terrain type.
 * - Ret 1: integer.
 */
static int intf_unit_vision_cost(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	lua_pushinteger(L, u.vision_cost(t));
	return 1;
}

/**
 * Returns unit jamming cost on a given terrain.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the terrain type.
 * - Ret 1: integer.
 */
static int intf_unit_jamming_cost(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	lua_pushinteger(L, u.jamming_cost(t));
	return 1;
}

/**
 * Returns unit defense on a given terrain.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the terrain type.
 * - Ret 1: integer.
 */
static int intf_unit_defense(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	t_translation::t_terrain t = t_translation::read_terrain_code(m);
	lua_pushinteger(L, u.defense_modifier(t));
	return 1;
}

/**
 * Returns true if the unit has the given ability enabled.
 * - Arg 1: unit userdata.
 * - Arg 2: string.
 * - Ret 1: boolean.
 */
static int intf_unit_ability(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	lua_pushboolean(L, u.get_ability_bool(m));
	return 1;
}

/**
 * Changes a unit to the given unit type.
 * - Arg 1: unit userdata.
 * - Arg 2: string.
 */
static int intf_transform_unit(lua_State *L)
{
	unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	const unit_type *utp = unit_types.find(m);
	if (!utp) return luaL_argerror(L, 2, "unknown unit type");
	u.advance_to(*utp);

	return 0;
}

/**
 * Puts a table at the top of the stack with some combat result.
 */
static void luaW_pushsimdata(lua_State *L, const combatant &cmb)
{
	int n = cmb.hp_dist.size();
	lua_createtable(L, 0, 4);
	lua_pushnumber(L, cmb.poisoned);
	lua_setfield(L, -2, "poisoned");
	lua_pushnumber(L, cmb.slowed);
	lua_setfield(L, -2, "slowed");
	lua_pushnumber(L, cmb.average_hp());
	lua_setfield(L, -2, "average_hp");
	lua_createtable(L, n, 0);
	for (int i = 0; i < n; ++i) {
		lua_pushnumber(L, cmb.hp_dist[i]);
		lua_rawseti(L, -2, i);
	}
	lua_setfield(L, -2, "hp_chance");
}

/**
 * Puts a table at the top of the stack with information about the combatants' weapons.
 */
static void luaW_pushsimweapon(lua_State *L, const battle_context_unit_stats &bcustats)
{

	lua_createtable(L, 0, 16);

	lua_pushnumber(L, bcustats.num_blows);
	lua_setfield(L, -2, "num_blows");
	lua_pushnumber(L, bcustats.damage);
	lua_setfield(L, -2, "damage");
	lua_pushnumber(L, bcustats.chance_to_hit);
	lua_setfield(L, -2, "chance_to_hit");
	lua_pushboolean(L, bcustats.poisons);
	lua_setfield(L, -2, "poisons");
	lua_pushboolean(L, bcustats.slows);
	lua_setfield(L, -2, "slows");
	lua_pushboolean(L, bcustats.petrifies);
	lua_setfield(L, -2, "petrifies");
	lua_pushboolean(L, bcustats.plagues);
	lua_setfield(L, -2, "plagues");
	lua_pushstring(L, bcustats.plague_type.c_str());
	lua_setfield(L, -2, "plague_type");
	lua_pushboolean(L, bcustats.backstab_pos);
	lua_setfield(L, -2, "backstabs");
	lua_pushnumber(L, bcustats.rounds);
	lua_setfield(L, -2, "rounds");
	lua_pushboolean(L, bcustats.firststrike);
	lua_setfield(L, -2, "firststrike");
	lua_pushboolean(L, bcustats.drains);
	lua_setfield(L, -2, "drains");
	lua_pushnumber(L, bcustats.drain_constant);
	lua_setfield(L, -2, "drain_constant");
	lua_pushnumber(L, bcustats.drain_percent);
	lua_setfield(L, -2, "drain_percent");


	//if we called simulate_combat without giving an explicit weapon this can be useful.
	lua_pushnumber(L, bcustats.attack_num);
	lua_setfield(L, -2, "attack_num");
	//this is NULL when there is no counter weapon
	if(bcustats.weapon != NULL)
	{
		lua_pushstring(L, bcustats.weapon->id().c_str());
		lua_setfield(L, -2, "name");
	}

}

/**
 * Simulates a combat between two units.
 * - Arg 1: attacker userdata.
 * - Arg 2: optional weapon index.
 * - Arg 3: defender userdata.
 * - Arg 4: optional weapon index.
 * - Ret 1: attacker results.
 * - Ret 2: defender results.
 * - Ret 3: info about the attacker weapon.
 * - Ret 4: info about the defender weapon.
 */
int game_lua_kernel::intf_simulate_combat(lua_State *L)
{
	int arg_num = 1, att_w = -1, def_w = -1;

	const unit& att = luaW_checkunit(L, arg_num);
	++arg_num;
	if (lua_isnumber(L, arg_num)) {
		att_w = lua_tointeger(L, arg_num) - 1;
		if (att_w < 0 || att_w >= int(att.attacks().size()))
			return luaL_argerror(L, arg_num, "weapon index out of bounds");
		++arg_num;
	}

	const unit& def = luaW_checkunit(L, arg_num, true);
	++arg_num;
	if (lua_isnumber(L, arg_num)) {
		def_w = lua_tointeger(L, arg_num) - 1;
		if (def_w < 0 || def_w >= int(def.attacks().size()))
			return luaL_argerror(L, arg_num, "weapon index out of bounds");
		++arg_num;
	}

	battle_context context(units(), att.get_location(),
		def.get_location(), att_w, def_w, 0.0, NULL, &att);

	luaW_pushsimdata(L, context.get_attacker_combatant());
	luaW_pushsimdata(L, context.get_defender_combatant());
	luaW_pushsimweapon(L, context.get_attacker_stats());
	luaW_pushsimweapon(L, context.get_defender_stats());
	return 4;
}

/**
 * Modifies the music playlist.
 * - Arg 1: WML table, or nil to force changes.
 */
static int intf_set_music(lua_State *L)
{
	if (lua_isnoneornil(L, 1)) {
		sound::commit_music_changes();
		return 0;
	}

	config cfg = luaW_checkconfig(L, 1);
	sound::play_music_config(cfg);
	return 0;
}

/**
 * Plays a sound, possibly repeated.
 * - Arg 1: string.
 * - Arg 2: optional integer.
 */
int game_lua_kernel::intf_play_sound(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	if (play_controller_.is_skipping_replay()) return 0;
	int repeats = lua_tointeger(L, 2);
	sound::play_sound(m, sound::SOUND_FX, repeats);
	return 0;
}

/**
 * Scrolls to given tile.
 * - Args 1,2: location.
 * - Arg 3: boolean preventing scroll to fog.
 * - Arg 4: boolean specifying whether to warp instantly.
 */
int game_lua_kernel::intf_scroll_to_tile(lua_State *L)
{
	int x = luaL_checkinteger(L, 1) - 1;
	int y = luaL_checkinteger(L, 2) - 1;
	bool check_fogged = luaW_toboolean(L, 3);
	bool immediate = luaW_toboolean(L, 4);
	if (game_display_) {
		game_display_->scroll_to_tile(map_location(x, y),
			immediate ? game_display::WARP : game_display::SCROLL, check_fogged);
	}
	return 0;
}

/**
 * Selects and highlights the given location on the map.
 * - Args 1,2: location.
 * - Args 3,4: booleans
 */
int game_lua_kernel::intf_select_hex(lua_State *L)
{
	const int x = luaL_checkinteger(L, 1) - 1;
	const int y = luaL_checkinteger(L, 2) - 1;

	const map_location loc(x, y);
	if(!map().on_board(loc)) return luaL_argerror(L, 1, "not on board");
	bool highlight = true;
	if(!lua_isnoneornil(L, 3))
		highlight = luaW_toboolean(L, 3);
	const bool fire_event = luaW_toboolean(L, 4);
	play_controller_.get_mouse_handler_base().select_hex(
		loc, false, highlight, fire_event);
	if(highlight && game_display_) {
		game_display_->highlight_hex(loc);
	}
	return 0;
}

/**
 * Deselects any highlighted hex on the map.
 * No arguments or return values
 */
int game_lua_kernel::intf_deselect_hex(lua_State*)
{
	const map_location loc;
	play_controller_.get_mouse_handler_base().select_hex(
		loc, false, false, false);
	if (game_display_) {
		game_display_->highlight_hex(loc);
	}
	return 0;
}

/**
 * Return true if a replay is in progress but the player has chosen to skip it
 */
int game_lua_kernel::intf_is_skipping_messages(lua_State *L)
{
	bool skipping = play_controller_.is_skipping_replay();
	if (!skipping && resources::game_events) {
		skipping = resources::game_events->pump().context_skip_messages();
	}
	lua_pushboolean(L, skipping);
	return 1;
}

/**
 * Set whether to skip messages
 * Arg 1 (optional) - boolean
 */
int game_lua_kernel::intf_skip_messages(lua_State *L)
{
	bool skip = true;
	if (!lua_isnone(L, 1)) {
		skip = luaW_toboolean(L, 1);
	}
	resources::game_events->pump().context_skip_messages(skip);
	return 0;
}

namespace
{
	struct lua_synchronize : mp_sync::user_choice
	{
		lua_State *L;
		int user_choice_index;
		int random_choice_index;
		int ai_choice_index;
		std::string  desc;
		lua_synchronize(lua_State *l, const std::string& descr, int user_index, int random_index = 0, int ai_index = 0)
			: L(l)
			, user_choice_index(user_index)
			, random_choice_index(random_index)
			, ai_choice_index(ai_index != 0 ? ai_index : user_index)
			, desc(descr)
		{}

		virtual config query_user(int side) const
		{
			bool is_local_ai = lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).teams()[side - 1].is_local_ai();
			config cfg;
			query_lua(side, is_local_ai ? ai_choice_index : user_choice_index, cfg);
			return cfg;
		}

		virtual config random_choice(int side) const
		{
			config cfg;
			if(random_choice_index != 0 && lua_isfunction(L, random_choice_index)) {
				query_lua(side, random_choice_index, cfg);
			}
			return cfg;
		}

		virtual std::string description() const OVERRIDE
		{
			return desc;
		}

		void query_lua(int side, int function_index, config& cfg) const
		{
			assert(cfg.empty());
			lua_pushvalue(L, function_index);
			lua_pushnumber(L, side);
			if (luaW_pcall(L, 1, 1, false)) {
				if(!luaW_toconfig(L, -1, cfg)) {
					lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).log_error("function returned to wesnoth.synchronize_choice a table which was partially invalid");
				}
			}
		}
		//Although luas sync_choice can show a dialog, (and will in most cases)
		//we return false to enable other possible things that do not contain UI things.
		//it's in the responsibility of the umc dev to not show dialogs during prestart events.
		virtual bool is_visible() const { return false; }
	};
}//unnamed namespace for lua_synchronize

/**
 * Ensures a value is synchronized among all the clients.
 * - Arg 1: optional string specifying the type id of the choice.
 * - Arg 2: function to compute the value, called if the client is the master.
 * - Arg 3: optional function, called instead of the first function if the user is not human.
 * - Arg 4: optional integer  specifying, on which side the function should be evaluated.
 * - Ret 1: WML table returned by the function.
 */
static int intf_synchronize_choice(lua_State *L)
{
	std::string tagname = "input";
	t_string desc = _("input");
	int human_func = 0;
	int ai_func = 0;
	int side_for;

	int nextarg = 1;
	if(!lua_isfunction(L, nextarg) && luaW_totstring(L, nextarg, desc) ) {
		++nextarg;
	}
	if(lua_isfunction(L, nextarg)) {
		human_func = nextarg++;
	}
	else {
		return luaL_argerror(L, nextarg, "expected a function");
	}
	if(lua_isfunction(L, nextarg)) {
		ai_func = nextarg++;
	}
	side_for = lua_tointeger(L, nextarg);

	config cfg = mp_sync::get_user_choice(tagname, lua_synchronize(L, desc, human_func, 0, ai_func), side_for);
	luaW_pushconfig(L, cfg);
	return 1;
}
/**
 * Ensures a value is synchronized among all the clients.
 * - Arg 1: optional string the id of this type of user input, may only contain chracters a-z and '_'
 * - Arg 2: function to compute the value, called if the client is the master.
 * - Arg 3: an optional function to compute the value, if the side was null/empty controlled.
 * - Arg 4: an array of integers specifying, on which side the function should be evaluated.
 * - Ret 1: a map int -> WML tabls.
 */
static int intf_synchronize_choices(lua_State *L)
{
	std::string tagname = "input";
	t_string desc = _("input");
	int human_func = 0;
	int null_func = 0;
	std::vector<int> sides_for;

	int nextarg = 1;
	if(!lua_isfunction(L, nextarg) && luaW_totstring(L, nextarg, desc) ) {
		++nextarg;
	}
	if(lua_isfunction(L, nextarg)) {
		human_func = nextarg++;
	}
	else {
		return luaL_argerror(L, nextarg, "expected a function");
	}
	if(lua_isfunction(L, nextarg)) {
		null_func = nextarg++;
	};
	sides_for = lua_check<std::vector<int> >(L, nextarg++);

	lua_push(L, mp_sync::get_user_choice_multiple_sides(tagname, lua_synchronize(L, desc, human_func, null_func), std::set<int>(sides_for.begin(), sides_for.end())));
	return 1;
}


/**
 * Calls a function in an unsynced context (this specially means that all random calls used by that function will be unsynced).
 * This is usualy used together with an unsynced if like 'if controller != network'
 * - Arg 1: function that will be called during the unsynced context.
 */
static int intf_do_unsynced(lua_State *L)
{
	set_scontext_unsynced sync;
	lua_pushvalue(L, 1);
	luaW_pcall(L, 0, 0, false);
	return 0;
}

/**
 * Gets all the locations matching a given filter.
 * - Arg 1: WML table.
 * - Ret 1: array of integer pairs.
 */
int game_lua_kernel::intf_get_locations(lua_State *L)
{
	vconfig filter = luaW_checkvconfig(L, 1);

	std::set<map_location> res;
	filter_context & fc = game_state_;
	const terrain_filter t_filter(filter, &fc);
	t_filter.get_locations(res, true);

	lua_createtable(L, res.size(), 0);
	int i = 1;
	BOOST_FOREACH(map_location const &loc, res)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, loc.x + 1);
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, loc.y + 1);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, i);
		++i;
	}
	return 1;
}

/**
 * Gets all the villages matching a given filter, or all the villages on the map if no filter is given.
 * - Arg 1: WML table (optional).
 * - Ret 1: array of integer pairs.
 */
int game_lua_kernel::intf_get_villages(lua_State *L)
{
	std::vector<map_location> locs = map().villages();
	lua_newtable(L);
	int i = 1;

	vconfig filter = luaW_checkvconfig(L, 1);

	filter_context & fc = game_state_;
	for(std::vector<map_location>::const_iterator it = locs.begin(); it != locs.end(); ++it) {
		bool matches = terrain_filter(filter, &fc).match(*it);
		if (matches) {
			lua_createtable(L, 2, 0);
			lua_pushinteger(L, it->x + 1);
			lua_rawseti(L, -2, 1);
			lua_pushinteger(L, it->y + 1);
			lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, i);
			++i;
		}
	}
	return 1;
}

/**
 * Matches a location against the given filter.
 * - Args 1,2: integers.
 * - Arg 3: WML table.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_match_location(lua_State *L)
{
	int x = luaL_checkinteger(L, 1) - 1;
	int y = luaL_checkinteger(L, 2) - 1;
	vconfig filter = luaW_checkvconfig(L, 3, true);

	if (filter.null()) {
		lua_pushboolean(L, true);
		return 1;
	}

	filter_context & fc = game_state_;
	const terrain_filter t_filter(filter, &fc);
	lua_pushboolean(L, t_filter.match(map_location(x, y)));
	return 1;
}



/**
 * Matches a side against the given filter.
 * - Args 1: side number.
 * - Arg 2: WML table.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_match_side(lua_State *L)
{
	unsigned side = luaL_checkinteger(L, 1) - 1;
	if (side >= teams().size()) return 0;
	vconfig filter = luaW_checkvconfig(L, 2, true);

	if (filter.null()) {
		lua_pushboolean(L, true);
		return 1;
	}

	filter_context & fc = game_state_;
	side_filter s_filter(filter, &fc);
	lua_pushboolean(L, s_filter.match(side + 1));
	return 1;
}

int game_lua_kernel::intf_modify_ai_wml(lua_State *L)
{
	vconfig cfg(luaW_checkvconfig(L, 1));

	side_filter ssf(cfg, &game_state_);
	std::vector<int> sides = ssf.get_teams();
	BOOST_FOREACH(const int &side_num, sides)
	{
		ai::manager::modify_active_ai_for_side(side_num,cfg.get_parsed_config());
	}
	return 0;
}

int game_lua_kernel::intf_modify_side(lua_State *L)
{
	vconfig cfg(luaW_checkvconfig(L, 1));

	bool invalidate_screen = false;

	std::string team_name = cfg["team_name"];
	std::string user_team_name = cfg["user_team_name"];
	std::string controller = cfg["controller"];
	std::string defeat_condition = cfg["defeat_condition"];
	std::string recruit_str = cfg["recruit"];
	std::string shroud_data = cfg["shroud_data"];
	std::string village_support = cfg["village_support"];
	const config& parsed = cfg.get_parsed_config();
	const config::const_child_itors &ai = parsed.child_range("ai");
	std::string switch_ai = cfg["switch_ai"];

	std::vector<int> sides = get_sides_vector(cfg);
	size_t team_index;

	BOOST_FOREACH(const int &side_num, sides)
	{
		team_index = side_num - 1;

		team & tm = teams()[team_index];

		LOG_LUA << "modifying side: " << side_num << "\n";
		if(!team_name.empty()) {
			LOG_LUA << "change side's team to team_name '" << team_name << "'\n";
			tm.change_team(team_name,
					user_team_name);
		} else if(!user_team_name.empty()) {
			LOG_LUA << "change side's user_team_name to '" << user_team_name << "'\n";
			tm.change_team(tm.team_name(),
					user_team_name);
		}
		// Modify recruit list (override)
		if (!recruit_str.empty()) {
			tm.set_recruits(utils::set_split(recruit_str));
		}
		// Modify income
		config::attribute_value income = cfg["income"];
		if (!income.empty()) {
			tm.set_base_income(income.to_int() + game_config::base_income);
		}
		// Modify total gold
		config::attribute_value gold = cfg["gold"];
		if (!gold.empty()) {
			tm.set_gold(gold);
		}
		// Set controller
		if (!controller.empty()) {
			tm.change_controller_by_wml(controller);
		}
		// Set defeat_condition
		if (!defeat_condition.empty()) {
			tm.set_defeat_condition_string(defeat_condition);
		}
		// Set shroud
		config::attribute_value shroud = cfg["shroud"];
		if (!shroud.empty()) {
			tm.set_shroud(shroud.to_bool(true));
			invalidate_screen = true;
		}
		// Reset shroud
		if ( cfg["reset_maps"].to_bool(false) ) {
			tm.reshroud();
			invalidate_screen = true;
		}
		// Merge shroud data
		if (!shroud_data.empty()) {
			tm.merge_shroud_map_data(shroud_data);
			invalidate_screen = true;
		}
		// Set whether team is hidden in status table
		config::attribute_value hidden = cfg["hidden"];
		if (!hidden.empty()) {
			tm.set_hidden(hidden.to_bool(true));
		}
		// Set fog
		config::attribute_value fog = cfg["fog"];
		if (!fog.empty()) {
			tm.set_fog(fog.to_bool(true));
			invalidate_screen = true;
		}
		// Reset fog
		if ( cfg["reset_view"].to_bool(false) ) {
			tm.refog();
			invalidate_screen = true;
		}
		// Set income per village
		config::attribute_value village_gold = cfg["village_gold"];
		if (!village_gold.empty()) {
			tm.set_village_gold(village_gold);
		}
		// Set support (unit levels supported per village, for upkeep purposes)
		if (!village_support.empty()) {
			tm.set_village_support(lexical_cast_default<int>(village_support, game_config::village_support));
		}
		// Redeploy ai from location (this ignores current AI parameters)
		if (!switch_ai.empty()) {
			ai::manager::add_ai_for_side_from_file(side_num,switch_ai,true);
		}
		// Override AI parameters
		if (ai.first != ai.second) {
			ai::manager::modify_active_ai_config_old_for_side(side_num,ai);
		}
		// Change team color
		config::attribute_value color = cfg["color"];
		if(!color.empty()) {
			tm.set_color(color);
			invalidate_screen = true;
		}
		// Change flag imageset
		config::attribute_value flag = cfg["flag"];
		if(!flag.empty()) {
			tm.set_flag(flag);
			// Needed especially when map isn't animated.
			invalidate_screen = true;
		}
		// If either the flag set or the team color changed, we need to
		// rebuild the team's flag cache to reflect the changes. Note that
		// this is not required for flag icons (used by the theme UI only).
		if((!color.empty() || !flag.empty()) && game_display_) {
			game_display_->reinit_flags_for_side(team_index);
		}
		// Change flag icon
		config::attribute_value flag_icon = cfg["flag_icon"];
		if(!flag_icon.empty()) {
			tm.set_flag_icon(flag_icon);
			// Not needed.
			//invalidate_screen = true;
		}
		tm.handle_legacy_share_vision(cfg.get_parsed_config());
		// Suppress end turn confirmations?
		config::attribute_value setc = cfg["suppress_end_turn_confirmation"];
		if ( !setc.empty() ) {
			tm.set_no_turn_confirmation(setc.to_bool());
		}

		// Change leader scrolling options
		config::attribute_value stl = cfg["scroll_to_leader"];
		if ( !stl.empty()) {
			tm.set_scroll_to_leader(stl.to_bool(true));
		}
	}

	// Flag an update of the screen, if needed.
	if ( invalidate_screen && game_display_) {
		game_display_->recalculate_minimap();
		game_display_->invalidate_all();
	}


	return 0;
}

/**
 * Returns a proxy table array for all sides matching the given SSF.
 * - Arg 1: SSF
 * - Ret 1: proxy table array
 */
int game_lua_kernel::intf_get_sides(lua_State* L)
{
	LOG_LUA << "intf_get_sides called: this = " << (formatter() << std::hex << this).str() << " myname = " << my_name() << std::endl;
	std::vector<int> sides;
	const vconfig ssf = luaW_checkvconfig(L, 1, true);
	if(ssf.null()){
		for(unsigned side_number = 1; side_number <= teams().size(); ++side_number)
			sides.push_back(side_number);
	} else {
		filter_context & fc = game_state_;

		side_filter filter(ssf, &fc);
		sides = filter.get_teams();
	}

	lua_settop(L, 0);
	lua_createtable(L, sides.size(), 0);
	unsigned index = 1;
	BOOST_FOREACH(int side, sides) {
		luaW_pushteam(L, teams()[side - 1]);
		lua_rawseti(L, -2, index);
		++index;
	}

	return 1;
}

/**
 * .Returns information about the global traits known to the engine.
 * - Ret 1: Table with named fields holding wml tables describing the traits.
 */
static int intf_get_traits(lua_State* L)
{
	lua_newtable(L);
	BOOST_FOREACH(const config& trait, unit_types.traits()) {
		const std::string& id = trait["id"];
		//It seems the engine does nowhere check the id field for emptyness or duplicates
		//(also not later on).
		//However, the worst thing to happen is that the trait read later overwrites the older one,
		//and this is not the right place for such checks.
		lua_pushstring(L, id.c_str());
		luaW_pushconfig(L, trait);
		lua_rawset(L, -3);
	}
	return 1;
}

/**
 * Adds a modification to a unit.
 * - Arg 1: unit.
 * - Arg 2: string.
 * - Arg 3: WML table.
 * - Arg 4: (optional) Whether to add to [modifications] - default true
 */
static int intf_add_modification(lua_State *L)
{
	unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	std::string sm = m;
	if (sm == "advance") { // Maintain backwards compatibility
		sm = "advancement";
		lg::wml_error << "(Lua) Modifications of type \"advance\" are deprecated, use \"advancement\" instead\n";
	}
	if (sm != "advancement" && sm != "object" && sm != "trait") {
		return luaL_argerror(L, 2, "unknown modification type");
	}
	bool write_to_mods = true;
	if (!lua_isnone(L, 4)) {
		write_to_mods = luaW_toboolean(L, 4);
	}

	config cfg = luaW_checkconfig(L, 3);
	u.add_modification(sm, cfg, !write_to_mods);
	return 0;
}

/**
 * Advances a unit if the unit has enough xp.
 * - Arg 1: unit.
 * - Arg 2: optional boolean whether to animate the advancement.
 * - Arg 3: optional boolean whether to fire advancement events.
 */
static int intf_advance_unit(lua_State *L)
{
	//TODO: check whether the unit is on the map.
	unit& u = luaW_checkunit(L, 1, true);
	advance_unit_params par(u.get_location());
	if(lua_isboolean(L, 2)) {
		par.animate(luaW_toboolean(L, 2));
	}
	if(lua_isboolean(L, 3)) {
		par.fire_events(luaW_toboolean(L, 3));
	}
	advance_unit_at(par);
	return 0;
}


/**
 * Adds a new known unit type to the help system.
 * - Arg 1: string.
 */
static int intf_add_known_unit(lua_State *L)
{
	char const *ty = luaL_checkstring(L, 1);
	if(!unit_types.find(ty))
	{
		std::stringstream ss;
		ss << "unknown unit type: '" << ty << "'";
		return luaL_argerror(L, 1, ss.str().c_str());
	}
	preferences::encountered_units().insert(ty);
	return 0;
}

/**
 * Adds an overlay on a tile.
 * - Args 1,2: location.
 * - Arg 3: WML table.
 */
int game_lua_kernel::intf_add_tile_overlay(lua_State *L)
{
	int x = luaL_checkinteger(L, 1) - 1;
	int y = luaL_checkinteger(L, 2) - 1;
	config cfg = luaW_checkconfig(L, 3);

	if (game_display_) {
		game_display_->add_overlay(map_location(x, y), cfg["image"], cfg["halo"],
			cfg["team_name"], cfg["visible_in_fog"].to_bool(true));
	}
	return 0;
}

/**
 * Adds an overlay on a tile.
 * - Args 1,2: location.
 * - Arg 3: optional string.
 */
int game_lua_kernel::intf_remove_tile_overlay(lua_State *L)
{
	int x = luaL_checkinteger(L, 1) - 1;
	int y = luaL_checkinteger(L, 2) - 1;
	char const *m = lua_tostring(L, 3);

	if (m) {
		if (game_display_) {
			game_display_->remove_single_overlay(map_location(x, y), m);
		}
	} else {
		if (game_display_) {
			game_display_->remove_overlay(map_location(x, y));
		}
	}
	return 0;
}

/// Adding new events
int game_lua_kernel::intf_add_event(lua_State *L)
{
	vconfig cfg(luaW_checkvconfig(L, 1));
	game_events::manager & man = *game_state_.events_manager_;

	if (!cfg["delayed_variable_substitution"].to_bool(true)) {
		man.add_event_handler(cfg.get_parsed_config());
	} else {
		man.add_event_handler(cfg.get_config());
	}
	return 0;
}

int game_lua_kernel::intf_remove_event(lua_State *L)
{
	game_state_.events_manager_->remove_event_handler(luaL_checkstring(L, 1));
	return 0;
}

int game_lua_kernel::intf_color_adjust(lua_State *L)
{
	if (game_display_) {
		vconfig cfg(luaW_checkvconfig(L, 1));

		game_display_->adjust_color_overlay(cfg["red"], cfg["green"], cfg["blue"]);
		game_display_->invalidate_all();
		game_display_->draw(true,true);
	}
	return 0;
}

/**
 * Delays engine for a while.
 * - Arg 1: integer.
 * - Arg 2: boolean (optional).
 */
int game_lua_kernel::intf_delay(lua_State *L)
{
	lua_Integer delay = luaL_checkinteger(L, 1);
	if(delay == 0) {
		play_controller_.play_slice(false);
		return 0;
	}
	if(luaW_toboolean(L, 2) && game_display_ && game_display_->turbo_speed() > 0) {
		delay /= game_display_->turbo_speed();
	}
	const unsigned final = SDL_GetTicks() + delay;
	do {
		play_controller_.play_slice(false);
		if (game_display_) {
			game_display_->delay(10);
		}
	} while (int(final - SDL_GetTicks()) > 0);
	return 0;
}

namespace { // Types

	class recursion_preventer {
		typedef std::map<map_location, int> t_counter;
		static t_counter counter_;
		static const int max_recursion = 10;

		map_location loc_;
		bool too_many_recursions_;

		public:
		recursion_preventer(map_location& loc) :
			loc_(loc),
			too_many_recursions_(false)
		{
			t_counter::iterator inserted = counter_.insert(std::make_pair(loc_, 0)).first;
			++inserted->second;
			too_many_recursions_ = inserted->second >= max_recursion;
		}
		~recursion_preventer()
		{
			t_counter::iterator itor = counter_.find(loc_);
			if (--itor->second == 0)
			{
				counter_.erase(itor);
			}
		}
		bool too_many_recursions() const
		{
			return too_many_recursions_;
		}
	};
	recursion_preventer::t_counter recursion_preventer::counter_;
	typedef boost::scoped_ptr<recursion_preventer> recursion_preventer_ptr;
} // end anonymouse namespace (types)

int game_lua_kernel::intf_kill(lua_State *L)
{
	vconfig cfg(luaW_checkvconfig(L, 1));

	const game_events::queued_event &event_info = get_event_info();

	size_t number_killed = 0;

	bool secondary_unit = cfg.has_child("secondary_unit");
	game_events::entity_location killer_loc(map_location(0, 0));
	if(cfg["fire_event"].to_bool() && secondary_unit)
	{
		secondary_unit = false;
		const unit_filter ufilt(cfg.child("secondary_unit"), &game_state_);
		for(unit_map::const_unit_iterator unit = units().begin(); unit; ++unit) {
				if ( ufilt( *unit) )
				{
					killer_loc = game_events::entity_location(*unit);
					secondary_unit = true;
					break;
				}
		}
		if(!secondary_unit) {
			WRN_LUA << "failed to match [secondary_unit] in [kill] with a single on-board unit" << std::endl;
		}
	}

	//Find all the dead units first, because firing events ruins unit_map iteration
	std::vector<unit *> dead_men_walking;
	const unit_filter ufilt(cfg, &game_state_);
	BOOST_FOREACH(unit & u, units()){
		if ( ufilt(u) ) {
			dead_men_walking.push_back(&u);
		}
	}

	BOOST_FOREACH(unit * un, dead_men_walking) {
		map_location loc(un->get_location());
		bool fire_event = false;
		game_events::entity_location death_loc(*un);
		if(!secondary_unit) {
			killer_loc = game_events::entity_location(*un);
		}

		if (cfg["fire_event"].to_bool())
			{
				// Prevent infinite recursion of 'die' events
				fire_event = true;
				recursion_preventer_ptr recursion_prevent;

				if (event_info.loc1 == death_loc && (event_info.name == "die" || event_info.name == "last breath"))
					{
						recursion_prevent.reset(new recursion_preventer(death_loc));

						if(recursion_prevent->too_many_recursions())
							{
								fire_event = false;

								ERR_LUA << "tried to fire 'die' or 'last breath' event on primary_unit inside its own 'die' or 'last breath' event with 'first_time_only' set to false!" << std::endl;
							}
					}
			}
		if (fire_event) {
			play_controller_.pump().fire("last breath", death_loc, killer_loc);
		}

		// Visual consequences of the kill.
		if (game_display_) {
			if (cfg["animate"].to_bool()) {
				game_display_->scroll_to_tile(loc);
				if (unit_map::iterator iun = units().find(loc)) {
					unit_display::unit_die(loc, *iun);
				}
			} else {
				// Make sure the unit gets (fully) cleaned off the screen.
				game_display_->invalidate(loc);
				if (unit_map::iterator iun = units().find(loc)) {
					iun->anim_comp().invalidate(*game_display_);
				}
			}
			game_display_->redraw_minimap();
		}

		if (fire_event) {
			play_controller_.pump().fire("die", death_loc, killer_loc);
			unit_map::iterator iun = units().find(death_loc);
			if ( death_loc.matches_unit(iun) ) {
				units().erase(iun);
			}
		}
		else units().erase(loc);

		++number_killed;
	}

	// If the filter doesn't contain positional information,
	// then it may match units on all recall lists.
	const config::attribute_value cfg_x = cfg["x"];
	const config::attribute_value cfg_y = cfg["y"];
	if((cfg_x.empty() || cfg_x == "recall")
	&& (cfg_y.empty() || cfg_y == "recall"))
	{
		const unit_filter ufilt(cfg, &game_state_);
		//remove the unit from the corresponding team's recall list
		for(std::vector<team>::iterator pi = teams().begin();
				pi!=teams().end(); ++pi)
		{
			for(std::vector<unit_ptr>::iterator j = pi->recall_list().begin(); j != pi->recall_list().end();) { //TODO: This block is really messy, cleanup somehow...
				scoped_recall_unit auto_store("this_unit", pi->save_id(), j - pi->recall_list().begin());
				if (ufilt( *(*j), map_location() )) {
					j = pi->recall_list().erase(j);
					++number_killed;
				} else {
					++j;
				}
			}
		}
	}

	lua_pushinteger(L, number_killed);

	return 1;
}

int game_lua_kernel::intf_label(lua_State *L)
{
	if (game_display_) {
		vconfig cfg(luaW_checkvconfig(L, 1));

		game_display &screen = *game_display_;

		terrain_label label(screen.labels(), cfg.get_config());

		screen.labels().set_label(label.location(), label.text(), label.creator(), label.team_name(), label.color(),
				label.visible_in_fog(), label.visible_in_shroud(), label.immutable(), label.category(), label.tooltip());
	}
	return 0;
}

int game_lua_kernel::intf_redraw(lua_State *L)
{
	if (game_display_) {
		game_display & screen = *game_display_;

		vconfig cfg(luaW_checkvconfig(L, 1));
		bool clear_shroud(luaW_toboolean(L, 2));

		// We do this twice so any applicable redraws happen both before and after
		// any events caused by redrawing shroud are fired
		bool result = screen.maybe_rebuild();
		if (!result) {
			screen.invalidate_all();
		}

		if (clear_shroud) {
			side_filter filter(cfg, &game_state_);
			BOOST_FOREACH(const int side, filter.get_teams()){
				actions::clear_shroud(side);
			}
			screen.recalculate_minimap();
		}

		result = screen.maybe_rebuild();
		if (!result) {
			screen.invalidate_all();
		}

		screen.draw(true,true);
	}
	return 0;
}

/**
 * Gets the dimension of an image.
 * - Arg 1: string.
 * - Ret 1: width.
 * - Ret 2: height.
 */
static int intf_get_image_size(lua_State *L)
{
	char const *m = luaL_checkstring(L, 1);
	image::locator img(m);
	if (!img.file_exists()) return 0;
	surface s = get_image(img);
	lua_pushinteger(L, s->w);
	lua_pushinteger(L, s->h);
	return 2;
}

/**
 * Returns the time stamp, exactly as [set_variable] time=stamp does.
 * - Ret 1: integer
 */
static int intf_get_time_stamp(lua_State *L)
{
	lua_pushinteger(L, SDL_GetTicks());
	return 1;
}

/**
 * Lua frontend to the modify_ai functionality
 * - Arg 1: config.
 */
static int intf_modify_ai(lua_State *L)
{
	config cfg;
	luaW_toconfig(L, 1, cfg);
	int side = cfg["side"];
	ai::manager::modify_active_ai_for_side(side, cfg);
	return 0;
}

static int cfun_exec_candidate_action(lua_State *L)
{
	bool exec = luaW_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ca_ptr");

	ai::candidate_action *ca = static_cast<ai::candidate_action*>(lua_touserdata(L, -1));
	lua_pop(L, 2);
	if (exec) {
		ca->execute();
		return 0;
	}
	lua_pushinteger(L, ca->evaluate());
	return 1;
}

static int cfun_exec_stage(lua_State *L)
{
	lua_getfield(L, -1, "stg_ptr");
	ai::stage *stg = static_cast<ai::stage*>(lua_touserdata(L, -1));
	lua_pop(L, 2);
	stg->play_stage();
	return 0;
}

static void push_component(lua_State *L, ai::component* c, const std::string &ct = "")
{
	lua_createtable(L, 0, 0); // Table for a component

	lua_pushstring(L, "name");
	lua_pushstring(L, c->get_name().c_str());
	lua_rawset(L, -3);

	lua_pushstring(L, "engine");
	lua_pushstring(L, c->get_engine().c_str());
	lua_rawset(L, -3);

	lua_pushstring(L, "id");
	lua_pushstring(L, c->get_id().c_str());
	lua_rawset(L, -3);

	if (ct == "candidate_action") {
		lua_pushstring(L, "ca_ptr");
		lua_pushlightuserdata(L, c);
		lua_rawset(L, -3);

		lua_pushstring(L, "exec");
		lua_pushcclosure(L, &cfun_exec_candidate_action, 0);
		lua_rawset(L, -3);
	}

	if (ct == "stage") {
		lua_pushstring(L, "stg_ptr");
		lua_pushlightuserdata(L, c);
		lua_rawset(L, -3);

		lua_pushstring(L, "exec");
		lua_pushcclosure(L, &cfun_exec_stage, 0);
		lua_rawset(L, -3);
	}


	std::vector<std::string> c_types = c->get_children_types();

	for (std::vector<std::string>::const_iterator t = c_types.begin(); t != c_types.end(); ++t)
	{
		std::vector<ai::component*> children = c->get_children(*t);
		std::string type = *t;
		if (type == "aspect" || type == "goal" || type == "engine")
		{
			continue;
		}

		lua_pushstring(L, type.c_str());
		lua_createtable(L, 0, 0); // this table will be on top of the stack during recursive calls

		for (std::vector<ai::component*>::const_iterator i = children.begin(); i != children.end(); ++i)
		{
			lua_pushstring(L, (*i)->get_name().c_str());
			push_component(L, *i, type);
			lua_rawset(L, -3);

			//if (type == "candidate_action")
			//{
			//	ai::candidate_action *ca = dynamic_cast<ai::candidate_action*>(*i);
			//	ca->execute();
			//}
		}

		lua_rawset(L, -3); // setting the child table
	}


}

/**
 * Debug access to the ai tables
 * - Arg 1: int
 * - Ret 1: ai table
 */
static int intf_debug_ai(lua_State *L)
{
	if (!game_config::debug) { // This function works in debug mode only
		return 0;
	}
	int side = lua_tointeger(L, 1);
	lua_pop(L, 1);

	ai::component* c = ai::manager::get_active_ai_holder_for_side_dbg(side).get_component(NULL, "");

	// Bad, but works
	std::vector<ai::component*> engines = c->get_children("engine");
	ai::engine_lua* lua_engine = NULL;
	for (std::vector<ai::component*>::const_iterator i = engines.begin(); i != engines.end(); ++i)
	{
		if ((*i)->get_name() == "lua")
		{
			lua_engine = dynamic_cast<ai::engine_lua *>(*i);
		}
	}

	// Better way, but doesn't work
	//ai::component* e = ai::manager::get_active_ai_holder_for_side_dbg(side).get_component(c, "engine[lua]");
	//ai::engine_lua* lua_engine = dynamic_cast<ai::engine_lua *>(e);

	if (lua_engine == NULL)
	{
		//no lua engine is defined for this side.
		//so set up a dummy engine

		ai::ai_composite * ai_ptr = dynamic_cast<ai::ai_composite *>(c);

		assert(ai_ptr);

		ai::ai_context& ai_context = ai_ptr->get_ai_context();
		config cfg = ai::configuration::get_default_ai_parameters();

		lua_engine = new ai::engine_lua(ai_context, cfg);
		LOG_LUA << "Created new dummy lua-engine for debug_ai(). \n";

		//and add the dummy engine as a component
		//to the manager, so we could use it later
		cfg.add_child("engine", lua_engine->to_config());
		ai::component_manager::add_component(c, "engine[]", cfg);
	}

	lua_engine->push_ai_table(); // stack: [-1: ai_context]

	lua_pushstring(L, "components");
	push_component(L, c); // stack: [-1: component tree; -2: ai context]
	lua_rawset(L, -3);

	return 1;
}

/// Allow undo sets the flag saying whether the event has mutated the game to false.
int game_lua_kernel::intf_allow_end_turn(lua_State * L)
{
	gamedata().set_allow_end_turn(luaW_toboolean(L, 1));
	return 0;
}

/// Allow undo sets the flag saying whether the event has mutated the game to false.
int game_lua_kernel::intf_allow_undo(lua_State * L)
{
	if(lua_isboolean(L, 1)) {
		play_controller_.pump().context_mutated(!luaW_toboolean(L, 1));
	}
	else {
		play_controller_.pump().context_mutated(false);
	}
	return 0;
}

/// Adding new time_areas dynamically with Standard Location Filters.
int game_lua_kernel::intf_add_time_area(lua_State * L)
{
	log_scope("time_area");

	vconfig cfg(luaW_checkvconfig(L, 1));
	const std::string id = cfg["id"];

	std::set<map_location> locs;
	const terrain_filter filter(cfg, &game_state_);
	filter.get_locations(locs, true);
	config parsed_cfg = cfg.get_parsed_config();
	tod_man().add_time_area(id, locs, parsed_cfg);
	LOG_LUA << "Lua inserted time_area '" << id << "'\n";
	return 0;
}

/// Removing new time_areas dynamically with Standard Location Filters.
int game_lua_kernel::intf_remove_time_area(lua_State * L)
{
	log_scope("remove_time_area");

	const char * id = luaL_checkstring(L, 1);
	tod_man().remove_time_area(id);
	LOG_LUA << "Lua removed time_area '" << id << "'\n";

	return 0;
}

/// Replacing the current time of day schedule.
int game_lua_kernel::intf_replace_schedule(lua_State * L)
{
	vconfig cfg = luaW_checkvconfig(L, 1);

	if(cfg.get_children("time").empty()) {
		ERR_LUA << "attempted to to replace ToD schedule with empty schedule" << std::endl;
	} else {
		tod_man().replace_schedule(cfg.get_parsed_config());
		if (game_display_) {
			game_display_->new_turn();
		}
		LOG_LUA << "replaced ToD schedule\n";
	}
	return 0;
}

int game_lua_kernel::intf_scroll(lua_State * L)
{
	vconfig cfg = luaW_checkvconfig(L, 1);

	if (game_display_) {
		const std::vector<int> side_list = get_sides_vector(cfg);
		bool side_match = false;
		BOOST_FOREACH(int side, side_list) {
			if(teams()[side-1].is_local_human()) {
				side_match = true;
				break;
			}
		}
		if ((cfg["side"].empty() && !cfg.has_child("filter_side")) || side_match) {
			game_display_->scroll(cfg["x"], cfg["y"], true);
			game_display_->draw(true,true);
		}
	}

	return 0;
}

namespace {
	struct lua_report_generator : reports::generator
	{
		lua_State *mState;
		std::string name;
		lua_report_generator(lua_State *L, const std::string &n)
			: mState(L), name(n) {}
		virtual config generate(reports::context & rc);
	};

	config lua_report_generator::generate(reports::context & /*rc*/)
	{
		lua_State *L = mState;
		config cfg;
		if (!luaW_getglobal(L, "wesnoth", "theme_items", name.c_str(), NULL))
			return cfg;
		if (!luaW_pcall(L, 0, 1)) return cfg;
		luaW_toconfig(L, -1, cfg);
		lua_pop(L, 1);
		return cfg;
	}
}//unnamed namespace for lua_report_generator

/**
 * Executes its upvalue as a theme item generator.
 */
int game_lua_kernel::impl_theme_item(lua_State *L, std::string m)
{
	reports::context temp_context = reports::context(board(), *game_display_, tod_man(), play_controller_.get_whiteboard(), play_controller_.get_mouse_handler_base());
	luaW_pushconfig(L, reports_.generate_report(m.c_str(), temp_context , true));
	return 1;
}

/**
 * Creates a field of the theme_items table and returns it (__index metamethod).
 */
int game_lua_kernel::impl_theme_items_get(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	lua_cpp::push_closure(L, boost::bind(&game_lua_kernel::impl_theme_item, this, _1, std::string(m)), 0);
	lua_pushvalue(L, 2);
	lua_pushvalue(L, -2);
	lua_rawset(L, 1);
	reports_.register_generator(m, new lua_report_generator(L, m));
	return 1;
}

/**
 * Sets a field of the theme_items table (__newindex metamethod).
 */
int game_lua_kernel::impl_theme_items_set(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	lua_pushvalue(L, 2);
	lua_pushvalue(L, 3);
	lua_rawset(L, 1);
	reports_.register_generator(m, new lua_report_generator(L, m));
	return 0;
}

/**
 * Gets all the WML variables currently set.
 * - Ret 1: WML table
 */
int game_lua_kernel::intf_get_all_vars(lua_State *L) {
	luaW_pushconfig(L, gamedata().get_variables());
	return 1;
}

// END CALLBACK IMPLEMENTATION

game_board & game_lua_kernel::board() {
	return game_state_.board_;
}

unit_map & game_lua_kernel::units() {
	return game_state_.board_.units_;
}

std::vector<team> & game_lua_kernel::teams() {
	return game_state_.board_.teams_;
}

const gamemap & game_lua_kernel::map() {
	return game_state_.board_.map();
}

game_data & game_lua_kernel::gamedata() {
	return game_state_.gamedata_;
}

tod_manager & game_lua_kernel::tod_man() {
	return game_state_.tod_manager_;
}

const game_events::queued_event & game_lua_kernel::get_event_info() {
	return *queued_events_.top();
}

// Template which allows to push member functions to the lua kernel base into lua as C functions, using a shim
typedef int (game_lua_kernel::*member_callback)(lua_State *);

template <member_callback method>
int dispatch(lua_State *L) {
	return ((lua_kernel_base::get_lua_kernel<game_lua_kernel>(L)).*method)(L);
}

// Pass a const bool also...
typedef int (game_lua_kernel::*member_callback2)(lua_State *, bool);

template <member_callback2 method, bool b>
int dispatch2(lua_State *L) {
	return ((lua_kernel_base::get_lua_kernel<game_lua_kernel>(L)).*method)(L, b);
}


game_lua_kernel::game_lua_kernel(CVideo * video, game_state & gs, play_controller & pc, reports & reports_object)
	: lua_kernel_base(video)
	, game_display_(NULL)
	, game_state_(gs)
	, play_controller_(pc)
	, reports_(reports_object)
	, level_lua_()
	, queued_events_()
{
	static game_events::queued_event default_queued_event("_from_lua", map_location(), map_location(), config());
	queued_events_.push(&default_queued_event);

	lua_State *L = mState;

	cmd_log_ << "Registering game-specific wesnoth lib functions...\n";

	// Put some callback functions in the scripting environment.
	static luaL_Reg const callbacks[] = {
		{ "add_known_unit",           &intf_add_known_unit           },
		{ "add_modification",         &intf_add_modification         },
		{ "advance_unit",             &intf_advance_unit             },
		{ "copy_unit",                &intf_copy_unit                },
		{ "create_unit",              &intf_create_unit              },
		{ "debug",                    &intf_debug                    },
		{ "debug_ai",                 &intf_debug_ai                 },
		{ "eval_conditional",         &intf_eval_conditional         },
		{ "get_era",                  &intf_get_era                  },
		{ "get_image_size",           &intf_get_image_size           },
		{ "get_time_stamp",           &intf_get_time_stamp           },
		{ "get_traits",               &intf_get_traits               },
		{ "get_viewing_side",         &intf_get_viewing_side         },
		{ "modify_ai",                &intf_modify_ai                },
		{ "set_music",                &intf_set_music                },
		{ "transform_unit",           &intf_transform_unit           },
		{ "unit_ability",             &intf_unit_ability             },
		{ "unit_defense",             &intf_unit_defense             },
		{ "unit_movement_cost",       &intf_unit_movement_cost       },
		{ "unit_vision_cost",         &intf_unit_vision_cost         },
		{ "unit_jamming_cost",        &intf_unit_jamming_cost        },
		{ "unit_resistance",          &intf_unit_resistance          },
		{ "unsynced",                 &intf_do_unsynced              },
		{ "add_event_handler",         &dispatch<&game_lua_kernel::intf_add_event                  >        },
		{ "add_tile_overlay",          &dispatch<&game_lua_kernel::intf_add_tile_overlay           >        },
		{ "add_time_area",             &dispatch<&game_lua_kernel::intf_add_time_area              >        },
		{ "allow_end_turn",            &dispatch<&game_lua_kernel::intf_allow_end_turn             >        },
		{ "allow_undo",                &dispatch<&game_lua_kernel::intf_allow_undo                 >        },
		{ "animate_unit",              &dispatch<&game_lua_kernel::intf_animate_unit               >        },
		{ "check_end_level_disabled",  &dispatch<&game_lua_kernel::intf_check_end_level_disabled   >        },
		{ "clear_menu_item",           &dispatch<&game_lua_kernel::intf_clear_menu_item            >        },
		{ "clear_messages",            &dispatch<&game_lua_kernel::intf_clear_messages             >        },
		{ "color_adjust",              &dispatch<&game_lua_kernel::intf_color_adjust               >        },
		{ "delay",                     &dispatch<&game_lua_kernel::intf_delay                      >        },
		{ "end_turn",                  &dispatch<&game_lua_kernel::intf_end_turn                   >        },
		{ "end_level",                 &dispatch<&game_lua_kernel::intf_end_level                  >        },
		{ "erase_unit",                &dispatch<&game_lua_kernel::intf_erase_unit                 >        },
		{ "extract_unit",              &dispatch<&game_lua_kernel::intf_extract_unit               >        },
		{ "find_cost_map",             &dispatch<&game_lua_kernel::intf_find_cost_map              >        },
		{ "find_path",                 &dispatch<&game_lua_kernel::intf_find_path                  >        },
		{ "find_reach",                &dispatch<&game_lua_kernel::intf_find_reach                 >        },
		{ "find_vacant_tile",          &dispatch<&game_lua_kernel::intf_find_vacant_tile           >        },
		{ "fire_event",                &dispatch<&game_lua_kernel::intf_fire_event                 >        },
		{ "fire_wml_menu_item",        &dispatch<&game_lua_kernel::intf_fire_wml_menu_item         >        },
		{ "float_label",               &dispatch<&game_lua_kernel::intf_float_label                >        },
		{ "gamestate_inspector",       &dispatch<&game_lua_kernel::intf_gamestate_inspector        >        },
		{ "get_all_vars",              &dispatch<&game_lua_kernel::intf_get_all_vars               >        },
		{ "get_locations",             &dispatch<&game_lua_kernel::intf_get_locations              >        },
		{ "get_map_size",              &dispatch<&game_lua_kernel::intf_get_map_size               >        },
		{ "get_mouseover_tile",        &dispatch<&game_lua_kernel::intf_get_mouseover_tile         >        },
		{ "get_recall_units",          &dispatch<&game_lua_kernel::intf_get_recall_units           >        },
		{ "get_selected_tile",         &dispatch<&game_lua_kernel::intf_get_selected_tile          >        },
		{ "get_sides",                 &dispatch<&game_lua_kernel::intf_get_sides                  >        },
		{ "get_starting_location",     &dispatch<&game_lua_kernel::intf_get_starting_location      >        },
		{ "get_terrain",               &dispatch<&game_lua_kernel::intf_get_terrain                >        },
		{ "get_terrain_info",          &dispatch<&game_lua_kernel::intf_get_terrain_info           >        },
		{ "get_time_of_day",           &dispatch<&game_lua_kernel::intf_get_time_of_day            >        },
		{ "get_unit",                  &dispatch<&game_lua_kernel::intf_get_unit                   >        },
		{ "get_units",                 &dispatch<&game_lua_kernel::intf_get_units                  >        },
		{ "get_variable",              &dispatch<&game_lua_kernel::intf_get_variable               >        },
		{ "get_side_variable",         &dispatch<&game_lua_kernel::intf_get_side_variable          >        },
		{ "get_villages",              &dispatch<&game_lua_kernel::intf_get_villages               >        },
		{ "get_village_owner",         &dispatch<&game_lua_kernel::intf_get_village_owner          >        },
		{ "get_displayed_unit",        &dispatch<&game_lua_kernel::intf_get_displayed_unit         >        },
		{ "heal_unit",                 &dispatch<&game_lua_kernel::intf_heal_unit                  >        },
		{ "highlight_hex",             &dispatch<&game_lua_kernel::intf_highlight_hex              >        },
		{ "is_enemy",                  &dispatch<&game_lua_kernel::intf_is_enemy                   >        },
		{ "kill",                      &dispatch<&game_lua_kernel::intf_kill                       >        },
		{ "label",                     &dispatch<&game_lua_kernel::intf_label                      >        },
		{ "lock_view",                 &dispatch<&game_lua_kernel::intf_lock_view                  >        },
		{ "match_location",            &dispatch<&game_lua_kernel::intf_match_location             >        },
		{ "match_side",                &dispatch<&game_lua_kernel::intf_match_side                 >        },
		{ "match_unit",                &dispatch<&game_lua_kernel::intf_match_unit                 >        },
		{ "message",                   &dispatch<&game_lua_kernel::intf_message                    >        },
		{ "modify_ai_wml",             &dispatch<&game_lua_kernel::intf_modify_ai_wml              >        },
		{ "modify_side",               &dispatch<&game_lua_kernel::intf_modify_side                >        },
		{ "open_help",                 &dispatch<&game_lua_kernel::intf_open_help                  >        },
		{ "play_sound",                &dispatch<&game_lua_kernel::intf_play_sound                 >        },
		{ "print",                     &dispatch<&game_lua_kernel::intf_print                      >        },
		{ "put_recall_unit",           &dispatch<&game_lua_kernel::intf_put_recall_unit            >        },
		{ "put_unit",                  &dispatch<&game_lua_kernel::intf_put_unit                   >        },
		{ "random",                    &dispatch<&game_lua_kernel::intf_random                     >        },
		{ "redraw",                    &dispatch<&game_lua_kernel::intf_redraw                     >        },
		{ "remove_event_handler",      &dispatch<&game_lua_kernel::intf_remove_event               >        },
		{ "remove_tile_overlay",       &dispatch<&game_lua_kernel::intf_remove_tile_overlay        >        },
		{ "remove_time_area",          &dispatch<&game_lua_kernel::intf_remove_time_area           >        },
		{ "replace_schedule",          &dispatch<&game_lua_kernel::intf_replace_schedule           >        },
		{ "scroll",                    &dispatch<&game_lua_kernel::intf_scroll                     >        },
		{ "scroll_to_tile",            &dispatch<&game_lua_kernel::intf_scroll_to_tile             >        },
		{ "select_hex",                &dispatch<&game_lua_kernel::intf_select_hex                 >        },
		{ "deselect_hex",              &dispatch<&game_lua_kernel::intf_deselect_hex               >        },
		{ "skip_messages",             &dispatch<&game_lua_kernel::intf_skip_messages              >        },
		{ "is_skipping_messages",      &dispatch<&game_lua_kernel::intf_is_skipping_messages       >        },
		{ "set_end_campaign_credits",  &dispatch<&game_lua_kernel::intf_set_end_campaign_credits   >        },
		{ "set_end_campaign_text",     &dispatch<&game_lua_kernel::intf_set_end_campaign_text      >        },
		{ "set_menu_item",             &dispatch<&game_lua_kernel::intf_set_menu_item              >        },
		{ "set_next_scenario",         &dispatch<&game_lua_kernel::intf_set_next_scenario          >        },
		{ "set_terrain",               &dispatch<&game_lua_kernel::intf_set_terrain                >        },
		{ "set_variable",              &dispatch<&game_lua_kernel::intf_set_variable               >        },
		{ "set_side_variable",         &dispatch<&game_lua_kernel::intf_set_side_variable          >        },
		{ "set_village_owner",         &dispatch<&game_lua_kernel::intf_set_village_owner          >        },
		{ "simulate_combat",           &dispatch<&game_lua_kernel::intf_simulate_combat            >        },
		{ "synchronize_choice",        &intf_synchronize_choice                                             },
		{ "synchronize_choices",       &intf_synchronize_choices                                            },
		{ "view_locked",               &dispatch<&game_lua_kernel::intf_view_locked                >        },
		{ "place_shroud",              &dispatch2<&game_lua_kernel::intf_shroud_op, true  >                 },
		{ "remove_shroud",             &dispatch2<&game_lua_kernel::intf_shroud_op, false >                 },
		{ NULL, NULL }
	};
	lua_cpp::Reg const cpp_callbacks[] = {
		{ NULL, NULL }
	};
	lua_getglobal(L, "wesnoth");
	if (!lua_istable(L,-1)) {
		lua_newtable(L);
	}
	luaL_setfuncs(L, callbacks, 0);
	lua_cpp::set_functions(L, cpp_callbacks);
	lua_setglobal(L, "wesnoth");

	// Create the getside metatable.
	cmd_log_ << lua_team::register_metatable(L);

	// Create the gettype metatable.
	cmd_log_ << lua_unit_type::register_metatable(L);

	//Create the getrace metatable
	cmd_log_ << lua_race::register_metatable(L);

	// Create the getunit metatable.
	cmd_log_ << "Adding getunit metatable...\n";

	lua_pushlightuserdata(L
			, getunitKey);
	lua_createtable(L, 0, 5);
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
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the unit status metatable.
	cmd_log_ << "Adding unit status metatable...\n";

	lua_pushlightuserdata(L
			, ustatusKey);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, impl_unit_status_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_unit_status_set);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "unit status");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the unit attacks metatable.
	cmd_log_ << "Adding unit attacks metatable...\n";

	lua_pushlightuserdata(L, uattacksKey);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, impl_unit_attacks_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_unit_attacks_len);
	lua_setfield(L, -2, "__len");
	lua_pushstring(L, "unit attacks");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);


	lua_pushlightuserdata(L, uattackKey);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, impl_unit_attack_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_unit_attack_set);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "unit attack");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the unit variables metatable.
	cmd_log_ << "Adding unit variables metatable...\n";

	lua_pushlightuserdata(L
			, unitvarKey);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, impl_unit_variables_get);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, impl_unit_variables_set);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "unit variables");
	lua_setfield(L, -2, "__metatable");
	lua_rawset(L, LUA_REGISTRYINDEX);

	// Create the vconfig metatable.
	cmd_log_ << lua_common::register_vconfig_metatable(L);

	// Create the ai elements table.
	cmd_log_ << "Adding ai elements table...\n";

	ai::lua_ai_context::init(L);

	// Create the game_config variable with its metatable.
	cmd_log_ << "Adding game_config table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newuserdata(L, 0);
	lua_createtable(L, 0, 3);
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_game_config_get>);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_game_config_set>);
	lua_setfield(L, -2, "__newindex");
	lua_pushstring(L, "game config");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "game_config");
	lua_pop(L, 1);

	// Create the current variable with its metatable.
	cmd_log_ << "Adding wesnoth current table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newuserdata(L, 0);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_current_get>);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "current config");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "current");
	lua_pop(L, 1);

	// Create the wml_actions table.
	cmd_log_ << "Adding wml_actions table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "wml_actions");
	lua_pop(L, 1);

	// Create the wml_conditionals table.
	cmd_log_ << "Adding wml_conditionals table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "wml_conditionals");
	lua_pop(L, 1);

	// Create the effects table.
	cmd_log_ << "Adding effects table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "effects");
	lua_pop(L, 1);

	// Create the game_events table.
	cmd_log_ << "Adding game_events table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "game_events");
	lua_pop(L, 1);

	// Create the theme_items table.
	cmd_log_ << "Adding theme_items table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_theme_items_get>);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_theme_items_set>);
	lua_setfield(L, -2, "__newindex");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "theme_items");
	lua_pop(L, 1);

	lua_settop(L, 0);

	FOREACH(const AUTO& handler, game_events::wml_action::registry())
	{
		set_wml_action(handler.first, handler.second);
	}
}

void game_lua_kernel::initialize(const config& level)
{
	lua_State *L = mState;
	assert(level_lua_.empty());
	level_lua_.append_children(level, "lua");
	// Create the sides table.
	// note:
	// This table is redundant to the return value of wesnoth.get_sides({}).
	// Still needed for backwards compatibility.
	lua_getglobal(L, "wesnoth");

	lua_pushstring(L, "get_sides");
	lua_rawget(L, -2);
	lua_createtable(L, 0, 0);

	if (!protected_call(1, 1, boost::bind(&lua_kernel_base::log_error, this, _1, _2))) {
		cmd_log_ << "Failed to compute wesnoth.sides\n";
	} else {
		lua_setfield(L, -2, "sides");
		cmd_log_ << "Added wesnoth.sides\n";
	}

	// Create the unit_types table.
	cmd_log_ << "Adding unit_types table...\n";

	lua_settop(L, 0);
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &ut, unit_types.types())
	{
		luaW_pushunittype(L, ut.first);
		lua_setfield(L, -2, ut.first.c_str());
	}
	lua_setfield(L, -2, "unit_types");

	//Create the races table.
	cmd_log_ << "Adding races table...\n";

	lua_settop(L, 0);
	lua_getglobal(L, "wesnoth");
	luaW_pushracetable(L);
	lua_setfield(L, -2, "races");
	lua_pop(L, 1);

	// Execute the preload scripts.
	cmd_log_ << "Running preload scripts...\n";

	game_config::load_config(game_lua_kernel::preload_config);
	BOOST_FOREACH(const config &cfg, game_lua_kernel::preload_scripts) {
		run(cfg["code"].str().c_str());
	}
	BOOST_FOREACH(const config &cfg, level_lua_.child_range("lua")) {
		run(cfg["code"].str().c_str());
	}

	load_game(level);
}

int game_lua_kernel::return_unit_method(lua_State *L, char const *m) {
	static luaL_Reg const methods[] = {
		{"matches",               &dispatch<&game_lua_kernel::intf_match_unit>},
		{"to_recall",             &dispatch<&game_lua_kernel::intf_put_recall_unit>},
		{"to_map",                &dispatch<&game_lua_kernel::intf_put_unit>},
		{"erase",                 &dispatch<&game_lua_kernel::intf_erase_unit>},
		{"clone",                 intf_copy_unit},
		{"extract",               &dispatch<&game_lua_kernel::intf_extract_unit>},
		{"advance",               intf_advance_unit},
		{"add_modification",      intf_add_modification},
		{"resistance",            intf_unit_resistance},
		{"defense",               intf_unit_defense},
		{"movement",              intf_unit_movement_cost},
		{"vision",                intf_unit_vision_cost},
		{"jamming",               intf_unit_jamming_cost},
		{"ability",               intf_unit_ability},
		{"transform",             intf_transform_unit},
	};

	BOOST_FOREACH(const luaL_Reg& r, methods) {
		if (strcmp(m, r.name) == 0) {
			lua_pushcfunction(L, r.func);
			return 1;
		}
	}

	return 0;
}

void game_lua_kernel::set_game_display(game_display * gd) {
	game_display_ = gd;
	if (gd) {
		set_video(&gd->video());
	}
}

/// These are the child tags of [scenario] (and the like) that are handled
/// elsewhere (in the C++ code).
/// Any child tags not in this list will be passed to Lua's on_load event.
static char const *handled_file_tags[] = {
	"color_palette", "color_range", "display", "end_level_data", "era",
	"event", "generator", "label", "lua", "map", "menu_item",
	"modification", "music", "options", "side", "sound_source",
	"story", "terrain_graphics", "time", "time_area", "tunnel",
	"undo_stack", "variables"
};

static bool is_handled_file_tag(const std::string &s)
{
	BOOST_FOREACH(char const *t, handled_file_tags) {
		if (s == t) return true;
	}
	return false;
}

/**
 * Executes the game_events.on_load function and passes to it all the
 * scenario tags not yet handled.
 */
void game_lua_kernel::load_game(const config& level)
{
	lua_State *L = mState;

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_load", NULL))
		return;

	lua_newtable(L);
	int k = 1;
	BOOST_FOREACH(const config::any_child &v, level.all_children_range())
	{
		if (is_handled_file_tag(v.key)) continue;
		lua_createtable(L, 2, 0);
		lua_pushstring(L, v.key.c_str());
		lua_rawseti(L, -2, 1);
		luaW_pushconfig(L, v.cfg);
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, k++);
	}

	luaW_pcall(L, 1, 0, true);
}

/**
 * Executes the game_events.on_save function and adds to @a cfg the
 * returned tags. Also flushes the [lua] tags.
 */
void game_lua_kernel::save_game(config &cfg)
{
	lua_State *L = mState;

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_save", NULL))
		return;

	if (!luaW_pcall(L, 0, 1, false))
		return;

	config v;
	luaW_toconfig(L, -1, v);
	lua_pop(L, 1);

	for (;;)
	{
		config::all_children_iterator i = v.ordered_begin();
		if (i == v.ordered_end()) break;
		if (is_handled_file_tag(i->key))
		{
			/*
			 * It seems the only tags appearing in the config v variable here
			 * are the core-lua-handled (currently [item] and [objectives])
			 * and the extra UMC ones.
			 */
			const std::string m = "Tag is already used: [" + i->key + "]";
			log_error(m.c_str());
			v.erase(i);
			continue;
		}
		cfg.splice_children(v, i->key);
	}
}

/**
 * Executes the game_events.on_event function.
 * Returns false if there was no lua handler for this event
 */
bool game_lua_kernel::run_event(game_events::queued_event const &ev)
{
	lua_State *L = mState;

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_event", NULL))
		return false;

	queued_event_context dummy(&ev, queued_events_);
	lua_pushstring(L, ev.name.c_str());
	luaW_pcall(L, 1, 0, false);
	return true;
}

/**
 * Executes its upvalue as a wml action.
 */
int game_lua_kernel::cfun_wml_action(lua_State *L)
{
	game_events::wml_action::handler h = reinterpret_cast<game_events::wml_action::handler>
		(lua_touserdata(L, lua_upvalueindex(1)));

	vconfig vcfg = luaW_checkvconfig(L, 1);
	h(get_event_info(), vcfg);
	return 0;
}

/**
 * Registers a function for use as an action handler.
 */
void game_lua_kernel::set_wml_action(std::string const &cmd, game_events::wml_action::handler h)
{
	lua_State *L = mState;

	lua_getglobal(L, "wesnoth");
	lua_pushstring(L, "wml_actions");
	lua_rawget(L, -2);
	lua_pushstring(L, cmd.c_str());
	lua_pushlightuserdata(L, reinterpret_cast<void *>(h));
	lua_pushcclosure(L, &dispatch<&game_lua_kernel::cfun_wml_action>, 1);
	lua_rawset(L, -3);
	lua_pop(L, 2);
}

/**
 * Runs a command from an event handler.
 * @return true if there is a handler for the command.
 * @note @a cfg should be either volatile or long-lived since the Lua
 *       code may grab it for an arbitrary long time.
 */
bool game_lua_kernel::run_wml_action(std::string const &cmd, vconfig const &cfg,
	game_events::queued_event const &ev)
{
	lua_State *L = mState;


	if (!luaW_getglobal(L, "wesnoth", "wml_actions", cmd.c_str(), NULL))
		return false;

	queued_event_context dummy(&ev, queued_events_);
	luaW_pushvconfig(L, cfg);
	luaW_pcall(L, 1, 0, true);
	return true;
}


/**
 * Runs a command from an event handler.
 * @return true if there is a handler for the command.
 * @note @a cfg should be either volatile or long-lived since the Lua
 *       code may grab it for an arbitrary long time.
 */
bool game_lua_kernel::run_wml_conditional(std::string const &cmd, vconfig const &cfg)
{
	lua_State *L = mState;


	if (!luaW_getglobal(L, "wesnoth", "wml_conditionals", cmd.c_str(), NULL)) {
		std::string err_msg = "unknown conditional wml: [";
		err_msg += cmd;
		err_msg += "]";
		luaL_argerror(L, 1, err_msg.c_str());
	}

	luaW_pushvconfig(L, cfg);
	luaW_pcall(L, 1, 1, true);

	bool b = luaW_toboolean(L, -1);
	lua_pop(L, 1);
	return b;
}


/**
 * Runs a script from a unit filter.
 * The script is an already compiled function given by its name.
 */
bool game_lua_kernel::run_filter(char const *name, unit const &u)
{
	lua_State *L = mState;

	unit_map::const_unit_iterator ui = units().find(u.get_location());
	if (!ui.valid()) return false;

	// Get the user filter by name.
	if(!luaW_getglobal(L, name, NULL))
	{
		std::string message = std::string() + "function " + name + " not found";
		log_error(message.c_str(), "Lua SUF Error");
		//we pushed nothing and can safeley return.
		return false;
	}
	// Pass the unit as argument.
	new(lua_newuserdata(L, sizeof(lua_unit))) lua_unit(ui->underlying_id());
	lua_pushlightuserdata(L
			, getunitKey);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_setmetatable(L, -2);

	if (!luaW_pcall(L, 1, 1)) return false;

	bool b = luaW_toboolean(L, -1);
	lua_pop(L, 1);
	return b;
}

void game_lua_kernel::apply_effect(const std::string& name, unit& u, const config& cfg)
{
	lua_State *L = mState;
	// Stack: nothing
	if (!luaW_getglobal(L, "wesnoth", "effects", name.c_str(), NULL)) {
		return;
	}
	// Stack: effect_func
	lua_unit* lu = LuaW_pushlocalunit(L, u);
	// Stack: effect_func, unit
	lua_pushvalue(L, -2);
	// Stack: effect_func, unit, effect_func
	// we need to push the lua unit to the stack twice to ensure that it won't be collected during the function call.
	lua_pushvalue(L, -2);
	// Stack: effect_func, unit, effect_func, unit
	lua_push(L, cfg);
	// Stack: effect_func, unit, effect_func, unit, cfg
	luaW_pcall(L, 2, 0, true);
	// Stack: effect_func, unit
	lu->clear_ref();
	lua_pop(L, 2);
	return;
}

ai::lua_ai_context* game_lua_kernel::create_lua_ai_context(char const *code, ai::engine_lua *engine)
{
	return ai::lua_ai_context::create(mState,code,engine);
}

ai::lua_ai_action_handler* game_lua_kernel::create_lua_ai_action_handler(char const *code, ai::lua_ai_context &context)
{
	return ai::lua_ai_action_handler::create(mState,code,context);
}
