/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <string>

namespace preferences {
	void set_preference_display_settings();

	void set_turbo(bool ison);
	void set_grid(bool ison);
	void set_turbo_speed(double speed);
	void set_color_cursors(bool value);

	// Control unit idle animations
	void set_idle_anim(bool ison);
	void set_idle_anim_rate(int rate);

	bool show_standing_animations();
	void set_show_standing_animations(bool value);

	void show_wesnothd_server_search();
	bool show_theme_dialog();
} // end namespace preferences
