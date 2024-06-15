/*
	Copyright (C) 2024 - 2024
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

#include "enum_base.hpp"

/**
 * Contains all valid preferences attributes.
 */
struct preferences_list_defines
{
	//
	// regular preferences
	//
	/** wesnoth version string when cache files were last deleted */
	static constexpr const char* const _last_cache_cleaned_ver = "_last_cache_cleaned_ver";
	/** the sort direction, ie: ascending */
	static constexpr const char* const addon_manager_saved_order_direction = "addon_manager_saved_order_direction";
	/** the name of the column in the add-ons manager to use by default to sort results */
	static constexpr const char* const addon_manager_saved_order_name = "addon_manager_saved_order_name";
	/** set an alternate way to call a command, similar to linux terminal command aliases */
	static constexpr const char* const alias = "alias";
	/** whether a game should allow observers */
	static constexpr const char* const allow_observers = "allow_observers";
	/** the orb color above allied units */
	static constexpr const char* const ally_orb_color = "ally_orb_color";
	/** whether to stop movement of a unit when an allied unit comes into sight range */
	static constexpr const char* const ally_sighted_interrupts = "ally_sighted_interrupts";
	/** whether to display animations on terrain (ie: windmill) */
	static constexpr const char* const animate_map = "animate_map";
	/** whether to animate water terrain */
	static constexpr const char* const animate_water = "animate_water";
	/** whether to automatically set the pixel scale multiplier based on the current window size */
	static constexpr const char* const auto_pixel_scale = "auto_pixel_scale";
	/** the maximum number of autosaves to keep before deleting old ones */
	static constexpr const char* const auto_save_max = "auto_save_max";
	/** how loud the turn bell sound is */
	static constexpr const char* const bell_volume = "bell_volume";
	/** whether to show the map before being given a side in online multiplayer */
	static constexpr const char* const blindfold_replay = "blindfold_replay";
	/** the add-ons server name, ie: add-ons.wesnoth.org */
	static constexpr const char* const campaign_server = "campaign_server";
	/** the number of lines of chat to display in-game */
	static constexpr const char* const chat_lines = "chat_lines";
	/** whether to show a timestamp in in-game chat messages */
	static constexpr const char* const chat_timestamp = "chat_timestamp";
	/** child tag name for completed campaign information */
	static constexpr const char* const completed_campaigns = "completed_campaigns";
	/** whether to confirm ending your turn when units can still take action */
	static constexpr const char* const confirm_end_turn = "confirm_end_turn";
	/** the current core to use */
	static constexpr const char* const core = "core";
	/**
	 * creates a single command composed of one or more other commands
	 * format - :custom show_terrain_codes;show_num_of_bitmaps
	 */
	static constexpr const char* const custom_command = "custom_command";
	/** whether to show a confirmation dialog when deleting a save */
	static constexpr const char* const delete_saves = "delete_saves";
	/** additional folders shown when using the file selection dialog */
	static constexpr const char* const dir_bookmarks = "dir_bookmarks";
	/** whether to automatically move units that were previously told to move towards hexes more than one turn away */
	static constexpr const char* const disable_auto_moves = "disable_auto_moves";
	/** used to enforce the FPS limit - minimum time between frame updates if the player's PC is capable of updating the screen faster than this */
	static constexpr const char* const draw_delay = "draw_delay";
	/** whether to have the editor automatically update terrain transitions when placing terrain immediately, after the mouse click is released, or not at all */
	static constexpr const char* const editor_auto_update_transitions = "editor_auto_update_transitions";
	/** the current add-on being used in the editor */
	static constexpr const char* const editor_chosen_addon = "editor_chosen_addon";
	/** effectively unused */
	static constexpr const char* const editor_default_dir = "editor_default_dir";
	/** whether to draw the x,y map coordinates in the editor */
	static constexpr const char* const editor_draw_hex_coordinates = "editor_draw_hex_coordinates";
	/** number of images used to draw the hex */
	static constexpr const char* const editor_draw_num_of_bitmaps = "editor_draw_num_of_bitmaps";
	/** whether to draw terrain codes on hexes in the editor */
	static constexpr const char* const editor_draw_terrain_codes = "editor_draw_terrain_codes";
	/** list of recently accessed files in the editor */
	static constexpr const char* const editor_recent_files = "editor_recent_files";
	/** whether to automatically start in planning mode in-game */
	static constexpr const char* const enable_planning_mode_on_start = "enable_planning_mode_on_start";
	/** list of terrain seen so far */
	static constexpr const char* const encountered_terrain_list = "encountered_terrain_list";
	/** list of units seen so far */
	static constexpr const char* const encountered_units = "encountered_units";
	/** the color of the orb over enemy units */
	static constexpr const char* const enemy_orb_color = "enemy_orb_color";
	/** in the multiplayer lobby, show games with blocked players in them */
	static constexpr const char* const fi_blocked_in_game = "fi_blocked_in_game";
	/** in the multiplayer lobby, show games with friends in them */
	static constexpr const char* const fi_friends_in_game = "fi_friends_in_game";
	/** in the multiplayer lobby, invert all other filters */
	static constexpr const char* const fi_invert = "fi_invert";
	/** in the multiplayer lobby, show games with vacant slots */
	static constexpr const char* const fi_vacant_slots = "fi_vacant_slots";
	/** whether to show floating labels on the game map */
	static constexpr const char* const floating_labels = "floating_labels";
	/** whether to use fullscreen mode */
	static constexpr const char* const fullscreen = "fullscreen";
	/** whether to show a hex grid overlay on the map */
	static constexpr const char* const grid = "grid";
	/** the gui2 theme name */
	static constexpr const char* const gui2_theme = "gui2_theme";
	/** whether to show teammate's whiteboard plans on the game map */
	static constexpr const char* const hide_whiteboard = "hide_whiteboard";
	/** list of lines of text entered into textboxes that support keeping a history */
	static constexpr const char* const history = "history";
	/** the most recent multiplayer server hostname */
	static constexpr const char* const host = "host";
	/** whether to play idle animations */
	static constexpr const char* const idle_anim = "idle_anim";
	/** how frequently to play idle animations */
	static constexpr const char* const idle_anim_rate = "idle_anim_rate";
	/** who to show notifications about when they join the multiplayer lobby */
	static constexpr const char* const lobby_joins = "lobby_joins";
	/** whether to only accept whisper messages from friends */
	static constexpr const char* const lobby_whisper_friends_only = "lobby_whisper_friends_only";
	/** player chosen language to use */
	static constexpr const char* const locale = "locale";
	/** most recently use username for logging into the multiplayer server */
	static constexpr const char* const login = "login";
	/** whether the window is maximized */
	static constexpr const char* const maximized = "maximized";
	/** whether to play a sound when receiving a message */
	static constexpr const char* const message_bell = "message_bell";
	/** whether to draw terrain in the in-game minimap */
	static constexpr const char* const minimap_draw_terrain = "minimap_draw_terrain";
	/** whether to draw units on in-game/editor minimap */
	static constexpr const char* const minimap_draw_units = "minimap_draw_units";
	/** whether to draw villages on the in-game/editor minimap */
	static constexpr const char* const minimap_draw_villages = "minimap_draw_villages";
	/**
	 * on the in-game minimap/editor minimap use a color for your side, a color for all allied sides, and a color for all enemy sides
	 * doesn't actually have anything to do with movement
	 */
	static constexpr const char* const minimap_movement_coding = "minimap_movement_coding";
	/** simplified color coding by terrain type in the in-game minimap/editor */
	static constexpr const char* const minimap_terrain_coding = "minimap_terrain_coding";
	/** the color of the orb above a unit that can no longer move */
	static constexpr const char* const moved_orb_color = "moved_orb_color";
	/** whether to enable the turn timer in multiplayer */
	static constexpr const char* const mp_countdown = "mp_countdown";
	/** seconds to add to the multiplayer turn timer when an action is taken */
	static constexpr const char* const mp_countdown_action_bonus = "mp_countdown_action_bonus";
	/** seconds for the multiplayer turn timer for each player's first turn */
	static constexpr const char* const mp_countdown_init_time = "mp_countdown_init_time";
	/** maximum seconds the multiplayer turn timer can have for a single turn */
	static constexpr const char* const mp_countdown_reservoir_time = "mp_countdown_reservoir_time";
	/** bonus seconds for the multiplayer turn timer for turns after turn 1 */
	static constexpr const char* const mp_countdown_turn_bonus = "mp_countdown_turn_bonus";
	/** the most recently played era in multiplayer */
	static constexpr const char* const mp_era = "mp_era";
	/** whether to enable fog in multiplayer games */
	static constexpr const char* const mp_fog = "mp_fog";
	/** the id of the most recently played multiplayer scenario */
	static constexpr const char* const mp_level = "mp_level";
	/** most recently selected type of game: scenario, campaign, random map, etc */
	static constexpr const char* const mp_level_type = "mp_level_type";
	/** list of the last selected multiplayer modifications */
	static constexpr const char* const mp_modifications = "mp_modifications";
	/** whether to use a random start time for the scenario */
	static constexpr const char* const mp_random_start_time = "mp_random_start_time";
	/** the name of the wesnothd executable */
	static constexpr const char* const mp_server_program_name = "mp_server_program_name";
	/** whether to show a warning dialog about starting wesnothd in the background when hosting LAN games */
	static constexpr const char* const mp_server_warning_disabled = "mp_server_warning_disabled";
	/** whether to enable shroud in multiplayer games */
	static constexpr const char* const mp_shroud = "mp_shroud";
	/** the turn limit for multiplayer games */
	static constexpr const char* const mp_turns = "mp_turns";
	/** whether to use the scenario's default settings */
	static constexpr const char* const mp_use_map_settings = "mp_use_map_settings";
	/** the amount of gold each village gives */
	static constexpr const char* const mp_village_gold = "mp_village_gold";
	/** the amount of unit upkeep each village offsets */
	static constexpr const char* const mp_village_support = "mp_village_support";
	/** the multiplier to apply to all units in a multiplayer game */
	static constexpr const char* const mp_xp_modifier = "mp_xp_modifier";
	/** whether music is enabled */
	static constexpr const char* const music = "music";
	/** the music's volume */
	static constexpr const char* const music_volume = "music_volume";
	/** custom multiplayer scenario options */
	static constexpr const char* const options = "options";
	/** the orb above units with that can still take some actions */
	static constexpr const char* const partial_orb_color = "partial_orb_color";
	/** the pixel scale multiplier to apply */
	static constexpr const char* const pixel_scale = "pixel_scale";
	/** whether to allow any type of mirrors in a multiplayer scenario */
	static constexpr const char* const random_faction_mode = "random_faction_mode";
	/** whether to remember passwords typed into password fields */
	static constexpr const char* const remember_password = "remember_password";
	/** audio sample rate */
	static constexpr const char* const sample_rate = "sample_rate";
	/** whether to save replays of games */
	static constexpr const char* const save_replays = "save_replays";
	/** the scroll speed */
	static constexpr const char* const scroll = "scroll";
	/** how close the mouse needs to get to the edge of the screen to start scrolling */
	static constexpr const char* const scroll_threshold = "scroll_threshold";
	/** the most recently selected achievement group in the achievements dialog */
	static constexpr const char* const selected_achievement_group = "selected_achievement_group";
	/** whether to show an orb over allied units */
	static constexpr const char* const show_ally_orb = "show_ally_orb";
	/** whether to show an orb over disengaged units */
	static constexpr const char* const show_disengaged_orb = "show_disengaged_orb";
	/** whether to show an orb over enemy units */
	static constexpr const char* const show_enemy_orb = "show_enemy_orb";
	/** whether to show an orb over units that have used all their actions */
	static constexpr const char* const show_moved_orb = "show_moved_orb";
	/** whether to show an orb over units that have only use some of their actions */
	static constexpr const char* const show_partial_orb = "show_partial_orb";
	/** whether to show the team colored circle under units */
	static constexpr const char* const show_side_colors = "show_side_colors";
	/** whether to show unit status (moved, unmoved, etc) for allied units as well */
	static constexpr const char* const show_status_on_ally_orb = "show_status_on_ally_orb";
	/** whether to show an orb over units that haven't done anything */
	static constexpr const char* const show_unmoved_orb = "show_unmoved_orb";
	/** whether to randomly assign sides in multiplayer games */
	static constexpr const char* const shuffle_sides = "shuffle_sides";
	/** whether to skip move animations for AI actions */
	static constexpr const char* const skip_ai_moves = "skip_ai_moves";
	/** whether to skip animation of all actions when joining an in-progress multiplayer game */
	static constexpr const char* const skip_mp_replay = "skip_mp_replay";
	/** whether to play non-UI sounds */
	static constexpr const char* const sound = "sound";
	/** audio buffer size */
	static constexpr const char* const sound_buffer_size = "sound_buffer_size";
	/** the volume for playing sounds */
	static constexpr const char* const sound_volume = "sound_volume";
	/** list of the last selected single player modifications */
	static constexpr const char* const sp_modifications = "sp_modifications";
	/** whether to continue playing music when wesnoth isn't the focused window */
	static constexpr const char* const stop_music_in_background = "stop_music_in_background";
	/** the ThemeWML theme */
	static constexpr const char* const theme = "theme";
	/** hex size used to determine zoom level */
	static constexpr const char* const tile_size = "tile_size";
	/** whether to enable accelerated animation speed */
	static constexpr const char* const turbo = "turbo";
	/** how much to accelerate animation speed */
	static constexpr const char* const turbo_speed = "turbo_speed";
	/** whether to play a sound when it's your turn */
	static constexpr const char* const turn_bell = "turn_bell";
	/** whether to show a dialog and add a blindfold between turns */
	static constexpr const char* const turn_dialog = "turn_dialog";
	/** whether to play sounds when clicking UI elements */
	static constexpr const char* const ui_sound = "UI_sound";
	/** how loud to make the sound when clicking UI elements */
	static constexpr const char* const ui_volume = "UI_volume";
	/** whether to show standing animations */
	static constexpr const char* const unit_standing_animations = "unit_standing_animations";
	/** the color of the orb over units that haven't done anything */
	static constexpr const char* const unmoved_orb_color = "unmoved_orb_color";
	/** whether to lock the FPS to the screen refresh rate */
	static constexpr const char* const vsync = "vsync";
	/** width of the wesnoth window */
	static constexpr const char* const xresolution = "xresolution";
	/** height of the wesnoth window */
	static constexpr const char* const yresolution = "yresolution";

