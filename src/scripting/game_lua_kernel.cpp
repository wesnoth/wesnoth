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

#include "actions/attack.hpp"           // for battle_context_unit_stats, etc
#include "actions/advancement.hpp"           // for advance_unit_at, etc
#include "actions/move.hpp"		// for clear_shroud
#include "actions/vision.hpp"		// for clear_shroud
#include "ai/composite/ai.hpp"          // for ai_composite
#include "ai/composite/component.hpp"   // for component, etc
#include "ai/composite/contexts.hpp"    // for ai_context
#include "ai/lua/engine_lua.hpp"  // for engine_lua
#include "ai/composite/rca.hpp"  // for candidate_action
#include "ai/composite/stage.hpp"  // for stage
#include "ai/configuration.hpp"         // for configuration
#include "ai/lua/core.hpp"              // for lua_ai_context, etc
#include "ai/manager.hpp"               // for manager, holder
#include "attack_prediction.hpp"        // for combatant
#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"                   // for config, etc
#include "display_chat_manager.hpp"	// for clear_chat_messages
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
#include "utils/make_enum.hpp"                // for operator<<
#include "map/map.hpp"                      // for gamemap
#include "map/label.hpp"
#include "map/location.hpp"             // for map_location
#include "mouse_events.hpp"             // for mouse_handler
#include "mp_game_settings.hpp"         // for mp_game_settings
#include "pathfind/pathfind.hpp"        // for full_cost_map, plain_route, etc
#include "pathfind/teleport.hpp"        // for get_teleport_locations, etc
#include "play_controller.hpp"          // for play_controller
#include "recall_list_manager.hpp"      // for recall_list_manager
#include "replay.hpp"                   // for get_user_choice, etc
#include "reports.hpp"                  // for register_generator, etc
#include "scripting/lua_audio.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_gui2.hpp"	// for show_gamestate_inspector
#include "scripting/lua_pathfind_cost_calculator.hpp"
#include "scripting/lua_race.hpp"
#include "scripting/lua_team.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/push_check.hpp"
#include "color.hpp"                // for surface
#include "sdl/surface.hpp"                // for surface
#include "side_filter.hpp"              // for side_filter
#include "sound.hpp"                    // for commit_music_changes, etc
#include "soundsource.hpp"
#include "synced_context.hpp"           // for synced_context, etc
#include "synced_user_choice.hpp"
#include "team.hpp"                     // for team, village_owner
#include "terrain/terrain.hpp"                  // for terrain_type
#include "terrain/filter.hpp"           // for terrain_filter
#include "terrain/translation.hpp"      // for read_terrain_code, etc
#include "terrain/type_data.hpp"
#include "time_of_day.hpp"              // for time_of_day
#include "tod_manager.hpp"              // for tod_manager
#include "tstring.hpp"                  // for t_string, operator+
#include "units/unit.hpp"                     // for unit
#include "units/animation_component.hpp"  // for unit_animation_component
#include "units/udisplay.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"  // for unit_map, etc
#include "units/ptr.hpp"                 // for unit_const_ptr, unit_ptr
#include "units/types.hpp"    // for unit_type_data, unit_types, etc
#include "variable.hpp"                 // for vconfig, etc
#include "variable_info.hpp"
#include "wml_exception.hpp"
#include "config_assign.hpp"

#include "utils/functional.hpp"               // for bind_t, bind
#include <boost/range/algorithm/copy.hpp>    // boost::copy
#include <boost/range/adaptors.hpp>     // boost::adaptors::filtered
#include <cassert>                      // for assert
#include <cstring>                      // for strcmp
#include <iterator>                     // for distance, advance
#include <map>                          // for map, map<>::value_type, etc
#include <new>                          // for operator new
#include <set>                          // for set
#include <sstream>                      // for operator<<, basic_ostream, etc
#include <utility>                      // for pair
#include <algorithm>
#include <vector>                       // for vector, etc
#include <SDL_timer.h>                  // for SDL_GetTicks
#include "lua/lauxlib.h"                // for luaL_checkinteger, etc
#include "lua/lua.h"                    // for lua_setfield, etc

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

struct map_locker
{
	map_locker(game_lua_kernel* kernel) : kernel_(kernel)
	{
		++kernel_->map_locked_;
	}
	~map_locker()
	{
		--kernel_->map_locked_;
	}
	game_lua_kernel* kernel_;
};


void game_lua_kernel::extract_preload_scripts(config const &game_config)
{
	game_lua_kernel::preload_scripts.clear();
	for (config const &cfg : game_config.child_range("lua")) {
		game_lua_kernel::preload_scripts.push_back(cfg);
	}
	game_lua_kernel::preload_config = game_config.child("game_config");
}

void game_lua_kernel::log_error(char const * msg, char const * context)
{
	lua_kernel_base::log_error(msg, context);
	lua_chat(context, msg);
}

void game_lua_kernel::lua_chat(const std::string& caption, const std::string& msg)
{
	if (game_display_) {
		game_display_->get_chat_manager().add_chat_message(time(nullptr), caption, 0, msg,
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



static int special_locations_len(lua_State *L)
{
	lua_pushnumber(L, lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).map().special_locations().size());
	return 1;
}

static int special_locations_next(lua_State *L)
{
	const t_translation::starting_positions::left_map& left = lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).map().special_locations().left;

	t_translation::starting_positions::left_const_iterator it;
	if (lua_isnoneornil(L, 2)) {
		it = left.begin();
	}
	else {
		it = left.find(luaL_checkstring(L, 2));
		if (it == left.end()) {
			return 0;
		}
		++it;
	}
	if (it == left.end()) {
		return 0;
	}
	lua_pushstring(L, it->first.c_str());
	luaW_pushlocation(L, it->second);
	return 2;
}

static int special_locations_pairs(lua_State *L)
{
	lua_pushcfunction(L, &special_locations_next);
	lua_pushvalue(L, -2);
	lua_pushnil(L);
	return 3;
}

static int special_locations_index(lua_State *L)
{
	const t_translation::starting_positions::left_map& left = lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).map().special_locations().left;
	auto it = left.find(luaL_checkstring(L, 2));
	if (it == left.end()) {
		return 0;
	}
	else {
		luaW_pushlocation(L, it->second);
		return 1;
	}
}

static int special_locations_newindex(lua_State *L)
{
	lua_pushstring(L, "special locations cannot be modified using wesnoth.special_locations");
	return lua_error(L);
}

static void push_locations_table(lua_State *L)
{
	lua_newtable(L); // The functor table
	lua_newtable(L); // The functor metatable
	lua_pushstring(L, "__len");
	lua_pushcfunction(L, &special_locations_len);
	lua_rawset(L, -3);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, &special_locations_index);
	lua_rawset(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, &special_locations_newindex);
	lua_rawset(L, -3);
	lua_pushstring(L, "__pairs");
	lua_pushcfunction(L, &special_locations_pairs);
	lua_rawset(L, -3);
	lua_setmetatable(L, -2); // Apply the metatable to the functor table
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

static const char animatorKey[] = "unit animator";

static int impl_animator_collect(lua_State* L) {
	unit_animator& anim = *static_cast<unit_animator*>(luaL_checkudata(L, 1, animatorKey));
	anim.~unit_animator();
	return 0;
}

static int impl_add_animation(lua_State* L)
{
	unit_animator& anim = *static_cast<unit_animator*>(luaL_checkudata(L, 1, animatorKey));
	unit& u = luaW_checkunit(L, 2);
	std::string which = luaL_checkstring(L, 3);

	using hit_type = unit_animation::hit_type;
	std::string hits_str = luaL_checkstring(L, 4);
	hit_type hits = hit_type::string_to_enum(hits_str, hit_type::INVALID);

	map_location dest;
	int v1 = 0, v2 = 0;
	bool bars = false;
	t_string text;
	color_t color{255, 255, 255};
	const_attack_ptr primary, secondary;

	if(lua_istable(L, 5)) {
		lua_getfield(L, 5, "target");
		if(luaW_tolocation(L, -1, dest)) {
			if(!tiles_adjacent(dest, u.get_location())) {
				return luaL_argerror(L, -1, "target must be adjacent to the animated unit");
			}
		} else {
			// luaW_tolocation may set the location to (0,0) if it fails
			dest = map_location();
			if(!lua_isnoneornil(L, -1)) {
				return luaW_type_error(L, -1, "location table");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "value");
		if(lua_isnumber(L, -1)) {
			v1 = lua_tonumber(L, -1);
		} else if(lua_istable(L, -1)) {
			lua_rawgeti(L, -1, 1);
			v1 = lua_tonumber(L, -1);
			lua_pop(L, 1);
			lua_rawgeti(L, -1, 2);
			v2 = lua_tonumber(L, -1);
			lua_pop(L, 1);
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, -1, "number or array of two numbers");
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "with_bars");
		if(lua_isboolean(L, -1)) {
			bars = luaW_toboolean(L, -1);
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, -1, lua_typename(L, LUA_TBOOLEAN));
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "text");
		if(lua_isstring(L, -1)) {
			text = lua_tostring(L, -1);
		} else if(luaW_totstring(L, -1, text)) {
			// Do nothing; luaW_totstring already assigned the value
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, -1, lua_typename(L, LUA_TSTRING));
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "color");
		if(lua_istable(L, -1) && lua_rawlen(L, -1) == 3) {
			int idx = lua_absindex(L, -1);
			lua_rawgeti(L, idx, 1); // red @ -3
			lua_rawgeti(L, idx, 2); // green @ -2
			lua_rawgeti(L, idx, 3); // blue @ -1
			color = color_t(lua_tonumber(L, -3), lua_tonumber(L, -2), lua_tonumber(L, -1));
			lua_pop(L, 3);
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, -1, "array of three numbers");
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "primary");
		primary = luaW_toweapon(L, -1);
		if(!primary && !lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, -1, "weapon");
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "secondary");
		secondary = luaW_toweapon(L, -1);
		if(!secondary && !lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, -1, "weapon");
		}
		lua_pop(L, 1);
	}

	anim.add_animation(&u, which, u.get_location(), dest, v1, bars, text, color, hits, primary, secondary, v2);
	return 0;
}

static int impl_run_animation(lua_State* L)
{
	unit_animator& anim = *static_cast<unit_animator*>(luaL_checkudata(L, 1, animatorKey));
	anim.start_animations();
	anim.wait_for_end();
	anim.set_all_standing();
	return 0;
}

static int impl_clear_animation(lua_State* L)
{
	unit_animator& anim = *static_cast<unit_animator*>(luaL_checkudata(L, 1, animatorKey));
	anim.clear();
	return 0;
}

static int impl_animator_get(lua_State* L)
{
	const char* m = lua_tostring(L, 2);
	return luaW_getmetafield(L, 1, m);
}

static int intf_create_animator(lua_State* L)
{
	new(L) unit_animator;
	if(luaL_newmetatable(L, animatorKey)) {
		luaL_Reg metafuncs[] {
			{"__gc", impl_animator_collect},
			{"__index", impl_animator_get},
			{"add", impl_add_animation},
			{"run", impl_run_animation},
			{"clear", impl_clear_animation},
			{nullptr, nullptr},
		};
		luaL_setfuncs(L, metafuncs, 0);
		lua_pushstring(L, "__metatable");
		lua_setfield(L, -2, animatorKey);
	}
	lua_setmetatable(L, -2);
	return 1;
}

int game_lua_kernel::intf_gamestate_inspector(lua_State *L)
{
	if (game_display_) {
		return lua_gui2::show_gamestate_inspector(game_display_->video(), luaW_checkvconfig(L, 1), gamedata(), game_state_);
	}
	return 0;
}

