/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

class CVideo;
class config;
class display;

#include <string>

namespace preferences {

    // FIXME: this box should be vertically centered on the screen, but is not
	static const int height = 400;
	static const int width = 465;

	struct display_manager
	{
		display_manager(display* disp);
		~display_manager();
	};


	/**
	 * Detect a good resolution.
	 *
	 * @param video               The video 'holding' the framebuffer.
	 * @param resolution          Any good resolution is returned through this reference.
	 * @param bpp                 A reference through which the best bpp is returned.
	 * @param video_flags         A reference through which the video flags for setting the video mode are returned.
	 *
	 * @returns                   Whether valid video settings were found.
	 */
	bool detect_video_settings(CVideo& video, std::pair<int,int>& resolution, int& bpp, int& video_flags);

	void set_fullscreen(CVideo& video, const bool ison);
	void set_fullscreen(bool ison);
	void set_scroll_to_action(bool ison);
	void set_resolution(const std::pair<int,int>& res);

	/**
	 * Set the resolution.
	 *
	 * @param video               The video 'holding' the framebuffer.
	 * @param width               The new width.
	 * @param height              The new height.
	 *
	 * @returns                   The status true if width and height are the
	 *                            size of the framebuffer, false otherwise.
	 */
	bool set_resolution(CVideo& video
			, const unsigned width, const unsigned height);
	void set_turbo(bool ison);
	void set_ellipses(bool ison);
	void set_grid(bool ison);
	void set_turbo_speed(double speed);
	void set_color_cursors(bool value);

	// Control unit idle animations
	void set_idle_anim(bool ison);
	void set_idle_anim_rate(int rate);

	std::string show_wesnothd_server_search(display&);
	void show_preferences_dialog(display& disp, const config& game_cfg);
	bool show_video_mode_dialog(display& disp);
	bool show_theme_dialog(display& disp);
	void show_paths_dialog(display& disp);
	void show_mp_alerts_dialog(display& disp);
	void show_advanced_graphics_dialog(display & disp);

	// If prefs is non-null, save the hotkeys in that config
	// instead of the default.
	void show_hotkeys_preferences_dialog(display & disp);
} // end namespace preferences

#endif
