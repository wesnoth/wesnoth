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

	double music_volume();
	void set_music_volume(double vol);

	double sound_volume();
	void set_sound_volume(double vol);

	bool grid();
	void set_grid(bool ison);

	const std::string& network_host();
	void set_network_host(const std::string& host);

	const std::string& login();
	void set_login(const std::string& username);

	double scroll_speed();
	double get_scroll_speed();
	void set_scroll_speed(double scroll);

	void show_preferences_dialog(display& disp);
	void show_video_mode_dialog(display& disp);
}

#endif
