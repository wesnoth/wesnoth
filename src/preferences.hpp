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

#include "config.hpp"
#include "display.hpp"

#include <string>
#include <utility>

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

	const std::string& locale();
	void set_locale(const std::string& s);

	int music_volume();
	void set_music_volume(int vol);

	int sound_volume();
	void set_sound_volume(int vol);

	void mute(bool muted);
	bool is_muted();

	bool grid();
	void set_grid(bool ison);

	const std::string& network_host();
	void set_network_host(const std::string& host);

	const std::string& login();
	void set_login(const std::string& username);

	int scroll_speed();
	void set_scroll_speed(int scroll);

	bool turn_bell();
	void set_turn_bell(bool ison);

	bool message_bell();
	void set_message_bell(bool ison);

	bool turn_dialog();
	void set_turn_dialog(bool ison);

	bool show_combat();

	bool show_ai_moves();

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

	std::string client_type();

	void set_theme(const std::string& theme);
	const std::string& theme();

	void show_preferences_dialog(display& disp);
	bool show_video_mode_dialog(display& disp);
	void show_hotkeys_dialog (display & disp);

	// Ask for end turn confirmation
	bool yellow_confirm();
	bool green_confirm();
	bool confirm_no_moves();
}

#endif
