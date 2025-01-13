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

#define ADDPREF(pref) static constexpr const char* const pref = #pref;

/**
 * Contains all valid preferences attributes.
 */
struct preferences_list_defines
{
	//
	// regular preferences
	//
	/** wesnoth version string when cache files were last deleted */
	ADDPREF(_last_cache_cleaned_ver)
	/** achievements completed for add-ons/UMC, are not steam achievements */
	ADDPREF(achievements)
	/** player names marked as either friends or as ignored */
	ADDPREF(acquaintance)
	/** whether to get the add-on icons when downloading the add-ons list */
	ADDPREF(addon_icons)
	/** the sort direction, ie: ascending */
	ADDPREF(addon_manager_saved_order_direction)
	/** the name of the column in the add-ons manager to use by default to sort results */
	ADDPREF(addon_manager_saved_order_name)
	/** set an alternate way to call a command, similar to linux terminal command aliases */
	ADDPREF(alias)
	/** whether a game should allow observers */
	ADDPREF(allow_observers)
	/** the orb color above allied units */
	ADDPREF(ally_orb_color)
	/** whether to stop movement of a unit when an allied unit comes into sight range */
	ADDPREF(ally_sighted_interrupts)
	/** whether to display animations on terrain (ie: windmill) */
	ADDPREF(animate_map)
	/** whether to animate water terrain */
	ADDPREF(animate_water)
	/** whether to automatically set the pixel scale multiplier based on the current window size */
	ADDPREF(auto_pixel_scale)
	/** the maximum number of autosaves to keep before deleting old ones */
	ADDPREF(auto_save_max)
	/** how loud the turn bell sound is */
	ADDPREF(bell_volume)
	/** whether to show the map before being given a side in online multiplayer */
	ADDPREF(blindfold_replay)
	/** the add-ons server name, ie: add-ons.wesnoth.org */
	ADDPREF(campaign_server)
	/** the number of lines of chat to display in-game */
	ADDPREF(chat_lines)
	/** whether to show a timestamp in in-game chat messages */
	ADDPREF(chat_timestamp)
	/** child tag name for completed campaign information */
	ADDPREF(completed_campaigns)
	/** whether to confirm ending your turn when units can still take action */
	ADDPREF(confirm_end_turn)
	/** the current core to use */
	ADDPREF(core)
	/**
	 * creates a single command composed of one or more other commands
	 * format - :custom show_terrain_codes;show_num_of_bitmaps
	 */
	ADDPREF(custom_command)
	/** whether to show a confirmation dialog when deleting a save */
	ADDPREF(delete_saves)
	/** additional folders shown when using the file selection dialog */
	ADDPREF(dir_bookmarks)
	/** whether to automatically move units that were previously told to move towards hexes more than one turn away */
	ADDPREF(disable_auto_moves)
	/** maximum FPS, if set, at which to refresh the screen */
	ADDPREF(refresh_rate)
	/** whether to have the editor automatically update terrain transitions when placing terrain immediately, after the mouse click is released, or not at all */
	ADDPREF(editor_auto_update_transitions)
	/** the current add-on being used in the editor */
	ADDPREF(editor_chosen_addon)
	/** whether to draw the x,y map coordinates in the editor */
	ADDPREF(editor_draw_hex_coordinates)
	/** number of images used to draw the hex */
	ADDPREF(editor_draw_num_of_bitmaps)
	/** whether to draw terrain codes on hexes in the editor */
	ADDPREF(editor_draw_terrain_codes)
	/** list of recently accessed files in the editor */
	ADDPREF(editor_recent_files)
	/** whether to display the active tool information help text at the bottom/top of the editor */
	ADDPREF(editor_help_text_shown)
	/** whether to automatically start in planning mode in-game */
	ADDPREF(enable_planning_mode_on_start)
	/** list of terrain seen so far */
	ADDPREF(encountered_terrain_list)
	/** list of units seen so far */
	ADDPREF(encountered_units)
	/** the color of the orb over enemy units */
	ADDPREF(enemy_orb_color)
	/** in the multiplayer lobby, show games with blocked players in them */
	ADDPREF(fi_blocked_in_game)
	/** in the multiplayer lobby, show games with friends in them */
	ADDPREF(fi_friends_in_game)
	/** in the multiplayer lobby, invert all other filters */
	ADDPREF(fi_invert)
	/** in the multiplayer lobby, show games with vacant slots */
	ADDPREF(fi_vacant_slots)
	/** whether to show floating labels on the game map */
	ADDPREF(floating_labels)
	/** whether to use fullscreen mode */
	ADDPREF(fullscreen)
	/** whether to show a hex grid overlay on the map */
	ADDPREF(grid)
	/** the gui2 theme name */
	ADDPREF(gui2_theme)
	/** whether to show teammate's whiteboard plans on the game map */
	ADDPREF(hide_whiteboard)
	/** list of lines of text entered into textboxes that support keeping a history */
	ADDPREF(history)
	/** the most recent multiplayer server hostname */
	ADDPREF(host)
	/** whether to play idle animations */
	ADDPREF(idle_anim)
	/** how frequently to play idle animations */
	ADDPREF(idle_anim_rate)
	/** who to show notifications about when they join the multiplayer lobby */
	ADDPREF(lobby_joins)
	/** whether to only accept whisper messages from friends */
	ADDPREF(lobby_whisper_friends_only)
	/** player chosen language to use */
	ADDPREF(locale)
	/** most recently use username for logging into the multiplayer server */
	ADDPREF(login)
	/** whether the window is maximized */
	ADDPREF(maximized)
	/** whether to play a sound when receiving a message */
	ADDPREF(message_bell)
	/** whether to draw terrain in the in-game minimap */
	ADDPREF(minimap_draw_terrain)
	/** whether to draw units on in-game/editor minimap */
	ADDPREF(minimap_draw_units)
	/** whether to draw villages on the in-game/editor minimap */
	ADDPREF(minimap_draw_villages)
	/**
	 * on the in-game minimap/editor minimap use a color for your side, a color for all allied sides, and a color for all enemy sides
	 * doesn't actually have anything to do with movement
	 */
	ADDPREF(minimap_movement_coding)
	/** simplified color coding by terrain type in the in-game minimap/editor */
	ADDPREF(minimap_terrain_coding)
	/** the color of the orb above a unit that can no longer move */
	ADDPREF(moved_orb_color)
	/** whether to enable the turn timer in multiplayer */
	ADDPREF(mp_countdown)
	/** seconds to add to the multiplayer turn timer when an action is taken */
	ADDPREF(mp_countdown_action_bonus)
	/** seconds for the multiplayer turn timer for each player's first turn */
	ADDPREF(mp_countdown_init_time)
	/** maximum seconds the multiplayer turn timer can have for a single turn */
	ADDPREF(mp_countdown_reservoir_time)
	/** bonus seconds for the multiplayer turn timer for turns after turn 1 */
	ADDPREF(mp_countdown_turn_bonus)
	/** the most recently played era in multiplayer */
	ADDPREF(mp_era)
	/** whether to enable fog in multiplayer games */
	ADDPREF(mp_fog)
	/** the id of the most recently played multiplayer scenario */
	ADDPREF(mp_level)
	/** most recently selected type of game: scenario, campaign, random map, etc */
	ADDPREF(mp_level_type)
	/** most recently selected mp playing mode/connection type */
	ADDPREF(mp_connect_type)
	/** list of the last selected multiplayer modifications */
	ADDPREF(mp_modifications)
	/** whether to use a random start time for the scenario */
	ADDPREF(mp_random_start_time)
	/** the name of the wesnothd executable */
	ADDPREF(mp_server_program_name)
	/** whether to show a warning dialog about starting wesnothd in the background when hosting LAN games */
	ADDPREF(mp_server_warning_disabled)
	/** whether to enable shroud in multiplayer games */
	ADDPREF(mp_shroud)
	/** the turn limit for multiplayer games */
	ADDPREF(mp_turns)
	/** whether to use the scenario's default settings */
	ADDPREF(mp_use_map_settings)
	/** the amount of gold each village gives */
	ADDPREF(mp_village_gold)
	/** the amount of unit upkeep each village offsets */
	ADDPREF(mp_village_support)
	/** the multiplier to apply to all units in a multiplayer game */
	ADDPREF(mp_xp_modifier)
	/** whether music is enabled */
	ADDPREF(music)
	/** the music's volume */
	ADDPREF(music_volume)
	/** custom multiplayer scenario options */
	ADDPREF(options)
	/** the orb above units with that can still take some actions */
	ADDPREF(partial_orb_color)
	/** the pixel scale multiplier to apply */
	ADDPREF(pixel_scale)
	/** whether to allow any type of mirrors in a multiplayer scenario */
	ADDPREF(random_faction_mode)
	/** whether to remember passwords typed into password fields */
	ADDPREF(remember_password)
	/** audio sample rate */
	ADDPREF(sample_rate)
	/** whether to save replays of games */
	ADDPREF(save_replays)
	/** the scroll speed */
	ADDPREF(scroll)
	/** how close the mouse needs to get to the edge of the screen to start scrolling */
	ADDPREF(scroll_threshold)
	/** the most recently selected achievement group in the achievements dialog */
	ADDPREF(selected_achievement_group)
	/** contains the list of any player-entered multiplayer servers */
	ADDPREF(server)
	/** whether to show an orb over allied units */
	ADDPREF(show_ally_orb)
	/** whether to show an orb over disengaged units */
	ADDPREF(show_disengaged_orb)
	/** whether to show an orb over enemy units */
	ADDPREF(show_enemy_orb)
	/** whether to show an orb over units that have used all their actions */
	ADDPREF(show_moved_orb)
	/** whether to show an orb over units that have only use some of their actions */
	ADDPREF(show_partial_orb)
	/** whether to show the team colored circle under units */
	ADDPREF(show_side_colors)
	/** whether to show unit status (moved, unmoved, etc) for allied units as well */
	ADDPREF(show_status_on_ally_orb)
	/** whether to show the tips panel on titlescreen */
	ADDPREF(show_tips)
	/** whether to show an orb over units that haven't done anything */
	ADDPREF(show_unmoved_orb)
	/** whether to randomly assign sides in multiplayer games */
	ADDPREF(shuffle_sides)
	/** whether to skip move animations for AI actions */
	ADDPREF(skip_ai_moves)
	/** whether to skip animation of all actions when joining an in-progress multiplayer game */
	ADDPREF(skip_mp_replay)
	/** whether to play non-UI sounds */
	ADDPREF(sound)
	/** audio buffer size */
	ADDPREF(sound_buffer_size)
	/** the volume for playing sounds */
	ADDPREF(sound_volume)
	/** list of the last selected single player modifications */
	ADDPREF(sp_modifications)
	/** whether to continue playing music when wesnoth isn't the focused window */
	ADDPREF(stop_music_in_background)
	/** the ThemeWML theme */
	ADDPREF(theme)
	/** hex size used to determine zoom level */
	ADDPREF(tile_size)
	/** whether to enable accelerated animation speed */
	ADDPREF(turbo)
	/** how much to accelerate animation speed */
	ADDPREF(turbo_speed)
	/** whether to play a sound when it's your turn */
	ADDPREF(turn_bell)
	/** whether to show a dialog and add a blindfold between turns */
	ADDPREF(turn_dialog)
	/** whether to play sounds when clicking UI elements */
	ADDPREF(ui_sound)
	/** how loud to make the sound when clicking UI elements */
	ADDPREF(ui_volume)
	/** whether to show standing animations */
	ADDPREF(unit_standing_animations)
	/** the color of the orb over units that haven't done anything */
	ADDPREF(unmoved_orb_color)
	/** whether to lock the FPS to the screen refresh rate */
	ADDPREF(vsync)
	/** width of the wesnoth window */
	ADDPREF(xresolution)
	/** height of the wesnoth window */
	ADDPREF(yresolution)
	//
	// MP alert preferences
	// Note, this list of items must match those ids defined in data/gui/dialogs/mp_alerts_options.cfg
	//
	/** whether to play a sound when a player joins the game you're in */
	ADDPREF(player_joins_sound)
	/** whether to show a notification when a player joins the game you're in */
	ADDPREF(player_joins_notif)
	/** whether to show the enabled player join sound or notification in the lobby as well */
	ADDPREF(player_joins_lobby)
	/** whether to play a sound when a player leaves the game you're in */
	ADDPREF(player_leaves_sound)
	/** whether to show a notification when a player leaves the game you're in */
	ADDPREF(player_leaves_notif)
	/** whether to show the enabled player leave sound or notification in the lobby as well */
	ADDPREF(player_leaves_lobby)
	/** whether to play a sound when receiving a private message aka whisper */
	ADDPREF(private_message_sound)
	/** whether to show a notification when receiving a private message aka whisper */
	ADDPREF(private_message_notif)
	/** whether to show the enabled private message aka whisper join sound or notification in the lobby as well */
	ADDPREF(private_message_lobby)
	/** whether to play a sound when a friend messages you while in game */
	ADDPREF(friend_message_sound)
	/** whether to show a notification when a friend messages you while in game */
	ADDPREF(friend_message_notif)
	/** whether to show the enabled friend message sound or notification in the lobby as well */
	ADDPREF(friend_message_lobby)
	/** whether to play a sound when a public message is sent */
	ADDPREF(public_message_sound)
	/** whether to show a notification when a public message is sent */
	ADDPREF(public_message_notif)
	/** whether to show the enabled public message sound or notification in the lobby as well */
	ADDPREF(public_message_lobby)
	/** whether to play a sound when a server message is sent */
	ADDPREF(server_message_sound)
	/** whether to show a notification when a server message is sent */
	ADDPREF(server_message_notif)
	/** whether to show the enabled server message sound or notification in the lobby as well */
	ADDPREF(server_message_lobby)
	/** whether to play a sound when the game is ready to be started */
	ADDPREF(ready_for_start_sound)
	/** whether to show a notification when the game is ready to be started */
	ADDPREF(ready_for_start_notif)
	/** used to make a UI element invisible in the mp alerts options dialog */
	ADDPREF(ready_for_start_lobby)
	/** whether to play a sound when the game has started */
	ADDPREF(game_has_begun_sound)
	/** whether to show a notification when the game has started */
	ADDPREF(game_has_begun_notif)
	/** used to make a UI element invisible in the mp alerts options dialog */
	ADDPREF(game_has_begun_lobby)
	/** used to make a UI element invisible in the mp alerts options dialog */
	ADDPREF(turn_changed_sound)
	/** whether to show a notification when the turn changes */
	ADDPREF(turn_changed_notif)
	/** used to make a UI element invisible in the mp alerts options dialog */
	ADDPREF(turn_changed_lobby)
	/** whether to play a sound when a new game is created */
	ADDPREF(game_created_sound)
	/** whether to show a notification when a new game is created */
	ADDPREF(game_created_notif)
	/** whether to show the new game creation message sound or notification in the lobby as well */
	ADDPREF(game_created_lobby)

