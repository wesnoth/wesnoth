/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

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

	struct display_manager
	{
		display_manager(display* disp);
		~display_manager();
	};

	bool fullscreen();
	void set_fullscreen(bool ison);

	std::pair<int,int> resolution();
	void set_resolution(const std::pair<int,int>& res);

	bool turbo();
	void set_turbo(bool ison);

	const std::string& language();
	void set_language(const std::string& s);

	int music_volume();
	void set_music_volume(int vol);

	int sound_volume();
	void set_sound_volume(int vol);

	void mute(bool muted);
	bool is_muted();

	bool adjust_gamma();
	void set_adjust_gamma(bool val);

	int gamma();
	void set_gamma(int gamma);

	bool grid();
	void set_grid(bool ison);

	const std::string& official_network_host();

	const std::string& network_host();
	void set_network_host(const std::string& host);

	const std::string& campaign_server();
	void set_campaign_server(const std::string& host);

	const std::string& login();
	void set_login(const std::string& username);

	int scroll_speed();
	void set_scroll_speed(int scroll);

	bool turn_bell();
	void set_turn_bell(bool ison);

	bool message_bell();
	void set_message_bell(bool ison);

	const std::string& turn_cmd();
	void set_turn_cmd(const std::string& cmd);

	bool turn_dialog();
	void set_turn_dialog(bool ison);

	bool show_combat();

	bool allow_observers();
	void set_allow_observers(bool value);

	bool fog();
	void set_fog(bool value);

	bool shroud();
	void set_shroud(bool value);

	int turns();
	void set_turns(int value);

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
	void set_colour_cursors(bool value);

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

	bool compress_saves();

	std::set<std::string> &encountered_units();
	std::set<std::string> &encountered_terrains();

	enum CACHE_SAVES_METHOD { CACHE_SAVES_ASK, CACHE_SAVES_NEVER, CACHE_SAVES_ALWAYS };
	CACHE_SAVES_METHOD cache_saves();
	void set_cache_saves(CACHE_SAVES_METHOD method);

	std::string client_type();

	void set_theme(const std::string& theme);
	const std::string& theme();

	void show_preferences_dialog(display& disp, const config& game_cfg);
	bool show_video_mode_dialog(display& disp);
	// If prefs is non-null, save the hotkeys in that config instead of
	// the default.
	void show_hotkeys_dialog (display & disp, config *prefs=NULL);

	// Ask for end turn confirmation
	bool yellow_confirm();
	bool green_confirm();
	bool confirm_no_moves();
}

#endif
