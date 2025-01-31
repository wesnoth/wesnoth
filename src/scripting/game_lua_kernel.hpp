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

#pragma once

#include "scripting/lua_kernel_base.hpp" // for lua_kernel_base

#include "game_events/action_wml.hpp"   // for wml_action, etc

#include <stack>
#include <string>                       // for string

class game_config_view;
class unit;
class vconfig;
namespace ai { class engine_lua; }
namespace ai { class lua_ai_action_handler; }
namespace ai { class lua_ai_context; }

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

struct map_location;
typedef int (*lua_CFunction) (lua_State *L);

class game_lua_kernel : public lua_kernel_base
{
	game_display * game_display_;
	game_state & game_state_;
	play_controller & play_controller_;
	reports & reports_;

	// Private functions to ease access to parts of game_state
	unit_map & units();
	game_data & gamedata();
	tod_manager & tod_man();

	config level_lua_;
	int EVENT_TABLE;
	bool has_preloaded_ = false;

	std::stack<game_events::queued_event const * > queued_events_;

	const game_events::queued_event & get_event_info();

	static void extract_preload_scripts(const game_config_view& game_config);
	static std::vector<config> preload_scripts;
	static config preload_config;

	friend class game_config_manager; // to allow it to call extract_preload_scripts
	friend struct current_tag;
	friend struct scenario_tag;
	friend struct schedule_tag;
	friend struct game_config_glk_tag;

	// Private lua callbacks
	int intf_allow_end_turn(lua_State *);
	int intf_allow_undo(lua_State *);
	int intf_cancel_action(lua_State *);
	int intf_add_time_area(lua_State *);
	int intf_remove_time_area(lua_State *);
	int intf_get_time_area(lua_State *);
	int intf_animate_unit(lua_State *);
	int intf_gamestate_inspector(lua_State *);
	int intf_show_recruit_dialog(lua_State* L);
	int intf_show_recall_dialog(lua_State* L);
	int impl_run_animation(lua_State *);
	int intf_create_animator(lua_State *);
	int intf_get_unit(lua_State *);
	int intf_get_units(lua_State *);
	int intf_get_displayed_unit(lua_State*);
	int intf_match_unit(lua_State *L);
	int intf_get_recall_units(lua_State *L);
	int intf_get_variable(lua_State *L);
	int intf_set_variable(lua_State *L);
	int intf_highlight_hex(lua_State *L);
	int intf_is_enemy(lua_State *L);
	int intf_unit_ability(lua_State *L);
	int intf_view_locked(lua_State *L);
	int intf_lock_view(lua_State *L);
	int impl_get_terrain_info(lua_State *L);
	int impl_get_terrain_list(lua_State *L);
	template<bool consider_illuminates>
	int intf_get_time_of_day(lua_State *L);
	int impl_schedule_get(lua_State *L);
	int impl_schedule_len(lua_State *L);
	void luaW_push_schedule(lua_State* L, int area_index);
	int intf_get_village_owner(lua_State *L);
	int intf_set_village_owner(lua_State *L);
	int intf_get_mouseover_tile(lua_State *L);
	int intf_get_selected_tile(lua_State *L);
	int impl_scenario_get(lua_State *L);
	int impl_scenario_set(lua_State *L);
	int impl_scenario_dir(lua_State *L);
	int impl_current_get(lua_State *L);
	int impl_current_dir(lua_State *L);
	int intf_clear_messages(lua_State*);
	int impl_end_level_data_set(lua_State*);
	int intf_end_turn(lua_State*);
	int intf_find_cost_map(lua_State *L);
	int intf_find_path(lua_State *L);
	int intf_find_reach(lua_State *L);
	int intf_find_vision_range(lua_State *L);
	int intf_heal_unit(lua_State *L);
	int intf_message(lua_State *L);
	int intf_play_sound(lua_State *L);
	int intf_set_achievement(lua_State *L);
	int intf_has_achievement(lua_State *L);
	int intf_has_sub_achievement(lua_State *L);
	int intf_get_achievement(lua_State *L);
	int intf_progress_achievement(lua_State *L);
	int intf_set_sub_achievement(lua_State *L);
	int intf_set_floating_label(lua_State* L, bool spawn);
	int intf_remove_floating_label(lua_State* L);
	int intf_move_floating_label(lua_State* L);
	void put_unit_helper(const map_location& loc);
	int intf_put_unit(lua_State *L);
	int intf_erase_unit(lua_State *L);
	int intf_put_recall_unit(lua_State *L);
	int intf_extract_unit(lua_State *L);
	int intf_find_vacant_tile(lua_State *L);
	int intf_float_label(lua_State *L);
	int intf_clear_menu_item(lua_State *L);
	int intf_create_side(lua_State *L);
	int intf_set_menu_item(lua_State *L);
	int intf_toggle_shroud(lua_State *L, bool place_shroud);
	int intf_override_shroud(lua_State *L);
	int intf_simulate_combat(lua_State *L);
	int intf_scroll_to_tile(lua_State *L);
	int intf_select_unit(lua_State *L);
	int intf_deselect_hex(lua_State *L);
	int intf_is_skipping_messages(lua_State *L);
	int intf_skip_messages(lua_State *L);
	int intf_get_locations(lua_State *L);
	int intf_match_location(lua_State *L);
	int intf_match_side(lua_State *L);
	int intf_set_side_id(lua_State *L);
	int intf_modify_ai_wml(lua_State *L);
	int intf_get_sides(lua_State* L);
	int intf_get_side(lua_State* L);
	int intf_add_tile_overlay(lua_State *L);
	int intf_remove_tile_overlay(lua_State *L);
	template<bool is_menu_item>
	int intf_add_event_simple(lua_State* L);
	int intf_add_event_wml(lua_State* L);
	int intf_add_event(lua_State *L);
	int intf_add_undo_actions(lua_State *L);
	int cfun_undoable_event(lua_State *L);
	int intf_remove_event(lua_State *L);
	int intf_color_adjust(lua_State *L);
	int intf_get_color_adjust(lua_State *L);
	int intf_screen_fade(lua_State *L);
	int intf_delay(lua_State *L);
	int intf_add_label(lua_State *L);
	int intf_remove_label(lua_State *L);
	int intf_get_label(lua_State* L);
	int intf_redraw(lua_State *L);
	int intf_replace_schedule(lua_State *l);
	int impl_schedule_set(lua_State *L);
	int impl_schedule_dir(lua_State *L);
	int intf_scroll(lua_State *L);
	int intf_get_all_vars(lua_State *L);
	int impl_theme_item(lua_State *L, const std::string& name);
	int impl_theme_items_get(lua_State *L);
	int impl_theme_items_set(lua_State *L);
	int impl_theme_items_dir(lua_State *L);
	int cfun_builtin_effect(lua_State *L);
	int cfun_wml_action(lua_State *L);
	int intf_fire_event(lua_State *L, const bool by_id);
	int intf_fire_wml_menu_item(lua_State *L);
	int intf_teleport(lua_State *L);
	int intf_log(lua_State *L);
	int intf_toggle_fog(lua_State *L, const bool clear);
	int intf_get_fog_or_shroud(lua_State *L, bool fog);
	int intf_log_replay(lua_State* L);
	int intf_zoom(lua_State* L);