	//
	// advanced preferences
	// these are also set via the preferences dialog without using their explicit setter methods
	//
	/** whether to show a confirmation dialog for deleting save files */
	ADDPREF(ask_delete)
	/** how many minutes to wait before removing chat messages */
	ADDPREF(chat_message_aging)
	/** whether to use a color cursor */
	ADDPREF(color_cursors)
	/** what compression to use for save files, if any */
	ADDPREF(compress_saves)
	/** whether to ask for confirmation when loading a save from a different version of wesnoth */
	ADDPREF(confirm_load_save_from_different_version)
	/** whether to use monte carlo instead of exact calculations for fight outcomes */
	ADDPREF(damage_prediction_allow_monte_carlo_simulation)
	/** how many recent files to keep a list of */
	ADDPREF(editor_max_recent_files)
	/** additional scaling for text on top of the pixel scale multiplier */
	ADDPREF(font_scale)
	/** how long to wait before assuming a connection to the multiplayer server has been lost */
	ADDPREF(keepalive_timeout)
	/** whether to open a new chat tab in the multiplayer lobby after sending a whisper */
	ADDPREF(lobby_auto_open_whisper_windows)
	/** whether holding middle click and moving the mouse will scroll around the map */
	ADDPREF(middle_click_scrolls)
	/** whether moving the mouse to the sides of the window scrolls around the map */
	ADDPREF(mouse_scrolling)
	/** whether to scroll to a unit when an action is taken */
	ADDPREF(scroll_to_action)
	/** keep scrolling when the mouse moves outside the wesnoth window */
	ADDPREF(scroll_when_mouse_outside)
	/** whether to show all units in the help rather than only the units encountered so far */
	ADDPREF(show_all_units_in_help)
	/** whether to show combat animations */
	ADDPREF(show_combat)
	/** whether to show deprecation warnings for WML, lua, etc APIs */
	ADDPREF(show_deprecation)
	/** whether to use a 12 hour vs 24 hours clock in various places on the UI */
	ADDPREF(use_twelve_hour_clock_format)

