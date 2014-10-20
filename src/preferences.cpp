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

/**
 *  @file
 *  Get and set user-preferences.
 */

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "filesystem.hpp"
#include "hotkey/hotkey_item.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "sound.hpp"
#include "video.hpp" // non_interactive()
#include "serialization/parser.hpp"
#include "util.hpp"

#include <sys/stat.h> // for setting the permissions of the preferences file
#include <boost/concept_check.hpp>

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace {

bool color_cursors = false;

bool no_preferences_save = false;

bool fps = false;

int draw_delay_ = 20;

config prefs;
}

namespace preferences {

base_manager::base_manager()
{
	try{
#ifdef DEFAULT_PREFS_PATH
		filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_default_prefs_file());
		read(prefs, *stream);

		config user_prefs;
		stream = filesystem::istream_file(filesystem::get_prefs_file());
		read(user_prefs, *stream);

		prefs.merge_with(user_prefs);
#else
		filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_prefs_file());
		read(prefs, *stream);
#endif
	} catch(const config::error& e) {
		ERR_CFG << "Error loading preference, message: "
				<< e.what()
				<< '\n';
	}
}

base_manager::~base_manager()
{
	if (no_preferences_save) return;

	// Set the 'hidden' preferences.
	prefs["scroll_threshold"] = mouse_scroll_threshold();

	write_preferences();
}

void write_preferences()
{
    #ifndef _WIN32

    bool prefs_file_existed = access(filesystem::get_prefs_file().c_str(), F_OK) == 0;

    #endif

	try {
		filesystem::scoped_ostream prefs_file = filesystem::ostream_file(filesystem::get_prefs_file());
		write(*prefs_file, prefs);
	} catch(filesystem::io_exception&) {
		ERR_FS << "error writing to preferences file '" << filesystem::get_prefs_file() << "'\n";
	}


    #ifndef _WIN32

    if(!prefs_file_existed) {

        if(chmod(filesystem::get_prefs_file().c_str(), 0600) == -1) {
			ERR_FS << "error setting permissions of preferences file '" << filesystem::get_prefs_file() << "'\n";
        }

    }

    #endif


}

void set(const std::string &key, bool value)
{
	prefs[key] = value;
}

void set(const std::string &key, int value)
{
	prefs[key] = value;
}

void set(const std::string &key, char const *value)
{
	prefs[key] = value;
}

void set(const std::string &key, const std::string &value)
{
	prefs[key] = value;
}

void clear(const std::string& key)
{
	prefs.recursive_clear_value(key);
}

void set_child(const std::string& key, const config& val) {
	prefs.clear_children(key);
	prefs.add_child(key, val);
}

const config &get_child(const std::string& key)
{
	return prefs.child(key);
}

void erase(const std::string& key) {
	prefs.remove_attribute(key);
}

bool have_setting(const std::string& key) {
	return prefs.has_attribute(key);
}

std::string get(const std::string& key) {
	return prefs[key];
}

bool get(const std::string &key, bool def)
{
	return prefs[key].to_bool(def);
}

void disable_preferences_save() {
	no_preferences_save = true;
}

config* get_prefs(){
	config* pointer = &prefs;
	return pointer;
}

bool fullscreen()
{
	return get("fullscreen", false);
}

void _set_fullscreen(bool ison)
{
	prefs["fullscreen"] = ison;
}

bool scroll_to_action()
{
	return get("scroll_to_action", true);
}

void _set_scroll_to_action(bool ison)
{
	prefs["scroll_to_action"] = ison;
}

int min_allowed_width()
{
	return 800;
}

int min_allowed_height()
{
	return 480;
}

std::pair<int,int> resolution()
{
	const std::string postfix = fullscreen() ? "resolution" : "windowsize";
	std::string x = prefs['x' + postfix], y = prefs['y' + postfix];
	if (!x.empty() && !y.empty()) {
		std::pair<int,int> res(std::max(atoi(x.c_str()), min_allowed_width()),
		                       std::max(atoi(y.c_str()), min_allowed_height()));

		// Make sure resolutions are always divisible by 4
		//res.first &= ~3;
		//res.second &= ~3;
		return res;
	} else {
	#ifdef __APPLE__
		// When windowsize is larger than the screen,  Wesnoth scales the video
		// which causes distortion with mouse tracking. 768 is simply too large
		// vertically to safely fit. We need a smaller default resolution here for Macs.
		// See bug #20332.
		return std::pair<int,int>(800,600);
	#else
		return std::pair<int,int>(1024,768);
	#endif
	}
}

