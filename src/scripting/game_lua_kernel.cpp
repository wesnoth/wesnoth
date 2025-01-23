/*
	Copyright (C) 2009 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "actions/advancement.hpp"      // for advance_unit_at, etc
#include "actions/move.hpp"             // for clear_shroud
#include "actions/vision.hpp"           // for clear_shroud and create_jamming_map
#include "actions/undo.hpp"             // for clear_shroud and create_jamming_map
#include "actions/undo_action.hpp"      // for clear_shroud and create_jamming_map
#include "ai/composite/ai.hpp"          // for ai_composite
#include "ai/composite/component.hpp"   // for component, etc
#include "ai/composite/contexts.hpp"    // for ai_context
#include "ai/lua/engine_lua.hpp"        // for engine_lua
#include "ai/composite/rca.hpp"         // for candidate_action
#include "ai/composite/stage.hpp"       // for stage
#include "ai/configuration.hpp"         // for configuration
#include "ai/lua/core.hpp"              // for lua_ai_context, etc
#include "ai/manager.hpp"               // for manager, holder
#include "attack_prediction.hpp"        // for combatant
#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"                   // for config, etc
#include "display_chat_manager.hpp"     // for clear_chat_messages
#include "floating_label.hpp"
#include "formatter.hpp"
#include "game_board.hpp"               // for game_board
#include "game_classification.hpp"      // for game_classification, etc
#include "game_config.hpp"              // for debug, base_income, etc
#include "game_config_manager.hpp"      // for game_config_manager
#include "game_data.hpp"                // for game_data, etc
#include "game_display.hpp"             // for game_display
#include "game_errors.hpp"              // for game_error
#include "game_events/conditional_wml.hpp"  // for conditional_passed
#include "game_events/entity_location.hpp"
#include "game_events/handlers.hpp"
#include "game_events/manager_impl.hpp" // for pending_event_handler
#include "game_events/pump.hpp"         // for queued_event
#include "preferences/preferences.hpp"  // for encountered_units
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "map/map.hpp"                  // for gamemap
#include "map/label.hpp"
#include "map/location.hpp"             // for map_location
#include "mouse_events.hpp"             // for mouse_handler
#include "mp_game_settings.hpp"         // for mp_game_settings
#include "overlay.hpp"
#include "pathfind/pathfind.hpp"        // for full_cost_map, plain_route, etc
#include "pathfind/teleport.hpp"        // for get_teleport_locations, etc
#include "play_controller.hpp"          // for play_controller
#include "preferences/preferences.hpp"
#include "recall_list_manager.hpp"      // for recall_list_manager
#include "replay.hpp"                   // for get_user_choice, etc
#include "reports.hpp"                  // for register_generator, etc
#include "resources.hpp"                // for whiteboard
#include "scripting/lua_attributes.hpp"
#include "scripting/lua_audio.hpp"
#include "scripting/lua_unit.hpp"
#include "scripting/lua_unit_attacks.hpp"
#include "scripting/lua_common.hpp"
#include "scripting/lua_cpp_function.hpp"
#include "scripting/lua_gui2.hpp"	    // for show_gamestate_inspector
#include "scripting/lua_pathfind_cost_calculator.hpp"
#include "scripting/lua_race.hpp"
#include "scripting/lua_team.hpp"
#include "scripting/lua_terrainmap.hpp"
#include "scripting/lua_unit_type.hpp"
#include "scripting/push_check.hpp"
#include "synced_commands.hpp"
#include "color.hpp"                    // for surface
#include "side_filter.hpp"              // for side_filter
#include "sound.hpp"                    // for commit_music_changes, etc
#include "synced_context.hpp"           // for synced_context, etc
#include "synced_user_choice.hpp"
#include "team.hpp"                     // for team, village_owner
#include "terrain/terrain.hpp"          // for terrain_type
#include "terrain/filter.hpp"           // for terrain_filter
#include "terrain/translation.hpp"      // for read_terrain_code, etc
#include "time_of_day.hpp"              // for time_of_day
#include "tod_manager.hpp"              // for tod_manager
#include "tstring.hpp"                  // for t_string, operator+
#include "units/unit.hpp"                 // for unit
#include "units/animation_component.hpp"  // for unit_animation_component
#include "units/udisplay.hpp"
#include "units/filter.hpp"
#include "units/map.hpp"                // for unit_map, etc
#include "units/ptr.hpp"                // for unit_const_ptr, unit_ptr
#include "units/types.hpp"              // for unit_type_data, unit_types, etc
#include "utils/scope_exit.hpp"
#include "variable.hpp"                 // for vconfig, etc
#include "variable_info.hpp"
#include "video.hpp"                    // only for faked
#include "whiteboard/manager.hpp"       // for whiteboard
#include "deprecation.hpp"

#include <functional>                   // for bind_t, bind
#include <array>
#include <cassert>                      // for assert
#include <cstring>                      // for strcmp
#include <iterator>                     // for distance, advance
#include <map>                          // for map, map<>::value_type, etc
#include <new>                          // for operator new
#include <set>                          // for set
#include <sstream>                      // for operator<<, basic_ostream, etc
#include <thread>
#include <utility>                      // for pair
#include <algorithm>
#include <vector>                       // for vector, etc

#ifdef DEBUG_LUA
#include "scripting/debug_lua.hpp"
#endif

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)

static lg::log_domain log_wml("wml");
#define ERR_WML LOG_STREAM(err, log_wml)

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


void game_lua_kernel::extract_preload_scripts(const game_config_view& game_config)
{
	game_lua_kernel::preload_scripts.clear();
	for (const config& cfg : game_config.child_range("lua")) {
		game_lua_kernel::preload_scripts.push_back(cfg);
	}
	game_lua_kernel::preload_config = game_config.mandatory_child("game_config");
}

void game_lua_kernel::log_error(char const * msg, char const * context)
{
	lua_kernel_base::log_error(msg, context);
	lua_chat(context, msg);
}

void game_lua_kernel::lua_chat(const std::string& caption, const std::string& msg)
{
	if (game_display_) {
		game_display_->get_chat_manager().add_chat_message(std::time(nullptr), caption, 0, msg,
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
		if(!sides.empty()) { WRN_LUA << "ignoring duplicate side filter information (inline side=)"; }
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
 * Gets currently viewing side.
 * - Ret 1: integer specifying the currently viewing side
 * - Ret 2: Bool whether the vision is not limited to that team, this can for example be true during replays.
 */
static int intf_get_viewing_side(lua_State *L)
{
	if(const display* disp = display::get_singleton()) {
		lua_pushinteger(L, disp->viewing_team().side());
		lua_pushboolean(L, disp->show_everything());
		return 2;
	}
	else {
		return 0;
	}
}

static int intf_handle_user_interact(lua_State *)
{
	ai::manager::get_singleton().raise_user_interact();
	return 0;
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
	unit_ptr up = luaW_checkunit_ptr(L, 2, false);
	unit& u = *up;
	std::string which = luaL_checkstring(L, 3);

	std::string hits_str = luaL_checkstring(L, 4);
	strike_result::type hits = strike_result::get_enum(hits_str).value_or(strike_result::type::invalid);

	map_location dest;
	int v1 = 0, v2 = 0;
	bool bars = false;
	t_string text;
	color_t color{255, 255, 255};
	const_attack_ptr primary, secondary;

	if(lua_istable(L, 5)) {
		lua_getfield(L, 5, "target");
		if(luaW_tolocation(L, -1, dest)) {
			if(dest == u.get_location()) {
				return luaL_argerror(L, 5, "target location must be different from animated unit's location");
			} else if(!tiles_adjacent(dest, u.get_location())) {
				return luaL_argerror(L, 5, "target location must be adjacent to the animated unit");
			}
		} else {
			// luaW_tolocation may set the location to (0,0) if it fails
			dest = map_location();
			if(!lua_isnoneornil(L, -1)) {
				return luaW_type_error(L, 5, "target", "location table");
			}
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "value");
		if(lua_isnumber(L, -1)) {
			v1 = lua_tointeger(L, -1);
		} else if(lua_istable(L, -1)) {
			lua_rawgeti(L, -1, 1);
			v1 = lua_tointeger(L, -1);
			lua_pop(L, 1);
			lua_rawgeti(L, -1, 2);
			v2 = lua_tointeger(L, -1);
			lua_pop(L, 1);
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, 5, "value", "number or array of two numbers");
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "with_bars");
		if(lua_isboolean(L, -1)) {
			bars = luaW_toboolean(L, -1);
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, 5, "with_bars", lua_typename(L, LUA_TBOOLEAN));
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "text");
		if(lua_isstring(L, -1)) {
			text = lua_tostring(L, -1);
		} else if(luaW_totstring(L, -1, text)) {
			// Do nothing; luaW_totstring already assigned the value
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, 5, "text", lua_typename(L, LUA_TSTRING));
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "color");
		if(lua_istable(L, -1) && lua_rawlen(L, -1) == 3) {
			int idx = lua_absindex(L, -1);
			lua_rawgeti(L, idx, 1); // red @ -3
			lua_rawgeti(L, idx, 2); // green @ -2
			lua_rawgeti(L, idx, 3); // blue @ -1
			color = color_t(lua_tointeger(L, -3), lua_tointeger(L, -2), lua_tointeger(L, -1));
			lua_pop(L, 3);
		} else if(!lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, 5, "color", "array of three numbers");
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "primary");
		primary = luaW_toweapon(L, -1);
		if(!primary && !lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, 5, "primary", "weapon");
		}
		lua_pop(L, 1);

		lua_getfield(L, 5, "secondary");
		secondary = luaW_toweapon(L, -1);
		if(!secondary && !lua_isnoneornil(L, -1)) {
			return luaW_type_error(L, 5, "secondary", "weapon");
		}
		lua_pop(L, 1);
	} else if(!lua_isnoneornil(L, 5)) {
		return luaW_type_error(L, 5, "table of options");
	}

	anim.add_animation(up, which, u.get_location(), dest, v1, bars, text, color, hits, primary, secondary, v2);
	return 0;
}