/**
 * Gets the unit at the given location or with the given id.
 * - Arg 1: location
 * OR
 * - Arg 1: string ID
 * - Ret 1: full userdata with __index pointing to impl_unit_get and
 *          __newindex pointing to impl_unit_set.
 */
int game_lua_kernel::intf_get_unit(lua_State *L)
{
	map_location loc;
	if(lua_isstring(L, 1) && !lua_isnumber(L, 1)) {
		std::string id = luaL_checkstring(L, 1);
		for(const unit& u : units()) {
			if(u.id() == id) {
				luaW_pushunit(L, u.underlying_id());
				return 1;
			}
		}
		return 0;
	}
	if(!luaW_tolocation(L, 1, loc)) {
		return luaL_argerror(L, 1, "expected string or location");
	}
	unit_map::const_iterator ui = units().find(loc);

	if (!ui.valid()) return 0;

	luaW_pushunit(L, ui->underlying_id());
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

	luaW_pushunit(L, ui->underlying_id());
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
	// 1: return table, 2: userdata
	lua_settop(L, 0);
	lua_newtable(L);
	int i = 1;

	// note that if filter is null, this yields a null filter matching everything (and doing no work)
	filter_context & fc = game_state_;
	for (const unit * ui : unit_filter(filter, &fc).all_matches_on_map()) {
		luaW_pushunit(L, ui->underlying_id());
		lua_rawseti(L, 1, i);
		++i;
	}
	return 1;
}

/**
 * Matches a unit against the given filter.
 * - Arg 1: full userdata.
 * - Arg 2: table containing a filter
 * - Arg 3: optional location OR optional "adjacent" unit
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_match_unit(lua_State *L)
{
	lua_unit& u = *luaW_checkunit_ref(L, 1);

	vconfig filter = luaW_checkvconfig(L, 2, true);

	if (filter.null()) {
		lua_pushboolean(L, true);
		return 1;
	}

	filter_context & fc = game_state_;

	if(unit* u_adj = luaW_tounit(L, 3)) {
		if(int side = u.on_recall_list()) {
			WRN_LUA << "wesnoth.match_unit called with a secondary unit (3rd argument), ";
			WRN_LUA << "but unit to match was on recall list. ";
			WRN_LUA << "Thus the 3rd argument is ignored.\n";
			team &t = board().get_team(side);
			scoped_recall_unit auto_store("this_unit", t.save_id(), t.recall_list().find_index(u->id()));
			lua_pushboolean(L, unit_filter(filter, &fc).matches(*u, map_location()));
			return 1;
		}
		if (!u_adj) {
			return luaL_argerror(L, 3, "unit not found");
		}
		lua_pushboolean(L, unit_filter(filter, &fc).matches(*u, *u_adj));
	} else if(int side = u.on_recall_list()) {
		map_location loc;
		luaW_tolocation(L, 3, loc); // If argument 3 isn't a location, loc is unchanged
		team &t = board().get_team(side);
		scoped_recall_unit auto_store("this_unit", t.save_id(), t.recall_list().find_index(u->id()));
		lua_pushboolean(L, unit_filter(filter, &fc).matches(*u, loc));
		return 1;
	} else {
		map_location loc = u->get_location();
		luaW_tolocation(L, 3, loc); // If argument 3 isn't a location, loc is unchanged
		lua_pushboolean(L, unit_filter(filter, &fc).matches(*u, loc));
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
	// 1: return table, 2: userdata
	lua_settop(L, 0);
	lua_newtable(L);
	int i = 1, s = 1;
	filter_context & fc = game_state_;
	const unit_filter ufilt(filter, &fc);
	for (team &t : teams())
	{
		for (unit_ptr & u : t.recall_list())
		{
			if (!filter.null()) {
				scoped_recall_unit auto_store("this_unit",
					t.save_id(), t.recall_list().find_index(u->id()));
				if (!ufilt( *u, map_location() ))
					continue;
			}
			luaW_pushunit(L, s, u->underlying_id());
			lua_rawseti(L, 1, i);
			++i;
		}
		++s;
	}
	return 1;
}

/**
 * Fires an event.
 * - Arg 1: string containing the event name or id.
 * - Arg 2: optional first location.
 * - Arg 3: optional second location.
 * - Arg 4: optional WML table used as the [weapon] tag.
 * - Arg 5: optional WML table used as the [second_weapon] tag.
 * - Ret 1: boolean indicating whether the event was processed or not.
 */
int game_lua_kernel::intf_fire_event(lua_State *L, const bool by_id)
{
	char const *m = luaL_checkstring(L, 1);

	int pos = 2;
	map_location l1, l2;
	config data;

	if (luaW_tolocation(L, 2, l1)) {
		if (luaW_tolocation(L, 3, l2)) {
			pos = 4;
		} else {
			pos = 3;
		}
	}

	if (!lua_isnoneornil(L, pos)) {
		data.add_child("first", luaW_checkconfig(L, pos));
	}
	++pos;
	if (!lua_isnoneornil(L, pos)) {
		data.add_child("second", luaW_checkconfig(L, pos));
	}

	bool b = false;

	if (by_id) {
	  b = play_controller_.pump().fire("", m, l1, l2, data);
	}
	else {
	  b = play_controller_.pump().fire(m, l1, l2, data);
	}
	lua_pushboolean(L, b);
	return 1;
}


/**
 * Fires a wml menu item.
 * - Arg 1: id of the item. it is not possible to fire items that don't have ids with this function.
 * - Arg 2: optional first location.
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

	map_location l1 = luaW_checklocation(L, 2);

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
	return luaW_pushvariable(L, v) ? 1 : 0;
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
	char const *m = luaL_checkstring(L, 2);
	variable_access_const v(m, teams()[side_index].variables());
	return luaW_pushvariable(L, v) ? 1 : 0;
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
	char const *m = luaL_checkstring(L, 2);
	//TODO: maybe support removing values with an empty arg3.
	variable_access_create v(m, teams()[side_index].variables());
	luaW_checkvariable(L, v, 3);
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
	luaW_checkvariable(L, v, 2);
	return 0;
}

int game_lua_kernel::intf_set_menu_item(lua_State *L)
{
	game_state_.get_wml_menu_items().set_item(luaL_checkstring(L, 1), luaW_checkvconfig(L,2));
	return 0;
}

int game_lua_kernel::intf_clear_menu_item(lua_State *L)
{
	std::string ids(luaL_checkstring(L, 1));
	for(const std::string& id : utils::split(ids, ',', utils::STRIP_SPACES)) {
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
	game_classification &classification = play_controller_.get_classification();
	classification.end_credits = luaW_toboolean(L, 1);
	return 0;
}

int game_lua_kernel::intf_set_end_campaign_text(lua_State *L)
{
	game_classification &classification = play_controller_.get_classification();
	classification.end_text = luaW_checktstring(L, 1);
	if (lua_isnumber(L, 2)) {
		classification.end_text_duration = static_cast<int> (lua_tonumber(L, 2));
	}

	return 0;
}

int game_lua_kernel::intf_set_next_scenario(lua_State *L)
{
	WRN_LUA << "wesnoth.set_next_scenario() is deprecated" << std::endl;
	gamedata().set_next_scenario(luaL_checkstring(L, 1));
	return 0;
}

int game_lua_kernel::intf_shroud_op(lua_State *L, bool place_shroud)
{

	int side_num = luaL_checkinteger(L, 1);

	if(lua_isstring(L, 2)) {
		std::string data = lua_tostring(L, 2);
		// Special case - using a shroud_data string, or "all"
		team& side = board().get_team(side_num);
		if(place_shroud) {
			side.reshroud();
		}
		if(data != "all") {
			side.merge_shroud_map_data(data);
		} else if(!place_shroud) {
			bool was_shrouded = side.uses_shroud();
			side.set_shroud(false);
			actions::clear_shroud(side.side());
			side.set_shroud(was_shrouded);
		}
		return 0;
	} else if(lua_istable(L, 2)) {
		std::vector<map_location> locs_v = lua_check<std::vector<map_location>>(L, 2);
		std::set<map_location> locs(locs_v.begin(), locs_v.end());
		team &t = board().get_team(side_num);

		for (map_location const &loc : locs)
		{
			if (place_shroud) {
				t.place_shroud(loc);
			} else {
				t.clear_shroud(loc);
			}
		}
	} else {
		return luaL_argerror(L, 2, "expected list of locations or shroud data string");
	}

	game_display_->labels().recalculate_shroud();
	game_display_->recalculate_minimap();
	game_display_->invalidate_all();

	return 0;
}


/**
 * Highlights the given location on the map.
 * - Arg 1: location.
 */
int game_lua_kernel::intf_highlight_hex(lua_State *L)
{
	if (!game_display_) {
		return 0;
	}

	const map_location loc = luaW_checklocation(L, 1);
	if(!map().on_board(loc)) return luaL_argerror(L, 1, "not on board");
	game_display_->highlight_hex(loc);
	game_display_->display_unit_hex(loc);

	return 0;
}

/**
 * Returns whether the first side is an enemy of the second one.
 * - Args 1,2: side numbers.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_is_enemy(lua_State *L)
{
	unsigned side_1 = luaL_checkinteger(L, 1) - 1;
	unsigned side_2 = luaL_checkinteger(L, 2) - 1;
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
 * - Arg 1: map location.
 * - Ret 1: string.
 */
int game_lua_kernel::intf_get_terrain(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);

	t_translation::terrain_code const &t = board().map().
		get_terrain(loc);
	lua_pushstring(L, t_translation::write_terrain_code(t).c_str());
	return 1;
}

/**
 * Sets a terrain code.
 * - Arg 1: map location.
 * - Arg 2: terrain code string.
 * - Arg 3: layer: (overlay|base|both, default=both)
 * - Arg 4: replace_if_failed, default = no
 */