void _set_resolution(const std::pair<int, int>& res)
{
	const std::string postfix = fullscreen() ? "resolution" : "windowsize";
	preferences::set('x' + postfix, lexical_cast<std::string>(res.first));
	preferences::set('y' + postfix, lexical_cast<std::string>(res.second));
}

bool turbo()
{
	if(non_interactive()) {
		return true;
	}

	return get("turbo", false);
}

void _set_turbo(bool ison)
{
	prefs["turbo"] = ison;
}

double turbo_speed()
{
	return prefs["turbo_speed"].to_double(2.0);
}

void save_turbo_speed(const double speed)
{
	prefs["turbo_speed"] = speed;
}

bool idle_anim()
{
	return  get("idle_anim", true);
}

void _set_idle_anim(const bool ison)
{
	prefs["idle_anim"] = ison;
}

int idle_anim_rate()
{
	return prefs["idle_anim_rate"];
}

void _set_idle_anim_rate(const int rate)
{
	prefs["idle_anim_rate"] = rate;
}

std::string language()
{
	return prefs["locale"];
}

void set_language(const std::string& s)
{
	preferences::set("locale", s);
}

bool ellipses()
{
	return get("show_side_colors", false);
}

void _set_ellipses(bool ison)
{
	preferences::set("show_side_colors",  ison);
}

bool grid()
{
	return get("grid", false);
}

void _set_grid(bool ison)
{
	preferences::set("grid", ison);
}

size_t sound_buffer_size()
{
	// Sounds don't sound good on Windows unless the buffer size is 4k,
	// but this seems to cause crashes on other systems...
	#ifdef _WIN32
		const size_t buf_size = 4096;
	#else
		const size_t buf_size = 1024;
	#endif

	return prefs["sound_buffer_size"].to_int(buf_size);
}

void save_sound_buffer_size(const size_t size)
{
	#ifdef _WIN32
		const char* buf_size = "4096";
	#else
		const char* buf_size = "1024";
	#endif

	const std::string new_size = lexical_cast_default<std::string>(size, buf_size);
	if (get("sound_buffer_size") == new_size)
		return;

	preferences::set("sound_buffer_size", new_size);

	sound::reset_sound();
}

int music_volume()
{
	return prefs["music_volume"].to_int(100);
}

void set_music_volume(int vol)
{
	if(music_volume() == vol) {
		return;
	}

	prefs["music_volume"] = vol;
	sound::set_music_volume(music_volume());
}

int sound_volume()
{
	return prefs["sound_volume"].to_int(100);
}

void set_sound_volume(int vol)
{
	if(sound_volume() == vol) {
		return;
	}

	prefs["sound_volume"] = vol;
	sound::set_sound_volume(sound_volume());
}

int bell_volume()
{
	return prefs["bell_volume"].to_int(100);
}

void set_bell_volume(int vol)
{
	if(bell_volume() == vol) {
		return;
	}

	prefs["bell_volume"] = vol;
	sound::set_bell_volume(bell_volume());
}

int UI_volume()
{
	return prefs["UI_volume"].to_int(100);
}

void set_UI_volume(int vol)
{
	if(UI_volume() == vol) {
		return;
	}

	prefs["UI_volume"] = vol;
	sound::set_UI_volume(UI_volume());
}

bool turn_bell()
{
	return get("turn_bell", true);
}

bool set_turn_bell(bool ison)
{
	if(!turn_bell() && ison) {
		preferences::set("turn_bell", true);
		if(!music_on() && !sound_on() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("turn_bell", false);
				return false;
			}
		}
	} else if(turn_bell() && !ison) {
		preferences::set("turn_bell", false);
		sound::stop_bell();
		if(!music_on() && !sound_on() && !UI_sound_on())
			sound::close_sound();
	}
	return true;
}

