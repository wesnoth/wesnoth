/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef PREFERENCES_HPP_INCLUDED
#define PREFERENCES_HPP_INCLUDED

class config;
class display;
class team;
struct game_state;

#include "game_config.hpp"
#include "unit.hpp"

#include <string>
#include <utility>
#include <set>

// only there temporary
#ifdef USE_TINY_GUI
const int min_allowed_width = 320;
const int min_allowed_height = 240;
#else
const int min_allowed_width = 800;
const int min_allowed_height = 600;
#endif

namespace preferences {

	struct manager
	{
		manager();
		~manager();
	};

	// low-level, should be seen only by preferences_display ?
	void set(std::string key, std::string value);
	std::string get(const std::string key);

	config* get_prefs();

	bool fullscreen();
	void _set_fullscreen(bool ison);

	std::pair<int,int> resolution();
	void _set_resolution(const std::pair<int,int>& res);

	bool turbo();
	void _set_turbo(bool ison);

	const std::string& language();
	void set_language(const std::string& s);

	// don't rename it to sound() because of a gcc-3.3 branch bug
	// which will cause it to conflict with the sound namespace
	bool sound_on();
	bool set_sound(bool ison);

	unsigned int sample_rate();
 	void save_sample_rate(const unsigned int rate);

	size_t sound_buffer_size();
	void save_sound_buffer_size(const size_t size);

	int sound_volume();
	void set_sound_volume(int vol);

	bool music_on();
	bool set_music(bool ison);

	int music_volume();
	void set_music_volume(int vol);

	bool adjust_gamma();
	void _set_adjust_gamma(bool val);

	int gamma();
	void _set_gamma(int gamma);

	bool grid();
	void _set_grid(bool ison);

	bool lobby_joins();
	void _set_lobby_joins(bool show);

	const std::vector<game_config::server_info>& server_list();

	const std::string& network_host();
	void set_network_host(const std::string& host);

	const std::string& campaign_server();
	void set_campaign_server(const std::string& host);

	const std::string& login();
	void set_login(const std::string& username);

	int scroll_speed();
	void set_scroll_speed(int scroll);
	bool mouse_scroll_enabled();
	void enable_mouse_scroll(bool value);

	bool turn_bell();
	void set_turn_bell(bool ison);

	bool message_bell();
	void set_message_bell(bool ison);

	bool turn_dialog();
	void set_turn_dialog(bool ison);

	bool show_combat();

	bool allow_observers();
	void set_allow_observers(bool value);

	bool use_map_settings();
	void set_use_map_settings(bool value);

	bool fog();
	void set_fog(bool value);

	bool shroud();
	void set_shroud(bool value);

	int turns();
	void set_turns(int value);

	bool skip_mp_replay();
	void set_skip_mp_replay(bool value);

	bool countdown();
	void set_countdown(bool value);
	int countdown_init_time();
	void set_countdown_init_time(int value);
	int countdown_turn_bonus();
	void set_countdown_turn_bonus(int value);
	int countdown_reservoir_time();
	void set_countdown_reservoir_time(int value);
	int countdown_action_bonus();
	void set_countdown_action_bonus(int value);

	int village_gold();
	void set_village_gold(int value);

	int xp_modifier();
	void set_xp_modifier(int value);

	int era();
	void set_era(int value);

	int map();
	void set_map(int value);

	bool show_ai_moves();
	void set_show_ai_moves(bool value);

	void set_show_side_colours(bool value);
	bool show_side_colours();

	void set_ask_delete_saves(bool value);
	bool ask_delete_saves();

	bool use_colour_cursors();
	void _set_colour_cursors(bool value);

	bool show_floating_labels();
	void set_show_floating_labels(bool value);

	bool message_private();
	void set_message_private(bool value);

	bool show_tip_of_day();
	void set_show_tip_of_day(bool value);

	bool show_haloes();
	void set_show_haloes(bool value);

	bool show_fps();
	void set_show_fps(bool value);

	bool flip_time();
	void set_flip_time(bool value);

	bool upload_log();
	void set_upload_log(bool value);
	const std::string &upload_id();

	// Multiplayer functions
	bool chat_timestamp();
	void set_chat_timestamp(bool value);

	int chat_lines();
	void set_chat_lines(int lines);

	bool compress_saves();

	std::set<std::string> &encountered_units();
	std::set<std::string> &encountered_terrains();

	enum CACHE_SAVES_METHOD { CACHE_SAVES_ASK, CACHE_SAVES_NEVER, CACHE_SAVES_ALWAYS };
	CACHE_SAVES_METHOD cache_saves();
	void set_cache_saves(CACHE_SAVES_METHOD method);

	std::string client_type();

        std::string clock_format();

	void set_theme(const std::string& theme);
	const std::string& theme();

	bool compare_resolutions(const std::pair<int,int>& lhs, const std::pair<int,int>& rhs);

	// Ask for end turn confirmation
	bool yellow_confirm();
	bool green_confirm();
	bool confirm_no_moves();

	// proxies for preferences_dialog
	void load_hotkeys();
	void save_hotkeys();

	// Add all recruitable units as encountered so that information
	// about them are displayed to the user in the help system.
	void encounter_recruitable_units(std::vector<team>& teams);
	// Add all units that exist at the start to the encountered units so
	// that information about them are displayed to the user in the help
	// system.
	void encounter_start_units(unit_map& units);
	// Add all units that are recallable as encountred units.
	void encounter_recallable_units(game_state& gamestate);
	// Add all terrains on the map as encountered terrains.
	void encounter_map_terrain(gamemap& map);
}

#endif
