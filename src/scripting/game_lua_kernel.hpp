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

#ifndef SCRIPTING_LUA_HPP
#define SCRIPTING_LUA_HPP

#include "scripting/lua_kernel_base.hpp" // for lua_kernel_base

#include "game_events/action_wml.hpp"   // for wml_action, etc

#include <stack>
#include <string>                       // for string

class config;
class CVideo;
class unit;
class vconfig;
namespace ai { class engine_lua; }
namespace ai { class lua_ai_action_handler; }
namespace ai { class lua_ai_context; }
namespace game_events { struct queued_event; }

class game_display;
class game_state;
class game_board;
class unit_map;
class gamemap;
class team;
class game_data;
class tod_manager;
class play_controller;
class reports;

typedef int (*lua_CFunction) (lua_State *L);

class game_lua_kernel : public lua_kernel_base
{
	game_display * game_display_;
	game_state & game_state_;
	play_controller & play_controller_;
	reports & reports_;

	// Private functions to ease access to parts of game_state
	game_board & board();
	unit_map & units();
	const gamemap & map();
	std::vector<team> & teams();
	game_data & gamedata();
	tod_manager & tod_man();

	config level_lua_;

	std::stack<game_events::queued_event const * > queued_events_;

	const game_events::queued_event & get_event_info();

	static void extract_preload_scripts(config const & game_config);
	static std::vector<config> preload_scripts;
	static config preload_config;

	friend class game_config_manager; // to allow it to call extract_preload_scripts

	// Private lua callbacks
	int intf_allow_end_turn(lua_State *);
	int intf_allow_undo(lua_State *);
	int intf_add_time_area(lua_State *);
	int intf_remove_time_area(lua_State *);
	int intf_animate_unit(lua_State *);
	int intf_gamestate_inspector(lua_State *);
	int intf_get_unit(lua_State *);
	int intf_get_units(lua_State *);
	int intf_get_displayed_unit(lua_State*);
	int intf_match_unit(lua_State *L);
	int intf_get_recall_units(lua_State *L);
	int intf_get_variable(lua_State *L);
	int intf_random(lua_State *L);
	int intf_set_variable(lua_State *L);
	int intf_highlight_hex(lua_State *L);
	int intf_is_enemy(lua_State *L);
	int intf_view_locked(lua_State *L);
	int intf_lock_view(lua_State *L);
	int intf_get_terrain(lua_State *L);
	int intf_set_terrain(lua_State *L);
	int intf_get_terrain_info(lua_State *L);
	int intf_get_time_of_day(lua_State *L);
	int intf_get_village_owner(lua_State *L);
	int intf_set_village_owner(lua_State *L);
	int intf_get_map_size(lua_State *L);
	int intf_get_mouseover_tile(lua_State *L);
	int intf_get_selected_tile(lua_State *L);
	int intf_get_starting_location(lua_State* L);
	int impl_game_config_get(lua_State *L);
	int impl_game_config_set(lua_State *L);
	int impl_current_get(lua_State *L);
	int intf_check_end_level_disabled(lua_State *L);
	int intf_clear_messages(lua_State*);
	int intf_end_level(lua_State*);
	int intf_end_turn(lua_State*);
	int intf_find_path(lua_State *L);
	int intf_find_reach(lua_State *L);
	int intf_find_cost_map(lua_State *L);
	int intf_heal_unit(lua_State *L);
	int intf_message(lua_State *L);
	int intf_open_help(lua_State *L);
	int intf_play_sound(lua_State *L);
	int intf_print(lua_State *L);
	int intf_put_unit(lua_State *L);
	int intf_put_recall_unit(lua_State *L);
	int intf_extract_unit(lua_State *L);
	int intf_find_vacant_tile(lua_State *L);
	int intf_float_label(lua_State *L);
	int intf_set_end_campaign_credits(lua_State *L);
	int intf_set_end_campaign_text(lua_State *L);
	int intf_clear_menu_item(lua_State *L);
	int intf_set_menu_item(lua_State *L);
	int intf_set_next_scenario(lua_State *L);
	int intf_shroud_op(lua_State *L, bool place_shroud);
	int intf_simulate_combat(lua_State *L);
	int intf_scroll_to_tile(lua_State *L);
	int intf_select_hex(lua_State *L);
	int intf_synchronize_choice(lua_State *L);
	int intf_get_locations(lua_State *L);
	int intf_get_villages(lua_State *L);
	int intf_match_location(lua_State *L);
	int intf_match_side(lua_State *L);
	int intf_modify_ai_wml(lua_State *L);
	int intf_modify_side(lua_State *L);
	int intf_get_sides(lua_State* L);
	int intf_add_tile_overlay(lua_State *L);
	int intf_remove_tile_overlay(lua_State *L);
	int intf_add_event(lua_State *L);
	int intf_remove_event(lua_State *L);
	int intf_color_adjust(lua_State *L);
	int intf_delay(lua_State *L);
	int intf_kill(lua_State *L);
	int intf_label(lua_State *L);
	int intf_redraw(lua_State *L);
	int intf_replace_schedule(lua_State *l);
	int intf_scroll(lua_State *L);
	int intf_get_all_vars(lua_State *L);
	int impl_theme_item(lua_State *L, std::string name);
	int impl_theme_items_get(lua_State *L);
	int impl_theme_items_set(lua_State *L);
	int cfun_wml_action(lua_State *L);
	int intf_fire_event(lua_State *L);
	int intf_fire_wml_menu_item(lua_State *L);

	//private helpers
	std::string synced_state();
	void lua_chat(std::string const &caption, std::string const &msg);
	std::vector<int> get_sides_vector(const vconfig& cfg);

public:
	game_lua_kernel(CVideo *, game_state &, play_controller &, reports &);

	void set_game_display(game_display * gd);

	virtual std::string my_name() { return "Game Lua Kernel"; }

	void initialize(const config& level);
	void save_game(config & level);
	void load_game(const config& level);
	bool run_event(game_events::queued_event const &);
	void set_wml_action(std::string const &, game_events::wml_action::handler);
	bool run_wml_action(std::string const &, vconfig const &,
		game_events::queued_event const &);
	bool run_filter(char const *name, unit const &u);
	bool run_wml_conditional(std::string const &, vconfig const &);

	virtual void log_error(char const* msg, char const* context = "Lua error");

	ai::lua_ai_context* create_lua_ai_context(char const *code, ai::engine_lua *engine);
	ai::lua_ai_action_handler* create_lua_ai_action_handler(char const *code, ai::lua_ai_context &context);
};

#endif