bool UI_sound_on()
{
	return get("UI_sound", true);
}

bool set_UI_sound(bool ison)
{
	if(!UI_sound_on() && ison) {
		preferences::set("UI_sound", true);
		if(!music_on() && !sound_on() && !turn_bell()) {
			if(!sound::init_sound()) {
				preferences::set("UI_sound", false);
				return false;
			}
		}
	} else if(UI_sound_on() && !ison) {
		preferences::set("UI_sound", false);
		sound::stop_UI_sound();
		if(!music_on() && !sound_on() && !turn_bell())
			sound::close_sound();
	}
	return true;
}

bool message_bell()
{
	return get("message_bell", true);
}

bool sound_on()
{
	return get("sound", true);
}

bool set_sound(bool ison) {
	if(!sound_on() && ison) {
		preferences::set("sound", true);
		if(!music_on() && !turn_bell() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("sound", false);
				return false;
			}
		}
	} else if(sound_on() && !ison) {
		preferences::set("sound", false);
		sound::stop_sound();
		if(!music_on() && !turn_bell() && !UI_sound_on())
			sound::close_sound();
	}
	return true;
}

bool music_on()
{
	return get("music", true);
}

bool set_music(bool ison) {
	if(!music_on() && ison) {
		preferences::set("music", true);
		if(!sound_on() && !turn_bell() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("music", false);
				return false;
			}
		}
		else
			sound::play_music();
	} else if(music_on() && !ison) {
		preferences::set("music", false);
		if(!sound_on() && !turn_bell() && !UI_sound_on())
			sound::close_sound();
		else
			sound::stop_music();
	}
	return true;
}

namespace {
	double scroll = 0.2;
}

bool joystick_support_enabled()
{
	return get("joystick_support_enabled", false);
}

int joystick_mouse_deadzone()
{
	const int value = lexical_cast_in_range<int>(get("joystick_scroll_deadzone"), 1500, 0, 16000);
	return value;
}

int joystick_num_mouse_xaxis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_scroll_xaxis"), 0, -1, 3);
	return value;
}

int joystick_mouse_xaxis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_scroll_xaxis_num"), 0, 0, 7);
	return value;
}

int joystick_num_mouse_yaxis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_scroll_yaxis"), 0, -1, 3);
	return value;
}

int joystick_mouse_yaxis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_scroll_yaxis_num"), 1, 0, 7);
	return value;
}

int joystick_scroll_deadzone()
{
	const int value = lexical_cast_in_range<int>(get("joystick_scroll_deadzone"), 1500, 0, 16000);
	return value;
}

int joystick_cursor_deadzone()
{
	const int value = lexical_cast_in_range<int>(get("joystick_cursor_deadzone"), 1500, 0, 16000);
	return value;
}

int joystick_thrusta_deadzone()
{
	const int value = lexical_cast_in_range<int>(get("joystick_thrusta_deadzone"), 1500, 0, 16000);
	return value;
}

int joystick_thrustb_deadzone()
{
	const int value = lexical_cast_in_range<int>(get("joystick_thrustb_deadzone"), 1500, 0, 16000);
	return value;
}

int joystick_cursor_threshold()
{
	const int value = lexical_cast_in_range<int>(get("joystick_cursor_threshold"), 10000, 0, 16000);
	return value;
}

int joystick_num_scroll_xaxis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_scroll_xaxis"), 0, -1, 3);
	return value;
}

int joystick_scroll_xaxis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_scroll_xaxis_num"), 0, 0, 7);
	return value;
}

int joystick_num_scroll_yaxis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_scroll_yaxis"), 0, -1, 3);
	return value;
}

int joystick_scroll_yaxis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_scroll_yaxis_num"), 1, 0, 7);
	return value;
}

int joystick_num_cursor_xaxis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_cursor_xaxis"), 0, -1, 3);
	return value;
}

int joystick_cursor_xaxis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_cursor_xaxis_num"), 3, 0, 7);
	return value;
}

int joystick_num_cursor_yaxis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_cursor_yaxis"), 0, -1, 3);
	return value;
}