	//
	// advanced preferences
	// these are also set via the preferences dialog without using their explicit setter methods
	//
	/** whether to show a confirmation dialog for deleting save files */
	static constexpr const char* const ask_delete = "ask_delete";
	/** how many minutes to wait before removing chat messages */
	static constexpr const char* const chat_message_aging = "chat_message_aging";
	/** whether to use a color cursor */
	static constexpr const char* const color_cursors = "color_cursors";
	/** what compression to use for save files, if any */
	static constexpr const char* const compress_saves = "compress_saves";
	/** whether to ask for confirmation when loading a save from a different version of wesnoth */
	static constexpr const char* const confirm_load_save_from_different_version = "confirm_load_save_from_different_version";
	/** whether to use monte carlo instead of exact calculations for fight outcomes */
	static constexpr const char* const damage_prediction_allow_monte_carlo_simulation = "damage_prediction_allow_monte_carlo_simulation";
	/** how many recent files to keep a list of */
	static constexpr const char* const editor_max_recent_files = "editor_max_recent_files";
	/** additional scaling for text on top of the pixel scale multiplier */
	static constexpr const char* const font_scale = "font_scale";
	/** how long to wait before assuming a connection to the multiplayer server has been lost */
	static constexpr const char* const keepalive_timeout = "keepalive_timeout";
	/** whether to open a new chat tab in the multiplayer lobby after sending a whisper */
	static constexpr const char* const lobby_auto_open_whisper_windows = "lobby_auto_open_whisper_windows";
	/** whether holding middle click and moving the mouse will scroll around the map */
	static constexpr const char* const middle_click_scrolls = "middle_click_scrolls";
	/** whether moving the mouse to the sides of the window scrolls around the map */
	static constexpr const char* const mouse_scrolling = "mouse_scrolling";
	/** whether to scroll to a unit when an action is taken */
	static constexpr const char* const scroll_to_action = "scroll_to_action";
	/** keep scrolling when the mouse moves outside the wesnoth window */
	static constexpr const char* const scroll_when_mouse_outside = "scroll_when_mouse_outside";
	/** whether to show all units in the help rather than only the units encountered so far */
	static constexpr const char* const show_all_units_in_help = "show_all_units_in_help";
	/** whether to show combat animations */
	static constexpr const char* const show_combat = "show_combat";
	/** whether to show deprecation warnings for WML, lua, etc APIs */
	static constexpr const char* const show_deprecation = "show_deprecation";
	/** whether to use a 12 hour vs 24 hours clock in various places on the UI */
	static constexpr const char* const use_twelve_hour_clock_format = "use_twelve_hour_clock_format";