	//private helpers
	std::string synced_state();
	void lua_chat(const std::string& caption, const std::string& msg);
	std::vector<int> get_sides_vector(const vconfig& cfg);

public:
	game_board & board();
	std::vector<team> & teams();
	const gamemap & map() const;
	game_display * get_display() const { return game_display_; }
	/**
		A value != 0 means that the shouldn't remove any units from the map, usually because
		we are currently operating on a unit& and removing it might cause memory corruptions
		note that we don't check for the dtor of lua owned units because we assume that
		we operate on such a unit that the lua function that invoked the operation on that unit
		(like wesnoth.units.add_modification, wesnoth.units.matches ..) have a local copy of that
		lua_unit* userdata in its stack that prevents it from being collected.
	*/
	int map_locked_;
	game_lua_kernel(game_state &, play_controller &, reports &);

	void set_game_display(game_display * gd);

	virtual std::string my_name() override { return "Game Lua Kernel"; }

	std::string apply_effect(const std::string& name, unit& u, const config& cfg, bool need_apply);
	void initialize(const config& level);
	void save_game(config & level);
	void load_game(const config& level);
	bool run_event(const game_events::queued_event&);
	void custom_command(const std::string&, const config&);
	void push_builtin_effect();
	void set_wml_action(const std::string&, game_events::wml_action::handler);
	void set_wml_condition(const std::string&, bool(*)(const vconfig&));
	bool run_wml_action(const std::string&, const vconfig&,
		const game_events::queued_event&);
	bool run_filter(char const *name, const unit& u);
	bool run_filter(char const *name, const map_location& l);
	bool run_filter(char const *name, const team& t);
	bool run_filter(char const *name, int nArgs);
	bool run_wml_conditional(const std::string&, const vconfig&);
	/**
	 * Store a WML event in the Lua registry, as a function.
	 * Uses a default function that interprets ActionWML.
	 * @return A unique index into the EVENT_TABLE within the Lua registry
	 */
	int save_wml_event();
	/**
	 * Store a WML event in the Lua registry, as a function.
	 * Compiles the function from the given code.
	 * @param name The event name, used to generate a chunk name for the compiled function
	 * @param id The event id, used to generate a chunk name for the compiled function
	 * @param code The actual code of the function
	 * @return A unique index into the EVENT_TABLE within the Lua registry
	 */
	int save_wml_event(const std::string& name, const std::string& id, const std::string& code);
	/**
	 * Store a WML event in the Lua registry, as a function.
	 * Uses the function at the specified Lua stack index.
	 * @param idx The Lua stack index of the function to store
	 * @return A unique index into the EVENT_TABLE within the Lua registry
	 */
	int save_wml_event(int idx);
	/**
	 * Clear a WML event store in the Lua registry.
	 * @param ref The unique index into the EVENT_TABLE within the Lua registry
	 */
	void clear_wml_event(int ref);
	/**
	 * Run a WML stored in the Lua registry.
	 * @param ref The unique index into the EVENT_TABLE within the Lua registry
	 * @param args Arguments to pass to the event function, as a config
	 * @param ev The event data for the event being fired
	 * @param out If non-null, receives the result of the called function (provided it is a boolean value)
	 * @return Whether the function was successfully called; could be false if @a ref was invalid or if the function raised an error
	 */
	bool run_wml_event(int ref, const vconfig& args, const game_events::queued_event& ev, bool* out = nullptr);

	virtual void log_error(char const* msg, char const* context = "Lua error") override;

	ai::lua_ai_context* create_lua_ai_context(char const *code, ai::engine_lua *engine);
	ai::lua_ai_action_handler* create_lua_ai_action_handler(char const *code, ai::lua_ai_context &context);

	void mouse_over_hex_callback(const map_location& loc);
	bool mouse_button_callback(const map_location& loc, const std::string &button, const std::string &event);
	void select_hex_callback(const map_location& loc);
	void preload_finished() {has_preloaded_ = true;}
};
