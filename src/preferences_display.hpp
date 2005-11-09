/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef PREFERENCES_DISPLAY_HPP_INCLUDED
#define PREFERENCES_DISPLAY_HPP_INCLUDED

#include "preferences.hpp"

namespace preferences {

	struct display_manager
	{
		display_manager(display* disp);
		~display_manager();
	};

	void set_fullscreen(bool ison);
	void set_resolution(const std::pair<int,int>& res);
	void set_turbo(bool ison);
	void set_adjust_gamma(bool val);
	void set_gamma(int gamma);
	void set_grid(bool ison);
	void set_colour_cursors(bool value);

	void show_preferences_dialog(display& disp, const config& game_cfg);
	bool show_video_mode_dialog(display& disp);
	// If prefs is non-null, save the hotkeys in that config instead of
	// the default.
	void show_hotkeys_dialog (display & disp, config *prefs=NULL);
}

#endif