	ENUM_AND_ARRAY(
		_last_cache_cleaned_ver,
		addon_manager_saved_order_direction,
		addon_manager_saved_order_name,
		alias,
		allow_observers,
		ally_orb_color,
		ally_sighted_interrupts,
		animate_map,
		animate_water,
		auto_pixel_scale,
		auto_save_max,
		bell_volume,
		blindfold_replay,
		campaign_server,
		chat_lines,
		chat_timestamp,
		completed_campaigns,
		confirm_end_turn,
		core,
		custom_command,
		delete_saves,
		dir_bookmarks,
		disable_auto_moves,
		draw_delay,
		editor_auto_update_transitions,
		editor_chosen_addon,
		editor_default_dir,
		editor_draw_hex_coordinates,
		editor_draw_num_of_bitmaps,
		editor_draw_terrain_codes,
		editor_recent_files,
		enable_planning_mode_on_start,
		encountered_terrain_list,
		encountered_units,
		enemy_orb_color,
		fi_blocked_in_game,
		fi_friends_in_game,
		fi_invert,
		fi_vacant_slots,
		floating_labels,
		fullscreen,
		grid,
		gui2_theme,
		hide_whiteboard,
		history,
		host,
		idle_anim,
		idle_anim_rate,
		lobby_joins,
		lobby_whisper_friends_only,
		locale,
		login,
		maximized,
		message_bell,
		minimap_draw_terrain,
		minimap_draw_units,
		minimap_draw_villages,
		minimap_movement_coding,
		minimap_terrain_coding,
		moved_orb_color,
		mp_countdown,
		mp_countdown_action_bonus,
		mp_countdown_init_time,
		mp_countdown_reservoir_time,
		mp_countdown_turn_bonus,
		mp_era,
		mp_fog,
		mp_level,
		mp_level_type,
		mp_modifications,
		mp_random_start_time,
		mp_server_program_name,
		mp_server_warning_disabled,
		mp_shroud,
		mp_turns,
		mp_use_map_settings,
		mp_village_gold,
		mp_village_support,
		mp_xp_modifier,
		music,
		music_volume,
		options,
		partial_orb_color,
		pixel_scale,
		random_faction_mode,
		remember_password,
		sample_rate,
		save_replays,
		scroll,
		scroll_threshold,
		selected_achievement_group,
		show_ally_orb,
		show_disengaged_orb,
		show_enemy_orb,
		show_moved_orb,
		show_partial_orb,
		show_side_colors,
		show_status_on_ally_orb,
		show_unmoved_orb,
		shuffle_sides,
		skip_ai_moves,
		skip_mp_replay,
		sound,
		sound_buffer_size,
		sound_volume,
		sp_modifications,
		stop_music_in_background,
		theme,
		tile_size,
		turbo,
		turbo_speed,
		turn_bell,
		turn_dialog,
		ui_sound,
		ui_volume,
		unit_standing_animations,
		unmoved_orb_color,
		vsync,
		xresolution,
		yresolution,
		ask_delete,
		chat_message_aging,
		color_cursors,
		compress_saves,
		confirm_load_save_from_different_version,
		damage_prediction_allow_monte_carlo_simulation,
		editor_max_recent_files,
		font_scale,
		keepalive_timeout,
		lobby_auto_open_whisper_windows,
		middle_click_scrolls,
		mouse_scrolling,
		scroll_to_action,
		scroll_when_mouse_outside,
		show_all_units_in_help,
		show_combat,
		show_deprecation,
		use_twelve_hour_clock_format
	)
};
using prefs_list = string_enums::enum_base<preferences_list_defines>;