int joystick_cursor_yaxis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_cursor_yaxis_num"), 4, 0, 7);
	return value;
}

int joystick_num_thrusta_axis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_thrusta_axis"), 0, -1, 3);
	return value;
}

int joystick_thrusta_axis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_thrusta_axis_num"), 2, 0, 7);
	return value;
}

int joystick_num_thrustb_axis()
{
	const int value = lexical_cast_in_range<int>(get("joystick_num_thrustb_axis"), 0, -1, 3);
	return value;
}

int joystick_thrustb_axis_num()
{
	const int value = lexical_cast_in_range<int>(get("joystick_thrustb_axis_num"), 2, 0, 7);
	return value;
}


int scroll_speed()
{
	const int value = lexical_cast_in_range<int>(get("scroll"), 50, 1, 100);
	scroll = value/100.0;

	return value;
}

void set_scroll_speed(const int new_speed)
{
	prefs["scroll"] = new_speed;
	scroll = new_speed / 100.0;
}

bool middle_click_scrolls()
{
	return get("middle_click_scrolls", true);
}

bool mouse_scroll_enabled()
{
	return get("mouse_scrolling", true);
}

void enable_mouse_scroll(bool value)
{
	set("mouse_scrolling", value);
}

int mouse_scroll_threshold()
{
	return prefs["scroll_threshold"].to_int(10);
}

bool animate_map()
{
	return preferences::get("animate_map", true);
}

bool minimap_movement_coding()
{
	return preferences::get("minimap_movement_coding", true);
}

void toggle_minimap_movement_coding()
{
	set("minimap_movement_coding", !minimap_movement_coding());
}

bool minimap_terrain_coding()
{
	return preferences::get("minimap_terrain_coding", true);
}

void toggle_minimap_terrain_coding()
{
	set("minimap_terrain_coding", !minimap_terrain_coding());
}

bool minimap_draw_units()
{
	return preferences::get("minimap_draw_units", true);
}

void toggle_minimap_draw_units()
{
	set("minimap_draw_units", !minimap_draw_units());
}

bool minimap_draw_villages()
{
	return preferences::get("minimap_draw_villages", true);
}

void toggle_minimap_draw_villages()
{
	set("minimap_draw_villages", !minimap_draw_villages());
}

bool minimap_draw_terrain()
{
	return preferences::get("minimap_draw_terrain", true);
}

void toggle_minimap_draw_terrain()
{
	set("minimap_draw_terrain", !minimap_draw_terrain());
}

void set_animate_map(bool value)
{
	set("animate_map", value);
}

bool show_standing_animations()
{
	return preferences::get("unit_standing_animations", true);
}

void set_show_standing_animations(bool value)
{
	set("unit_standing_animations", value);
}

bool show_fps()
{
	return fps;
}

void set_show_fps(bool value)
{
	fps = value;
}

int draw_delay()
{
	return draw_delay_;
}

void set_draw_delay(int value)
{
	draw_delay_ = value;
}

bool use_color_cursors()
{
	return color_cursors;
}

void _set_color_cursors(bool value)
{
	preferences::set("color_cursors", value);
	color_cursors = value;
}

void load_hotkeys()
{
	hotkey::load_hotkeys(prefs, false);
}

void save_hotkeys()
{
	hotkey::save_hotkeys(prefs);
}

void clear_hotkeys()
{
	hotkey::reset_default_hotkeys();
	prefs.clear_children("hotkey");
}

void add_alias(const std::string &alias, const std::string &command)
{
	config &alias_list = prefs.child_or_add("alias");
	alias_list[alias] = command;
}


const config &get_alias()
{
	return get_child("alias");
}

unsigned int sample_rate()
{
	return prefs["sample_rate"].to_int(44100);
}

void save_sample_rate(const unsigned int rate)
{
	if (sample_rate() == rate)
		return;

	prefs["sample_rate"] = int(rate);

	// If audio is open, we have to re set sample rate
	sound::reset_sound();
}

bool confirm_load_save_from_different_version()
{
	return get("confirm_load_save_from_different_version", true);
}

bool use_twelve_hour_clock_format()
{
	return get("use_twelve_hour_clock_format", false);
}

} // end namespace preferences

