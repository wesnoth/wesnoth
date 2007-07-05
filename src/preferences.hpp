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

#include "game_config.hpp"
#include "terrain_translation.hpp"
#include "config.hpp"

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

	struct base_manager
	{
		base_manager();
		~base_manager();
	};

	// low-level, should be seen only by preferences_display ?
	void set(std::string key, std::string value);
	std::string get(const std::string key);
	void erase(const std::string key);

	void disable_preferences_save();

	config* get_prefs();

	bool fullscreen();
	void _set_fullscreen(bool ison);

	std::pair<int,int> resolution();
	void _set_resolution(const std::pair<int,int>& res);

	bool turbo();
	void _set_turbo(bool ison);

	double turbo_speed();
	void save_turbo_speed(const double speed);

	const std::string& language();
	void set_language(const std::string& s);

	// proxies for preferences_dialog
	void load_hotkeys();
	void save_hotkeys();

	int scroll_speed();
	void set_scroll_speed(int scroll);

	int draw_delay();
	void set_draw_delay(int value);

	bool show_fps();
	void set_show_fps(bool value);

	bool grid();
	void _set_grid(bool ison);
}

#endif
