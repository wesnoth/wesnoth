/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

class display;

#include "config.hpp"
#include "terrain/translation.hpp"
#include "utils/make_enum.hpp"

#include <utility>

namespace hotkey {
	class hotkey_item;
}

namespace preferences {

	struct base_manager
	{
		base_manager();
		~base_manager();
	};

	extern const int min_window_width;
	extern const int min_window_height;

	extern const int def_window_width;
	extern const int def_window_height;

	extern const int min_font_scaling;
	extern const int max_font_scaling;

	MAKE_ENUM(SCALING_ALGORITHM,
		(LINEAR, "linear")
		(NEAREST_NEIGHBOR, "nn")
		(XBRZ_LIN, "xbrzlin")
		(XBRZ_NN, "xbrznn")
	)

	extern const SCALING_ALGORITHM default_scaling_algorithm;

	void write_preferences();

	void set(const std::string& key, const std::string &value);
	void set(const std::string& key, char const *value);
	void set(const std::string& key, bool value);
	void set(const std::string& key, int value);
	void set(const std::string& key, const config::attribute_value& value);
	void clear(const std::string& key);
	void set_child(const std::string& key, const config& val);
	const config &get_child(const std::string &key);
	std::string get(const std::string& key);
	std::string get(const std::string& key, const std::string& def);
	bool get(const std::string& key, bool def);
	config::attribute_value get_as_attribute(const std::string& key);
	void erase(const std::string& key);
	bool have_setting(const std::string& key);

	void disable_preferences_save();

	config* get_prefs();

	std::string core_id();
	void set_core_id(const std::string& root);

	bool scroll_to_action();
	void set_scroll_to_action(bool ison);

	std::pair<int,int> resolution();
	void _set_resolution(const std::pair<int,int>& res);

	bool maximized();
	void _set_maximized(bool ison);

	bool fullscreen();
	void _set_fullscreen(bool ison);

	bool turbo();
	void _set_turbo(bool ison);

	double turbo_speed();
	void save_turbo_speed(const double speed);

	int font_scaling();
	void set_font_scaling(int scale);
	int font_scaled(int size);

	bool idle_anim();
	void _set_idle_anim(const bool ison);

	int idle_anim_rate();
	void _set_idle_anim_rate(const int rate);

	std::string language();
	void set_language(const std::string& s);

	std::string gui_theme();
	void set_gui_theme(const std::string& s);

	// Don't rename it to sound() because of a gcc-3.3 branch bug,
	// which will cause it to conflict with the sound namespace.
	bool sound_on();
	bool set_sound(bool ison);

	unsigned int sample_rate();
	void save_sample_rate(const unsigned int rate);

	size_t sound_buffer_size();
	void save_sound_buffer_size(const size_t size);

	int sound_volume();
	void set_sound_volume(int vol);

	int bell_volume();
	void set_bell_volume(int vol);

	int UI_volume();
	void set_UI_volume(int vol);

	bool music_on();
	bool set_music(bool ison);

	int music_volume();
	void set_music_volume(int vol);

	bool stop_music_in_background();
	void set_stop_music_in_background(bool ison);

	bool turn_bell();
	bool set_turn_bell(bool ison);

	bool UI_sound_on();
	bool set_UI_sound(bool ison);

	bool message_bell();

	// Proxies for preferences_dialog
	void load_hotkeys();
	void save_hotkeys();
	void clear_hotkeys();

	void add_alias(const std::string& alias, const std::string& command);
	const config &get_alias();


	std::string allied_color();
	void set_allied_color(const std::string& color_id);

	std::string enemy_color();
	void set_enemy_color(const std::string& color_id);

	std::string unmoved_color();
	void set_unmoved_color(const std::string& color_id);

	std::string partial_color();
	void set_partial_color(const std::string& color_id);

	std::string moved_color();
	void set_moved_color(const std::string& color_id);


	bool show_allied_orb();
	void set_show_allied_orb(bool show_orb);

	bool show_enemy_orb();
	void set_show_enemy_orb(bool show_orb);

	bool show_moved_orb();
	void set_show_moved_orb(bool show_orb);

	bool show_unmoved_orb();
	void set_show_unmoved_orb(bool show_orb);

	bool show_partial_orb();
	void set_show_partial_orb(bool show_orb);


	bool use_color_cursors();
	void _set_color_cursors(bool value);

	bool joystick_support_enabled();
	int joystick_mouse_deadzone();
	int joystick_num_mouse_xaxis();
	int joystick_num_mouse_yaxis();
	int joystick_mouse_xaxis_num();
	int joystick_mouse_yaxis_num();

	int joystick_scroll_deadzone();
	int joystick_num_scroll_xaxis();
	int joystick_num_scroll_yaxis();
	int joystick_scroll_xaxis_num();
	int joystick_scroll_yaxis_num();

	int joystick_cursor_deadzone();
	int joystick_num_cursor_xaxis();
	int joystick_num_cursor_yaxis();
	int joystick_cursor_xaxis_num();
	int joystick_cursor_yaxis_num();
	int joystick_cursor_threshold();

	int joystick_thrusta_deadzone();
	int joystick_num_thrusta_axis();
	int joystick_thrusta_axis_num();

	int joystick_thrustb_deadzone();
	int joystick_num_thrustb_axis();
	int joystick_thrustb_axis_num();

	int scroll_speed();
	void set_scroll_speed(const int scroll);

	bool middle_click_scrolls();
	bool mouse_scroll_enabled();
	void enable_mouse_scroll(bool value);

	/**
	 * Gets the threshold for when to scroll.
	 *
	 * This scrolling happens when the mouse is in the application and near
	 * the border.
	 */
	int mouse_scroll_threshold();

	int draw_delay();
	void set_draw_delay(int value);

	bool animate_map();
	void set_animate_map(bool value);

	bool animate_water();
	void set_animate_water(bool value);

	bool minimap_movement_coding();
	void toggle_minimap_movement_coding();

	bool minimap_terrain_coding();
	void toggle_minimap_terrain_coding();

	bool minimap_draw_units();
	void toggle_minimap_draw_units();

	bool minimap_draw_villages();
	void toggle_minimap_draw_villages();

	bool minimap_draw_terrain();
	void toggle_minimap_draw_terrain();

	bool show_standing_animations();
	void set_show_standing_animations(bool value);

	bool show_fps();
	void set_show_fps(bool value);

	bool ellipses();
	void set_ellipses(bool ison);

	bool grid();
	void _set_grid(bool ison);

	bool confirm_load_save_from_different_version();

	bool use_twelve_hour_clock_format();
	void set_use_twelve_hour_clock_format(bool value);

	bool disable_auto_moves();
	void set_disable_auto_moves(bool value);

	bool disable_loadingscreen_animation();
	void set_disable_loadingscreen_animation(bool value);

	bool damage_prediction_allow_monte_carlo_simulation();
	void set_damage_prediction_allow_monte_carlo_simulation(bool value);

} // end namespace preferences
