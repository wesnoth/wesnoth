/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PREFERENCES_DISPLAY_HPP_INCLUDED
#define PREFERENCES_DISPLAY_HPP_INCLUDED

class config;
class display;
class CVideo;
#include <string>

namespace preferences {

	enum DIALOG_OPEN_TO {
		VIEW_DEFAULT,
		VIEW_FRIENDS
	};

	void set_scroll_to_action(bool ison);

	void set_turbo(bool ison);
	void set_ellipses(bool ison);
	void set_grid(bool ison);
	void set_turbo_speed(double speed);
	void set_color_cursors(bool value);

	// Control unit idle animations
	void set_idle_anim(bool ison);
	void set_idle_anim_rate(int rate);

	std::string show_wesnothd_server_search(CVideo&);
	void show_preferences_dialog(CVideo& disp, const config& game_cfg,
		const DIALOG_OPEN_TO initial_view = VIEW_DEFAULT);
	bool show_theme_dialog(CVideo& disp);
} // end namespace preferences

#endif