int game_lua_kernel::intf_set_terrain(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	std::string t_str(luaL_checkstring(L, 2));

	std::string mode_str = "both";
	bool replace_if_failed = false;
	if (!lua_isnone(L, 3)) {
		if (!lua_isnil(L, 3)) {
			mode_str = luaL_checkstring(L, 3);
		}

		if(!lua_isnoneornil(L, 4)) {
			replace_if_failed = luaW_toboolean(L, 4);
		}
	}

	bool result = board().change_terrain(loc, t_str, mode_str, replace_if_failed);

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
	t_translation::terrain_code t = t_translation::read_terrain_code(m);
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
	lua_push(L, info.icon_image());
	lua_setfield(L, -2, "icon");
	lua_push(L, info.editor_image());
	lua_setfield(L, -2, "editor_image");
	lua_pushinteger(L, info.light_bonus(0));
	lua_setfield(L, -2, "light");
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
 * - Arg 2: optional location
 * - Arg 3: optional boolean (consider_illuminates)
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
		for_turn = luaL_checkinteger(L, 1);
		int number_of_turns = tod_man().number_of_turns();
		if(for_turn < 1 || (number_of_turns != -1 && for_turn > number_of_turns)) {
			return luaL_argerror(L, 1, "turn number out of range");
		}
	}
	else if(lua_isnil(L, arg)) ++arg;

	if(luaW_tolocation(L, arg, loc)) {
		if(!board().map().on_board(loc)) return luaL_argerror(L, arg, "coordinates are not on board");

		if(lua_istable(L, arg)) {
			lua_rawgeti(L, arg, 3);
			consider_illuminates = luaW_toboolean(L, -1);
			lua_pop(L, 1);
		} else if(lua_isboolean(L, arg + 1)) {
			consider_illuminates = luaW_toboolean(L, arg + 1);
		}
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
 * - Arg 1: map location.
 * - Ret 1: integer.
 */
int game_lua_kernel::intf_get_village_owner(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	if (!board().map().is_village(loc))
		return 0;

	int side = board().village_owner(loc) + 1;
	if (!side) return 0;
	lua_pushinteger(L, side);
	return 1;
}

/**
 * Sets the owner of a village.
 * - Arg 1: map location.
 * - Arg 2: integer for the side or empty to remove ownership.
 */
int game_lua_kernel::intf_set_village_owner(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	int new_side = lua_isnoneornil(L, 2) ? 0 : luaL_checkinteger(L, 2);

	if (!board().map().is_village(loc))
		return 0;

	int old_side = board().village_owner(loc) + 1;

	if (new_side == old_side || new_side < 0 || new_side > static_cast<int>(teams().size()) || board().team_is_defeated(board().get_team(new_side))) {
		return 0;
	}

	if (old_side) {
		board().get_team(old_side).lose_village(loc);
	}
	if (new_side) {
		board().get_team(new_side).get_village(loc, old_side, (luaW_toboolean(L, 4) ? &gamedata() : nullptr) );
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
	lua_pushinteger(L, loc.wml_x());
	lua_pushinteger(L, loc.wml_y());
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
	lua_pushinteger(L, loc.wml_x());
	lua_pushinteger(L, loc.wml_y());
	return 2;
}

/**
 * Returns the starting position of a side.
 * Arg 1: side number
 * Ret 1: table with unnamed indices holding wml coordinates x and y
*/
int game_lua_kernel::intf_get_starting_location(lua_State* L)
{
	const int side = luaL_checkinteger(L, 1);
	if(side < 1 || static_cast<int>(teams().size()) < side)
		return luaL_argerror(L, 1, "out of bounds");
	const map_location& starting_pos = board().map().starting_position(side);
	if(!board().map().on_board(starting_pos)) return 0;

	luaW_pushlocation(L, starting_pos);
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
	return_string_attrib("next_scenario", gamedata().next_scenario());
	return_string_attrib("theme", gamedata().get_theme());
	return_bool_attrib("debug", game_config::debug);
	return_bool_attrib("debug_lua", game_config::debug_lua);
	return_bool_attrib("mp_debug", game_config::mp_debug);
	return_string_attrib("scenario_id", gamedata().get_id());
	return_vector_string_attrib("defeat_music", gamedata().get_defeat_music());
	return_vector_string_attrib("victory_music", gamedata().get_victory_music());

	const mp_game_settings& mp_settings = play_controller_.get_mp_settings();
	const game_classification & classification = play_controller_.get_classification();

	return_string_attrib("campaign_type", classification.campaign_type.to_string());
	if(classification.campaign_type==game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
		return_cfgref_attrib("mp_settings", mp_settings.to_config());
		return_cfgref_attrib("era", game_config_manager::get()->game_config().find_child("era","id",mp_settings.mp_era));
		//^ finds the era with name matching mp_era, and creates a lua reference from the config of that era.

		//This code for SigurdFD, not the cleanest implementation but seems to work just fine.
		config::const_child_itors its = game_config_manager::get()->game_config().child_range("era");
		std::string eras_list(its.front()["id"]);
		its.pop_front();
		for(const auto& cfg : its) {
			eras_list = eras_list + "," + cfg["id"];
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
	modify_string_attrib("next_scenario", gamedata().set_next_scenario(value));
	modify_string_attrib("theme",
		gamedata().set_theme(value);
		const config& game_config = game_config_manager::get()->game_config();
		game_display_->set_theme(play_controller_.get_theme(game_config, value));
	);
	modify_vector_string_attrib("defeat_music", gamedata().set_defeat_music(std::move(value)));
	modify_vector_string_attrib("victory_music", gamedata().set_victory_music(std::move(value)));
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
		cfg["id"]   = ev.id;
		if (const config &weapon = ev.data.child("first")) {
			cfg.add_child("weapon", weapon);
		}
		if (const config &weapon = ev.data.child("second")) {
			cfg.add_child("second_weapon", weapon);
		}
		if (ev.loc1.valid()) {
			cfg["x1"] = ev.loc1.filter_loc().wml_x();
			cfg["y1"] = ev.loc1.filter_loc().wml_y();
			// The position of the unit involved in this event, currently the only case where this is different from x1/y1 are enter/exit_hex events
			cfg["unit_x"] = ev.loc1.wml_x();
			cfg["unit_y"] = ev.loc1.wml_y();
		}
		if (ev.loc2.valid()) {
			cfg["x2"] = ev.loc2.filter_loc().wml_x();
			cfg["y2"] = ev.loc2.filter_loc().wml_y();
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
		help::show_help(game_display_->video(), luaL_checkstring(L, 1));
	}
	return 0;
}

int game_lua_kernel::intf_zoom(lua_State* L)
{
	if(!game_display_) {
		return 0;
	}
	double factor = luaL_checknumber(L, 1);
	bool relative = luaW_toboolean(L, 2);
	if(relative) {
		factor *= game_display_->get_zoom_factor();
	}
	// Passing true explicitly to avoid casting to int.
	// Without doing one of the two, the call is ambiguous.
	game_display_->set_zoom(factor * game_config::tile_size, true);
	lua_pushnumber(L, game_display_->get_zoom_factor());
	return 1;
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

static int impl_end_level_data_get(lua_State* L)
{
	const end_level_data& data = *static_cast<end_level_data*>(lua_touserdata(L, 1));
	const char* m = luaL_checkstring(L, 2);

	return_bool_attrib("linger_mode", data.transient.linger_mode);
	return_bool_attrib("reveal_map", data.transient.reveal_map);
	return_bool_attrib("carryover_report", data.transient.carryover_report);
	return_bool_attrib("prescenario_save", data.prescenario_save);
	return_bool_attrib("replay_save", data.replay_save);
	return_bool_attrib("proceed_to_next_level", data.proceed_to_next_level);
	return_bool_attrib("is_victory", data.is_victory);
	return_bool_attrib("is_loss", !data.is_victory);
	return_cstring_attrib("result", data.is_victory ? "victory" : "loss"); // to match wesnoth.end_level()
	return_cfg_attrib("__cfg", data.to_config_full());

	return 0;
}

namespace {
	struct end_level_committer {
		end_level_committer(end_level_data& data, play_controller& pc) : data_(data), pc_(pc) {}
		~end_level_committer() {
			pc_.set_end_level_data(data_);
		}
	private:
		end_level_data& data_;
		play_controller& pc_;
	};
}

int game_lua_kernel::impl_end_level_data_set(lua_State* L)
{
	end_level_data& data = *static_cast<end_level_data*>(lua_touserdata(L, 1));
	const char* m = luaL_checkstring(L, 2);
	end_level_committer commit(data, play_controller_);

	modify_bool_attrib("linger_mode", data.transient.linger_mode = value);
	modify_bool_attrib("reveal_map", data.transient.reveal_map = value);
	modify_bool_attrib("carryover_report", data.transient.carryover_report = value);
	modify_bool_attrib("prescenario_save", data.prescenario_save = value);
	modify_bool_attrib("replay_save", data.replay_save = value);

	return 0;
}

static int impl_end_level_data_collect(lua_State* L)
{
	end_level_data* data = static_cast<end_level_data*>(lua_touserdata(L, 1));
	(void)data; // Suppress an erroneous MSVC warning (a destructor call doesn't count as a reference)
	data->~end_level_data();
	return 0;
}

int game_lua_kernel::intf_get_end_level_data(lua_State* L)
{
	if (!play_controller_.is_regular_game_end()) {
		return 0;
	}
	auto data = play_controller_.get_end_level_data_const();
	new(L) end_level_data(data);
	if(luaL_newmetatable(L, "end level data")) {
		static luaL_Reg const callbacks[] {
			{ "__index", 	    &impl_end_level_data_get},
			{ "__newindex",     &dispatch<&game_lua_kernel::impl_end_level_data_set>},
			{ "__gc",           &impl_end_level_data_collect},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, callbacks, 0);
	}
	lua_setmetatable(L, -2);
	return 1;
}

int game_lua_kernel::intf_end_level(lua_State *L)
{
	vconfig cfg(luaW_checkvconfig(L, 1));
	end_level_data data;

	data.proceed_to_next_level = cfg["proceed_to_next_level"].to_bool(true);
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


/**
 * Finds a path between two locations.
 * - Arg 1: source location. (Or Arg 1: unit.)
 * - Arg 2: destination.
 * - Arg 3: optional cost function or
 *          table (optional fields: ignore_units, ignore_teleport, max_cost, viewing_side).
 * - Ret 1: array of pairs containing path steps.
 * - Ret 2: path cost.
 */
int game_lua_kernel::intf_find_path(lua_State *L)
{
	int arg = 1;
	map_location src, dst;
	const unit* u = nullptr;

	if (lua_isuserdata(L, arg))
	{
		u = &luaW_checkunit(L, arg);
		src = u->get_location();
		++arg;
	}
	else
	{
		src = luaW_checklocation(L, arg);
		unit_map::const_unit_iterator ui = units().find(src);
		if (ui.valid()) {
			u = ui.get_shared_ptr().get();
		}
		++arg;
	}

	dst = luaW_checklocation(L, arg);
	++arg;

	if (!board().map().on_board(src))
		return luaL_argerror(L, 1, "invalid location");
	if (!board().map().on_board(dst))
		return luaL_argerror(L, arg - 2, "invalid location");

	const gamemap &map = board().map();
	int viewing_side = 0;
	bool ignore_units = false, see_all = false, ignore_teleport = false;
	double stop_at = 10000;
	std::unique_ptr<pathfind::cost_calculator> calc;

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
		calc.reset(new lua_pathfind_cost_calculator(L, arg));
	}

	pathfind::teleport_map teleport_locations;

	if (!calc) {
		if (!u) return luaL_argerror(L, 1, "unit not found");

		const team& viewing_team = viewing_side
			? board().get_team(viewing_side)
			: board().get_team(u->side());

		if (!ignore_teleport) {
			teleport_locations = pathfind::get_teleport_locations(
				*u, viewing_team, see_all, ignore_units);
		}
		calc.reset(new pathfind::shortest_path_calculator(*u, viewing_team,
			teams(), map, ignore_units, false, see_all));
	}

	pathfind::plain_route res = pathfind::a_star_search(src, dst, stop_at, *calc, map.w(), map.h(),
		&teleport_locations);

	int nb = res.steps.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, res.steps[i].wml_x());
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, res.steps[i].wml_y());
		lua_rawseti(L, -2, 2);
		lua_rawseti(L, -2, i + 1);
	}
	lua_pushinteger(L, res.move_cost);

	return 2;
}

/**
 * Finds all the locations reachable by a unit.
 * - Arg 1: source location OR unit.
 * - Arg 2: optional table (optional fields: ignore_units, ignore_teleport, additional_turns, viewing_side).
 * - Ret 1: array of triples (coordinates + remaining movement).
 */
int game_lua_kernel::intf_find_reach(lua_State *L)
{
	int arg = 1;
	const unit* u = nullptr;

	if (lua_isuserdata(L, arg))
	{
		u = &luaW_checkunit(L, arg);
		++arg;
	}
	else
	{
		map_location src = luaW_checklocation(L, arg);
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

	const team& viewing_team = viewing_side
		? board().get_team(viewing_side)
		: board().get_team(u->side());

	pathfind::paths res(*u, ignore_units, !ignore_teleport,
		viewing_team, additional_turns, see_all, ignore_units);

	int nb = res.destinations.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		pathfind::paths::step &s = res.destinations[i];
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, s.curr.wml_x());
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, s.curr.wml_y());
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

template<typename T> // This is only a template so I can avoid typing out the long typename. >_>
static int load_fake_units(lua_State* L, int arg, T& fake_units)
{
	for (int i = 1, i_end = lua_rawlen(L, arg); i <= i_end; ++i)
	{
		map_location src;
		lua_rawgeti(L, arg, i);
		int entry = lua_gettop(L);
		if (!lua_istable(L, entry)) {
			goto error;
		}

		if (!luaW_tolocation(L, entry, src)) {
			goto error;
		}

		lua_rawgeti(L, entry, 3);
		if (!lua_isnumber(L, -1)) {
			lua_getfield(L, entry, "side");
			if (!lua_isnumber(L, -1)) {
				goto error;
			}
		}
		int side = lua_tointeger(L, -1);

		lua_rawgeti(L, entry, 4);
		if (!lua_isstring(L, -1)) {
			lua_getfield(L, entry, "type");
			if (!lua_isstring(L, -1)) {
				goto error;
			}
		}
		std::string unit_type = lua_tostring(L, -1);

		std::tuple<map_location, int, std::string> tuple(src, side, unit_type);
		fake_units.push_back(tuple);

		lua_settop(L, entry - 1);
	}
	return 0;
error:
	return luaL_argerror(L, arg, "unit type table malformed - each entry should be either array of 4 elements or table with keys x, y, side, type");
}

/**
 * Is called with one or more units and builds a cost map.
 * - Arg 1: source location. (Or Arg 1: unit. Or Arg 1: table containing a filter)
 * - Arg 2: optional array of tables with 4 elements (coordinates + side + unit type string)
 * - Arg 3: optional table (optional fields: ignore_units, ignore_teleport, viewing_side, debug).
 * - Arg 4: optional table: standard location filter.
 * - Ret 1: array of triples (coordinates + array of tuples(summed cost + reach counter)).
 */
int game_lua_kernel::intf_find_cost_map(lua_State *L)
{
	int arg = 1;
	unit* unit = luaW_tounit(L, arg, true);
	vconfig filter = vconfig::unconstructed_vconfig();
	luaW_tovconfig(L, arg, filter);

	std::vector<const ::unit*> real_units;
	typedef std::vector<std::tuple<map_location, int, std::string> > unit_type_vector;
	unit_type_vector fake_units;


	if (unit)  // 1. arg - unit
	{
		real_units.push_back(unit);
	}
	else if (!filter.null())  // 1. arg - filter
	{
		filter_context & fc = game_state_;
		boost::copy(unit_filter(filter, &fc).all_matches_on_map() | boost::adaptors::filtered(&intf_find_cost_map_helper), std::back_inserter(real_units));
	}
	else  // 1. arg - coordinates
	{
		map_location src = luaW_checklocation(L, arg);
		unit_map::const_unit_iterator ui = units().find(src);
		if (ui.valid())
		{
			real_units.push_back(&(*ui));
		}
	}
	++arg;

	if (lua_istable(L, arg))  // 2. arg - optional types
	{
		load_fake_units(L, arg, fake_units);
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
	const team& viewing_team = viewing_side
		? board().get_team(viewing_side)
		: board().teams()[0];

	pathfind::full_cost_map cost_map(
			ignore_units, !ignore_teleport, viewing_team, see_all, ignore_units);

	for (const ::unit* const u : real_units)
	{
		cost_map.add_unit(*u, use_max_moves);
	}
	for (const unit_type_vector::value_type& fu : fake_units)
	{
		const unit_type* ut = unit_types.find(std::get<2>(fu));
		cost_map.add_unit(std::get<0>(fu), ut, std::get<1>(fu));
	}

	if (debug)
	{
		if (game_display_) {
			game_display_->labels().clear_all();
			for (const map_location& loc : location_set)
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
	for (const map_location& loc : location_set)
	{
		lua_createtable(L, 4, 0);

		lua_pushinteger(L, loc.wml_x());
		lua_rawseti(L, -2, 1);

		lua_pushinteger(L, loc.wml_y());
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

	color_t color = font::LABEL_COLOR;

	if(!cfg["color"].empty()) {
		color = color_t::from_rgb_string(cfg["color"]);
	} else if(cfg.has_attribute("red") || cfg.has_attribute("green") || cfg.has_attribute("blue")) {
		color = color_t(cfg["red"], cfg["green"], cfg["blue"]);
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

void game_lua_kernel::put_unit_helper(const map_location& loc)
{
	if(game_display_) {
		game_display_->invalidate(loc);
	}

	units().erase(loc);
}

/**
 * Places a unit on the map.
 * - Arg 1: (optional) location.
 * - Arg 2: Unit (WML table or proxy), or nothing/nil to delete.
 * OR
 * - Arg 1: Unit (WML table or proxy)
 * - Arg 2: (optional) location
 * - Arg 3: (optional) boolean
 */
int game_lua_kernel::intf_put_unit(lua_State *L)
{
	if(map_locked_) {
		return luaL_error(L, "Attempted to move a unit while the map is locked");
	}
	int unit_arg = 1;

	map_location loc;
	if (lua_isnumber(L, 1)) {
		// Since this form is deprecated, I didn't bother updating it to luaW_tolocation.
		unit_arg = 3;
		loc.set_wml_x(lua_tointeger(L, 1));
		loc.set_wml_y(luaL_checkinteger(L, 2));
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 1, "invalid location");
		}
	} else if (luaW_tolocation(L, 2, loc)) {
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 2, "invalid location");
		}
	}

	if((luaW_isunit(L, unit_arg))) {
		lua_unit& u = *luaW_checkunit_ref(L, unit_arg);
		if(u.on_map() && u->get_location() == loc) {
			return 0;
		}
		if (!loc.valid()) {
			loc = u->get_location();
			if (!map().on_board(loc))
				return luaL_argerror(L, 1, "invalid location");
		} else if (unit_arg != 1) {
			WRN_LUA << "wesnoth.put_unit(x, y, unit) is deprecated. Use wesnoth.put_unit(unit, x, y) instead\n";
		}
		put_unit_helper(loc);
		u.put_map(loc);
		u.get_shared()->anim_comp().set_standing();
	} else if(!lua_isnoneornil(L, unit_arg)) {
		const vconfig* vcfg = nullptr;
		config cfg = luaW_checkconfig(L, unit_arg, vcfg);
		if (unit_arg == 1 && !map().on_board(loc)) {
			loc.set_wml_x(cfg["x"]);
			loc.set_wml_y(cfg["y"]);
			if (!map().on_board(loc))
				return luaL_argerror(L, 2, "invalid location");
		} else if (unit_arg != 1) {
			WRN_LUA << "wesnoth.put_unit(x, y, unit) is deprecated. Use wesnoth.put_unit(unit, x, y) instead\n";
		}
		unit_ptr u(new unit(cfg, true, vcfg));
		put_unit_helper(loc);
		u->set_location(loc);
		units().insert(u);
	} else {
		WRN_LUA << "wesnoth.put_unit(x, y) is deprecated. Use wesnoth.erase_unit(x, y) instead\n";
		put_unit_helper(loc);
		return 0; // Don't fire event when unit is only erase
	}

	if(unit_arg != 1 || luaW_toboolean(L, 3)) {
		play_controller_.pump().fire("unit_placed", loc);
	}
	return 0;
}

/**
 * Erases a unit from the map
 * - Arg 1: Unit to erase OR Location to erase unit
 */
int game_lua_kernel::intf_erase_unit(lua_State *L)
{
	if(map_locked_) {
		return luaL_error(L, "Attempted to remove a unit while the map is locked");
	}
	map_location loc;

	if(luaW_isunit(L, 1)) {
		lua_unit& u = *luaW_checkunit_ref(L, 1);
		if (u.on_map()) {
			loc = u->get_location();
			if (!map().on_board(loc)) {
				return luaL_argerror(L, 1, "invalid location");
			}
		} else if (int side = u.on_recall_list()) {
			team &t = board().get_team(side);
			// Should it use underlying ID instead?
			t.recall_list().erase_if_matches_id(u->id());
		} else {
			return luaL_argerror(L, 1, "can't erase private units");
		}
	} else if (luaW_tolocation(L, 1, loc)) {
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 1, "invalid location");
		}
	} else if (!lua_isnoneornil(L, 1)) {
		config cfg = luaW_checkconfig(L, 1);
		loc.set_wml_x(cfg["x"]);
		loc.set_wml_y(cfg["y"]);
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
	if(map_locked_) {
		return luaL_error(L, "Attempted to move a unit while the map is locked");
	}
	lua_unit *lu = nullptr;
	unit_ptr u = unit_ptr();
	int side = lua_tointeger(L, 2);
	if (unsigned(side) > teams().size()) side = 0;

	if(luaW_isunit(L, 1)) {
		lu = luaW_checkunit_ref(L, 1);
		u = lu->get_shared();
		if(lu->on_recall_list() && lu->on_recall_list() == side) {
			return luaL_argerror(L, 1, "unit already on recall list");
		}
	} else {
		const vconfig* vcfg = nullptr;
		config cfg = luaW_checkconfig(L, 1, vcfg);
		u = unit_ptr(new unit(cfg, true, vcfg));
	}

	if (!side) {
		side = u->side();
	} else {
		u->set_side(side);
	}
	team &t = board().get_team(side);
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
	if(map_locked_) {
		return luaL_error(L, "Attempted to remove a unit while the map is locked");
	}
	lua_unit* lu = luaW_checkunit_ref(L, 1);
	unit_ptr u = lu->get_shared();

	if (lu->on_map()) {
		u = units().extract(u->get_location());
		assert(u);
		u->anim_comp().clear_haloes();
	} else if (int side = lu->on_recall_list()) {
		team &t = board().get_team(side);
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
 * - Arg 1: location.
 * - Arg 2: optional unit for checking movement type.
 * - Rets 1,2: location.
 */
int game_lua_kernel::intf_find_vacant_tile(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);

	unit_ptr u;
	if (!lua_isnoneornil(L, 2)) {
		if(luaW_isunit(L, 2)) {
			u = luaW_checkunit_ptr(L, 2, false);
		} else {
			const vconfig* vcfg = nullptr;
			config cfg = luaW_checkconfig(L, 2, vcfg);
			u.reset(new unit(cfg, false, vcfg));
		}
	}

	map_location res = find_vacant_tile(loc, pathfind::VACANT_ANY, u.get());

	if (!res.valid()) return 0;
	lua_pushinteger(L, res.wml_x());
	lua_pushinteger(L, res.wml_y());
	return 2;
}

/**
 * Floats some text on the map.
 * - Arg 1: location.
 * - Arg 2: string.
 * - Arg 3: color.
 */
int game_lua_kernel::intf_float_label(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	color_t color = font::LABEL_COLOR;

	t_string text = luaW_checktstring(L, 2);
	if (!lua_isnoneornil(L, 3)) {
		color = color_t::from_rgb_string(luaL_checkstring(L, 3));
	}

	if (game_display_) {
		game_display_->float_label(loc, text, color);
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
	const vconfig* vcfg = nullptr;
	config cfg = luaW_checkconfig(L, 1, vcfg);
	unit_ptr u = unit_ptr(new unit(cfg, true, vcfg));
	luaW_pushunit(L, u);
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
	luaW_pushunit(L, unit_ptr(new unit(u)));
	return 1;
}

/**
 * Returns unit resistance against a given attack type.
 * - Arg 1: unit userdata.
 * - Arg 2: string containing the attack type.
 * - Arg 3: boolean indicating if attacker.
 * - Arg 4: optional location.
 * - Ret 1: integer.
 */
static int intf_unit_resistance(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	bool a = luaW_toboolean(L, 3);

	map_location loc = u.get_location();
	if (!lua_isnoneornil(L, 4)) {
		loc = luaW_checklocation(L, 4);
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
	t_translation::terrain_code t = t_translation::read_terrain_code(m);
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
	t_translation::terrain_code t = t_translation::read_terrain_code(m);
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
	t_translation::terrain_code t = t_translation::read_terrain_code(m);
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
	t_translation::terrain_code t = t_translation::read_terrain_code(m);
	lua_pushinteger(L, u.defense_modifier(t));
	return 1;
}

/**
 * Returns true if the unit has the given ability enabled.
 * - Arg 1: unit userdata.
 * - Arg 2: string.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_unit_ability(lua_State *L)
{
	const unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	lua_pushboolean(L, u.get_ability_bool(m, board()));
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
	//this is nullptr when there is no counter weapon
	if(bcustats.weapon != nullptr)
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
		def.get_location(), att_w, def_w, 0.0, nullptr, &att);

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
	lg::wml_error() << "set_music is deprecated; please use the wesnoth.playlist table instead!\n";
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
 * Gets/sets the current sound volume
 * - Arg 1: (optional) New volume to set
 * - Return: Original volume
 */
static int intf_sound_volume(lua_State* L)
{
	int vol = preferences::sound_volume();
	lua_pushnumber(L, sound::get_sound_volume() * 100.0f / vol);
	if(lua_isnumber(L, 1)) {
		float rel = lua_tonumber(L, 1);
		if(rel < 0.0f || rel > 100.0f) {
			return luaL_argerror(L, 1, "volume must be in range 0..100");
		}
		vol = static_cast<int>(rel*vol / 100.0f);
		sound::set_sound_volume(vol);
	}
	return 1;
}

/**
 * Scrolls to given tile.
 * - Arg 1: location.
 * - Arg 2: boolean preventing scroll to fog.
 * - Arg 3: boolean specifying whether to warp instantly.
 * - Arg 4: boolean specifying whether to skip if already onscreen
 */
int game_lua_kernel::intf_scroll_to_tile(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	bool check_fogged = luaW_toboolean(L, 2);
	game_display::SCROLL_TYPE scroll = luaW_toboolean(L, 4)
		? luaW_toboolean(L, 3)
			? game_display::ONSCREEN_WARP
			: game_display::ONSCREEN
		: luaW_toboolean(L, 3)
			? game_display::WARP
			: game_display::SCROLL
	;
	if (game_display_) {
		game_display_->scroll_to_tile(loc, scroll, check_fogged);
	}
	return 0;
}

int game_lua_kernel::intf_select_hex(lua_State *L)
{
	ERR_LUA << "wesnoth.select_hex is deprecated, use wesnoth.select_unit and/or wesnoth.highlight_hex" << std::endl;

	// Need this because check_location may change the stack
	// By doing this now, we ensure that it won't do so when
	// intf_select_unit and intf_highlight_hex call it.
	const map_location loc = luaW_checklocation(L, 1);
	luaW_pushlocation(L, loc);
	lua_replace(L, 1);

	intf_select_unit(L);
	if(!lua_isnoneornil(L, 2) && luaW_toboolean(L,2)) {
		intf_highlight_hex(L);
	}
	return 0;
}

/**
 * Selects and highlights the given location on the map.
 * - Arg 1: location.
 * - Args 2,3: booleans
 */
int game_lua_kernel::intf_select_unit(lua_State *L)
{
	if(lua_isnoneornil(L, 1)) {
		play_controller_.get_mouse_handler_base().select_hex(map_location::null_location(), false, false, false);
		return 0;
	}
	const map_location loc = luaW_checklocation(L, 1);
	if(!map().on_board(loc)) return luaL_argerror(L, 1, "not on board");
	bool highlight = true;
	if(!lua_isnoneornil(L, 2))
		highlight = luaW_toboolean(L, 2);
	const bool fire_event = luaW_toboolean(L, 3);
	play_controller_.get_mouse_handler_base().select_hex(
		loc, false, highlight, fire_event);
	return 0;
}

/**
 * Deselects any highlighted hex on the map.
 * No arguments or return values
 */
int game_lua_kernel::intf_deselect_hex(lua_State*)
{
	if(game_display_) {
		game_display_->highlight_hex(map_location::null_location());
	}

	return 0;
}

/**
 * Return true if a replay is in progress but the player has chosen to skip it
 */
int game_lua_kernel::intf_is_skipping_messages(lua_State *L)
{
	bool skipping = play_controller_.is_skipping_replay();
	if (!skipping) {
		skipping = game_state_.events_manager_->pump().context_skip_messages();
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
	game_state_.events_manager_->pump().context_skip_messages(skip);
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

		virtual config query_user(int side) const override
		{
			bool is_local_ai = lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).board().get_team(side).is_local_ai();
			config cfg;
			query_lua(side, is_local_ai ? ai_choice_index : user_choice_index, cfg);
			return cfg;
		}

		virtual config random_choice(int side) const override
		{
			config cfg;
			if(random_choice_index != 0 && lua_isfunction(L, random_choice_index)) {
				query_lua(side, random_choice_index, cfg);
			}
			return cfg;
		}

		virtual std::string description() const override
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
		virtual bool is_visible() const override { return false; }
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
	for (map_location const &loc : res)
	{
		lua_createtable(L, 2, 0);
		lua_pushinteger(L, loc.wml_x());
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, loc.wml_y());
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
			lua_pushinteger(L, it->wml_x());
			lua_rawseti(L, -2, 1);
			lua_pushinteger(L, it->wml_y());
			lua_rawseti(L, -2, 2);
			lua_rawseti(L, -2, i);
			++i;
		}
	}
	return 1;
}

/**
 * Matches a location against the given filter.
 * - Arg 1: location.
 * - Arg 2: WML table.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_match_location(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	vconfig filter = luaW_checkvconfig(L, 2, true);

	if (filter.null()) {
		lua_pushboolean(L, true);
		return 1;
	}

	filter_context & fc = game_state_;
	const terrain_filter t_filter(filter, &fc);
	lua_pushboolean(L, t_filter.match(loc));
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
	vconfig filter = luaW_checkvconfig(L, 2, true);

	if (filter.null()) {
		lua_pushboolean(L, true);
		return 1;
	}

	filter_context & fc = game_state_;
	side_filter s_filter(filter, &fc);

	if(team* t = luaW_toteam(L, 1)) {
		lua_pushboolean(L, s_filter.match(*t));
	} else {
		unsigned side = luaL_checkinteger(L, 1) - 1;
		if (side >= teams().size()) return 0;
		lua_pushboolean(L, s_filter.match(side + 1));
	}
	return 1;
}

int game_lua_kernel::intf_set_side_id(lua_State *L)
{
	int team_i = luaL_checkinteger(L, 1) - 1;
	std::string flag = luaL_optlstring(L, 2, "", nullptr);
	std::string color = luaL_optlstring(L, 3, "", nullptr);

	if(flag.empty() && color.empty()) {
		return 0;
	}
	if(team_i < 0 || static_cast<size_t>(team_i) >= teams().size()) {
		return luaL_error(L, "set_side_id: side number %d out of range", team_i);
	}
	team& side = teams()[team_i];

	if(!color.empty()) {
		side.set_color(color);
	}
	if(!flag.empty()) {
		side.set_flag(flag);
	}

	game_display_->reinit_flags_for_side(team_i);
	return 0;
}

static int intf_modify_ai(lua_State *L, const char* action)
{
	int side_num = luaL_checkinteger(L, 1);
	std::string path = luaL_checkstring(L, 2);
	config cfg = config_of("action", action)("path", path);
	if(strcmp(action, "delete") == 0) {
		ai::manager::modify_active_ai_for_side(side_num, cfg);
		return 0;
	}
	config component = luaW_checkconfig(L, 3);
	size_t len = std::string::npos, open_brak = path.find_last_of('[');
	size_t dot = path.find_last_of('.');
	if(open_brak != len) {
		len = open_brak - dot - 1;
	}
	cfg.add_child(path.substr(dot + 1, len), component);
	ai::manager::modify_active_ai_for_side(side_num, cfg);
	return 0;
}

static int intf_switch_ai(lua_State *L)
{
	int side_num = luaL_checkinteger(L, 1);
	std::string file = luaL_checkstring(L, 2);
	if(!ai::manager::add_ai_for_side_from_file(side_num, file)) {
		std::string err = formatter() << "Could not load AI for side " << side_num + 1 << " from file " << file;
		lua_pushlstring(L, err.c_str(), err.length());
		return lua_error(L);
	}
	return 0;
}

static int intf_append_ai(lua_State *L)
{
	int side_num = luaL_checkinteger(L, 1);
	config cfg = luaW_checkconfig(L, 2);
	if(!cfg.has_child("ai")) {
		cfg = config_of("ai", cfg);
	}
	bool added_dummy_stage = false;
	if(!cfg.child("ai").has_child("stage")) {
		added_dummy_stage = true;
		cfg.child("ai").add_child("stage", config_of("name", "empty"));
	}
	ai::configuration::expand_simplified_aspects(side_num, cfg);
	if(added_dummy_stage) {
		// TODO: Delete the dummy stage
	}
	ai::manager::append_active_ai_for_side(side_num, cfg.child("ai"));
	return 0;
}

/**
 * Returns a proxy table array for all sides matching the given SSF.
 * - Arg 1: SSF
 * - Ret 1: proxy table array
 */
int game_lua_kernel::intf_get_sides(lua_State* L)
{
	LOG_LUA << "intf_get_sides called: this = " << std::hex << this << std::dec << " myname = " << my_name() << std::endl;
	std::vector<int> sides;
	const vconfig ssf = luaW_checkvconfig(L, 1, true);
	if(ssf.null()) {
		for (unsigned side_number = 1; side_number <= teams().size(); ++side_number) {
			sides.push_back(side_number);
		}
	} else {
		filter_context & fc = game_state_;

		side_filter filter(ssf, &fc);
		sides = filter.get_teams();
	}

	lua_settop(L, 0);
	lua_createtable(L, sides.size(), 0);
	unsigned index = 1;
	for(int side : sides) {
		luaW_pushteam(L, board().get_team(side));
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
	for(const config& trait : unit_types.traits()) {
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
		lg::wml_error() << "(Lua) Modifications of type \"advance\" are deprecated, use \"advancement\" instead\n";
	}
	if (sm != "advancement" && sm != "object" && sm != "trait") {
		return luaL_argerror(L, 2, "unknown modification type");
	}
	bool write_to_mods = true;
	if (!lua_isnone(L, 4)) {
		write_to_mods = luaW_toboolean(L, 4);
	}
	if(sm.empty()) {
		write_to_mods = false;
	}

	config cfg = luaW_checkconfig(L, 3);
	u.add_modification(sm, cfg, !write_to_mods);
	return 0;
}

/**
 * Removes modifications from a unit
 * - Arg 1: unit
 * - Arg 2: table (filter as [filter_wml])
 * - Arg 3: type of modification (default "object")
 */
static int intf_remove_modifications(lua_State *L)
{
	unit& u = luaW_checkunit(L, 1);
	config filter = luaW_checkconfig(L, 2);
	std::string tag = luaL_optstring(L, 3, "object");
	//TODO
	if(filter.attribute_count() == 1 && filter.all_children_count() == 0 && filter.attribute_range().front().first == "duration") {
		u.expire_modifications(filter["duration"]);
	} else {
		for(config& obj : u.get_modifications().child_range(tag)) {
			if(obj.matches(filter)) {
				obj["duration"] = "now";
			}
		}
		u.expire_modifications("now");
	}
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
 * - Arg 1: location.
 * - Arg 2: WML table.
 */
int game_lua_kernel::intf_add_tile_overlay(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	config cfg = luaW_checkconfig(L, 2);

	if (game_display_) {
		game_display_->add_overlay(loc, cfg["image"], cfg["halo"],
			cfg["team_name"], cfg["name"], cfg["visible_in_fog"].to_bool(true));
	}
	return 0;
}

/**
 * Removes an overlay from a tile.
 * - Arg 1: location.
 * - Arg 2: optional string.
 */
int game_lua_kernel::intf_remove_tile_overlay(lua_State *L)
{
	map_location loc = luaW_checklocation(L, 1);
	char const *m = lua_tostring(L, 2);

	if (m) {
		if (game_display_) {
			game_display_->remove_single_overlay(loc, m);
		}
	} else {
		if (game_display_) {
			game_display_->remove_overlay(loc);
		}
	}
	return 0;
}

int game_lua_kernel::intf_log_replay(lua_State* L) {
	replay& recorder = play_controller_.get_replay();
	const int nargs = lua_gettop(L);
	if(nargs < 2 || nargs > 3) {
		return luaL_error(L, "Wrong number of arguments to ai.log_replay() - should be 2 or 3 arguments.");
	}
	const std::string key = nargs == 2 ? luaL_checkstring(L, 1) : luaL_checkstring(L, 2);
	config cfg;
	if(nargs == 2) {
		recorder.add_log_data(key, luaL_checkstring(L, 2));
	} else if(luaW_toconfig(L, 3, cfg)) {
		recorder.add_log_data(luaL_checkstring(L, 1), key, cfg);
	} else if(!lua_isstring(L, 3)) {
		return luaL_argerror(L, 3, "accepts only string or config");
	} else {
		recorder.add_log_data(luaL_checkstring(L, 1), key, luaL_checkstring(L, 3));
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
		CVideo::delay(10);
	} while (int(final - SDL_GetTicks()) > 0);
	return 0;
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
			for (const int side : filter.get_teams()){
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
static int intf_modify_ai_old(lua_State *L)
{
	config cfg;
	luaW_toconfig(L, 1, cfg);
	int side = cfg["side"];
	WRN_LUA << "wesnoth.modify_ai is deprecated\n";
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

	ai::component* c = ai::manager::get_active_ai_holder_for_side_dbg(side).get_component(nullptr, "");

	// Bad, but works
	std::vector<ai::component*> engines = c->get_children("engine");
	ai::engine_lua* lua_engine = nullptr;
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

	if (lua_engine == nullptr)
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

int game_lua_kernel::intf_set_time_of_day(lua_State * L)
{
	if(!game_display_) {
		return 0;
	}
	std::string area_id;
	size_t area_i = 0;
	if (lua_isstring(L, 2)) {
		area_id = lua_tostring(L, 1);
		std::vector<std::string> area_ids = tod_man().get_area_ids();
		area_i = std::find(area_ids.begin(), area_ids.end(), area_id) - area_ids.begin();
		if(area_i >= area_ids.size()) {
			return luaL_argerror(L, 1, "invalid time area ID");
		}
	}
	int is_num = false;
	int new_time = lua_tonumberx(L, 1, &is_num) - 1;
	const std::vector<time_of_day>& times = area_id.empty()
		? tod_man().times()
		: tod_man().times(area_i);
	int num_times = times.size();
	if(!is_num) {
		std::string time_id = luaL_checkstring(L, 1);
		new_time = 0;
		for(const time_of_day& time : times) {
			if(time_id == time.id) {
				break;
			}
			new_time++;
		}
		if(new_time >= num_times) {
			return luaL_argerror(L, 1, "invalid time of day ID");
		}
	}
	if(new_time < 0 || new_time >= num_times) {
		return luaL_argerror(L, 1, "invalid time of day index");
	}

	if(area_id.empty()) {
		tod_man().set_current_time(new_time);
	} else {
		tod_man().set_current_time(new_time, area_i);
	}
	return 0;
}

int game_lua_kernel::intf_scroll(lua_State * L)
{
	int x = luaL_checkinteger(L, 1), y = luaL_checkinteger(L, 2);

	if (game_display_) {
		game_display_->scroll(x, y, true);
		game_display_->draw(true, true);
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
		if (!luaW_getglobal(L, "wesnoth", "theme_items", name))
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
	lua_cpp::push_closure(L, std::bind(&game_lua_kernel::impl_theme_item, this, _1, std::string(m)), 0);
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

/**
 * Teeleports a unit to a location.
 * Arg 1: unit
 * Arg 2: target location
 * Arg 3: bool (ignore_passability)
 * Arg 4: bool (clear_shroud)
 * Arg 5: bool (animate)
 */
int game_lua_kernel::intf_teleport(lua_State *L)
{
	unit_ptr u = luaW_checkunit_ptr(L, 1, true);
	map_location dst = luaW_checklocation(L, 2);
	bool check_passability = !luaW_toboolean(L, 3);
	bool clear_shroud = luaW_toboolean(L, 4);
	bool animate = luaW_toboolean(L, 5);

	if (dst == u->get_location() || !map().on_board(dst)) {
		return 0;
	}
	const map_location vacant_dst = find_vacant_tile(dst, pathfind::VACANT_ANY, check_passability ? u.get() : nullptr);
	if (!map().on_board(vacant_dst)) {
		return 0;
	}
	// Clear the destination hex before the move (so the animation can be seen).
	actions::shroud_clearer clearer;
	if ( clear_shroud ) {
		clearer.clear_dest(vacant_dst, *u);
	}

	map_location src_loc = u->get_location();

	std::vector<map_location> teleport_path;
	teleport_path.push_back(src_loc);
	teleport_path.push_back(vacant_dst);
	unit_display::move_unit(teleport_path, u, animate);

	units().move(src_loc, vacant_dst);
	unit::clear_status_caches();

	u = &*units().find(vacant_dst);
	u->anim_comp().set_standing();

	if ( clear_shroud ) {
		// Now that the unit is visibly in position, clear the shroud.
		clearer.clear_unit(vacant_dst, *u);
	}

	if (map().is_village(vacant_dst)) {
		actions::get_village(vacant_dst, u->side());
	}

	game_display_->invalidate_unit_after_move(src_loc, vacant_dst);
	game_display_->draw();

	// Sighted events.
	clearer.fire_events();
	return 0;
}

/**
 * Removes a sound source by its ID
 * Arg 1: sound source ID
 */
int game_lua_kernel::intf_remove_sound_source(lua_State *L)
{
	soundsource::manager* man = play_controller_.get_soundsource_man();
	std::string id = luaL_checkstring(L, 1);
	man->remove(id);
	return 0;
}

/**
 * Add a new sound source
 * Arg 1: Table containing keyword arguments
 */
int game_lua_kernel::intf_add_sound_source(lua_State *L)
{
	soundsource::manager* man = play_controller_.get_soundsource_man();
	config cfg = luaW_checkconfig(L, 1);
	try {
		soundsource::sourcespec spec(cfg);
		man->add(spec);
	} catch (bad_lexical_cast &) {
		ERR_LUA << "Error when parsing sound_source config: invalid parameter." << std::endl;
		ERR_LUA << "sound_source config was: " << cfg.debug() << std::endl;
		ERR_LUA << "Skipping this sound source..." << std::endl;
	}
	return 0;
}

/**
 * Get an existing sound source
 * Arg 1: The sound source ID
 * Return: Config of sound source info, or nil if it didn't exist
 * This is a copy of the sound source info, so you need to call
 * add_sound_source again after changing it.
 */
int game_lua_kernel::intf_get_sound_source(lua_State *L)
{
	soundsource::manager* man = play_controller_.get_soundsource_man();
	std::string id = luaL_checkstring(L, 1);
	config cfg = man->get(id);
	if(cfg.empty()) {
		return 0;
	}
	// Sound sources do not know their own string ID
	// Thus, we need to add this manually
	cfg["id"] = id;
	luaW_pushconfig(L, cfg);
	return 1;
}

/**
 * Logs a message
 * Arg 1: (optional) Logger; "wml" for WML errors or deprecations
 * Arg 2: Message
 * Arg 3: Whether to print to chat (always true if arg 1 is "wml")
 */
int game_lua_kernel::intf_log(lua_State *L)
{
	const std::string& logger = lua_isstring(L, 2) ? luaL_checkstring(L, 1) : "";
	const std::string& msg = lua_isstring(L, 2) ? luaL_checkstring(L, 2) : luaL_checkstring(L, 1);

	if(logger == "wml" || logger == "WML") {
		lg::wml_error() << msg << '\n';
	} else {
		bool in_chat = luaW_toboolean(L, -1);
		game_state_.events_manager_->pump().put_wml_message(logger,msg,in_chat);
	}
	return 0;
}

int game_lua_kernel::intf_get_fog_or_shroud(lua_State *L, bool fog)
{
	int side = luaL_checknumber(L, 1);
	map_location loc = luaW_checklocation(L, 2);
	if(side < 1 || static_cast<size_t>(side) > teams().size()) {
		std::string error = "side " + std::to_string(side) + " does not exist";
		return luaL_argerror(L, 1, error.c_str());
	}

	team& t = board().get_team(side);
	lua_pushboolean(L, fog ? t.fogged(loc) : t.shrouded(loc));
	return 1;
}

/**
 * Implements the lifting and resetting of fog via WML.
 * Keeping affect_normal_fog as false causes only the fog override to be affected.
 * Otherwise, fog lifting will be implemented similar to normal sight (cannot be
 * individually reset and ends at the end of the turn), and fog resetting will, in
 * addition to removing overrides, extend the specified teams' normal fog to all
 * hexes.
 *
 * Arg 1: (optional) Side number, or list of side numbers
 * Arg 2: List of locations; each is a two-element array or a table with x and y keys
 * Arg 3: (optional) boolean
 */
int game_lua_kernel::intf_toggle_fog(lua_State *L, const bool clear)
{
	bool affect_normal_fog = false;
	if(lua_isboolean(L, -1)) {
		affect_normal_fog = luaW_toboolean(L, -1);
	}
	std::set<int> sides;
	if(lua_isnumber(L, 1)) {
		sides.insert(lua_tonumber(L, 1));
	} else if(lua_istable(L, 1) && lua_istable(L, 2)) {
		const auto& v = lua_check<std::vector<int>>(L, 1);
		sides.insert(v.begin(), v.end());
	} else {
		for(const team& t : teams()) {
			sides.insert(t.side()+1);
		}
	}
	const auto& v_locs = lua_check<std::vector<map_location>>(L, lua_istable(L, 2) ? 2 : 1);
	std::set<map_location> locs(v_locs.begin(), v_locs.end());

	for(const int &side_num : sides) {
		if(side_num < 1 || static_cast<size_t>(side_num) > teams().size()) {
			continue;
		}
		team &t = board().get_team(side_num);
		if(!clear) {
			// Extend fog.
			t.remove_fog_override(locs);
			if(affect_normal_fog) {
				t.refog();
			}
		} else if(!affect_normal_fog) {
			// Force the locations clear of fog.
			t.add_fog_override(locs);
		} else {
			// Simply clear fog from the locations.
			for(const map_location &hex : locs) {
				t.clear_fog(hex);
			}
		}
	}

	// Flag a screen update.
	game_display_->recalculate_minimap();
	game_display_->invalidate_all();
	return 0;
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

const gamemap & game_lua_kernel::map() const {
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


game_lua_kernel::game_lua_kernel(game_state & gs, play_controller & pc, reports & reports_object)
	: lua_kernel_base()
	, game_display_(nullptr)
	, game_state_(gs)
	, play_controller_(pc)
	, reports_(reports_object)
	, level_lua_()
	, queued_events_()
	, map_locked_(0)
{
	static game_events::queued_event default_queued_event("_from_lua", "", map_location(), map_location(), config());
	queued_events_.push(&default_queued_event);

	lua_State *L = mState;

	cmd_log_ << "Registering game-specific wesnoth lib functions...\n";

	// Put some callback functions in the scripting environment.
	static luaL_Reg const callbacks[] {
		{ "add_known_unit",           &intf_add_known_unit           },
		{ "add_modification",         &intf_add_modification         },
		{ "advance_unit",             &intf_advance_unit             },
		{ "copy_unit",                &intf_copy_unit                },
		{ "create_animator",          &intf_create_animator          },
		{ "create_unit",              &intf_create_unit              },
		{ "debug",                    &intf_debug                    },
		{ "debug_ai",                 &intf_debug_ai                 },
		{ "eval_conditional",         &intf_eval_conditional         },
		{ "get_era",                  &intf_get_era                  },
		{ "get_image_size",           &intf_get_image_size           },
		{ "get_time_stamp",           &intf_get_time_stamp           },
		{ "get_traits",               &intf_get_traits               },
		{ "get_viewing_side",         &intf_get_viewing_side         },
		{ "modify_ai",                &intf_modify_ai_old            },
		{ "remove_modifications",     &intf_remove_modifications     },
		{ "set_music",                &intf_set_music                },
		{ "sound_volume",             &intf_sound_volume             },
		{ "transform_unit",           &intf_transform_unit           },
		{ "unit_defense",             &intf_unit_defense             },
		{ "unit_movement_cost",       &intf_unit_movement_cost       },
		{ "unit_vision_cost",         &intf_unit_vision_cost         },
		{ "unit_jamming_cost",        &intf_unit_jamming_cost        },
		{ "unit_resistance",          &intf_unit_resistance          },
		{ "unsynced",                 &intf_do_unsynced              },
		{ "add_event_handler",         &dispatch<&game_lua_kernel::intf_add_event                  >        },
		{ "add_fog",                   &dispatch2<&game_lua_kernel::intf_toggle_fog, false         >        },
		{ "add_tile_overlay",          &dispatch<&game_lua_kernel::intf_add_tile_overlay           >        },
		{ "add_time_area",             &dispatch<&game_lua_kernel::intf_add_time_area              >        },
		{ "add_sound_source",          &dispatch<&game_lua_kernel::intf_add_sound_source           >        },
		{ "allow_end_turn",            &dispatch<&game_lua_kernel::intf_allow_end_turn             >        },
		{ "allow_undo",                &dispatch<&game_lua_kernel::intf_allow_undo                 >        },
		{ "append_ai",                 &intf_append_ai                                                      },
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
		{ "fire_event",                &dispatch2<&game_lua_kernel::intf_fire_event, false         >        },
		{ "fire_event_by_id",          &dispatch2<&game_lua_kernel::intf_fire_event, true          >        },
		{ "float_label",               &dispatch<&game_lua_kernel::intf_float_label                >        },
		{ "gamestate_inspector",       &dispatch<&game_lua_kernel::intf_gamestate_inspector        >        },
		{ "get_all_vars",              &dispatch<&game_lua_kernel::intf_get_all_vars               >        },
		{ "get_end_level_data",        &dispatch<&game_lua_kernel::intf_get_end_level_data         >        },
		{ "get_locations",             &dispatch<&game_lua_kernel::intf_get_locations              >        },
		{ "get_map_size",              &dispatch<&game_lua_kernel::intf_get_map_size               >        },
		{ "get_mouseover_tile",        &dispatch<&game_lua_kernel::intf_get_mouseover_tile         >        },
		{ "get_recall_units",          &dispatch<&game_lua_kernel::intf_get_recall_units           >        },
		{ "get_selected_tile",         &dispatch<&game_lua_kernel::intf_get_selected_tile          >        },
		{ "get_sides",                 &dispatch<&game_lua_kernel::intf_get_sides                  >        },
		{ "get_sound_source",          &dispatch<&game_lua_kernel::intf_get_sound_source           >        },
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
		{ "highlight_hex",             &dispatch<&game_lua_kernel::intf_highlight_hex              >        },
		{ "is_enemy",                  &dispatch<&game_lua_kernel::intf_is_enemy                   >        },
		{ "label",                     &dispatch<&game_lua_kernel::intf_label                      >        },
		{ "lock_view",                 &dispatch<&game_lua_kernel::intf_lock_view                  >        },
		{ "log_replay",                &dispatch<&game_lua_kernel::intf_log_replay                 >        },
		{ "log",                       &dispatch<&game_lua_kernel::intf_log                        >        },
		{ "match_location",            &dispatch<&game_lua_kernel::intf_match_location             >        },
		{ "match_side",                &dispatch<&game_lua_kernel::intf_match_side                 >        },
		{ "match_unit",                &dispatch<&game_lua_kernel::intf_match_unit                 >        },
		{ "message",                   &dispatch<&game_lua_kernel::intf_message                    >        },
		{ "open_help",                 &dispatch<&game_lua_kernel::intf_open_help                  >        },
		{ "play_sound",                &dispatch<&game_lua_kernel::intf_play_sound                 >        },
		{ "print",                     &dispatch<&game_lua_kernel::intf_print                      >        },
		{ "put_recall_unit",           &dispatch<&game_lua_kernel::intf_put_recall_unit            >        },
		{ "put_unit",                  &dispatch<&game_lua_kernel::intf_put_unit                   >        },
		{ "redraw",                    &dispatch<&game_lua_kernel::intf_redraw                     >        },
		{ "remove_event_handler",      &dispatch<&game_lua_kernel::intf_remove_event               >        },
		{ "remove_fog",                &dispatch2<&game_lua_kernel::intf_toggle_fog, true          >        },
		{ "remove_tile_overlay",       &dispatch<&game_lua_kernel::intf_remove_tile_overlay        >        },
		{ "remove_time_area",          &dispatch<&game_lua_kernel::intf_remove_time_area           >        },
		{ "remove_sound_source",       &dispatch<&game_lua_kernel::intf_remove_sound_source        >        },
		{ "replace_schedule",          &dispatch<&game_lua_kernel::intf_replace_schedule           >        },
		{ "scroll",                    &dispatch<&game_lua_kernel::intf_scroll                     >        },
		{ "scroll_to_tile",            &dispatch<&game_lua_kernel::intf_scroll_to_tile             >        },
		{ "select_hex",                &dispatch<&game_lua_kernel::intf_select_hex                 >        },
		{ "set_time_of_day",           &dispatch<&game_lua_kernel::intf_set_time_of_day            >        },
		{ "deselect_hex",              &dispatch<&game_lua_kernel::intf_deselect_hex               >        },
		{ "select_unit",               &dispatch<&game_lua_kernel::intf_select_unit                >        },
		{ "skip_messages",             &dispatch<&game_lua_kernel::intf_skip_messages              >        },
		{ "is_fogged",                 &dispatch2<&game_lua_kernel::intf_get_fog_or_shroud, true   >        },
		{ "is_shrouded",               &dispatch2<&game_lua_kernel::intf_get_fog_or_shroud, false  >        },
		{ "is_skipping_messages",      &dispatch<&game_lua_kernel::intf_is_skipping_messages       >        },
		{ "set_end_campaign_credits",  &dispatch<&game_lua_kernel::intf_set_end_campaign_credits   >        },
		{ "set_end_campaign_text",     &dispatch<&game_lua_kernel::intf_set_end_campaign_text      >        },
		{ "set_menu_item",             &dispatch<&game_lua_kernel::intf_set_menu_item              >        },
		{ "set_next_scenario",         &dispatch<&game_lua_kernel::intf_set_next_scenario          >        },
		{ "set_side_id",               &dispatch<&game_lua_kernel::intf_set_side_id                >        },
		{ "set_terrain",               &dispatch<&game_lua_kernel::intf_set_terrain                >        },
		{ "set_variable",              &dispatch<&game_lua_kernel::intf_set_variable               >        },
		{ "set_side_variable",         &dispatch<&game_lua_kernel::intf_set_side_variable          >        },
		{ "set_village_owner",         &dispatch<&game_lua_kernel::intf_set_village_owner          >        },
		{ "simulate_combat",           &dispatch<&game_lua_kernel::intf_simulate_combat            >        },
		{ "switch_ai",                 &intf_switch_ai                                                      },
		{ "synchronize_choice",        &intf_synchronize_choice                                             },
		{ "synchronize_choices",       &intf_synchronize_choices                                            },
		{ "zoom",                      &dispatch<&game_lua_kernel::intf_zoom                       >        },
		{ "teleport",                  &dispatch<&game_lua_kernel::intf_teleport                   >        },
		{ "unit_ability",              &dispatch<&game_lua_kernel::intf_unit_ability               >        },
		{ "view_locked",               &dispatch<&game_lua_kernel::intf_view_locked                >        },
		{ "place_shroud",              &dispatch2<&game_lua_kernel::intf_shroud_op, true  >                 },
		{ "remove_shroud",             &dispatch2<&game_lua_kernel::intf_shroud_op, false >                 },
		{ nullptr, nullptr }
	};
	std::vector<lua_cpp::Reg> const cpp_callbacks {
		{"add_ai_component", std::bind(intf_modify_ai, _1, "add")},
		{"delete_ai_component", std::bind(intf_modify_ai, _1, "delete")},
		{"change_ai_component", std::bind(intf_modify_ai, _1, "change")},
		{nullptr, nullptr}
	};
	lua_getglobal(L, "wesnoth");
	if (!lua_istable(L,-1)) {
		lua_newtable(L);
	}
	luaL_setfuncs(L, callbacks, 0);
	lua_cpp::set_functions(L, cpp_callbacks);

	if(play_controller_.get_classification().campaign_type == game_classification::CAMPAIGN_TYPE::TEST) {
		static luaL_Reg const test_callbacks[] {
			{ "fire_wml_menu_item",        &dispatch<&game_lua_kernel::intf_fire_wml_menu_item         >        },
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, test_callbacks , 0);
	}

	lua_setglobal(L, "wesnoth");

	// Create the getside metatable.
	cmd_log_ << lua_team::register_metatable(L);

	// Create the gettype metatable.
	cmd_log_ << lua_unit_type::register_metatable(L);

	//Create the getrace metatable
	cmd_log_ << lua_race::register_metatable(L);

	//Create the unit metatables
	cmd_log_ << lua_units::register_metatables(L);
	cmd_log_ << lua_units::register_attacks_metatables(L);

	// Create the vconfig metatable.
	cmd_log_ << lua_common::register_vconfig_metatable(L);

	// Create the unit_types table
	cmd_log_ << lua_unit_type::register_table(L);

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

	// Create the playlist table with its metatable
	cmd_log_ << lua_audio::register_table(L);

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
	set_wml_condition("have_unit", &game_events::builtin_conditions::have_unit);
	set_wml_condition("have_location", &game_events::builtin_conditions::have_location);
	set_wml_condition("variable", &game_events::builtin_conditions::variable_matches);

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
	push_locations_table(L);
	lua_setfield(L, -2, "special_locations");
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

	for(const auto& handler : game_events::wml_action::registry())
	{
		set_wml_action(handler.first, handler.second);
	}
	luaW_getglobal(L, "wesnoth", "effects");
	for(const std::string& effect : unit::builtin_effects) {
		lua_pushstring(L, effect.c_str());
		push_builtin_effect();
		lua_rawset(L, -3);
	}
	lua_settop(L, 0);
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
	lua_settop(L, 0);
	lua_getglobal(L, "wesnoth");

	lua_pushstring(L, "get_sides");
	lua_rawget(L, -2);
	lua_createtable(L, 0, 0);

	if (!protected_call(1, 1, std::bind(&lua_kernel_base::log_error, this, _1, _2))) {
		cmd_log_ << "Failed to compute wesnoth.sides\n";
	} else {
		lua_setfield(L, -2, "sides");
		cmd_log_ << "Added wesnoth.sides\n";
	}

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
	for (const config &cfg : game_lua_kernel::preload_scripts) {
		run_lua_tag(cfg);
	}
	for (const config &cfg : level_lua_.child_range("lua")) {
		run_lua_tag(cfg);
	}
}

void game_lua_kernel::set_game_display(game_display * gd) {
	game_display_ = gd;
}

/// These are the child tags of [scenario] (and the like) that are handled
/// elsewhere (in the C++ code).
/// Any child tags not in this list will be passed to Lua's on_load event.
static char const *handled_file_tags[] {
	"color_palette", "color_range", "display", "end_level_data", "era",
	"event", "generator", "label", "lua", "map", "menu_item",
	"modification", "music", "options", "side", "sound_source",
	"story", "terrain_graphics", "time", "time_area", "tunnel",
	"undo_stack", "variables"
};

static bool is_handled_file_tag(const std::string &s)
{
	for (char const *t : handled_file_tags) {
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

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_load"))
		return;

	lua_newtable(L);
	int k = 1;
	for (const config::any_child &v : level.all_children_range())
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

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_save"))
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

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_event"))
		return false;

	queued_event_context dummy(&ev, queued_events_);
	lua_pushstring(L, ev.name.c_str());
	luaW_pcall(L, 1, 0, false);
	return true;
}

/**
 * Applies its upvalue as an effect
 * Arg 1: The unit to apply to
 * Arg 3: The [effect] tag contents
 * Arg 3: If false, only build description
 * Return: The description of the effect
 */
int game_lua_kernel::cfun_builtin_effect(lua_State *L)
{
	std::string which_effect = lua_tostring(L, lua_upvalueindex(1));
	bool need_apply = luaW_toboolean(L, lua_upvalueindex(2));
	// Argument 1 is the implicit "self" argument, which isn't needed here
	lua_unit u(luaW_checkunit(L, 2));
	config cfg = luaW_checkconfig(L, 3);

	// The times= key is supposed to be ignored by the effect function.
	// However, just in case someone doesn't realize this, we will set it to 1 here.
	cfg["times"] = 1;

	if(need_apply) {
		u.get()->apply_builtin_effect(which_effect, cfg);
		return 0;
	} else {
		std::string description = u.get()->describe_builtin_effect(which_effect, cfg);
		lua_pushstring(L, description.c_str());
		return 1;
	}
}

/**
* Registers a function for use as an effect handler.
*/
void game_lua_kernel::push_builtin_effect()
{
	lua_State *L = mState;

	// The effect name is at the top of the stack
	int str_i = lua_gettop(L);
	lua_newtable(L); // The functor table
	lua_newtable(L); // The functor metatable
	lua_pushstring(L, "__call");
	lua_pushvalue(L, str_i);
	lua_pushboolean(L, true);
	lua_pushcclosure(L, &dispatch<&game_lua_kernel::cfun_builtin_effect>, 2);
	lua_rawset(L, -3); // Set the call metafunction
	lua_pushstring(L, "__descr");
	lua_pushvalue(L, str_i);
	lua_pushboolean(L, false);
	lua_pushcclosure(L, &dispatch<&game_lua_kernel::cfun_builtin_effect>, 2);
	lua_rawset(L, -3); // Set the descr "metafunction"
	lua_setmetatable(L, -2); // Apply the metatable to the functor table
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
void game_lua_kernel::set_wml_action(const std::string& cmd, game_events::wml_action::handler h)
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

using wml_conditional_handler = bool(*)(const vconfig&);

/**
 * Executes its upvalue as a wml condition and returns the result.
 */
static int cfun_wml_condition(lua_State *L)
{
	wml_conditional_handler h = reinterpret_cast<wml_conditional_handler>
		(lua_touserdata(L, lua_upvalueindex(1)));

	vconfig vcfg = luaW_checkvconfig(L, 1);
	lua_pushboolean(L, h(vcfg));
	return 1;
}

/**
 * Registers a function for use as a conditional handler.
 */
void game_lua_kernel::set_wml_condition(const std::string& cmd, wml_conditional_handler h)
{
	lua_State *L = mState;

	lua_getglobal(L, "wesnoth");
	lua_pushstring(L, "wml_conditionals");
	lua_rawget(L, -2);
	lua_pushstring(L, cmd.c_str());
	lua_pushlightuserdata(L, reinterpret_cast<void *>(h));
	lua_pushcclosure(L, &cfun_wml_condition, 1);
	lua_rawset(L, -3);
	lua_pop(L, 2);
}

/**
 * Runs a command from an event handler.
 * @return true if there is a handler for the command.
 * @note @a cfg should be either volatile or long-lived since the Lua
 *       code may grab it for an arbitrary long time.
 */
bool game_lua_kernel::run_wml_action(const std::string& cmd, vconfig const &cfg,
	game_events::queued_event const &ev)
{
	lua_State *L = mState;


	if (!luaW_getglobal(L, "wesnoth", "wml_actions", cmd))
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
bool game_lua_kernel::run_wml_conditional(const std::string& cmd, vconfig const &cfg)
{
	lua_State *L = mState;


	if (!luaW_getglobal(L, "wesnoth", "wml_conditionals", cmd)) {
		lg::wml_error() << "unknown conditional wml: [" << cmd << "]\n";
		return true;
	}

	luaW_pushvconfig(L, cfg);
	luaW_pcall(L, 1, 1, true);

	bool b = luaW_toboolean(L, -1);
	lua_pop(L, 1);
	return b;
}


/**
* Runs a script from a location filter.
* The script is an already compiled function given by its name.
*/
bool game_lua_kernel::run_filter(char const *name, const map_location& l)
{
	lua_pushinteger(mState, l.wml_x());
	lua_pushinteger(mState, l.wml_y());
	return run_filter(name, 2);
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
	// Pass the unit as argument.
	luaW_pushunit(L, ui->underlying_id());

	return run_filter(name, 1);
}
/**
* Runs a script from a filter.
* The script is an already compiled function given by its name.
*/
bool game_lua_kernel::run_filter(char const *name, int nArgs)
{
	map_locker(this);
	lua_State *L = mState;
	// Get the user filter by name.
	const std::vector<std::string>& path = utils::split(name, '.', utils::STRIP_SPACES);
	if (!luaW_getglobal(L, path))
	{
		std::string message = std::string() + "function " + name + " not found";
		log_error(message.c_str(), "Lua SUF Error");
		//we pushed nothing and can safeley return.
		return false;
	}
	lua_insert(L, -nArgs - 1);

	if (!luaW_pcall(L, nArgs, 1)) return false;

	bool b = luaW_toboolean(L, -1);
	lua_pop(L, 1);
	return b;
}

std::string game_lua_kernel::apply_effect(const std::string& name, unit& u, const config& cfg, bool need_apply)
{
	lua_State *L = mState;
	int top = lua_gettop(L);
	std::string descr;
	// Stack: nothing
	lua_unit* lu = luaW_pushlocalunit(L, u);
	// Stack: unit
	// (Note: The unit needs to be on the stack twice to prevent untimely GC.)
	luaW_pushconfig(L, cfg);
	// Stack: unit, cfg
	if(luaW_getglobal(L, "wesnoth", "effects", name)) {
		map_locker(this);
		// Stack: unit, cfg, effect
		if(lua_istable(L, -1)) {
			// Effect is implemented by a table with __call and __descr
			if(need_apply) {
				lua_pushvalue(L, -1);
				// Stack: unit, cfg, effect, effect
				lua_pushvalue(L, top + 1);
				// Stack: unit, cfg, effect, effect, unit
				lua_pushvalue(L, top + 2);
				// Stack: unit, cfg, effect, effect, unit, cfg
				luaW_pcall(L, 2, 0);
				// Stack: unit, cfg, effect
			}
			if(luaL_getmetafield(L, -1, "__descr")) {
				// Stack: unit, cfg, effect, __descr
				if(lua_isstring(L, -1)) {
					// __descr was a static string
					descr = lua_tostring(L, -1);
				} else {
					lua_pushvalue(L, -2);
					// Stack: unit, cfg, effect, __descr, effect
					lua_pushvalue(L, top + 1);
					// Stack: unit, cfg, effect, __descr, effect, unit
					lua_pushvalue(L, top + 2);
					// Stack: unit, cfg, effect, __descr, effect, unit, cfg
					luaW_pcall(L, 3, 1);
					if(lua_isstring(L, -1) && !lua_isnumber(L, -1)) {
						descr = lua_tostring(L, -1);
					} else {
						ERR_LUA << "Effect __descr metafunction should have returned a string, but instead returned ";
						if(lua_isnone(L, -1)) {
							ERR_LUA << "nothing";
						} else {
							ERR_LUA << lua_typename(L, lua_type(L, -1));
						}
					}
				}
			}
		} else if(need_apply) {
			// Effect is assumed to be a simple function; no description is provided
			lua_pushvalue(L, top + 1);
			// Stack: unit, cfg, effect, unit
			lua_pushvalue(L, top + 2);
			// Stack: unit, cfg, effect, unit, cfg
			luaW_pcall(L, 2, 0);
			// Stack: unit, cfg
		}
	}
	lua_settop(L, top);
	lu->clear_ref();
	return descr;
}

ai::lua_ai_context* game_lua_kernel::create_lua_ai_context(char const *code, ai::engine_lua *engine)
{
	return ai::lua_ai_context::create(mState,code,engine);
}

ai::lua_ai_action_handler* game_lua_kernel::create_lua_ai_action_handler(char const *code, ai::lua_ai_context &context)
{
	return ai::lua_ai_action_handler::create(mState,code,context);
}

void game_lua_kernel::mouse_over_hex_callback(const map_location& loc)
{
	lua_State *L = mState;

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_mouse_move")) {
		return;
	}
	lua_push(L, loc.wml_x());
	lua_push(L, loc.wml_y());
	luaW_pcall(L, 2, 0, false);
	return;
}

void game_lua_kernel::select_hex_callback(const map_location& loc)
{
	lua_State *L = mState;

	if (!luaW_getglobal(L, "wesnoth", "game_events", "on_mouse_action")) {
		return;
	}
	lua_push(L, loc.wml_x());
	lua_push(L, loc.wml_y());
	luaW_pcall(L, 2, 0, false);
	return;
}