	ENUM_AND_ARRAY(
		_last_cache_cleaned_ver,
		achievements,
		acquaintance,
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
		refresh_rate,
		editor_auto_update_transitions,
		editor_chosen_addon,
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
		mp_connect_type,
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
		server,
		show_ally_orb,
		show_disengaged_orb,
		show_enemy_orb,
		show_moved_orb,
		show_partial_orb,
		show_side_colors,
		show_status_on_ally_orb,
		show_tips,
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
		use_twelve_hour_clock_format,
		player_joins_sound,
		player_joins_notif,
		player_joins_lobby,
		player_leaves_sound,
		player_leaves_notif,
		player_leaves_lobby,
		private_message_sound,
		private_message_notif,
		private_message_lobby,
		friend_message_sound,
		friend_message_notif,
		friend_message_lobby,
		public_message_sound,
		public_message_notif,
		public_message_lobby,
		server_message_sound,
		server_message_notif,
		server_message_lobby,
		ready_for_start_sound,
		ready_for_start_notif,
		ready_for_start_lobby,
		game_has_begun_sound,
		game_has_begun_notif,
		game_has_begun_lobby,
		turn_changed_sound,
		turn_changed_notif,
		turn_changed_lobby,
		game_created_sound,
		game_created_notif,
		game_created_lobby,
		addon_icons
	)
};
using prefs_list = string_enums::enum_base<preferences_list_defines>;
