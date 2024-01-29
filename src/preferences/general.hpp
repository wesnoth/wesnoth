/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "gui/sort_order.hpp"
#include "terrain/translation.hpp"

#include <utility>

struct point;

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

	extern const int max_window_width;
	extern const int max_window_height;

	extern const int min_font_scaling;
	extern const int max_font_scaling;

	extern const int min_pixel_scale;
	extern const int max_pixel_scale;

	void write_preferences();

	void set(const std::string& key, const std::string &value);
	void set(const std::string& key, char const *value);
	void set(const std::string& key, bool value);
	void set(const std::string& key, int value);
	void set(const std::string& key, const config::attribute_value& value);
	void clear(const std::string& key);
	void set_child(const std::string& key, const config& val);
	optional_const_config get_child(const std::string &key);
	std::string get(const std::string& key);
	std::string get(const std::string& key, const std::string& def);
	bool get(const std::string& key, bool def);
	config::attribute_value get_as_attribute(const std::string& key);
	void erase(const std::string& key);
	bool have_setting(const std::string& key);

	void disable_preferences_save();

	config* get_prefs();
	void load_base_prefs();

	std::string core_id();
	void set_core_id(const std::string& root);

	bool scroll_to_action();
	void set_scroll_to_action(bool ison);

	point resolution();
	void _set_resolution(const point& res);

	int pixel_scale();
	void set_pixel_scale(const int scale);

	bool auto_pixel_scale();
	void set_auto_pixel_scale(bool choice);

	bool maximized();
	void _set_maximized(bool ison);

	bool fullscreen();
	void _set_fullscreen(bool ison);

	bool vsync();
	void set_vsync(bool ison);

	bool turbo();
	void set_turbo(bool ison);

	double turbo_speed();
	void set_turbo_speed(const double speed);

	int font_scaling();
	void set_font_scaling(int scale);
	int font_scaled(int size);

	int keepalive_timeout();
	void keepalive_timeout(int seconds);

	bool idle_anim();
	void set_idle_anim(const bool ison);

	double idle_anim_rate();
	void set_idle_anim_rate(const int rate);

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

	std::size_t sound_buffer_size();
	void save_sound_buffer_size(const std::size_t size);

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

	unsigned int tile_size();
	void set_tile_size(const unsigned int size);

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
	optional_const_config get_alias();


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

	bool show_ally_orb();
	void set_show_ally_orb(bool show_orb);

	bool show_status_on_ally_orb();
	void set_show_status_on_ally_orb(bool show_orb);

	bool show_enemy_orb();
	void set_show_enemy_orb(bool show_orb);

	bool show_moved_orb();
	void set_show_moved_orb(bool show_orb);

	bool show_unmoved_orb();
	void set_show_unmoved_orb(bool show_orb);

	bool show_partial_orb();
	void set_show_partial_orb(bool show_orb);

	bool show_disengaged_orb();
	void set_show_disengaged_orb(bool show_orb);

	bool use_color_cursors();
	void _set_color_cursors(bool value);

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

	bool show_fps();
	void set_show_fps(bool value);

	bool ellipses();
	void set_ellipses(bool ison);

	bool grid();
	void set_grid(bool ison);

	bool confirm_load_save_from_different_version();

	bool use_twelve_hour_clock_format();
	void set_use_twelve_hour_clock_format(bool value);

	bool disable_auto_moves();
	void set_disable_auto_moves(bool value);

	bool damage_prediction_allow_monte_carlo_simulation();
	void set_damage_prediction_allow_monte_carlo_simulation(bool value);

	std::string addon_manager_saved_order_name();
	void set_addon_manager_saved_order_name(const std::string& value);

	sort_order::type addon_manager_saved_order_direction();
	void set_addon_manager_saved_order_direction(sort_order::type value);

	std::string selected_achievement_group();
	void set_selected_achievement_group(const std::string& content_for);

	/**
	 * @param content_for The achievement group the achievement is part of.
	 * @param id The ID of the achievement within the achievement group.
	 * @return True if the achievement exists and is completed, false otherwise.
	 */
	bool achievement(const std::string& content_for, const std::string& id);
	/**
	 * Marks the specified achievement as completed.
	 *
	 * @param content_for The achievement group the achievement is part of.
	 * @param id The ID of the achievement within the achievement group.
	 */
	void set_achievement(const std::string& content_for, const std::string& id);

	/**
	 * Increments the achievement's current progress by @a amount if it hasn't already been completed.
	 * If you only want to check the achievement's current progress, then omit the last three arguments.
	 * @a amount defaults to 0, which will result in the current progress value being returned without being changed (x + 0 == x).
	 *
	 * Note that this uses the same [in_progress] as is used for set_sub_achievement().
	 *
	 * @param content_for The id of the achievement group this achievement is in.
	 * @param id The id for the specific achievement in the achievement group.
	 * @param limit The maximum value that a specific call to this function can increase the achievement progress value.
	 * @param max_progress The value when the achievement is considered completed.
	 * @param amount The amount to progress the achievement.
	 * @return The achievement's current progress, or -1 if it has already been completed.
	 */
	int progress_achievement(const std::string& content_for, const std::string& id, int limit = 999999, int max_progress = 999999, int amount = 0);

	/**
	 * @param content_for The achievement group the achievement is part of.
	 * @param id The ID of the achievement within the achievement group.
	 * @param sub_id The ID of the sub-achievement within the achievement.
	 * @return True if the sub-achievement exists and is completed, false otherwise.
	 */
	bool sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id);

	/**
	 * Marks the specified sub-achievement as completed.
	 *
	 * Note that this uses the same [in_progress] as is used for progress_achievement().
	 *
	 * @param content_for The achievement group the achievement is part of.
	 * @param id The ID of the achievement within the achievement group.
	 * @param sub_id The ID of the sub-achievement within the achievement.
	 */
	void set_sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id);

	/**
	 * @param addon_id The chosen addon id from the editor to store in the preferences.
	 */
	void set_editor_chosen_addon(const std::string& addon_id);

	/**
	 * @return The most recently selected add-on id from the editor. May be an empty string.
	 */
	std::string editor_chosen_addon();

} // end namespace preferences