int game_lua_kernel::impl_run_animation(lua_State* L)
{
	if(video::headless() || resources::controller->is_skipping_replay()) {
		return 0;
	}
	events::command_disabler command_disabler;
	unit_animator& anim = *static_cast<unit_animator*>(luaL_checkudata(L, 1, animatorKey));
	play_controller_.play_slice();
	anim.start_animations();
	anim.wait_for_end();
	anim.set_all_standing();
	anim.clear();
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

int game_lua_kernel::intf_create_animator(lua_State* L)
{
	new(L) unit_animator;
	if(luaL_newmetatable(L, animatorKey)) {
		luaL_Reg metafuncs[] {
			{"__gc", impl_animator_collect},
			{"__index", impl_animator_get},
			{"add", impl_add_animation},
			{"run", &dispatch<&game_lua_kernel::impl_run_animation>},
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
		vconfig cfg = vconfig::unconstructed_vconfig();
		std::string name;
		if(luaW_tovconfig(L, 1, cfg)) {
			name = cfg["name"].str();
			deprecated_message("gui.show_inspector(cfg)", DEP_LEVEL::INDEFINITE, {1, 19, 0}, "Instead of {name = 'title' }, pass just 'title'.");
		} else {
			name = luaL_optstring(L, 1, "");
		}
		return lua_gui2::show_gamestate_inspector(name, gamedata(), game_state_);
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
		game_display_->viewing_team(),
		game_display_->show_everything());
	if (!ui.valid()) return 0;

	luaW_pushunit(L, ui->underlying_id());
	return 1;
}

/**
 * Gets all the units matching a given filter.
 * - Arg 1: optional table containing a filter
 * - Arg 2: optional location (to find all units that would match on that location)
 *          OR unit (to find all units that would match adjacent to that unit)
 * - Ret 1: table containing full userdata with __index pointing to
 *          impl_unit_get and __newindex pointing to impl_unit_set.
 */
int game_lua_kernel::intf_get_units(lua_State *L)
{
	vconfig filter = luaW_checkvconfig(L, 1, true);
	unit_filter filt(filter);
	std::vector<const unit*> units;

	if(unit* u_adj = luaW_tounit(L, 2)) {
		if(!u_adj) {
			return luaL_argerror(L, 2, "unit not found");
		}
		units = filt.all_matches_with_unit(*u_adj);
	} else if(!lua_isnoneornil(L, 2)) {
		map_location loc;
		luaW_tolocation(L, 2, loc);
		if(!loc.valid()) {
			return luaL_argerror(L, 2, "invalid location");
		}
		units = filt.all_matches_at(loc);
	} else {
		units = filt.all_matches_on_map();
	}

	// Go through all the units while keeping the following stack:
	// 1: return table, 2: userdata
	lua_settop(L, 0);
	lua_newtable(L);
	int i = 1;

	for (const unit * ui : units) {
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

	if(unit* u_adj = luaW_tounit(L, 3)) {
		if(int side = u.on_recall_list()) {
			WRN_LUA << "wesnoth.units.matches called with a secondary unit (3rd argument), ";
			WRN_LUA << "but unit to match was on recall list. ";
			WRN_LUA << "Thus the 3rd argument is ignored.";
			team &t = board().get_team(side);
			scoped_recall_unit auto_store("this_unit", t.save_id_or_number(), t.recall_list().find_index(u->id()));
			lua_pushboolean(L, unit_filter(filter).matches(*u, map_location()));
			return 1;
		}
		if (!u_adj) {
			return luaL_argerror(L, 3, "unit not found");
		}
		lua_pushboolean(L, unit_filter(filter).matches(*u, *u_adj));
	} else if(int side = u.on_recall_list()) {
		map_location loc;
		luaW_tolocation(L, 3, loc); // If argument 3 isn't a location, loc is unchanged
		team &t = board().get_team(side);
		scoped_recall_unit auto_store("this_unit", t.save_id_or_number(), t.recall_list().find_index(u->id()));
		lua_pushboolean(L, unit_filter(filter).matches(*u, loc));
		return 1;
	} else {
		map_location loc = u->get_location();
		luaW_tolocation(L, 3, loc); // If argument 3 isn't a location, loc is unchanged
		lua_pushboolean(L, unit_filter(filter).matches(*u, loc));
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
	const unit_filter ufilt(filter);
	for (team &t : teams())
	{
		for (unit_ptr & u : t.recall_list())
		{
			if (!filter.null()) {
				scoped_recall_unit auto_store("this_unit",
					t.save_id_or_number(), t.recall_list().find_index(u->id()));
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
 * - Arg 4: optional WML table used used as the event data
 * Typically this contains [first] as the [weapon] tag and [second] as the [second_weapon] tag.
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

	luaW_toconfig(L, pos, data);

	// Support WML names for some common data
	if(data.has_child("primary_attack")) {
		data.add_child("first", data.mandatory_child("primary_attack"));
		data.remove_children("primary_attack");
	}
	if(data.has_child("secondary_attack")) {
		data.add_child("second", data.mandatory_child("secondary_attack"));
		data.remove_children("secondary_attack");
	}

	bool b = false;

	if (by_id) {
	  b = std::get<0>(play_controller_.pump().fire("", m, l1, l2, data));
	}
	else {
	  b = std::get<0>(play_controller_.pump().fire(m, l1, l2, data));
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


int game_lua_kernel::intf_create_side(lua_State *L)
{
	config cfg = luaW_checkconfig(L, 1);
	cfg["side"] = teams().size() + 1;
	game_state_.add_side_wml(cfg);
	lua_pushinteger(L, teams().size());

	return 1;
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
			WRN_LUA << "[clear_menu_item] has been given an empty id=, ignoring";
			continue;
		}
		game_state_.get_wml_menu_items().erase(id);
	}
	return 0;
}

/**
 * Toggle shroud on some locations
 * Arg 1: Side number
 * Arg 2: List of locations on which to place/remove shroud
 */
int game_lua_kernel::intf_toggle_shroud(lua_State *L, bool place_shroud)
{
	team& t = luaW_checkteam(L, 1, board());

	if(lua_istable(L, 2)) {
		std::set<map_location> locs = luaW_check_locationset(L, 2);

		for (const map_location& loc : locs)
		{
			if (place_shroud) {
				t.place_shroud(loc);
			} else {
				t.clear_shroud(loc);
			}
		}
	} else {
		return luaL_argerror(L, 2, "expected list of locations");
	}

	game_display_->labels().recalculate_shroud();
	game_display_->recalculate_minimap();
	game_display_->invalidate_all();

	return 0;
}

/**
 * Overrides the shroud entirely. All locations are shrouded, except for the ones passed in as argument 2.
 * Arg 1: Side number
 * Arg 2: List of locations that should be unshrouded
 */
int game_lua_kernel::intf_override_shroud(lua_State *L)
{
	team& t = luaW_checkteam(L, 1, board());

	if(lua_istable(L, 2)) {
		std::set<map_location> locs = luaW_check_locationset(L, 2);
		t.reshroud();
		for(const map_location& loc : locs) {
			t.clear_shroud(loc);
		}
	} else {
		return luaW_type_error(L, 2, "list of locations");
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
	unsigned side_1, side_2;
	if(team* t = luaW_toteam(L, 1)) {
		side_1 = t->side();
	} else {
		side_1 = luaL_checkinteger(L, 1);
	}
	if(team* t = luaW_toteam(L, 2)) {
		side_2 = t->side();
	} else {
		side_2 = luaL_checkinteger(L, 2);
	}
	if (side_1 > teams().size() || side_2 > teams().size()) return 0;
	lua_pushboolean(L, board().get_team(side_1).is_enemy(side_2));
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

static void luaW_push_tod(lua_State* L, const time_of_day& tod)
{
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
	lua_pushstring(L, tod.sounds.c_str());
	lua_setfield(L, -2, "sound");
	lua_pushstring(L, tod.image_mask.c_str());
	lua_setfield(L, -2, "mask");

	lua_pushinteger(L, tod.color.r);
	lua_setfield(L, -2, "red");
	lua_pushinteger(L, tod.color.g);
	lua_setfield(L, -2, "green");
	lua_pushinteger(L, tod.color.b);
	lua_setfield(L, -2, "blue");
}

// A schedule object is an index with a special metatable.
// The global schedule uses index -1
void game_lua_kernel::luaW_push_schedule(lua_State* L, int area_index)
{
	lua_newuserdatauv(L, 0, 1);
	lua_pushinteger(L, area_index);
	lua_setiuservalue(L, -2, 1);
	if(luaL_newmetatable(L, "schedule")) {
		static luaL_Reg const schedule_meta[] {
			{"__index", &dispatch<&game_lua_kernel::impl_schedule_get>},
			{"__newindex", &dispatch<&game_lua_kernel::impl_schedule_set>},
			{"__dir", &dispatch<&game_lua_kernel::impl_schedule_dir>},
			{"__len", &dispatch<&game_lua_kernel::impl_schedule_len>},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, schedule_meta, 0);
	}
	lua_setmetatable(L, -2);
}

static int luaW_check_schedule(lua_State* L, int idx)
{
	int save_top = lua_gettop(L);
	luaL_checkudata(L, idx, "schedule");
	lua_getiuservalue(L, idx, 1);
	int i = luaL_checkinteger(L, -1);
	lua_settop(L, save_top);
	return i;
}

struct schedule_tag {
	game_lua_kernel& ref;
	int area_index;
	schedule_tag(game_lua_kernel& k) : ref(k) {}
	auto& tod_man() const { return ref.tod_man(); }
};
#define SCHEDULE_GETTER(name, type) LATTR_GETTER(name, type, schedule_tag, sched)
#define SCHEDULE_SETTER(name, type) LATTR_SETTER(name, type, schedule_tag, sched)
#define SCHEDULE_VALID(name) LATTR_VALID(name, schedule_tag, sched)
luaW_Registry scheduleReg{"schedule"};

template<> struct lua_object_traits<schedule_tag> {
	inline static auto metatable = "schedule";
	inline static schedule_tag get(lua_State* L, int n) {
		schedule_tag sched{lua_kernel_base::get_lua_kernel<game_lua_kernel>(L)};
		sched.area_index = luaW_check_schedule(L, n);
		return sched;
	}
};

int game_lua_kernel::impl_schedule_get(lua_State *L)
{
	int area_index = luaW_check_schedule(L, 1);
	if(lua_isnumber(L, 2)) {
		const auto& times = area_index < 0 ? tod_man().times() : tod_man().times(area_index);
		int i = lua_tointeger(L, 2) - 1;
		if(i < 0 || i >= static_cast<int>(times.size())) {
			return luaL_argerror(L, 2, "invalid time of day index");
		}
		luaW_push_tod(L, times[i]);
		return 1;
	}
	return scheduleReg.get(L);
}

int game_lua_kernel::impl_schedule_len(lua_State *L)
{
	int area_index = luaW_check_schedule(L, 1);
	const auto& times = area_index < 0 ? tod_man().times() : tod_man().times(area_index);
	lua_pushinteger(L, times.size());
	return 1;
}

int game_lua_kernel::impl_schedule_set(lua_State *L)
{
	int area_index = luaW_check_schedule(L, 1);
	if(lua_isnumber(L, 2)) {
		std::vector<time_of_day> times = area_index < 0 ? tod_man().times() : tod_man().times(area_index);
		int i = lua_tointeger(L, 2) - 1;
		if(i < 0 || i >= static_cast<int>(times.size())) {
			return luaL_argerror(L, 2, "invalid time of day index");
		}
		config time_cfg = luaW_checkconfig(L, 3);
		times[i] = time_of_day(time_cfg);
		if(area_index < 0) {
			tod_man().replace_schedule(times);
		} else {
			tod_man().replace_local_schedule(times, area_index);
		}
	}
	return scheduleReg.set(L);
}

int game_lua_kernel::impl_schedule_dir(lua_State *L) {
	return scheduleReg.dir(L);
}

namespace {
SCHEDULE_GETTER("time_of_day", std::string) {
	if(sched.area_index >= 0) {
		return sched.tod_man().get_area_time_of_day(sched.area_index).id;
	}
	return sched.tod_man().get_time_of_day().id;
}

SCHEDULE_SETTER("time_of_day", std::string) {
	const auto& times = sched.area_index < 0 ? sched.tod_man().times() : sched.tod_man().times(sched.area_index);
	auto iter = std::find_if(times.begin(), times.end(), [&value](const time_of_day& tod) {
		return tod.id == value;
	});
	if(iter == times.end()) {
		std::ostringstream err;
		err << "invalid time of day ID for ";
		if(sched.area_index < 0) {
			err << "global schedule";
		} else {
			const std::string& id = sched.tod_man().get_area_id(sched.area_index);
			if(id.empty()) {
				const auto& hexes = sched.tod_man().get_area_by_index(sched.area_index);
				if(hexes.empty()) {
					err << "anonymous empty time area";
				} else {
					err << "anonymous time area at (" << hexes.begin()->wml_x() << ',' << hexes.begin()->wml_y() << ")";
				}
			} else {
				err << "time area with id=" << id;
			}
		}
		lua_push(L, err.str());
		throw lua_error(L);
	}
	int n = std::distance(times.begin(), iter);
	if(sched.area_index < 0) {
		sched.tod_man().set_current_time(n);
	} else {
		sched.tod_man().set_current_time(n, sched.area_index);
	}
}

SCHEDULE_VALID("liminal_bonus") {
	return sched.area_index < 0;
}

SCHEDULE_GETTER("liminal_bonus", utils::optional<int>) {
	if(sched.area_index >= 0) return utils::nullopt;
	return sched.tod_man().get_max_liminal_bonus();
}

SCHEDULE_SETTER("liminal_bonus", utils::optional<int>) {
	if(sched.area_index >= 0) {
		throw luaL_error(L, "liminal_bonus can only be set on the global schedule");
	}
	if(value) {
		sched.tod_man().set_max_liminal_bonus(*value);
	} else {
		sched.tod_man().reset_max_liminal_bonus();
	}
}

SCHEDULE_VALID("id") {
	return sched.area_index >= 0;
}

SCHEDULE_GETTER("id", utils::optional<std::string>) {
	if(sched.area_index < 0) return utils::nullopt;
	return sched.tod_man().get_area_id(sched.area_index);
}

SCHEDULE_SETTER("id", std::string) {
	if(sched.area_index < 0) {
		throw luaL_error(L, "can't set id of global schedule");
	}
	sched.tod_man().set_area_id(sched.area_index, value);
}

SCHEDULE_VALID("hexes") {
	return sched.area_index >= 0;
}

SCHEDULE_GETTER("hexes", utils::optional<std::set<map_location>>) {
	if(sched.area_index < 0) return utils::nullopt;
	return sched.tod_man().get_area_by_index(sched.area_index);
}

SCHEDULE_SETTER("hexes", std::set<map_location>) {
	if(sched.area_index < 0) {
		throw luaL_error(L, "can't set hexes of global schedule");
	}
	sched.tod_man().replace_area_locations(sched.area_index, value);
}
}

/**
 * Gets details about a terrain.
 * - Arg 1: terrain code string.
 * - Ret 1: table.
 */
int game_lua_kernel::impl_get_terrain_info(lua_State *L)
{
	char const *m = luaL_checkstring(L, 2);
	t_translation::terrain_code t = t_translation::read_terrain_code(m);
	if (t == t_translation::NONE_TERRAIN || !board().map().tdata()->is_known(t)) return 0;
	const terrain_type& info = board().map().tdata()->get_terrain_info(t);

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
 * Gets a list of known terrain codes.
 * - Ret 1: array of terrain codes
 */
int game_lua_kernel::impl_get_terrain_list(lua_State *L)
{
	auto codes = board().map().tdata()->list();
	std::vector<std::string> terrains;
	terrains.reserve(codes.size());
	for(auto code : codes) {
		terrains.push_back(t_translation::write_terrain_code(code));
	}
	lua_push(L, terrains);
	return 1;
}

/**
 * Gets time of day information.
 * - Arg 1: schedule object, location, time area ID, or nil
 * - Arg 2: optional turn number
 * - Ret 1: table.
 */
template<bool consider_illuminates>
int game_lua_kernel::intf_get_time_of_day(lua_State *L)
{
	int for_turn = tod_man().turn();
	map_location loc = map_location();

	if(luaW_tolocation(L, 1, loc)) {
		if(!board().map().on_board_with_border(loc)) {
			return luaL_argerror(L, 1, "coordinates are not on board");
		}
	} else if(lua_isstring(L, 1)) {
		auto area = tod_man().get_area_by_id(lua_tostring(L, 1));
		if(area.empty()) {
			return luaL_error(L, "invalid or empty time_area ID");
		}
		// We just need SOME location in that area, it doesn't matter which one.
		loc = *area.begin();
	} else if(!lua_isnil(L, 1)) {
		auto area = tod_man().get_area_by_index(luaW_check_schedule(L, 1));
		if(area.empty()) {
			return luaL_error(L, "empty time_area");
		}
		// We just need SOME location in that area, it doesn't matter which one.
		loc = *area.begin();
	}

	if(lua_isnumber(L, 2)) {
		for_turn = luaL_checkinteger(L, 2);
		int number_of_turns = tod_man().number_of_turns();
		if(for_turn < 1 || (number_of_turns != -1 && for_turn > number_of_turns)) {
			return luaL_argerror(L, 2, "turn number out of range");
		}
	}

	const time_of_day& tod = consider_illuminates ?
		tod_man().get_illuminated_time_of_day(board().units(), board().map(), loc, for_turn) :
		tod_man().get_time_of_day(loc, for_turn);

	luaW_push_tod(L, tod);

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

	int side = board().village_owner(loc);
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
	if(!board().map().is_village(loc)) {
		return 0;
	}

	const int old_side_num = board().village_owner(loc);
	const int new_side_num = lua_isnoneornil(L, 2) ? 0 : luaL_checkinteger(L, 2);

	team* old_side = nullptr;
	team* new_side = nullptr;

	if(old_side_num == new_side_num) {
		return 0;
	}

	try {
		old_side = &board().get_team(old_side_num);
	} catch(const std::out_of_range&) {
		// old_side_num is invalid, most likely because the village wasn't captured.
		old_side = nullptr;
	}

	try {
		new_side = &board().get_team(new_side_num);
	} catch(const std::out_of_range&) {
		// new_side_num is invalid.
		new_side = nullptr;
	}

	// The new side was valid, but already defeated. Do nothing.
	if(new_side && board().team_is_defeated(*new_side)) {
		return 0;
	}

	// Even if the new side is not valid, we still want to remove the village from the old side.
	// This covers the case where new_side_num equals 0. The behavior in that case is to simply
	// un-assign the village from the old side, which of course we also want to happen if the new
	// side IS valid. If the village in question hadn't been captured, this won't fire (old_side
	// will be a nullptr).
	if(old_side) {
		old_side->lose_village(loc);
	}

	// If the new side was valid, re-assign the village.
	if(new_side) {
		new_side->get_village(loc, old_side_num, (luaW_toboolean(L, 3) ? &gamedata() : nullptr));
	}

	return 0;
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
 * Gets a table for an resource tag.
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing id of the desired resource
 * - Ret 1: config for the era
 */
static int intf_get_resource(lua_State *L)
{
	std::string m = luaL_checkstring(L, 1);
	if(auto res = game_config_manager::get()->game_config().find_child("resource","id",m)) {
		luaW_pushconfig(L, *res);
		return 1;
	}
	else {
		return luaL_argerror(L, 1, ("Cannot find resource with id '" + m + "'").c_str());
	}
}

/**
 * Gets a table for an era tag.
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing id of the desired era
 * - Ret 1: config for the era
 */
static int intf_get_era(lua_State *L)
{
	std::string m = luaL_checkstring(L, 1);
	if(auto res = game_config_manager::get()->game_config().find_child("era","id",m)) {
		luaW_pushconfig(L, *res);
		return 1;
	}
	else {
		return luaL_argerror(L, 1, ("Cannot find era with id '" + m + "'").c_str());
	}
	return 1;
}

extern luaW_Registry& gameConfigReg();
static auto& dummy = gameConfigReg(); // just to ensure it's constructed.

struct game_config_glk_tag {
	game_lua_kernel& ref;
	game_config_glk_tag(lua_kernel_base& k) : ref(dynamic_cast<game_lua_kernel&>(k)) {}
	auto& pc() const { return ref.play_controller_; }
	auto& gamedata() const { return ref.gamedata(); }
	auto& disp() const { return ref.game_display_; }
};
#define GAME_CONFIG_SIMPLE_SETTER(name) \
GAME_CONFIG_SETTER(#name, decltype(game_config::name), game_lua_kernel) { \
	(void) k; \
	game_config::name = value; \
}

namespace {
GAME_CONFIG_GETTER("do_healing", bool, game_lua_kernel) {
	game_config_glk_tag k2{k.ref};
	return k2.pc().gamestate().do_healing_;
}

GAME_CONFIG_SETTER("do_healing", bool, game_lua_kernel) {
	game_config_glk_tag k2{k.ref};
	k2.pc().gamestate().do_healing_ = value;}

GAME_CONFIG_GETTER("theme", std::string, game_lua_kernel) {
	game_config_glk_tag k2{k.ref};
	return k2.gamedata().get_theme();
}

GAME_CONFIG_SETTER("theme", std::string, game_lua_kernel) {
	game_config_glk_tag k2{k.ref};
	k2.gamedata().set_theme(value);
	k2.disp()->set_theme(value);
}

using traits_map = std::map<std::string, config>;
GAME_CONFIG_GETTER("global_traits", traits_map, game_lua_kernel) {
	(void)k;
	std::map<std::string, config> result;
	for(const config& trait : unit_types.traits()) {
		//It seems the engine never checks the id field for emptiness or duplicates
		//However, the worst that could happen is that the trait read later overwrites the older one,
		//and this is not the right place for such checks.
		result.emplace(trait["id"], trait);
	}
	return result;
}

GAME_CONFIG_SIMPLE_SETTER(base_income);
GAME_CONFIG_SIMPLE_SETTER(village_income);
GAME_CONFIG_SIMPLE_SETTER(village_support);
GAME_CONFIG_SIMPLE_SETTER(poison_amount);
GAME_CONFIG_SIMPLE_SETTER(rest_heal_amount);
GAME_CONFIG_SIMPLE_SETTER(recall_cost);
GAME_CONFIG_SIMPLE_SETTER(kill_experience);
GAME_CONFIG_SIMPLE_SETTER(combat_experience);
}

namespace {
	static config find_addon(const std::string& type, const std::string& id)
	{
		return game_config_manager::get()->game_config().find_mandatory_child(type, "id", id);
	}
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
	return_cstring_attrib("result", data.is_victory ? level_result::victory : "loss"); // to match wesnoth.end_level()
	return_string_attrib("test_result", data.test_result);
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
	modify_string_attrib("test_result", data.test_result = value);

	return 0;
}

static int impl_end_level_data_collect(lua_State* L)
{
	end_level_data* data = static_cast<end_level_data*>(lua_touserdata(L, 1));
	data->~end_level_data();
	return 0;
}

static int impl_mp_settings_get(lua_State* L)
{
	void* p = lua_touserdata(L, lua_upvalueindex(1));
	const mp_game_settings& settings = static_cast<play_controller*>(p)->get_mp_settings();
	if(lua_type(L, 2) == LUA_TNUMBER) {
		// Simulates a WML table with one [options] child and a variable number of [addon] children
		// TODO: Deprecate this -> mp_settings.options and mp_settings.addons
		size_t i = luaL_checkinteger(L, 2);
		if(i == 1) {
			lua_createtable(L, 2, 0);
			lua_pushstring(L, "options");
			lua_seti(L, -2, 1);
			luaW_pushconfig(L, settings.options);
			lua_seti(L, -2, 2);
			return 1;
		} else if(i >= 2) {
			i -= 2;
			if(i < settings.addons.size()) {
				auto iter = settings.addons.begin();
				std::advance(iter, i);
				config cfg;
				iter->second.write(cfg);
				cfg["id"] = iter->first;

				lua_createtable(L, 2, 0);
				lua_pushstring(L, "addon");
				lua_seti(L, -2, 1);
				luaW_pushconfig(L, cfg);
				lua_seti(L, -2, 2);
				return 1;
			}
		}
	} else {
		char const *m = luaL_checkstring(L, 2);
		return_string_attrib("scenario", settings.name);
		return_string_attrib("game_name", settings.name);
		return_string_attrib("hash", settings.hash);
		return_string_attrib("mp_era_name", settings.mp_era_name);
		return_string_attrib("mp_scenario", settings.mp_scenario);
		return_string_attrib("mp_scenario_name", settings.mp_scenario_name);
		return_string_attrib("mp_campaign", settings.mp_campaign);
		return_string_attrib("side_users", utils::join_map(settings.side_users));
		return_int_attrib("experience_modifier", settings.xp_modifier);
		return_bool_attrib("mp_countdown", settings.mp_countdown);
		return_int_attrib("mp_countdown_init_time", settings.mp_countdown_init_time.count());
		return_int_attrib("mp_countdown_turn_bonus", settings.mp_countdown_turn_bonus.count());
		return_int_attrib("mp_countdown_reservoir_bonus", settings.mp_countdown_reservoir_time.count());
		return_int_attrib("mp_countdown_action_bonus", settings.mp_countdown_action_bonus.count());
		return_int_attrib("mp_num_turns", settings.num_turns);
		return_int_attrib("mp_village_gold", settings.village_gold);
		return_int_attrib("mp_village_support", settings.village_support);
		return_bool_attrib("mp_fog", settings.fog_game);
		return_bool_attrib("mp_shroud", settings.shroud_game);
		return_bool_attrib("mp_use_map_settings", settings.use_map_settings);
		return_bool_attrib("mp_random_start_time", settings.random_start_time);
		return_bool_attrib("observer", settings.allow_observers);
		return_bool_attrib("allow_observers", settings.allow_observers);
		return_bool_attrib("private_replay", settings.private_replay);
		return_bool_attrib("shuffle_sides", settings.shuffle_sides);
		return_string_attrib("random_faction_mode", random_faction_mode::get_string(settings.mode));
		return_cfgref_attrib("options", settings.options);
		if(strcmp(m, "savegame") == 0) {
			auto savegame = settings.saved_game;
			if(savegame == saved_game_mode::type::no) {
				lua_pushboolean(L, false);
			} else {
				lua_push(L, saved_game_mode::get_string(savegame));
			}
			return 1;
		}
		if(strcmp(m, "side_players") == 0) {
			lua_push(L, settings.side_users);
			return 1;
		}
		if(strcmp(m, "addons") == 0) {
			for(const auto& [id, addon] : settings.addons) {
				lua_createtable(L, 0, 4);
				lua_push(L, id);
				lua_setfield(L, -2, "id");
				lua_push(L, addon.name);
				lua_setfield(L, -2, "name");
				lua_pushboolean(L, addon.required);
				lua_setfield(L, -2, "required");
				if(addon.min_version) {
					luaW_getglobal(L, "wesnoth", "version");
					lua_push(L, addon.min_version->str());
					lua_call(L, 1, 1);
					lua_setfield(L, -2, "min_version");
				}
				if(addon.version) {
					luaW_getglobal(L, "wesnoth", "version");
					lua_push(L, addon.version->str());
					lua_call(L, 1, 1);
					lua_setfield(L, -2, "version");
				}
				lua_createtable(L, addon.content.size(), 0);
				for(const auto& content : addon.content) {
					lua_createtable(L, 0, 3);
					lua_push(L, content.id);
					lua_setfield(L, -2, "id");
					lua_push(L, content.name);
					lua_setfield(L, -2, "name");
					lua_push(L, content.type);
					lua_setfield(L, -2, "type");
					lua_seti(L, -2, lua_rawlen(L, -2) + 1);
				}
				lua_setfield(L, -2, "content");
			}
			return 1;
		}
		// Deprecated things that were moved out of mp_settings and into game_classification
		const game_classification& game = static_cast<play_controller*>(p)->get_classification();
		return_string_attrib_deprecated("mp_era", "wesnoth.scenario.mp_settings", INDEFINITE, "1.17", "Use wesnoth.scenario.era.id instead", game.era_id);
		return_string_attrib_deprecated("active_mods", "wesnoth.scenario.mp_settings", INDEFINITE, "1.17", "Use wesnoth.scenario.modifications instead (returns an array of modification tables)", utils::join(game.active_mods));
		// Expose the raw config; this is a way to ensure any new stuff can be accessed even if someone forgot to add it here.
		return_cfgref_attrib("__cfg", settings.to_config());
	}
	return 0;
}

static int impl_mp_settings_len(lua_State* L)
{
	void* p = lua_touserdata(L, lua_upvalueindex(1));
	const mp_game_settings& settings = static_cast<play_controller*>(p)->get_mp_settings();
	lua_pushinteger(L, settings.addons.size() + 1);
	return 1;
}

struct scenario_tag {
	game_lua_kernel& ref;
	scenario_tag(game_lua_kernel& k) : ref(k) {}
	auto& tod_man() const { return ref.tod_man(); }
	auto& gamedata() const { return ref.gamedata(); }
	auto& pc() const { return ref.play_controller_; }
	auto& cls() const { return ref.play_controller_.get_classification(); }
	auto end_level_set() const { return &dispatch<&game_lua_kernel::impl_end_level_data_set>; }
};
#define SCENARIO_GETTER(name, type) LATTR_GETTER(name, type, scenario_tag, k)
#define SCENARIO_SETTER(name, type) LATTR_SETTER(name, type, scenario_tag, k)
#define SCENARIO_VALID(name) LATTR_VALID(name, scenario_tag, k)
luaW_Registry scenarioReg{"scenario"};

template<> struct lua_object_traits<scenario_tag> {
	inline static auto metatable = "scenario";
	inline static scenario_tag get(lua_State* L, int) {
		return lua_kernel_base::get_lua_kernel<game_lua_kernel>(L);
	}
};

namespace {
SCENARIO_GETTER("turns", int) {
	return k.tod_man().number_of_turns();
}

SCENARIO_SETTER("turns", int) {
	k.tod_man().set_number_of_turns_by_wml(value);
}

SCENARIO_GETTER("next", std::string) {
	return k.gamedata().next_scenario();
}

SCENARIO_SETTER("next", std::string) {
	k.gamedata().set_next_scenario(value);
}

SCENARIO_GETTER("id", std::string) {
	return k.gamedata().get_id();
}

SCENARIO_GETTER("name", t_string) {
	return k.pc().get_scenario_name();
}

SCENARIO_GETTER("defeat_music", std::vector<std::string>) {
	return k.gamedata().get_defeat_music();
}

SCENARIO_SETTER("defeat_music", std::vector<std::string>) {
	k.gamedata().set_defeat_music(value);
}

SCENARIO_GETTER("victory_music", std::vector<std::string>) {
	return k.gamedata().get_victory_music();
}

SCENARIO_SETTER("victory_music", std::vector<std::string>) {
	k.gamedata().set_victory_music(value);
}

SCENARIO_GETTER("resources", std::vector<config>) {
	std::vector<config> resources;
	for(const std::string& rsrc : utils::split(k.pc().get_loaded_resources())) {
		resources.push_back(find_addon("resource", rsrc));
	}
	return resources;
}

SCENARIO_GETTER("type", std::string) {
	return campaign_type::get_string(k.cls().type);
}

SCENARIO_GETTER("difficulty", std::string) {
	return k.cls().difficulty;
}

SCENARIO_GETTER("show_credits", bool) {
	return k.cls().end_credits;
}

SCENARIO_SETTER("show_credits", bool) {
	k.cls().end_credits = value;
}

SCENARIO_GETTER("end_text", t_string) {
	return k.cls().end_text;
}

SCENARIO_SETTER("end_text", t_string) {
	k.cls().end_text = value;
}

SCENARIO_GETTER("end_text_duration", int) {
	return k.cls().end_text_duration.count();
}

SCENARIO_SETTER("end_text_duration", int) {
	k.cls().end_text_duration = std::chrono::milliseconds{value};
}

SCENARIO_VALID("campaign") {
	return !k.cls().campaign.empty();
}

SCENARIO_GETTER("campaign", utils::optional<config>) {
	if(k.cls().campaign.empty()) return utils::nullopt;
	return find_addon("campaign", k.cls().campaign);
}

SCENARIO_GETTER("modifications", std::vector<config>) {
	std::vector<config> mods;
	for(const std::string& mod : k.cls().active_mods) {
		mods.push_back(find_addon("modification", mod));
	}
	return mods;
}

SCENARIO_GETTER("end_level_data", lua_index_raw) {
	if (!k.pc().is_regular_game_end()) {
		lua_pushnil(L);
		return lua_index_raw(L);
	}
	auto data = k.pc().get_end_level_data();
	new(L) end_level_data(data);
	if(luaL_newmetatable(L, "end level data")) {
		static luaL_Reg const callbacks[] {
			{ "__index", 	    &impl_end_level_data_get},
			{ "__newindex",     k.end_level_set()},
			{ "__gc",           &impl_end_level_data_collect},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, callbacks, 0);
	}
	lua_setmetatable(L, -2);
	return lua_index_raw(L);
}

SCENARIO_SETTER("end_level_data", vconfig) {
	end_level_data data;

	data.proceed_to_next_level = value["proceed_to_next_level"].to_bool(true);
	data.transient.carryover_report = value["carryover_report"].to_bool(true);
	data.prescenario_save = value["save"].to_bool(true);
	data.replay_save = value["replay_save"].to_bool(true);
	data.transient.linger_mode = value["linger_mode"].to_bool(true) && !k.ref.teams().empty();
	data.transient.reveal_map = value["reveal_map"].to_bool(k.pc().reveal_map_default());
	data.is_victory = value["result"] == level_result::victory;
	data.test_result = value["test_result"].str();
	k.pc().set_end_level_data(data);
}

SCENARIO_VALID("mp_settings") {
	return k.cls().is_multiplayer();
}

SCENARIO_GETTER("mp_settings", lua_index_raw) {
	if(!k.cls().is_multiplayer()) {
		lua_pushnil(L);
		return lua_index_raw(L);
	}
	lua_newuserdatauv(L, 0, 0);
	if(luaL_newmetatable(L, "mp settings")) {
		lua_pushlightuserdata(L, &k.pc());
		lua_pushcclosure(L, impl_mp_settings_get, 1);
		lua_setfield(L, -2, "__index");
		lua_pushlightuserdata(L, &k.pc());
		lua_pushcclosure(L, impl_mp_settings_len, 1);
		lua_setfield(L, -2, "__len");
		lua_pushstring(L, "mp settings");
		lua_setfield(L, -2, "__metatable");
	}
	lua_setmetatable(L, -2);
	return lua_index_raw(L);
}

SCENARIO_VALID("era") {
	return k.cls().is_multiplayer();
}

SCENARIO_GETTER("era", utils::optional<config>) {
	if(!k.cls().is_multiplayer()) return utils::nullopt;
	return find_addon("era", k.cls().era_id);
}
}

/**
 * Gets some scenario data (__index metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Ret 1: something containing the attribute.
 */
int game_lua_kernel::impl_scenario_get(lua_State *L)
{
	DBG_LUA << "impl_scenario_get";
	return scenarioReg.get(L);
}

/**
 * Sets some scenario data (__newindex metamethod).
 * - Arg 1: userdata (ignored).
 * - Arg 2: string containing the name of the property.
 * - Arg 3: something containing the attribute.
 */
int game_lua_kernel::impl_scenario_set(lua_State *L)
{
	DBG_LUA << "impl_scenario_set";
	return scenarioReg.set(L);
}

/**
 * Get a list of scenario data (__dir metamethod).
 */
int game_lua_kernel::impl_scenario_dir(lua_State *L)
{
	DBG_LUA << "impl_scenario_dir";
	return scenarioReg.dir(L);
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

struct current_tag {
	game_lua_kernel& ref;
	current_tag(game_lua_kernel& k) : ref(k) {}
	auto& pc() const { return ref.play_controller_; }
	auto ss() const { return ref.synced_state(); }
	auto& gd() const { return ref.gamedata(); }
	auto& ev() const { return ref.get_event_info(); }
	void push_schedule(lua_State* L) const { ref.luaW_push_schedule(L, -1); }
};
#define CURRENT_GETTER(name, type) LATTR_GETTER(name, type, current_tag, k)
luaW_Registry currentReg{"current"};

template<> struct lua_object_traits<current_tag> {
	inline static auto metatable = "current";
	inline static game_lua_kernel& get(lua_State* L, int) {
		return lua_kernel_base::get_lua_kernel<game_lua_kernel>(L);
	}
};

namespace {
CURRENT_GETTER("side", int) {
	return k.pc().current_side();
}

CURRENT_GETTER("turn", int) {
	return k.pc().turn();
}

CURRENT_GETTER("synced_state", std::string) {
	return k.ss();
}

CURRENT_GETTER("user_can_invoke_commands", bool) {
	return !events::commands_disabled && k.gd().phase() == game_data::TURN_PLAYING;
}

CURRENT_GETTER("map", lua_index_raw) {
	(void)k;
	intf_terrainmap_get(L);
	return lua_index_raw(L);
}

CURRENT_GETTER("schedule", lua_index_raw) {
	k.push_schedule(L);
	return lua_index_raw(L);
}

CURRENT_GETTER("user_is_replaying", bool) {
	return k.pc().is_replay();
}

CURRENT_GETTER("event_context", config) {
	const game_events::queued_event &ev = k.ev();
	config cfg;
	cfg["name"] = ev.name;
	cfg["id"]   = ev.id;
	cfg.add_child("data", ev.data);
	if (auto weapon = ev.data.optional_child("first")) {
		cfg.add_child("weapon", *weapon);
	}
	if (auto weapon = ev.data.optional_child("second")) {
		cfg.add_child("second_weapon", *weapon);
	}

	const config::attribute_value di = ev.data["damage_inflicted"];
	if(!di.empty()) {
		cfg["damage_inflicted"] = di;
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
	return cfg;
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
	return currentReg.get(L);
}

/**
 * Gets a list of date about current point of game (__dir metamethod).
 */
int game_lua_kernel::impl_current_dir(lua_State *L)
{
	return currentReg.dir(L);
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
	LOG_LUA << "Script says: \"" << m << "\"";
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
 * Removes all messages from the chat window.
 */
int game_lua_kernel::intf_clear_messages(lua_State*)
{
	if (game_display_) {
		game_display_->get_chat_manager().clear_chat_messages();
	}
	return 0;
}

int game_lua_kernel::intf_end_turn(lua_State* L)
{
	//note that next_player_number = 1, next_player_number = nteams+1 both set the next team to be the first team
	//but the later will make the turn counter change aswell fire turn end events accoringly etc.
	if (!lua_isnoneornil(L, 1)) {
		int max = 2 * teams().size();
		int npn = luaL_checkinteger(L, 1);
		if (npn <= 0 || npn > max) {
			return luaL_argerror(L, 1, "side number out of range");
		}
		resources::controller->gamestate().next_player_number_ = npn;
	}
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
	int viewing_side = 0;

	if (lua_isuserdata(L, arg))
	{
		u = &luaW_checkunit(L, arg);
		src = u->get_location();
		viewing_side = u->side();
		++arg;
	}
	else
	{
		src = luaW_checklocation(L, arg);
		unit_map::const_unit_iterator ui = units().find(src);
		if (ui.valid()) {
			u = ui.get_shared_ptr().get();
			viewing_side = u->side();
		}
		++arg;
	}

	dst = luaW_checklocation(L, arg);

	if (!board().map().on_board(src))
		return luaL_argerror(L, 1, "invalid location");
	if (!board().map().on_board(dst))
		return luaL_argerror(L, arg, "invalid location");
	++arg;

	const gamemap &map = board().map();
	bool ignore_units = false, see_all = false, ignore_teleport = false;
	double stop_at = 10000;
	std::unique_ptr<pathfind::cost_calculator> calc;

	if (lua_istable(L, arg))
	{
		ignore_units = luaW_table_get_def<bool>(L, arg, "ignore_units", false);
		see_all = luaW_table_get_def<bool>(L, arg, "ignore_visibility", false);
		ignore_teleport = luaW_table_get_def<bool>(L, arg, "ignore_teleport", false);

		stop_at = luaW_table_get_def<double>(L, arg, "max_cost", stop_at);


		lua_pushstring(L, "viewing_side");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1)) {
			int i = luaL_checkinteger(L, -1);
			if (i >= 1 && i <= static_cast<int>(teams().size())) viewing_side = i;
			else {
				// If there's a unit, we have a valid side, so fall back to legacy behaviour.
				// If we don't have a unit, legacy behaviour would be a crash, so let's not.
				if(u) see_all = true;
				deprecated_message("wesnoth.paths.find_path with viewing_side=0 (or an invalid side)", DEP_LEVEL::FOR_REMOVAL, {1, 17, 0}, "To consider fogged and hidden units, use ignore_visibility=true instead.");
			}
		}
		lua_pop(L, 1);

		lua_pushstring(L, "calculate");
		lua_rawget(L, arg);
		if(lua_isfunction(L, -1)) {
			calc.reset(new lua_pathfind_cost_calculator(L, lua_gettop(L)));
		}
		// Don't pop, the lua_pathfind_cost_calculator requires it to stay on the stack.
	}
	else if (lua_isfunction(L, arg))
	{
		deprecated_message("wesnoth.paths.find_path with cost_function as last argument", DEP_LEVEL::FOR_REMOVAL, {1, 17, 0}, "Use calculate=cost_function inside the path options table instead.");
		calc.reset(new lua_pathfind_cost_calculator(L, arg));
	}

	pathfind::teleport_map teleport_locations;

	if(!ignore_teleport) {
		if(viewing_side == 0) {
			lua_warning(L, "wesnoth.paths.find_path: ignore_teleport=false requires a valid viewing_side; continuing with ignore_teleport=true", false);
			ignore_teleport = true;
		} else {
			teleport_locations = pathfind::get_teleport_locations(*u, board().get_team(viewing_side), see_all, ignore_units);
		}
	}

	if (!calc) {
		if(!u) {
			return luaL_argerror(L, 1, "unit not found OR custom cost function not provided");
		}

		calc.reset(new pathfind::shortest_path_calculator(*u, board().get_team(viewing_side),
			teams(), map, ignore_units, false, see_all));
	}

	pathfind::plain_route res = pathfind::a_star_search(src, dst, stop_at, *calc, map.w(), map.h(),
		&teleport_locations);

	int nb = res.steps.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		luaW_pushlocation(L, res.steps[i]);
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

	int viewing_side = u->side();
	bool ignore_units = false, see_all = false, ignore_teleport = false;
	int additional_turns = 0;

	if (lua_istable(L, arg))
	{
		ignore_units = luaW_table_get_def<bool>(L, arg, "ignore_units", false);
		see_all = luaW_table_get_def<bool>(L, arg, "ignore_visibility", false);
		ignore_teleport = luaW_table_get_def<bool>(L, arg, "ignore_teleport", false);
		additional_turns = luaW_table_get_def<int>(L, arg, "max_cost", additional_turns);

		lua_pushstring(L, "viewing_side");
		lua_rawget(L, arg);
		if (!lua_isnil(L, -1)) {
			int i = luaL_checkinteger(L, -1);
			if (i >= 1 && i <= static_cast<int>(teams().size())) viewing_side = i;
			else {
				// If there's a unit, we have a valid side, so fall back to legacy behaviour.
				// If we don't have a unit, legacy behaviour would be a crash, so let's not.
				if(u) see_all = true;
				deprecated_message("wesnoth.find_reach with viewing_side=0 (or an invalid side)", DEP_LEVEL::FOR_REMOVAL, {1, 17, 0}, "To consider fogged and hidden units, use ignore_visibility=true instead.");
			}
		}
		lua_pop(L, 1);
	}

	const team& viewing_team = board().get_team(viewing_side);

	pathfind::paths res(*u, ignore_units, !ignore_teleport,
		viewing_team, additional_turns, see_all, ignore_units);

	int nb = res.destinations.size();
	lua_createtable(L, nb, 0);
	for (int i = 0; i < nb; ++i)
	{
		pathfind::paths::step &s = res.destinations[i];
		luaW_push_namedtuple(L, {"x", "y", "moves_left"});
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

/**
 * Finds all the locations for which a given unit would remove the fog (if there was fog on the map).
 *
 * - Arg 1: source location OR unit.
 * - Ret 1: array of triples (coordinates + remaining vision points).
 */
int game_lua_kernel::intf_find_vision_range(lua_State *L)
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

	if(!u)
	{
		return luaL_error(L, "wesnoth.find_vision_range: requires a valid unit");
	}

	std::map<map_location, int> jamming_map;
	actions::create_jamming_map(jamming_map, resources::gameboard->get_team(u->side()));
	pathfind::vision_path res(*u, u->get_location(), jamming_map);

	lua_createtable(L, res.destinations.size() + res.edges.size(), 0);
	for(const auto& d : res.destinations) {
		luaW_push_namedtuple(L, {"x", "y", "vision_left"});
		lua_pushinteger(L, d.curr.wml_x());
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, d.curr.wml_y());
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, d.move_left);
		lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
	}
	for(const auto& e : res.edges) {
		luaW_push_namedtuple(L, {"x", "y", "vision_left"});
		lua_pushinteger(L, e.wml_x());
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, e.wml_y());
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, -1);
		lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
	}
	return 1;
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

		fake_units.emplace_back(src, side, unit_type);

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
	typedef std::vector<std::tuple<map_location, int, std::string>> unit_type_vector;
	unit_type_vector fake_units;


	if (unit)  // 1. arg - unit
	{
		real_units.push_back(unit);
	}
	else if (!filter.null())  // 1. arg - filter
	{
		for(const ::unit* match : unit_filter(filter).all_matches_on_map()) {
			if(match->get_location().valid()) {
				real_units.push_back(match);
			}
		}
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
			if (i >= 1 && i <= static_cast<int>(teams().size()))
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
	const terrain_filter t_filter(filter, &fc, false);
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
				s << cost_map.get_pair_at(loc).first;
				s << " / ";
				s << cost_map.get_pair_at(loc).second;
				game_display_->labels().set_label(loc, s.str());
			}
		}
	}

	// create return value
	lua_createtable(L, location_set.size(), 0);
	int counter = 1;
	for (const map_location& loc : location_set)
	{
		luaW_push_namedtuple(L, {"x", "y", "cost", "reach"});

		lua_pushinteger(L, loc.wml_x());
		lua_rawseti(L, -2, 1);

		lua_pushinteger(L, loc.wml_y());
		lua_rawseti(L, -2, 2);

		lua_pushinteger(L, cost_map.get_pair_at(loc).first);
		lua_rawseti(L, -2, 3);

		lua_pushinteger(L, cost_map.get_pair_at(loc).second);
		lua_rawseti(L, -2, 4);

		lua_rawseti(L, -2, counter);
		++counter;
	}
	return 1;
}

const char* labelKey = "floating label";

static int* luaW_check_floating_label(lua_State* L, int idx)
{
	return reinterpret_cast<int*>(luaL_checkudata(L, idx, labelKey));
}

static int impl_floating_label_getmethod(lua_State* L)
{
	const char* m = luaL_checkstring(L, 2);
	return_bool_attrib("valid", *luaW_check_floating_label(L, 1) != 0);
	return luaW_getmetafield(L, 1, m);
}

int game_lua_kernel::intf_remove_floating_label(lua_State* L)
{
	int* handle = luaW_check_floating_label(L, 1);
	std::chrono::milliseconds fade{luaL_optinteger(L, 2, -1)};
	if(*handle != 0) {
		// Passing -1 as the second argument means it uses the fade time that was set when the label was created
		font::remove_floating_label(*handle, fade);
	}
	*handle = 0;
	return 0;
}

int game_lua_kernel::intf_move_floating_label(lua_State* L)
{
	int* handle = luaW_check_floating_label(L, 1);
	if(*handle != 0) {
		font::move_floating_label(*handle, luaL_checknumber(L, 2), luaL_checknumber(L, 3));
	}
	return 0;
}

/**
 * Arg 1: text - string
 * Arg 2: options table
 * - size: font size
 * - max_width: max width for word wrapping
 * - color: font color
 * - bgcolor: background color
 * - bgalpha: background opacity
 * - duration: display duration (integer or the string "unlimited")
 * - fade_time: duration of fade-out
 * - location: screen offset
 * - valign: vertical alignment and anchoring - "top", "center", or "bottom"
 * - halign: horizontal alignment and anchoring - "left", "center", or "right"
 * Returns: label handle
 */
int game_lua_kernel::intf_set_floating_label(lua_State* L, bool spawn)
{
	t_string text = luaW_checktstring(L, 1);
	int size = font::SIZE_SMALL;
	int width = 0;
	double width_ratio = 0;
	color_t color = font::LABEL_COLOR, bgcolor{0, 0, 0, 0};
	int lifetime = 2'000, fadeout = 100;
	font::ALIGN alignment = font::ALIGN::CENTER_ALIGN, vertical_alignment = font::ALIGN::CENTER_ALIGN;
	// This is actually a relative screen location in pixels, but map_location already supports
	// everything needed to read in a pair of coordinates.
	// Depending on the chosen alignment, it may be relative to centre, an edge centre, or a corner.
	map_location loc{0, 0, wml_loc()};
	if(lua_istable(L, 2)) {
		if(luaW_tableget(L, 2, "size")) {
			size = luaL_checkinteger(L, -1);
		}
		if(luaW_tableget(L, 2, "max_width")) {
			int found_number;
			width = lua_tointegerx(L, -1, &found_number);
			if(!found_number) {
				auto value = luaW_tostring(L, -1);
				try {
					if(!value.empty() && value.back() == '%') {
						value.remove_suffix(1);
						width_ratio = std::stoi(std::string(value)) / 100.0;
					} else throw std::invalid_argument(value.data());
				} catch(std::invalid_argument&) {
					return luaL_argerror(L, -1, "max_width should be integer or percentage");
				}

			}
		}
		if(luaW_tableget(L, 2, "color")) {
			if(lua_isstring(L, -1)) {
				color = color_t::from_hex_string(lua_tostring(L, -1));
			} else {
				auto vec = lua_check<std::vector<int>>(L, -1);
				if(vec.size() != 3) {
					int idx = lua_absindex(L, -1);
					if(luaW_tableget(L, idx, "r") && luaW_tableget(L, idx, "g") && luaW_tableget(L, idx, "b")) {
						color.r = luaL_checkinteger(L, -3);
						color.g = luaL_checkinteger(L, -2);
						color.b = luaL_checkinteger(L, -1);
					} else {
						return luaL_error(L, "floating label text color should be a hex string, an array of 3 integers, or a table with r,g,b keys");
					}
				} else {
					color.r = vec[0];
					color.g = vec[1];
					color.b = vec[2];
				}
			}
		}
		if(luaW_tableget(L, 2, "bgcolor")) {
			if(lua_isstring(L, -1)) {
				bgcolor = color_t::from_hex_string(lua_tostring(L, -1));
			} else {
				auto vec = lua_check<std::vector<int>>(L, -1);
				if(vec.size() != 3) {
					int idx = lua_absindex(L, -1);
					if(luaW_tableget(L, idx, "r") && luaW_tableget(L, idx, "g") && luaW_tableget(L, idx, "b")) {
						bgcolor.r = luaL_checkinteger(L, -3);
						bgcolor.g = luaL_checkinteger(L, -2);
						bgcolor.b = luaL_checkinteger(L, -1);
					} else {
						return luaL_error(L, "floating label background color should be a hex string, an array of 3 integers, or a table with r,g,b keys");
					}
				} else {
					bgcolor.r = vec[0];
					bgcolor.g = vec[1];
					bgcolor.b = vec[2];
				}
				bgcolor.a = ALPHA_OPAQUE;
			}
			if(luaW_tableget(L, 2, "bgalpha")) {
				bgcolor.a = luaL_checkinteger(L, -1);
			}
		}
		if(luaW_tableget(L, 2, "duration")) {
			int found_number;
			lifetime = lua_tointegerx(L, -1, &found_number);
			if(!found_number) {
				auto value = luaW_tostring(L, -1);
				if(value == "unlimited") {
					lifetime = -1;
				} else {
					return luaL_argerror(L, -1, "duration should be integer or 'unlimited'");
				}
			}
		}
		if(luaW_tableget(L, 2, "fade_time")) {
			fadeout = lua_tointeger(L, -1);
		}
		if(luaW_tableget(L, 2, "location")) {
			loc = luaW_checklocation(L, -1);
		}
		if(luaW_tableget(L, 2, "halign")) {
			static const char* options[] = {"left", "center", "right"};
			alignment = font::ALIGN(luaL_checkoption(L, -1, nullptr, options));
		}
		if(luaW_tableget(L, 2, "valign")) {
			static const char* options[] = {"top", "center", "bottom"};
			vertical_alignment = font::ALIGN(luaL_checkoption(L, -1, nullptr, options));
		}
	}

	int* handle = nullptr;
	if(spawn) {
		// Creating a new label, allocate a new handle
		handle = new(L)int();
	} else {
		// First argument is the label handle
		handle = luaW_check_floating_label(L, 1);
	}
	int handle_idx = lua_gettop(L);

	if(*handle != 0) {
		font::remove_floating_label(*handle);
	}

	const SDL_Rect rect = game_display_->map_outside_area();
	if(width_ratio > 0) {
		width = static_cast<int>(std::round(rect.w * width_ratio));
	}
	int x = 0, y = 0;
	switch(alignment) {
		case font::ALIGN::LEFT_ALIGN:
			x = rect.x + loc.wml_x();
			break;
		case font::ALIGN::CENTER_ALIGN:
			x = rect.x + rect.w / 2 + loc.wml_x();
			break;
		case font::ALIGN::RIGHT_ALIGN:
			x = rect.x + rect.w - loc.wml_x();
			break;
	}
	switch(vertical_alignment) {
		case font::ALIGN::LEFT_ALIGN: // top
			y = rect.y + loc.wml_y();
			break;
		case font::ALIGN::CENTER_ALIGN:
			y = rect.y + rect.h / 2 + loc.wml_y();
			break;
		case font::ALIGN::RIGHT_ALIGN: // bottom
			// The size * 1.5 adjustment avoids the text being cut off if placed at y = 0
			// This is necessary because the text is positioned by the top edge but we want it to
			// seem like it's positioned by the bottom edge.
			// This wouldn't work for multiline text, but we don't expect that to be common in this API anyway.
			y = rect.y + rect.h - loc.wml_y() - static_cast<int>(size * 1.5);
			break;
	}

	using std::chrono::milliseconds;
	font::floating_label flabel(text);
	flabel.set_font_size(size);
	flabel.set_color(color);
	flabel.set_bg_color(bgcolor);
	flabel.set_alignment(alignment);
	flabel.set_position(x, y);
	flabel.set_lifetime(milliseconds{lifetime}, milliseconds{fadeout});
	flabel.set_clip_rect(rect);

	// Adjust fallback width (map area) to avoid text escaping when location != 0
	if(width > 0) {
		flabel.set_width(std::min(width, rect.w - loc.wml_x()));
	} else {
		flabel.set_width(rect.w - loc.wml_x());
	}

	*handle = font::add_floating_label(flabel);
	lua_settop(L, handle_idx);
	if(luaL_newmetatable(L, labelKey)) {
		// Initialize the metatable
		static const luaL_Reg methods[] = {
			{"remove", &dispatch<&game_lua_kernel::intf_remove_floating_label>},
			{"move", &dispatch<&game_lua_kernel::intf_move_floating_label>},
			{"replace", &dispatch2<&game_lua_kernel::intf_set_floating_label, false>},
			{"__index", &impl_floating_label_getmethod},
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, methods, 0);
		luaW_table_set(L, -1, "__metatable", std::string(labelKey));
	}
	lua_setmetatable(L, handle_idx);
	lua_settop(L, handle_idx);
	return 1;
}

void game_lua_kernel::put_unit_helper(const map_location& loc)
{
	if(game_display_) {
		game_display_->invalidate(loc);
	}

	resources::whiteboard->on_kill_unit();
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

	map_location loc;
	if (luaW_tolocation(L, 2, loc)) {
		if (!map().on_board(loc)) {
			return luaL_argerror(L, 2, "invalid location");
		}
	}

	if((luaW_isunit(L, 1))) {
		lua_unit& u = *luaW_checkunit_ref(L, 1);
		if(u.on_map() && u->get_location() == loc) {
			return 0;
		}
		if (!loc.valid()) {
			loc = u->get_location();
			if (!map().on_board(loc))
				return luaL_argerror(L, 1, "invalid location");
		}

		put_unit_helper(loc);
		u.put_map(loc);
		u.get_shared()->anim_comp().set_standing();
	} else if(!lua_isnoneornil(L, 1)) {
		const vconfig* vcfg = nullptr;
		config cfg = luaW_checkconfig(L, 1, vcfg);
		if (!map().on_board(loc)) {
			loc.set_wml_x(cfg["x"].to_int());
			loc.set_wml_y(cfg["y"].to_int());
			if (!map().on_board(loc))
				return luaL_argerror(L, 2, "invalid location");
		}

		unit_ptr u = unit::create(cfg, true, vcfg);
		units().erase(loc);
		put_unit_helper(loc);
		u->set_location(loc);
		units().insert(u);
	}

	// Fire event if using the deprecated version or if the final argument is not false
	// If the final boolean argument is omitted, the actual final argument (the unit or location) will always yield true.
	if(luaW_toboolean(L, -1)) {
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
	} else {
		return luaL_argerror(L, 1, "expected unit or location");
	}

	units().erase(loc);
	resources::whiteboard->on_kill_unit();
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
	if (static_cast<unsigned>(side) > teams().size()) side = 0;

	if(luaW_isunit(L, 1)) {
		lu = luaW_checkunit_ref(L, 1);
		u = lu->get_shared();
		if(lu->on_recall_list() && lu->on_recall_list() == side) {
			return luaL_argerror(L, 1, "unit already on recall list");
		}
	} else {
		const vconfig* vcfg = nullptr;
		config cfg = luaW_checkconfig(L, 1, vcfg);
		u = unit::create(cfg, true, vcfg);
	}

	if (!side) {
		side = u->side();
	} else {
		u->set_side(side);
	}
	team &t = board().get_team(side);
	// Avoid duplicates in the recall list.
	std::size_t uid = u->underlying_id();
	t.recall_list().erase_by_underlying_id(uid);
	t.recall_list().add(u);
	if (lu) {
		if (lu->on_map()) {
			units().erase(u->get_location());
			resources::whiteboard->on_kill_unit();
			u->anim_comp().clear_haloes();
		}
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
		unit_ptr v = u->clone();
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
			u = unit::create(cfg, false, vcfg);
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
	unit_ptr u  = unit::create(cfg, true, vcfg);
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
	luaW_pushunit(L, u.clone());
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
	bool a = false;
	map_location loc = u.get_location();

	if(lua_isboolean(L, 3)) {
		a = luaW_toboolean(L, 3);
		if(!lua_isnoneornil(L, 4)) {
			loc = luaW_checklocation(L, 4);
		}
	} else if(!lua_isnoneornil(L, 3)) {
		loc = luaW_checklocation(L, 3);
	}

	lua_pushinteger(L, 100 - u.resistance_against(m, a, loc));
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
	t_translation::terrain_code t;
	map_location loc;
	if(luaW_tolocation(L, 2, loc)) {
		t = static_cast<const game_board*>(resources::gameboard)->map()[loc];
	} else if(lua_isstring(L, 2)) {
		char const *m = luaL_checkstring(L, 2);
		t = t_translation::read_terrain_code(m);
	} else return luaW_type_error(L, 2, "location or terrain string");
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
	t_translation::terrain_code t;
	map_location loc;
	if(luaW_tolocation(L, 2, loc)) {
		t = static_cast<const game_board*>(resources::gameboard)->map()[loc];
	} else if(lua_isstring(L, 2)) {
		char const *m = luaL_checkstring(L, 2);
		t = t_translation::read_terrain_code(m);
	} else return luaW_type_error(L, 2, "location or terrain string");
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
	t_translation::terrain_code t;
	map_location loc;
	if(luaW_tolocation(L, 2, loc)) {
		t = static_cast<const game_board*>(resources::gameboard)->map()[loc];
	} else if(lua_isstring(L, 2)) {
		char const *m = luaL_checkstring(L, 2);
		t = t_translation::read_terrain_code(m);
	} else return luaW_type_error(L, 2, "location or terrain string");
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
	t_translation::terrain_code t;
	map_location loc;
	if(luaW_tolocation(L, 2, loc)) {
		t = static_cast<const game_board*>(resources::gameboard)->map()[loc];
	} else if(lua_isstring(L, 2)) {
		char const *m = luaL_checkstring(L, 2);
		t = t_translation::read_terrain_code(m);
	} else return luaW_type_error(L, 2, "location or terrain string");
	lua_pushinteger(L, 100 - u.defense_modifier(t));
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
	lua_pushboolean(L, u.get_ability_bool(m));
	return 1;
}

/**
 * Changes a unit to the given unit type.
 * - Arg 1: unit userdata.
 * - Arg 2: unit type name
 * - Arg 3: (optional) unit variation name
 */
static int intf_transform_unit(lua_State *L)
{
	unit& u = luaW_checkunit(L, 1);
	char const *m = luaL_checkstring(L, 2);
	const unit_type *utp = unit_types.find(m);
	if (!utp) return luaL_argerror(L, 2, "unknown unit type");
	if(lua_isstring(L, 3)) {
		const std::string& m2 = lua_tostring(L, 3);
		if(!utp->has_variation(m2)) return luaL_argerror(L, 2, "unknown unit variation");
		utp = &utp->get_variation(m2);
	}
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
	lua_pushnumber(L, cmb.untouched);
	lua_setfield(L, -2, "untouched");
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
	lua_setfield(L, -2, "attack_num"); // DEPRECATED
	lua_pushnumber(L, bcustats.attack_num + 1);
	lua_setfield(L, -2, "number");
	//this is nullptr when there is no counter weapon
	if(bcustats.weapon != nullptr)
	{
		lua_pushstring(L, bcustats.weapon->id().c_str());
		lua_setfield(L, -2, "name");
		luaW_pushweapon(L, bcustats.weapon);
		lua_setfield(L, -2, "weapon");
	}

}

/**
 * Simulates a combat between two units.
 * - Arg 1: attacker userdata.
 * - Arg 2: optional weapon index.
 * - Arg 3: defender userdata.
 * - Arg 4: optional weapon index.
 *
 * - Ret 1: attacker results.
 * - Ret 2: defender results.
 * - Ret 3: info about the attacker weapon.
 * - Ret 4: info about the defender weapon.
 */
int game_lua_kernel::intf_simulate_combat(lua_State *L)
{
	int arg_num = 1, att_w = -1, def_w = -1;

	unit_const_ptr att = luaW_checkunit(L, arg_num).shared_from_this();
	++arg_num;
	if (lua_isnumber(L, arg_num)) {
		att_w = lua_tointeger(L, arg_num) - 1;
		if (att_w < 0 || att_w >= static_cast<int>(att->attacks().size()))
			return luaL_argerror(L, arg_num, "weapon index out of bounds");
		++arg_num;
	}

	unit_const_ptr def = luaW_checkunit(L, arg_num).shared_from_this();
	++arg_num;
	if (lua_isnumber(L, arg_num)) {
		def_w = lua_tointeger(L, arg_num) - 1;
		if (def_w < 0 || def_w >= static_cast<int>(def->attacks().size()))
			return luaL_argerror(L, arg_num, "weapon index out of bounds");
		++arg_num;
	}

	battle_context context(units(), att->get_location(),
		def->get_location(), att_w, def_w, 0.0, nullptr, att, def);

	luaW_pushsimdata(L, context.get_attacker_combatant());
	luaW_pushsimdata(L, context.get_defender_combatant());
	luaW_pushsimweapon(L, context.get_attacker_stats());
	luaW_pushsimweapon(L, context.get_defender_stats());
	return 4;
}

/**
 * Plays a sound, possibly repeated.
 * - Arg 1: string.
 * - Arg 2: optional integer.
 */
int game_lua_kernel::intf_play_sound(lua_State *L)
{
	if (play_controller_.is_skipping_replay()) return 0;
	char const *m = luaL_checkstring(L, 1);
	int repeats = luaL_optinteger(L, 2, 0);
	sound::play_sound(m, sound::SOUND_FX, repeats);
	return 0;
}

/**
 * Sets an achievement as being completed.
 * - Arg 1: string - content_for.
 * - Arg 2: string - id.
 */
int game_lua_kernel::intf_set_achievement(lua_State *L)
{
	const char* content_for = luaL_checkstring(L, 1);
	const char* id = luaL_checkstring(L, 2);

	for(achievement_group& group : game_config_manager::get()->get_achievements()) {
		if(group.content_for_ == content_for) {
			for(achievement& achieve : group.achievements_) {
				if(achieve.id_ == id) {
					// already achieved
					if(achieve.achieved_) {
						return 0;
					}
					// found the achievement - mark it as completed
					if(!play_controller_.is_replay()) {
						prefs::get().set_achievement(content_for, id);
					}
					achieve.achieved_ = true;
					// progressable achievements can also check for current progress equals -1
					if(achieve.max_progress_ != 0) {
						achieve.current_progress_ = -1;
					}
					if(achieve.sound_path_ != "") {
						sound::play_sound(achieve.sound_path_, sound::SOUND_FX);
					}
					// show the achievement popup
					luaW_getglobal(L, "gui", "show_popup");
					luaW_pushtstring(L, achieve.name_completed_);
					luaW_pushtstring(L, achieve.description_completed_);
					lua_pushstring(L, achieve.icon_completed_.c_str());
					luaW_pcall(L, 3, 0, 0);
					return 0;
				}
			}
			// achievement not found - existing achievement group but non-existing achievement id
			ERR_LUA << "Achievement " << id << " not found for achievement group " << content_for;
			return 0;
		}
	}

	// achievement group not found
	ERR_LUA << "Achievement group " << content_for << " not found";
	return 0;
}

/**
 * Returns whether an achievement has been completed.
 * - Arg 1: string - content_for.
 * - Arg 2: string - id.
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_has_achievement(lua_State *L)
{
	const char* content_for = luaL_checkstring(L, 1);
	const char* id = luaL_checkstring(L, 2);

	if(resources::controller->is_networked_mp() && synced_context::is_synced()) {
		ERR_LUA << "Returning false for whether a player has completed an achievement due to being networked multiplayer.";
		lua_pushboolean(L, false);
	} else {
		lua_pushboolean(L, prefs::get().achievement(content_for, id));
	}

	return 1;
}

/**
 * Returns information on a single achievement, or no data if the achievement is not found.
 * - Arg 1: string - content_for.
 * - Arg 2: string - id.
 * - Ret 1: WML table returned by the function.
 */
int game_lua_kernel::intf_get_achievement(lua_State *L)
{
	const char* content_for = luaL_checkstring(L, 1);
	const char* id = luaL_checkstring(L, 2);

	config cfg;
	for(const auto& group : game_config_manager::get()->get_achievements()) {
		if(group.content_for_ == content_for) {
			for(const auto& achieve : group.achievements_) {
				if(achieve.id_ == id) {
					// found the achievement - return it as a config
					cfg["id"] = achieve.id_;
					cfg["name"] = achieve.name_;
					cfg["name_completed"] = achieve.name_completed_;
					cfg["description"] = achieve.description_;
					cfg["description_completed"] = achieve.description_completed_;
					cfg["icon"] = achieve.icon_;
					cfg["icon_completed"] = achieve.icon_completed_;
					cfg["hidden"] = achieve.hidden_;
					cfg["achieved"] = achieve.achieved_;
					cfg["max_progress"] = achieve.max_progress_;
					cfg["current_progress"] = achieve.current_progress_;

					for(const auto& sub_ach : achieve.sub_achievements_) {
						config& sub = cfg.add_child("sub_achievement");
						sub["id"] = sub_ach.id_;
						sub["description"] = sub_ach.description_;
						sub["icon"] = sub_ach.icon_;
						sub["achieved"] = sub_ach.achieved_;
					}

					luaW_pushconfig(L, cfg);
					return 1;
				}
			}
			// return empty config - existing achievement group but non-existing achievement id
			ERR_LUA << "Achievement " << id << " not found for achievement group " << content_for;
			luaW_pushconfig(L, cfg);
			return 1;
		}
	}
	// return empty config - non-existing achievement group
	ERR_LUA << "Achievement group " << content_for << " not found";
	luaW_pushconfig(L, cfg);
	return 1;
}

/**
 * Progresses the provided achievement.
 * - Arg 1: string - content_for.
 * - Arg 2: string - achievement id.
 * - Arg 3: int - the amount to progress the achievement.
 * - Arg 4: int - the limit the achievement can progress by
 * - Ret 1: int - the achievement's current progress after adding amount or -1 if not a progressable achievement (including if it's already achieved)
 * - Ret 2: int - the achievement's max progress or -1 if not a progressable achievement
 */
int game_lua_kernel::intf_progress_achievement(lua_State *L)
{
	const char* content_for = luaL_checkstring(L, 1);
	const char* id = luaL_checkstring(L, 2);
	int amount = luaL_checkinteger(L, 3);
	int limit = luaL_optinteger(L, 4, 999999999);

	for(achievement_group& group : game_config_manager::get()->get_achievements()) {
		if(group.content_for_ == content_for) {
			for(achievement& achieve : group.achievements_) {
				if(achieve.id_ == id) {
					// check that this is a progressable achievement
					if(achieve.max_progress_ == 0 || achieve.sub_achievements_.size() > 0) {
						ERR_LUA << "Attempted to progress achievement " << id << " for achievement group " << content_for << ", is not a progressible achievement.";
						lua_pushinteger(L, -1);
						lua_pushinteger(L, -1);
						return 2;
					}

					if(!achieve.achieved_) {
						int progress = 0;
						if(!play_controller_.is_replay()) {
							progress = prefs::get().progress_achievement(content_for, id, limit, achieve.max_progress_, amount);
						}
						if(progress >= achieve.max_progress_) {
							intf_set_achievement(L);
							achieve.current_progress_ = -1;
						} else {
							achieve.current_progress_ = progress;
						}
						lua_pushinteger(L, progress);
					} else {
						lua_pushinteger(L, -1);
					}
					lua_pushinteger(L, achieve.max_progress_);

					return 2;
				}
			}
			// achievement not found - existing achievement group but non-existing achievement id
			lua_push(L, "Achievement " + std::string(id) + " not found for achievement group " + content_for);
			return lua_error(L);
		}
	}

	// achievement group not found
	lua_push(L, "Achievement group " + std::string(content_for) + " not found");
	return lua_error(L);
}

/**
 * Returns whether an achievement has been completed.
 * - Arg 1: string - content_for.
 * - Arg 2: string - achievement id.
 * - Arg 3: string - sub-achievement id
 * - Ret 1: boolean.
 */
int game_lua_kernel::intf_has_sub_achievement(lua_State *L)
{
	const char* content_for = luaL_checkstring(L, 1);
	const char* id = luaL_checkstring(L, 2);
	const char* sub_id = luaL_checkstring(L, 3);

	if(resources::controller->is_networked_mp() && synced_context::is_synced()) {
		ERR_LUA << "Returning false for whether a player has completed an achievement due to being networked multiplayer.";
		lua_pushboolean(L, false);
	} else {
		lua_pushboolean(L, prefs::get().sub_achievement(content_for, id, sub_id));
	}

	return 1;
}

/**
 * Marks a single sub-achievement as completed.
 * - Arg 1: string - content_for.
 * - Arg 2: string - achievement id.
 * - Arg 3: string - sub-achievement id
 */
int game_lua_kernel::intf_set_sub_achievement(lua_State *L)
{
	const char* content_for = luaL_checkstring(L, 1);
	const char* id = luaL_checkstring(L, 2);
	const char* sub_id = luaL_checkstring(L, 3);

	for(achievement_group& group : game_config_manager::get()->get_achievements()) {
		if(group.content_for_ == content_for) {
			for(achievement& achieve : group.achievements_) {
				if(achieve.id_ == id) {
					// the whole achievement is already completed
					if(achieve.achieved_) {
						return 0;
					}

					for(sub_achievement& sub_ach : achieve.sub_achievements_) {
						if(sub_ach.id_ == sub_id) {
							// this particular sub-achievement is already achieved
							if(sub_ach.achieved_) {
								return 0;
							} else {
								if(!play_controller_.is_replay()) {
									prefs::get().set_sub_achievement(content_for, id, sub_id);
								}
								sub_ach.achieved_ = true;
								achieve.current_progress_++;
								if(achieve.current_progress_ == achieve.max_progress_) {
									intf_set_achievement(L);
								}
								return 0;
							}
						}
					}
					// sub-achievement not found - existing achievement group and achievement but non-existing sub-achievement id
					lua_push(L, "Sub-achievement " + std::string(id) + " not found for achievement" + id + " in achievement group " + content_for);
					return lua_error(L);
				}
			}
			// achievement not found - existing achievement group but non-existing achievement id
			lua_push(L, "Achievement " + std::string(id) + " not found for achievement group " + content_for);
			return lua_error(L);
		}
	}

	// achievement group not found
	lua_push(L, "Achievement group " + std::string(content_for) + " not found");
	return lua_error(L);
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

/**
 * Selects and highlights the given location on the map.
 * - Arg 1: location.
 * - Args 2,3: booleans
 */
int game_lua_kernel::intf_select_unit(lua_State *L)
{
	events::command_disabler command_disabler;
	if(lua_isnoneornil(L, 1)) {
		play_controller_.get_mouse_handler_base().select_hex(map_location::null_location(), false, false, false, true);
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
	bool skipping = play_controller_.is_skipping_replay() || play_controller_.is_skipping_story();
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
			lua_pushvalue(L, function_index);
			lua_pushnumber(L, side);
			if (luaW_pcall(L, 1, 1, false)) {
				if(!luaW_toconfig(L, -1, cfg)) {
					static const char* msg = "function returned to wesnoth.sync.[multi_]evaluate a table which was partially invalid";
					lua_kernel_base::get_lua_kernel<game_lua_kernel>(L).log_error(msg);
					lua_warning(L, msg, false);
				}
			}
		}
		//Although lua's sync_choice can show a dialog, (and will in most cases)
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
 * - Arg 1: optional string the id of this type of user input, may only contain characters a-z and '_'
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
	sides_for = lua_check<std::vector<int>>(L, nextarg++);

	lua_push(L, mp_sync::get_user_choice_multiple_sides(tagname, lua_synchronize(L, desc, human_func, null_func), std::set<int>(sides_for.begin(), sides_for.end())));
	return 1;
}


/**
 * Calls a function in an unsynced context (this specially means that all random calls used by that function will be unsynced).
 * This is usually used together with an unsynced if like 'if controller != network'
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
 * - Arg 2: Optional reference unit (teleport_unit)
 * - Ret 1: array of integer pairs.
 */
int game_lua_kernel::intf_get_locations(lua_State *L)
{
	vconfig filter = luaW_checkvconfig(L, 1);

	std::set<map_location> res;
	filter_context & fc = game_state_;
	const terrain_filter t_filter(filter, &fc, false);
	if(luaW_isunit(L, 2)) {
		t_filter.get_locations(res, *luaW_tounit(L, 2), true);
	} else {
		t_filter.get_locations(res, true);
	}

	luaW_push_locationset(L, res);
	return 1;
}

/**
 * Matches a location against the given filter.
 * - Arg 1: location.
 * - Arg 2: WML table.
 * - Arg 3: Optional reference unit (teleport_unit)
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
	const terrain_filter t_filter(filter, &fc, false);
	if(luaW_isunit(L, 3)) {
		lua_pushboolean(L, t_filter.match(loc, *luaW_tounit(L, 3)));
	} else {
		lua_pushboolean(L, t_filter.match(loc));
	}
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
	int team_i;
	if(team* t = luaW_toteam(L, 1)) {
		team_i = t->side();
	} else {
		team_i = luaL_checkinteger(L, 1);
	}
	std::string flag = luaL_optlstring(L, 2, "", nullptr);
	std::string color = luaL_optlstring(L, 3, "", nullptr);

	if(flag.empty() && color.empty()) {
		return 0;
	}
	if(team_i < 1 || static_cast<std::size_t>(team_i) > teams().size()) {
		return luaL_error(L, "set_side_id: side number %d out of range", team_i);
	}
	team& side = board().get_team(team_i);

	if(!color.empty()) {
		side.set_color(color);
	}
	if(!flag.empty()) {
		side.set_flag(flag);
	}

	game_display_->reinit_flags_for_team(side);
	return 0;
}

static int intf_modify_ai(lua_State *L, const char* action)
{
	int side_num;
	if(team* t = luaW_toteam(L, 1)) {
		side_num = t->side();
	} else {
		side_num = luaL_checkinteger(L, 1);
	}
	std::string path = luaL_checkstring(L, 2);
	config cfg {
		"action", action,
		"path", path
	};
	if(strcmp(action, "delete") == 0) {
		ai::manager::get_singleton().modify_active_ai_for_side(side_num, cfg);
		return 0;
	}
	config component = luaW_checkconfig(L, 3);
	std::size_t len = std::string::npos, open_brak = path.find_last_of('[');
	std::size_t dot = path.find_last_of('.');
	if(open_brak != len) {
		len = open_brak - dot - 1;
	}
	cfg.add_child(path.substr(dot + 1, len), component);
	ai::manager::get_singleton().modify_active_ai_for_side(side_num, cfg);
	return 0;
}

static int intf_switch_ai(lua_State *L)
{
	int side_num;
	if(team* t = luaW_toteam(L, 1)) {
		side_num = t->side();
	} else {
		side_num = luaL_checkinteger(L, 1);
	}
	if(lua_isstring(L, 2)) {
		std::string file = luaL_checkstring(L, 2);
		if(!ai::manager::get_singleton().add_ai_for_side_from_file(side_num, file)) {
			std::string err = formatter() << "Could not load AI for side " << side_num << " from file " << file;
			lua_pushlstring(L, err.c_str(), err.length());
			return lua_error(L);
		}
	} else {
		ai::manager::get_singleton().add_ai_for_side_from_config(side_num, luaW_checkconfig(L, 2));
	}
	return 0;
}

static int intf_append_ai(lua_State *L)
{
	int side_num;
	if(team* t = luaW_toteam(L, 1)) {
		side_num = t->side();
	} else {
		side_num = luaL_checkinteger(L, 1);
	}
	config cfg = luaW_checkconfig(L, 2);
	if(!cfg.has_child("ai")) {
		cfg = config {"ai", cfg};
	}
	bool added_dummy_stage = false;
	if(!cfg.mandatory_child("ai").has_child("stage")) {
		added_dummy_stage = true;
		cfg.mandatory_child("ai").add_child("stage", config {"name", "empty"});
	}
	ai::configuration::expand_simplified_aspects(side_num, cfg);
	if(added_dummy_stage) {
		cfg.remove_children("stage", [](const config& stage_cfg) { return stage_cfg["name"] == "empty"; });
	}
	ai::manager::get_singleton().append_active_ai_for_side(side_num, cfg.mandatory_child("ai"));
	return 0;
}

int game_lua_kernel::intf_get_side(lua_State* L)
{
	unsigned i = luaL_checkinteger(L, 1);
	if(i < 1 || i > teams().size()) return 0;
	luaW_pushteam(L, board().get_team(i));
	return 1;
}

/**
 * Returns a proxy table array for all sides matching the given SSF.
 * - Arg 1: SSF
 * - Ret 1: proxy table array
 */
int game_lua_kernel::intf_get_sides(lua_State* L)
{
	LOG_LUA << "intf_get_sides called: this = " << std::hex << this << std::dec << " myname = " << my_name();
	std::vector<int> sides;
	const vconfig ssf = luaW_checkvconfig(L, 1, true);
	if(ssf.null()) {
		for(const team& t : teams()) {
			sides.push_back(t.side());
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
		deprecated_message("\"advance\" modification type", DEP_LEVEL::PREEMPTIVE, {1, 15, 0}, "Use \"advancement\" instead.");
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
	std::vector<std::string> tags;
	if(lua_isstring(L, 3)) {
		tags.push_back(lua_check<std::string>(L, 3));
	} else if (lua_istable(L, 3)){
		tags = lua_check<std::vector<std::string>>(L, 3);
	} else {
		tags.push_back("object");
	}
	//TODO
	if(filter.attribute_count() == 1 && filter.all_children_count() == 0 && filter.attribute_range().front().first == "duration") {
		u.expire_modifications(filter["duration"]);
	} else {
		for(const std::string& tag : tags) {
			for(config& obj : u.get_modifications().child_range(tag)) {
				if(obj.matches(filter)) {
					obj["duration"] = "now";
				}
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
	events::command_disabler command_disabler;
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
	prefs::get().encountered_units().insert(ty);
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
	vconfig cfg = luaW_checkvconfig(L, 2);
	const vconfig &ssf = cfg.child("filter_team");

	std::string team_name;
	if (!ssf.null()) {
		const std::vector<int>& teams = side_filter(ssf, &game_state_).get_teams();
		std::vector<std::string> team_names;
		std::transform(teams.begin(), teams.end(), std::back_inserter(team_names),
			[&](int team) { return game_state_.get_disp_context().get_team(team).team_name(); });
		team_name = utils::join(team_names);
	} else {
		team_name = cfg["team_name"].str();
	}

	if (game_display_) {
		game_display_->add_overlay(loc, overlay(
			cfg["image"],
			cfg["halo"],
			team_name,
			cfg["name"], // Name is treated as the ID
			cfg["visible_in_fog"].to_bool(true),
			cfg["submerge"].to_double(0),
			cfg["z_order"].to_double(0)
		));
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

int game_lua_kernel::intf_log_replay(lua_State* L)
{
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

struct lua_event_filter : public game_events::event_filter
{
	lua_event_filter(game_lua_kernel& lk, int idx, const config& args) : lk(lk), args_(args)
	{
		ref_ = lk.save_wml_event(idx);
	}
	bool operator()(const game_events::queued_event& event_info) const override
	{
		bool result;
		return lk.run_wml_event(ref_, args_, event_info, &result) && result;
	}
	~lua_event_filter()
	{
		lk.clear_wml_event(ref_);
	}
	void serialize(config& cfg) const override {
		cfg.add_child("filter_lua")["code"] = "<function>";
	}
private:
	game_lua_kernel& lk;
	int ref_;
	vconfig args_;
};

static std::string read_event_name(lua_State* L, int idx)
{
	if(lua_isstring(L, idx)) {
		return lua_tostring(L, idx);
	} else {
		return utils::join(lua_check<std::vector<std::string>>(L, idx));
	}
}

/**
 * Add undo actions for the current active event
 * Arg 1: Either a table of ActionWML or a function to call
 * Arg 2: (optional) If Arg 1 is a function, this is a WML table that will be passed to it
 */
int game_lua_kernel::intf_add_undo_actions(lua_State *L)
{
	config cfg;
	if(luaW_toconfig(L, 1, cfg)) {
		game_state_.undo_stack_->add_custom<actions::undo_event>(cfg, get_event_info());
	} else {
		luaW_toconfig(L, 2, cfg);
		game_state_.undo_stack_->add_custom<actions::undo_event>(save_wml_event(1), cfg, get_event_info());
	}
	return 0;
}

/** Add a new event handler
 * Arg 1: Table of options.
 * name: Event to handle, as a string or list of strings
 * id: Event ID
 * menu_item: True if this is a menu item (an ID is required); this means removing the menu item will automatically remove this event. Default false.
 * first_time_only: Whether this event should fire again after the first time; default true.
 * priority: Number that determines execution order. Events execute in order of decreasing priority, and secondarily in order of addition.
 * filter: Event filters as a config with filter tags, a table of the form {filter_type = filter_contents}, or a function
 * filter_args: Arbitrary data that will be passed to the filter, if it is a function. Ignored if the filter is specified as WML or a table.
 * content: The content of the event. This is a WML table passed verbatim into the event when it fires. If no function is specified, it will be interpreted as ActionWML.
 * action: The function to call when the event triggers. Defaults to wesnoth.wml_actions.command.
 *
 * Lua API: wesnoth.game_events.add
 */
int game_lua_kernel::intf_add_event(lua_State *L)
{
	game_events::manager & man = *game_state_.events_manager_;
	using namespace std::literals;
	std::string name, id = luaW_table_get_def(L, 1, "id", ""s);
	bool repeat = !luaW_table_get_def(L, 1, "first_time_only", true), is_menu_item = luaW_table_get_def(L, 1, "menu_item", false);
	double priority = luaW_table_get_def(L, 1, "priority", 0.);
	if(luaW_tableget(L, 1, "name")) {
		name = read_event_name(L, -1);
	} else if(is_menu_item) {
		if(id.empty()) {
			return luaL_argerror(L, 1, "non-empty id is required for a menu item");
		}
		name = "menu item " + id;
	}
	if(id.empty() && name.empty()) {
		return luaL_argerror(L, 1, "either a name or id is required");
	}
	auto new_handler = man.add_event_handler_from_lua(name, id, repeat, priority, is_menu_item);
	if(new_handler.valid()) {
		bool has_lua_filter = false;
		new_handler->set_arguments(luaW_table_get_def(L, 1, "content", config{"__empty_lua_event", true}));

		if(luaW_tableget(L, 1, "filter")) {
			int filterIdx = lua_gettop(L);
			config filters;
			if(!luaW_toconfig(L, filterIdx, filters)) {
				if(lua_isfunction(L, filterIdx)) {
					int fcnIdx = lua_absindex(L, -1);
					new_handler->add_filter(std::make_unique<lua_event_filter>(*this, fcnIdx, luaW_table_get_def(L, 1, "filter_args", config())));
					has_lua_filter = true;
				} else {
#define READ_ONE_FILTER(key, tag) \
					do { \
						if(luaW_tableget(L, filterIdx, key)) { \
							if(lua_isstring(L, -1)) { \
								filters.add_child("insert_tag", config{ \
									"name", tag, \
									"variable", luaL_checkstring(L, -1) \
								}); \
							} else { \
								filters.add_child(tag, luaW_checkconfig(L, -1)); \
							} \
						} \
					} while(false);
					READ_ONE_FILTER("condition", "filter_condition");
					READ_ONE_FILTER("side", "filter_side");
					READ_ONE_FILTER("unit", "filter");
					READ_ONE_FILTER("attack", "filter_attack");
					READ_ONE_FILTER("second_unit", "filter_second");
					READ_ONE_FILTER("second_attack", "filter_second_attack");
#undef READ_ONE_FILTER
					if(luaW_tableget(L, filterIdx, "formula")) {
						filters["filter_formula"] = luaL_checkstring(L, -1);
					}
				}
			}
			new_handler->read_filters(filters);
		}

		if(luaW_tableget(L, 1, "action")) {
			new_handler->set_event_ref(save_wml_event(-1), has_preloaded_);
		} else {
			if(has_lua_filter) {
				// This just sets the appropriate flags so the engine knows it cannot be serialized.
				// The register_wml_event call will override the actual event_ref so just pass LUA_NOREF here.
				new_handler->set_event_ref(LUA_NOREF, has_preloaded_);
			}
			new_handler->register_wml_event(*this);
		}
	}
	return 0;
}

/**
 * Upvalue 1: The event function
 * Upvalue 2: The undo function
 * Arg 1: The event content
 */
int game_lua_kernel::cfun_undoable_event(lua_State* L)
{
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_push(L, 1);
	luaW_pcall(L, 1, 0);
	game_state_.undo_stack_->add_custom<actions::undo_event>(lua_upvalueindex(2), config(), get_event_info());
	return 0;
}

/** Add a new event handler
 * Arg 1: Event to handle, as a string or list of strings; or menu item ID if this is a menu item
 * Arg 2: The function to call when the event triggers
 * Arg 3: (optional) Event priority
 * Arg 4: (optional, non-menu-items only) The function to call when the event is undone
 *
 * Lua API:
 * - wesnoth.game_events.add_repeating
 * - wesnoth.game_events.add_menu
 */
template<bool is_menu_item>
int game_lua_kernel::intf_add_event_simple(lua_State *L)
{
	game_events::manager & man = *game_state_.events_manager_;
	bool repeat = true;
	std::string name = read_event_name(L, 1), id;
	double priority = luaL_optnumber(L, 3, 0.);
	if(name.empty()) {
		return luaL_argerror(L, 1, "must not be empty");
	}
	if(is_menu_item) {
		id = name;
		name = "menu item " + name;
	} else if(lua_absindex(L, -1) > 2 && lua_isfunction(L, -1)) {
		// If undo is provided as a separate function, link them together into a single function
		// The function can be either the 3rd or 4th argument.
		lua_pushcclosure(L, &dispatch<&game_lua_kernel::cfun_undoable_event>, 2);
	}
	auto new_handler = man.add_event_handler_from_lua(name, id, repeat, priority, is_menu_item);
	if(new_handler.valid()) {
		// An event with empty arguments is not added, so set some dummy arguments
		new_handler->set_arguments(config{"__quick_lua_event", true});
		new_handler->set_event_ref(save_wml_event(2), has_preloaded_);
	}
	return 0;
}

/** Add a new event handler
 * Arg: A full event specification as a WML config
 *
 * WML API: [event]
 */
int game_lua_kernel::intf_add_event_wml(lua_State *L)
{
	game_events::manager & man = *game_state_.events_manager_;
	vconfig cfg(luaW_checkvconfig(L, 1));
	bool delayed_variable_substitution = cfg["delayed_variable_substitution"].to_bool(true);
	if(delayed_variable_substitution) {
		man.add_event_handler_from_wml(cfg.get_config(), *this);
	} else {
		man.add_event_handler_from_wml(cfg.get_parsed_config(), *this);
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
		game_display_->adjust_color_overlay(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3));
		game_display_->invalidate_all();
	}
	return 0;
}

int game_lua_kernel::intf_get_color_adjust(lua_State *L)
{
	if(game_display_) {
		auto color = game_display_->get_color_overlay();
		lua_pushinteger(L, color.r);
		lua_pushinteger(L, color.g);
		lua_pushinteger(L, color.b);
		return 3;
	}
	return 0;
}

int game_lua_kernel::intf_screen_fade(lua_State *L)
{
	if(game_display_) {
		auto vec = lua_check<std::vector<uint8_t>>(L, 1);
		if(vec.size() != 4) {
			return luaW_type_error(L, 1, "array of 4 integers");
		}
		color_t fade{vec[0], vec[1], vec[2], vec[3]};
		game_display_->fade_to(fade, std::chrono::milliseconds{luaL_checkinteger(L, 2)});
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
	if(gamedata().phase() == game_data::PRELOAD || gamedata().phase() == game_data::PRESTART || gamedata().phase() == game_data::INITIAL) {
		//don't call play_slice if the game ui is not active yet.
		return 0;
	}
	events::command_disabler command_disabler;
	using namespace std::chrono_literals;
	std::chrono::milliseconds delay{luaL_checkinteger(L, 1)};
	if(delay == 0ms) {
		play_controller_.play_slice();
		return 0;
	}
	if(luaW_toboolean(L, 2) && game_display_ && game_display_->turbo_speed() > 0) {
		delay /= game_display_->turbo_speed();
	}
	const auto end_time = std::chrono::steady_clock::now() + delay;
	do {
		play_controller_.play_slice();
		std::this_thread::sleep_for(10ms);
	} while (std::chrono::steady_clock::now() < end_time);
	return 0;
}

int game_lua_kernel::intf_add_label(lua_State *L)
{
	// TODO: Support color = {r = 0, g = 0, b = 0}
	if (game_display_) {
		vconfig cfg(luaW_checkvconfig(L, 1));

		game_display &screen = *game_display_;

		terrain_label label(screen.labels(), cfg.get_config());

		screen.labels().set_label(label.location(), label.text(), label.creator(), label.team_name(), label.color(),
				label.visible_in_fog(), label.visible_in_shroud(), label.immutable(), label.category(), label.tooltip());
	}
	return 0;
}

int game_lua_kernel::intf_remove_label(lua_State *L)
{
	if (game_display_) {
		map_location loc = luaW_checklocation(L, 1);
		std::string team_name;

		// If there's only one parameter and it's a table, check if it contains team_name
		if(lua_gettop(L) == 1 && lua_istable(L, 1)) {
			using namespace std::literals;
			team_name = luaW_table_get_def(L, 1, "team_name", ""sv);
		} else {
			team_name = luaL_optstring(L, 2, "");
		}

		game_display_->labels().set_label(loc, "", -1, team_name);
	}
	return 0;
}

int game_lua_kernel::intf_get_label(lua_State* L)
{
	if(game_display_) {
		game_display &screen = *game_display_;
		auto loc = luaW_checklocation(L, 1);
		const terrain_label* label = nullptr;
		switch(lua_type(L, 2)) {
			// Missing 2nd argument - get global label
			case LUA_TNONE: case LUA_TNIL:
				label = screen.labels().get_label(loc, "");
				break;
			// Side number - get label belonging to that side's team
			case LUA_TNUMBER:
				if(size_t n = luaL_checkinteger(L, 2); n > 0 && n <= teams().size()) {
					label = screen.labels().get_label(loc, teams().at(n - 1).team_name());
				}
				break;
			// String - get label belonging to the team with that name
			case LUA_TSTRING:
				label = screen.labels().get_label(loc, luaL_checkstring(L, 2));
				break;
			// Side userdata - get label belonging to that side's team
			case LUA_TUSERDATA:
				label = screen.labels().get_label(loc, luaW_checkteam(L, 2).team_name());
				break;
		}
		if(label) {
			config cfg;
			label->write(cfg);
			luaW_pushconfig(L, cfg);
			return 1;
		}
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
	}
	return 0;
}

/**
 * Lua frontend to the modify_ai functionality
 * - Arg 1: config.
 */
static int intf_modify_ai_old(lua_State *L)
{
	config cfg;
	luaW_toconfig(L, 1, cfg);
	int side = cfg["side"].to_int();
	ai::manager::get_singleton().modify_active_ai_for_side(side, cfg);
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
	lua_pushnumber(L, ca->evaluate());
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
	int side;
	if(team* t = luaW_toteam(L, 1)) {
		side = t->side();
	} else {
		side = luaL_checkinteger(L, 1);
	}
	lua_pop(L, 1);

	ai::component* c = ai::manager::get_singleton().get_active_ai_holder_for_side_dbg(side).get_component(nullptr, "");

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
	//ai::component* e = ai::manager::get_singleton().get_active_ai_holder_for_side_dbg(side).get_component(c, "engine[lua]");
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
		LOG_LUA << "Created new dummy lua-engine for debug_ai().";

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

/** Allow undo sets the flag saying whether the event has mutated the game to false. */
int game_lua_kernel::intf_allow_end_turn(lua_State * L)
{
	bool allow;
	t_string reason;
	// The extra iststring is required to prevent totstring from converting a bool value
	if(luaW_iststring(L, 1) && luaW_totstring(L, 1, reason)) {
		allow = false;
	} else {
		allow = luaW_toboolean(L, 1);
		luaW_totstring(L, 2, reason);
	}
	gamedata().set_allow_end_turn(allow, reason);
	return 0;
}

/** Allow undo sets the flag saying whether the event has mutated the game to false. */
int game_lua_kernel::intf_allow_undo(lua_State * L)
{
	if(lua_isboolean(L, 1)) {
		play_controller_.pump().set_undo_disabled(!luaW_toboolean(L, 1));
	}
	else {
		play_controller_.pump().set_undo_disabled(false);
	}
	return 0;
}

int game_lua_kernel::intf_cancel_action(lua_State*)
{
	play_controller_.pump().set_action_canceled();
	return 0;
}

/** Adding new time_areas dynamically with Standard Location Filters.
 * Arg 1: Area ID
 * Arg 2: Area locations (either a filter or a list of locations)
 * Arg 3: (optional) Area schedule - WML table with [time] tags and optional current_time=
 */
int game_lua_kernel::intf_add_time_area(lua_State * L)
{
	log_scope("time_area");

	std::string id;
	std::set<map_location> locs;
	config times;

	if(lua_gettop(L) == 1) {
		vconfig cfg = luaW_checkvconfig(L, 1);
		deprecated_message("Single-argument wesnoth.map.place_area is deprecated. Instead, pass ID, filter, and schedule as three separate arguments.", DEP_LEVEL::INDEFINITE, {1, 17, 0});
		id = cfg["id"].str();
		const terrain_filter filter(cfg, &game_state_, false);
		filter.get_locations(locs, true);
		times = cfg.get_parsed_config();
	} else {
		id = luaL_checkstring(L, 1);
		if(!lua_isnoneornil(L, 3))
			times = luaW_checkconfig(L, 3);
		vconfig cfg{config()};
		if(luaW_tovconfig(L, 2, cfg)) {
			// Second argument is a location filter
			const terrain_filter filter(cfg, &game_state_, false);
			filter.get_locations(locs, true);
		} else {
			// Second argument is an array of locations
			luaW_check_locationset(L, 2);
		}
	}

	tod_man().add_time_area(id, locs, times);
	LOG_LUA << "Lua inserted time_area '" << id << "'";
	return 0;
}

/** Removing new time_areas dynamically with Standard Location Filters. */
int game_lua_kernel::intf_remove_time_area(lua_State * L)
{
	log_scope("remove_time_area");

	const char * id = luaL_checkstring(L, 1);
	tod_man().remove_time_area(id);
	LOG_LUA << "Lua removed time_area '" << id << "'";

	return 0;
}

int game_lua_kernel::intf_get_time_area(lua_State* L)
{
	map_location loc;
	if(luaW_tolocation(L, 1, loc)) {
		int area_index = tod_man().get_area_on_hex(loc).first;
		if(area_index < 0) {
			lua_pushnil(L);
			return 1;
		}
		luaW_push_schedule(L, area_index);
		return 1;
	} else {
		std::string area_id = luaL_checkstring(L, 1);
		const auto& area_ids = tod_man().get_area_ids();
		if(auto iter = std::find(area_ids.begin(), area_ids.end(), area_id); iter == area_ids.end()) {
			lua_pushnil(L);
			return 1;
		} else {
			luaW_push_schedule(L, std::distance(area_ids.begin(), iter));
			return 1;
		}
	}
}

/** Replacing the current time of day schedule. */
int game_lua_kernel::intf_replace_schedule(lua_State * L)
{
	map_location loc;
	if(luaL_testudata(L, 1, "schedule")) {
		// Replace the global schedule with a time area's schedule
		// Replacing the global schedule with the global schedule
		// is also supported but obviously a no-op
		int area = luaW_check_schedule(L, 1);
		if(area >= 0) tod_man().replace_schedule(tod_man().times(area));
	} else {
		vconfig cfg = luaW_checkvconfig(L, 1);

		if(cfg.get_children("time").empty()) {
			ERR_LUA << "attempted to to replace ToD schedule with empty schedule";
		} else {
			tod_man().replace_schedule(cfg.get_parsed_config());
			if (game_display_) {
				game_display_->new_turn();
			}
			LOG_LUA << "replaced ToD schedule";
		}
	}
	return 0;
}

int game_lua_kernel::intf_scroll(lua_State * L)
{
	if (game_display_) {
		point scroll_to(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
		game_display_->scroll(scroll_to, true);

		lua_remove(L, 1);
		lua_remove(L, 1);
		lua_push(L, 25);
		intf_delay(L);
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
		virtual config generate(const reports::context & rc);
	};

	config lua_report_generator::generate(const reports::context & /*rc*/)
	{
		lua_State *L = mState;
		config cfg;
		if (!luaW_getglobal(L, "wesnoth", "interface", "game_display", name))
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
int game_lua_kernel::impl_theme_item(lua_State *L, const std::string& m)
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
	lua_cpp::push_closure(L, std::bind(&game_lua_kernel::impl_theme_item, this, std::placeholders::_1, std::string(m)), 0);
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
 * Get all available theme_items (__dir metamethod).
 */
int game_lua_kernel::impl_theme_items_dir(lua_State *L)
{
	lua_push(L, reports_.report_list());
	return 1;
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
	events::command_disabler command_disabler;
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

	u = units().find(vacant_dst).get_shared_ptr();
	u->anim_comp().set_standing();

	if ( clear_shroud ) {
		// Now that the unit is visibly in position, clear the shroud.
		clearer.clear_unit(vacant_dst, *u);
	}

	if (map().is_village(vacant_dst)) {
		actions::get_village(vacant_dst, u->side());
	}

	game_display_->invalidate_unit_after_move(src_loc, vacant_dst);

	// Sighted events.
	clearer.fire_events();
	return 0;
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
		lg::log_to_chat() << msg << '\n';
		ERR_WML << msg;
	} else {
		bool in_chat = luaW_toboolean(L, -1);
		game_state_.events_manager_->pump().put_wml_message(logger,msg,in_chat);
	}
	return 0;
}

int game_lua_kernel::intf_get_fog_or_shroud(lua_State *L, bool fog)
{
	team& t = luaW_checkteam(L, 1, board());
	map_location loc = luaW_checklocation(L, 2);
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
	if(team* t = luaW_toteam(L, 1)) {
		sides.insert(t->side());
	} else if(lua_isnumber(L, 1)) {
		sides.insert(lua_tointeger(L, 1));
	} else if(lua_istable(L, 1) && lua_istable(L, 2)) {
		const auto& v = lua_check<std::vector<int>>(L, 1);
		sides.insert(v.begin(), v.end());
	} else {
		for(const team& t : teams()) {
			sides.insert(t.side()+1);
		}
	}
	const auto& locs = luaW_check_locationset(L, lua_istable(L, 2) ? 2 : 1);

	for(const int &side_num : sides) {
		if(side_num < 1 || static_cast<std::size_t>(side_num) > teams().size()) {
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

// Invokes a synced command
static int intf_invoke_synced_command(lua_State* L)
{
	const std::string name = luaL_checkstring(L, 1);
	auto it = synced_command::registry().find(name);
	config cmd;
	if(it == synced_command::registry().end()) {
		// Custom command
		if(!luaW_getglobal(L, "wesnoth", "custom_synced_commands", name)) {
			return luaL_argerror(L, 1, "Unknown synced command");
		}
		config& cmd_tag = cmd.child_or_add("custom_command");
		cmd_tag["name"] = name;
		if(!lua_isnoneornil(L, 2)) {
			cmd_tag.add_child("data", luaW_checkconfig(L, 2));
		}
	} else {
		// Built-in command
		cmd.add_child(name, luaW_checkconfig(L, 2));
	}
	// Now just forward to the WML action.
	luaW_getglobal(L, "wesnoth", "wml_actions", "do_command");
	luaW_pushconfig(L, cmd);
	luaW_pcall(L, 1, 0);
	return 0;
}

struct callbacks_tag {
	game_lua_kernel& ref;
	callbacks_tag(game_lua_kernel& k) : ref(k) {}
};
#define CALLBACK_GETTER(name, type) LATTR_GETTER(name, lua_index_raw, callbacks_tag, ) { lua_pushcfunction(L, &impl_null_callback<type>); return lua_index_raw(L); }
luaW_Registry callbacksReg{"game_events"};

template<typename Ret>
static int impl_null_callback(lua_State* L) {
	if constexpr(std::is_same_v<Ret, void>) return 0;
	else lua_push(L, Ret());
	return 1;
};

template<> struct lua_object_traits<callbacks_tag> {
	inline static auto metatable = "game_events";
	inline static game_lua_kernel& get(lua_State* L, int) {
		return lua_kernel_base::get_lua_kernel<game_lua_kernel>(L);
	}
};

namespace {
CALLBACK_GETTER("on_event", void);
CALLBACK_GETTER("on_load", void);
CALLBACK_GETTER("on_save", config);
CALLBACK_GETTER("on_mouse_action", void);
CALLBACK_GETTER("on_mouse_button", bool);
CALLBACK_GETTER("on_mouse_move", void);
}

static int impl_game_events_dir(lua_State* L) {
	return callbacksReg.dir(L);
}

static int impl_game_events_get(lua_State* L) {
	return callbacksReg.get(L);
}

template<typename Ret = void>
static bool impl_get_callback(lua_State* L, const std::string& name) {
	int top = lua_gettop(L);
	if(!luaW_getglobal(L, "wesnoth", "game_events")) {
		return false;
	}
	lua_getfield(L, -1, name.c_str()); // calls impl_game_events_get
	lua_pushcfunction(L, &impl_null_callback<Ret>);
	if(lua_rawequal(L, -1, -2)) {
		lua_settop(L, top);
		return false;
	}
	lua_pop(L, 1);
	lua_remove(L, -2);
	return true;
}

// END CALLBACK IMPLEMENTATION

game_board & game_lua_kernel::board() {
	return game_state_.board_;
}

unit_map & game_lua_kernel::units() {
	return game_state_.board_.units();
}

std::vector<team> & game_lua_kernel::teams() {
	return game_state_.board_.teams();
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
	, EVENT_TABLE(LUA_NOREF)
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
		{ "get_era",                  &intf_get_era                  },
		{ "get_resource",             &intf_get_resource             },
		{ "modify_ai",                &intf_modify_ai_old            },
		{ "cancel_action",            &dispatch<&game_lua_kernel::intf_cancel_action              >        },
		{ "log_replay",               &dispatch<&game_lua_kernel::intf_log_replay                 >        },
		{ "log",                      &dispatch<&game_lua_kernel::intf_log                        >        },
		{ "redraw",                   &dispatch<&game_lua_kernel::intf_redraw                     >        },
		{ "simulate_combat",          &dispatch<&game_lua_kernel::intf_simulate_combat            >        },
		{ nullptr, nullptr }
	};lua_getglobal(L, "wesnoth");
	if (!lua_istable(L,-1)) {
		lua_newtable(L);
	}
	luaL_setfuncs(L, callbacks, 0);

	lua_setglobal(L, "wesnoth");

	lua_getglobal(L, "gui");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::intf_gamestate_inspector>);
	lua_setfield(L, -2, "show_inspector");
	lua_pushcfunction(L, &lua_gui2::intf_show_recruit_dialog);
	lua_setfield(L, -2, "show_recruit_dialog");
	lua_pushcfunction(L, &lua_gui2::intf_show_recall_dialog);
	lua_setfield(L, -2, "show_recall_dialog");
	lua_pop(L, 1);

	if(play_controller_.get_classification().is_test()) {
		// Create the unit_test module
		lua_newtable(L);
		static luaL_Reg const test_callbacks[] {
			{ "fire_wml_menu_item", &dispatch<&game_lua_kernel::intf_fire_wml_menu_item> },
			{ nullptr, nullptr }
		};
		luaL_setfuncs(L, test_callbacks, 0);
		lua_setglobal(L, "unit_test");
	}

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

	// Create the unit_types table
	cmd_log_ << lua_terrainmap::register_metatables(L);

	// Create the unit_types table
	cmd_log_ << "Adding terrain_types table...\n";
	lua_getglobal(L, "wesnoth");
	lua_newuserdatauv(L, 0, 0);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_get_terrain_info>);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_get_terrain_list>);
	lua_setfield(L, -2, "__dir");
	lua_pushstring(L, "terrain types");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "terrain_types");
	lua_pop(L, 1);

	// Create the ai elements table.
	cmd_log_ << "Adding ai elements table...\n";

	ai::lua_ai_context::init(L);

	// Create the current variable with its metatable.
	cmd_log_ << "Adding wesnoth current table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newuserdatauv(L, 0, 0);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_current_get>);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_current_dir>);
	lua_setfield(L, -2, "__dir");
	lua_pushboolean(L, true);
	lua_setfield(L, -2, "__dir_tablelike");
	lua_pushstring(L, "current config");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "current");
	lua_pop(L, 1);

	// Add functions to the WML module
	lua_getglobal(L, "wml");
	static luaL_Reg const wml_callbacks[] {
		{"tovconfig", &lua_common::intf_tovconfig},
		{"eval_conditional", &intf_eval_conditional},
		// These aren't actually part of the API - they're used internally by the variable metatable.
		{ "get_variable", &dispatch<&game_lua_kernel::intf_get_variable>},
		{ "set_variable", &dispatch<&game_lua_kernel::intf_set_variable>},
		{ "get_all_vars", &dispatch<&game_lua_kernel::intf_get_all_vars>},
		{ nullptr, nullptr }
	};
	luaL_setfuncs(L, wml_callbacks, 0);
	lua_pop(L, 1);

	// Add functions to the map module
	luaW_getglobal(L, "wesnoth", "map");
	static luaL_Reg const map_callbacks[] {
		// Map methods
		{"terrain_mask", &intf_terrain_mask},
		{"on_board", &intf_on_board},
		{"on_border", &intf_on_border},
		{"iter", &intf_terrainmap_iter},
		// Village operations
		{"get_owner", &dispatch<&game_lua_kernel::intf_get_village_owner>},
		{"set_owner", &dispatch<&game_lua_kernel::intf_set_village_owner>},
		// Label operations
		{"add_label", &dispatch<&game_lua_kernel::intf_add_label>},
		{"remove_label", &dispatch<&game_lua_kernel::intf_remove_label>},
		{"get_label", &dispatch<&game_lua_kernel::intf_get_label>},
		// Time area operations
		{"place_area", &dispatch<&game_lua_kernel::intf_add_time_area>},
		{"remove_area", &dispatch<&game_lua_kernel::intf_remove_time_area>},
		{"get_area", &dispatch<&game_lua_kernel::intf_get_time_area>},
		// Filters
		{"find", &dispatch<&game_lua_kernel::intf_get_locations>},
		{"matches", &dispatch<&game_lua_kernel::intf_match_location>},
		{"replace_if_failed", intf_replace_if_failed},
		{ nullptr, nullptr }
	};
	luaL_setfuncs(L, map_callbacks, 0);
	lua_pop(L, 1);

	// Create the units module
	cmd_log_ << "Adding units module...\n";
	static luaL_Reg const unit_callbacks[] {
		{"advance", &intf_advance_unit},
		{"clone", &intf_copy_unit},
		{"erase", &dispatch<&game_lua_kernel::intf_erase_unit>},
		{"extract", &dispatch<&game_lua_kernel::intf_extract_unit>},
		{"matches", &dispatch<&game_lua_kernel::intf_match_unit>},
		{"select", &dispatch<&game_lua_kernel::intf_select_unit>},
		{"to_map", &dispatch<&game_lua_kernel::intf_put_unit>},
		{"to_recall", &dispatch<&game_lua_kernel::intf_put_recall_unit>},
		{"transform", &intf_transform_unit},
		{"teleport", &dispatch<&game_lua_kernel::intf_teleport>},

		{"ability", &dispatch<&game_lua_kernel::intf_unit_ability>},
		{"defense_on", &intf_unit_defense},
		{"jamming_on", &intf_unit_jamming_cost},
		{"movement_on", &intf_unit_movement_cost},
		{"resistance_against", intf_unit_resistance},
		{"vision_on", &intf_unit_vision_cost},

		{"add_modification", &intf_add_modification},
		{"remove_modifications", &intf_remove_modifications},
		// Static functions
		{"create", &intf_create_unit},
		{"find_on_map", &dispatch<&game_lua_kernel::intf_get_units>},
		{"find_on_recall", &dispatch<&game_lua_kernel::intf_get_recall_units>},
		{"get", &dispatch<&game_lua_kernel::intf_get_unit>},
		{"get_hovered", &dispatch<&game_lua_kernel::intf_get_displayed_unit>},
		{"create_animator", &dispatch<&game_lua_kernel::intf_create_animator>},
		{"create_weapon", intf_create_attack},

		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, unit_callbacks, 0);
	lua_setfield(L, -2, "units");
	lua_pop(L, 1);

	// Create sides module
	cmd_log_ << "Adding sides module...\n";
	static luaL_Reg const side_callbacks[] {
		{ "is_enemy", &dispatch<&game_lua_kernel::intf_is_enemy> },
		{ "matches", &dispatch<&game_lua_kernel::intf_match_side> },
		{ "set_id", &dispatch<&game_lua_kernel::intf_set_side_id> },
		{ "append_ai", &intf_append_ai },
		{ "debug_ai", &intf_debug_ai },
		{ "switch_ai", &intf_switch_ai },
		// Static functions
		{ "find", &dispatch<&game_lua_kernel::intf_get_sides> },
		{ "get", &dispatch<&game_lua_kernel::intf_get_side> },
		{ "create", &dispatch<&game_lua_kernel::intf_create_side> },
		// Shroud operations
		{"place_shroud", &dispatch2<&game_lua_kernel::intf_toggle_shroud, true>},
		{"remove_shroud", &dispatch2<&game_lua_kernel::intf_toggle_shroud, false>},
		{"override_shroud", &dispatch<&game_lua_kernel::intf_override_shroud>},
		{"is_shrouded", &dispatch2<&game_lua_kernel::intf_get_fog_or_shroud, false>},
		// Fog operations
		{"place_fog", &dispatch2<&game_lua_kernel::intf_toggle_fog, false>},
		{"remove_fog", &dispatch2<&game_lua_kernel::intf_toggle_fog, true>},
		{"is_fogged", &dispatch2<&game_lua_kernel::intf_get_fog_or_shroud, true>},
		{ nullptr, nullptr }
	};
	std::vector<lua_cpp::Reg> const cpp_side_callbacks {
		{"add_ai_component", std::bind(intf_modify_ai, std::placeholders::_1, "add")},
		{"delete_ai_component", std::bind(intf_modify_ai, std::placeholders::_1, "delete")},
		{"change_ai_component", std::bind(intf_modify_ai, std::placeholders::_1, "change")},
		{nullptr, nullptr}
	};

	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, side_callbacks, 0);
	lua_cpp::set_functions(L, cpp_side_callbacks);
	lua_setfield(L, -2, "sides");
	lua_pop(L, 1);

	// Create the interface module
	cmd_log_ << "Adding interface module...\n";
	static luaL_Reg const intf_callbacks[] {
		{"add_hex_overlay", &dispatch<&game_lua_kernel::intf_add_tile_overlay>},
		{"remove_hex_overlay", &dispatch<&game_lua_kernel::intf_remove_tile_overlay>},
		{"get_color_adjust", &dispatch<&game_lua_kernel::intf_get_color_adjust>},
		{"color_adjust", &dispatch<&game_lua_kernel::intf_color_adjust>},
		{"screen_fade", &dispatch<&game_lua_kernel::intf_screen_fade>},
		{"delay", &dispatch<&game_lua_kernel::intf_delay>},
		{"deselect_hex", &dispatch<&game_lua_kernel::intf_deselect_hex>},
		{"highlight_hex", &dispatch<&game_lua_kernel::intf_highlight_hex>},
		{"float_label", &dispatch<&game_lua_kernel::intf_float_label>},
		{"get_displayed_unit", &dispatch<&game_lua_kernel::intf_get_displayed_unit>},
		{"get_hovered_hex", &dispatch<&game_lua_kernel::intf_get_mouseover_tile>},
		{"get_selected_hex", &dispatch<&game_lua_kernel::intf_get_selected_tile>},
		{"lock", &dispatch<&game_lua_kernel::intf_lock_view>},
		{"is_locked", &dispatch<&game_lua_kernel::intf_view_locked>},
		{"scroll", &dispatch<&game_lua_kernel::intf_scroll>},
		{"scroll_to_hex", &dispatch<&game_lua_kernel::intf_scroll_to_tile>},
		{"skip_messages", &dispatch<&game_lua_kernel::intf_skip_messages>},
		{"is_skipping_messages", &dispatch<&game_lua_kernel::intf_is_skipping_messages>},
		{"zoom", &dispatch<&game_lua_kernel::intf_zoom>},
		{"clear_menu_item", &dispatch<&game_lua_kernel::intf_clear_menu_item>},
		{"set_menu_item", &dispatch<&game_lua_kernel::intf_set_menu_item>},
		{"allow_end_turn", &dispatch<&game_lua_kernel::intf_allow_end_turn>},
		{"clear_chat_messages", &dispatch<&game_lua_kernel::intf_clear_messages>},
		{"end_turn", &dispatch<&game_lua_kernel::intf_end_turn>},
		{"get_viewing_side", &intf_get_viewing_side},
		{"add_chat_message", &dispatch<&game_lua_kernel::intf_message>},
		{"add_overlay_text", &dispatch2<&game_lua_kernel::intf_set_floating_label, true>},
		{"handle_user_interact", &intf_handle_user_interact},
		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, intf_callbacks, 0);
	lua_setfield(L, -2, "interface");
	lua_pop(L, 1);

	// Create the achievements module
	cmd_log_ << "Adding achievements module...\n";
	static luaL_Reg const achievement_callbacks[] {
		{ "set", &dispatch<&game_lua_kernel::intf_set_achievement> },
		{ "has", &dispatch<&game_lua_kernel::intf_has_achievement> },
		{ "get", &dispatch<&game_lua_kernel::intf_get_achievement> },
		{ "progress", &dispatch<&game_lua_kernel::intf_progress_achievement> },
		{ "has_sub_achievement", &dispatch<&game_lua_kernel::intf_has_sub_achievement> },
		{ "set_sub_achievement", &dispatch<&game_lua_kernel::intf_set_sub_achievement> },
		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, achievement_callbacks, 0);
	lua_setfield(L, -2, "achievements");
	lua_pop(L, 1);

	// Create the audio module
	cmd_log_ << "Adding audio module...\n";
	static luaL_Reg const audio_callbacks[] {
		{ "play",                &dispatch<&game_lua_kernel::intf_play_sound                 >        },
		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, audio_callbacks, 0);
	lua_setfield(L, -2, "audio");
	lua_pop(L, 1);

	// Create the paths module
	cmd_log_ << "Adding paths module...\n";
	static luaL_Reg const path_callbacks[] {
		{ "find_cost_map",             &dispatch<&game_lua_kernel::intf_find_cost_map              >        },
		{ "find_path",                 &dispatch<&game_lua_kernel::intf_find_path                  >        },
		{ "find_reach",                &dispatch<&game_lua_kernel::intf_find_reach                 >        },
		{ "find_vacant_hex",          &dispatch<&game_lua_kernel::intf_find_vacant_tile           >        },
		{ "find_vision_range",         &dispatch<&game_lua_kernel::intf_find_vision_range          >        },
		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, path_callbacks, 0);
	lua_setfield(L, -2, "paths");
	lua_pop(L, 1);

	// Create the sync module
	cmd_log_ << "Adding sync module...\n";
	static luaL_Reg const sync_callbacks[] {
		{ "invoke_command",    &intf_invoke_synced_command },
		{ "run_unsynced",      &intf_do_unsynced },
		{ "evaluate_single",   &intf_synchronize_choice },
		{ "evaluate_multiple", &intf_synchronize_choices },
		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, sync_callbacks, 0);
	lua_setfield(L, -2, "sync");
	lua_pop(L, 1);

	// Create the schedule module
	cmd_log_ << "Adding schedule module...\n";
	static luaL_Reg const schedule_callbacks[] {
		{ "get_time_of_day", &dispatch<&game_lua_kernel::intf_get_time_of_day<false>>},
		{ "get_illumination", &dispatch<&game_lua_kernel::intf_get_time_of_day<true>>},
		{ "replace", &dispatch<&game_lua_kernel::intf_replace_schedule>},
		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, schedule_callbacks, 0);
	lua_createtable(L, 0, 2);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "schedule");
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

	// Create the custom_synced_commands table.
	cmd_log_ << "Adding custom_synced_commands table...\n";

	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_setfield(L, -2, "custom_synced_commands");
	lua_pop(L, 1);

	// Create the game_events table.
	cmd_log_ << "Adding game_events module...\n";
	static luaL_Reg const event_callbacks[] {
		{ "add", &dispatch<&game_lua_kernel::intf_add_event> },
		{ "add_repeating", &dispatch<&game_lua_kernel::intf_add_event_simple<false>> },
		{ "add_menu", &dispatch<&game_lua_kernel::intf_add_event_simple<true>> },
		{ "add_wml", &dispatch<&game_lua_kernel::intf_add_event_wml> },
		{ "remove", &dispatch<&game_lua_kernel::intf_remove_event> },
		{ "fire", &dispatch2<&game_lua_kernel::intf_fire_event, false> },
		{ "fire_by_id", &dispatch2<&game_lua_kernel::intf_fire_event, true> },
		{ "add_undo_actions", &dispatch<&game_lua_kernel::intf_add_undo_actions> },
		{ "set_undoable", &dispatch<&game_lua_kernel::intf_allow_undo >        },
		{ nullptr, nullptr }
	};
	lua_getglobal(L, "wesnoth");
	lua_newtable(L);
	luaL_setfuncs(L, event_callbacks, 0);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, &impl_game_events_dir);
	lua_setfield(L, -2, "__dir");
	lua_pushcfunction(L, &impl_game_events_get);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "game_events");
	lua_setfield(L, -2, "__metatable");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "game_events");
	lua_pop(L, 1);

	// Create the theme_items table.
	cmd_log_ << "Adding game_display table...\n";

	luaW_getglobal(L, "wesnoth", "interface");
	lua_newtable(L);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_theme_items_get>);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_theme_items_set>);
	lua_setfield(L, -2, "__newindex");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_theme_items_dir>);
	lua_setfield(L, -2, "__dir");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "game_display");
	lua_pop(L, 1);

	// Create the scenario table.
	cmd_log_ << "Adding scenario table...\n";

	luaW_getglobal(L, "wesnoth");
	lua_newtable(L);
	lua_createtable(L, 0, 2);
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_scenario_get>);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_scenario_set>);
	lua_setfield(L, -2, "__newindex");
	lua_pushcfunction(L, &dispatch<&game_lua_kernel::impl_scenario_dir>);
	lua_setfield(L, -2, "__dir");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "scenario");
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

	// Set up the registry table for event handlers
	lua_newtable(L);
	EVENT_TABLE = luaL_ref(L, LUA_REGISTRYINDEX);
}

void game_lua_kernel::initialize(const config& level)
{
	lua_State *L = mState;
	assert(level_lua_.empty());
	level_lua_.append_children(level, "lua");

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

/**
 * These are the child tags of [scenario] (and the like) that are handled
 * elsewhere (in the C++ code).
 * Any child tags not in this list will be passed to Lua's on_load event.
 */
static bool is_handled_file_tag(std::string_view s)
{
	// Make sure this is sorted, since we binary_search!
	using namespace std::literals::string_view_literals;
	static constexpr std::array handled_file_tags {
		"color_palette"sv,
		"color_range"sv,
		"display"sv,
		"end_level_data"sv,
		"era"sv,
		"event"sv,
		"generator"sv,
		"label"sv,
		"lua"sv,
		"map"sv,
		"menu_item"sv,
		"modification"sv,
		"modify_unit_type"sv,
		"music"sv,
		"options"sv,
		"side"sv,
		"sound_source"sv,
		"story"sv,
		"terrain_graphics"sv,
		"time"sv,
		"time_area"sv,
		"tunnel"sv,
		"undo_stack"sv,
		"variables"sv
	};

	return std::binary_search(handled_file_tags.begin(), handled_file_tags.end(), s);
}

/**
 * Executes the game_events.on_load function and passes to it all the
 * scenario tags not yet handled.
 */
void game_lua_kernel::load_game(const config& level)
{
	lua_State *L = mState;

	if(!impl_get_callback(L, "on_load"))
		return;

	lua_newtable(L);
	int k = 1;
	for(const auto [child_key, child_cfg] : level.all_children_view())
	{
		if (is_handled_file_tag(child_key)) continue;
		lua_createtable(L, 2, 0);
		lua_pushstring(L, child_key.c_str());
		lua_rawseti(L, -2, 1);
		luaW_pushconfig(L, child_cfg);
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

	if(!impl_get_callback<config>(L, "on_save"))
		return;

	if (!luaW_pcall(L, 0, 1, false))
		return;

	config v;
	luaW_toconfig(L, -1, v);
	lua_pop(L, 1);

	// Make a copy of the source tag names. Since splice is a destructive operation,
	// we can't guarantee that the view will remain valid during iteration.
	const auto temp = v.child_name_view();
	const std::vector<std::string> src_tags(temp.begin(), temp.end());

	for(const auto& key : src_tags) {
		if(is_handled_file_tag(key)) {
			/*
			 * It seems the only tags appearing in the config v variable here
			 * are the core-lua-handled (currently [item] and [objectives])
			 * and the extra UMC ones.
			 */
			const std::string m = "Tag is already used: [" + key + "]";
			log_error(m.c_str());
			continue;
		} else {
			cfg.splice_children(v, key);
		}
	}
}

/**
 * Executes the game_events.on_event function.
 * Returns false if there was no lua handler for this event
 */
bool game_lua_kernel::run_event(const game_events::queued_event& ev)
{
	lua_State *L = mState;

	if(!impl_get_callback(L, "on_event"))
		return false;

	queued_event_context dummy(&ev, queued_events_);
	lua_pushstring(L, ev.name.c_str());
	luaW_pcall(L, 1, 0, false);
	return true;
}

void game_lua_kernel::custom_command(const std::string& name, const config& cfg)
{
	lua_State *L = mState;

	if (!luaW_getglobal(L, "wesnoth", "custom_synced_commands", name)) {
		return;
	}
	luaW_pushconfig(L, cfg);
	luaW_pcall(L, 1, 0, false);
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
		u->apply_builtin_effect(which_effect, cfg);
		return 0;
	} else {
		std::string description = u->describe_builtin_effect(which_effect, cfg);
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
bool game_lua_kernel::run_wml_action(const std::string& cmd, const vconfig& cfg,
	const game_events::queued_event& ev)
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
 * Evaluates a WML conidition.
 *
 * @returns Whether the condition passed.
 * @note    @a cfg should be either volatile or long-lived since the Lua
 *          code may grab it for an arbitrarily long time.
 */
bool game_lua_kernel::run_wml_conditional(const std::string& cmd, const vconfig& cfg)
{
	lua_State* L = mState;

	// If an invalid coniditional tag is used, consider it a pass.
	if(!luaW_getglobal(L, "wesnoth", "wml_conditionals", cmd)) {
		lg::log_to_chat() << "unknown conditional wml: [" << cmd << "]\n";
		ERR_WML << "unknown conditional wml: [" << cmd << "]";
		return true;
	}

	luaW_pushvconfig(L, cfg);

	// Any runtime error is considered a fail.
	if(!luaW_pcall(L, 1, 1, true)) {
		return false;
	}

	bool b = luaW_toboolean(L, -1);

	lua_pop(L, 1);
	return b;
}

static int intf_run_event_wml(lua_State* L)
{
	int argIdx = lua_gettop(L);
	if(!luaW_getglobal(L, "wesnoth", "wml_actions", "command")) {
		return luaL_error(L, "wesnoth.wml_actions.command is missing");
	}
	lua_pushvalue(L, argIdx);
	lua_call(L, 1, 0);
	return 0;
}

int game_lua_kernel::save_wml_event()
{
	lua_State* L = mState;
	lua_geti(L, LUA_REGISTRYINDEX, EVENT_TABLE);
	int evtIdx = lua_gettop(L);
	ON_SCOPE_EXIT(L) {
		lua_pop(L, 1);
	};
	lua_pushcfunction(L, intf_run_event_wml);
	return luaL_ref(L, evtIdx);
}

int game_lua_kernel::save_wml_event(const std::string& name, const std::string& id, const std::string& code)
{
	lua_State* L = mState;
	lua_geti(L, LUA_REGISTRYINDEX, EVENT_TABLE);
	int evtIdx = lua_gettop(L);
	ON_SCOPE_EXIT(L) {
		lua_pop(L, 1);
	};
	std::ostringstream lua_name;
	lua_name << "event ";
	if(name.empty()) {
		lua_name << "<anon>";
	} else {
		lua_name << name;
	}
	if(!id.empty()) {
		lua_name << "[id=" << id << "]";
	}
	if(!load_string(code.c_str(), lua_name.str())) {
		ERR_LUA << "Failed to register WML event: " << lua_name.str();
		return LUA_NOREF;
	}
	return luaL_ref(L, evtIdx);
}

int game_lua_kernel::save_wml_event(int idx)
{
	lua_State* L = mState;
	idx = lua_absindex(L, idx);
	lua_geti(L, LUA_REGISTRYINDEX, EVENT_TABLE);
	int evtIdx = lua_gettop(L);
	ON_SCOPE_EXIT(L) {
		lua_pop(L, 1);
	};
	lua_pushvalue(L, idx);
	return luaL_ref(L, evtIdx);
}

void game_lua_kernel::clear_wml_event(int ref)
{
	lua_State* L = mState;
	lua_geti(L, LUA_REGISTRYINDEX, EVENT_TABLE);
	luaL_unref(L, -1, ref);
	lua_pop(L, 1);
}

bool game_lua_kernel::run_wml_event(int ref, const vconfig& args, const game_events::queued_event& ev, bool* out)
{
	lua_State* L = mState;
	lua_geti(L, LUA_REGISTRYINDEX, EVENT_TABLE);
	ON_SCOPE_EXIT(L) {
		lua_pop(L, 1);
	};
	lua_geti(L, -1, ref);
	if(lua_isnil(L, -1)) return false;
	luaW_pushvconfig(L, args);
	queued_event_context dummy(&ev, queued_events_);
	if(luaW_pcall(L, 1, out ? 1 : 0, true)) {
		if(out) {
			*out = luaW_toboolean(L, -1);
			lua_pop(L, 1);
		}
		return true;
	}
	return false;
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
* Runs a script from a location filter.
* The script is an already compiled function given by its name.
*/
bool game_lua_kernel::run_filter(char const *name, const team& t)
{
	//TODO: instead of passing the lua team object we coudl also jsut pass its
	//      number. then we wouldn't need this const cast.
	luaW_pushteam(mState, const_cast<team&>(t));
	return run_filter(name, 1);
}
/**
* Runs a script from a unit filter.
* The script is an already compiled function given by its name.
*/
bool game_lua_kernel::run_filter(char const *name, const unit& u)
{
	lua_State *L = mState;
	lua_unit* lu = luaW_pushlocalunit(L, const_cast<unit&>(u));
	// stack: unit
	// put the unit to the stack twice to prevent gc.
	lua_pushvalue(L, -1);
	// stack: unit, unit
	bool res = run_filter(name, 1);
	// stack: unit
	lu->clear_ref();
	lua_pop(L, 1);
	return res;
}
/**
* Runs a script from a filter.
* The script is an already compiled function given by its name.
*/
bool game_lua_kernel::run_filter(char const *name, int nArgs)
{
	auto ml = map_locker(this);
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
		auto ml = map_locker(this);
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

	if(!impl_get_callback(L, "on_mouse_move")) {
		return;
	}
	lua_push(L, loc.wml_x());
	lua_push(L, loc.wml_y());
	luaW_pcall(L, 2, 0, false);
	return;
}

bool game_lua_kernel::mouse_button_callback(const map_location& loc, const std::string &button, const std::string &event)
{
	lua_State *L = mState;

	if(!impl_get_callback<bool>(L, "on_mouse_button")) {
		return false;
	}

	lua_push(L, loc.wml_x());
	lua_push(L, loc.wml_y());
	lua_push(L, button);
	lua_push(L, event);

	if (!luaW_pcall(L, 4, 1)) return false;
	bool result = luaW_toboolean(L, -1);
	lua_pop(L, 1);
	return result;
}

void game_lua_kernel::select_hex_callback(const map_location& loc)
{
	lua_State *L = mState;

	if(!impl_get_callback(L, "on_mouse_action")) {
		return;
	}
	lua_push(L, loc.wml_x());
	lua_push(L, loc.wml_y());
	luaW_pcall(L, 2, 0, false);
	return;
}
