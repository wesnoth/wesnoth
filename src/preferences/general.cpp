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

/**
 *  @file
 *  Get and set user-preferences.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "preferences/general.hpp"

#include "config.hpp"
#include "credentials.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "hotkey/hotkey_item.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "sdl/point.hpp"
#include "serialization/parser.hpp"
#include "sound.hpp"
#include "utils/general.hpp"
#include "video.hpp" // non_interactive()

#include <sys/stat.h> // for setting the permissions of the preferences file
#ifndef _WIN32
#include <unistd.h>
#endif

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace {

bool no_preferences_save = false;

bool fps = false;

int draw_delay_ = 0;

config prefs;
}

namespace preferences {

/*
 * Stores all the static, default values for certain game preferences. The values
 * are kept here for easy modification without a lengthy rebuild.
 *
 * Add any variables of similar type here.
 */
const int min_window_width  = 800;
const int min_window_height = 600;

const int def_window_width  = 1024;
const int def_window_height = 768;

const int min_font_scaling  = 80;
const int max_font_scaling  = 150;

const SCALING_ALGORITHM default_scaling_algorithm = SCALING_ALGORITHM::XBRZ_NN;

class prefs_event_handler : public events::sdl_handler {
public:
	virtual void handle_event(const SDL_Event &) {}
	virtual void handle_window_event(const SDL_Event &event);
	prefs_event_handler() :	sdl_handler(false) {}
};

prefs_event_handler event_handler_;

base_manager::base_manager()
{
	event_handler_.join_global();

	try{
#ifdef DEFAULT_PREFS_PATH
		filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_default_prefs_file(),false);
		read(prefs, *stream);

		config user_prefs;
		stream = filesystem::istream_file(filesystem::get_prefs_file());
		read(user_prefs, *stream);

		prefs.merge_with(user_prefs);
#else
		filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_prefs_file(),false);
		read(prefs, *stream);
#endif
	} catch(const config::error& e) {
		ERR_CFG << "Error loading preference, message: "
				<< e.what()
				<< std::endl;
	}
	preferences::load_credentials();
}

base_manager::~base_manager()
{
	event_handler_.leave_global();

	try {
		if (no_preferences_save) return;

		// Set the 'hidden' preferences.
		prefs["scroll_threshold"] = mouse_scroll_threshold();

		write_preferences();
	} catch (...) {}
}

/*
 * Hook for setting window state variables on window resize and maximize
 * events. Since there is no fullscreen window event, that setter is called
 * from the CVideo function instead.
 */
void prefs_event_handler::handle_window_event(const SDL_Event& event)
{

	// Saftey check to make sure this is a window event
	if (event.type != SDL_WINDOWEVENT) return;

	switch(event.window.event) {
	case SDL_WINDOWEVENT_RESIZED:
		_set_resolution(point(event.window.data1,event.window.data2));

		break;

	case SDL_WINDOWEVENT_MAXIMIZED:
		_set_maximized(true);

		break;

	case SDL_WINDOWEVENT_RESTORED:
		_set_maximized(fullscreen() || false);

		break;
	}
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
		ERR_FS << "error writing to preferences file '" << filesystem::get_prefs_file() << "'" << std::endl;
	}

	preferences::save_credentials();

#ifndef _WIN32
    if(!prefs_file_existed) {

        if(chmod(filesystem::get_prefs_file().c_str(), 0600) == -1) {
			ERR_FS << "error setting permissions of preferences file '" << filesystem::get_prefs_file() << "'" << std::endl;
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

void set(const std::string &key, const config::attribute_value &value)
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

std::string get(const std::string& key, const std::string& def) {
	return prefs[key].empty() ? def : prefs[key];
}

bool get(const std::string &key, bool def)
{
	return prefs[key].to_bool(def);
}

config::attribute_value get_as_attribute(const std::string &key)
{
	return prefs[key];
}

void disable_preferences_save() {
	no_preferences_save = true;
}

config* get_prefs(){
	config* pointer = &prefs;
	return pointer;
}


bool show_allied_orb() {
	return get("show_ally_orb", game_config::show_ally_orb);
}
void set_show_allied_orb(bool show_orb) {
	prefs["show_ally_orb"] = show_orb;
}

bool show_enemy_orb() {
	return get("show_enemy_orb", game_config::show_enemy_orb);
}
void set_show_enemy_orb(bool show_orb) {
	prefs["show_enemy_orb"] = show_orb;
}

bool show_moved_orb() {
	return get("show_moved_orb", game_config::show_moved_orb);
}
void set_show_moved_orb(bool show_orb) {
	prefs["show_moved_orb"] = show_orb;
}

bool show_unmoved_orb() {
	return get("show_unmoved_orb", game_config::show_unmoved_orb);
}
void set_show_unmoved_orb(bool show_orb) {
	prefs["show_unmoved_orb"] = show_orb;
}

bool show_partial_orb() {
	return get("show_partial_orb", game_config::show_partial_orb);
}
void set_show_partial_orb(bool show_orb) {
	prefs["show_partial_orb"] = show_orb;
}


static std::string fix_orb_color_name(const std::string& color) {
	if (color.substr(0,4) == "orb_") {
		if(color[4] >= '0' && color[4] <= '9') {
			return color.substr(5);
		} else {
			return color.substr(4);
		}
	}
	return color;
}

std::string allied_color() {
	std::string ally_color = get("ally_orb_color");
	if (ally_color.empty())
		return game_config::colors::ally_orb_color;
	return fix_orb_color_name(ally_color);
}
void set_allied_color(const std::string& color_id) {
	prefs["ally_orb_color"] = color_id;
}

std::string core_id() {
	std::string core_id = get("core");
	if (core_id.empty())
		return "default";
	return core_id;
}
void set_core_id(const std::string& core_id) {
	prefs["core"] = core_id;
}

std::string enemy_color() {
	std::string enemy_color = get("enemy_orb_color");
	if (enemy_color.empty())
		return game_config::colors::enemy_orb_color;
	return fix_orb_color_name(enemy_color);
}
void set_enemy_color(const std::string& color_id) {
	prefs["enemy_orb_color"] = color_id;
}

std::string moved_color() {
	std::string moved_color = get("moved_orb_color");
	if (moved_color.empty())
		return game_config::colors::moved_orb_color;
	return fix_orb_color_name(moved_color);
}
void set_moved_color(const std::string& color_id) {
	prefs["moved_orb_color"] = color_id;
}

std::string unmoved_color() {
	std::string unmoved_color = get("unmoved_orb_color");
	if (unmoved_color.empty())
		return game_config::colors::unmoved_orb_color;
	return fix_orb_color_name(unmoved_color);
}
void set_unmoved_color(const std::string& color_id) {
	prefs["unmoved_orb_color"] = color_id;
}

std::string partial_color() {
	std::string partmoved_color = get("partial_orb_color");
	if (partmoved_color.empty())
		return game_config::colors::partial_orb_color;
	return fix_orb_color_name(partmoved_color);
}
void set_partial_color(const std::string& color_id) {
	prefs["partial_orb_color"] = color_id;
}

bool scroll_to_action()
{
	return get("scroll_to_action", true);
}

void set_scroll_to_action(bool ison)
{
	prefs["scroll_to_action"] = ison;
}

point resolution()
{
	const std::string& x = prefs["xresolution"], y = prefs["yresolution"];

	if (!x.empty() && !y.empty()) {
		try {
			return point(
				std::max(std::stoi(x), min_window_width),
				std::max(std::stoi(y), min_window_height));
		} catch(std::invalid_argument) {}
	}

	return point(def_window_width, def_window_height);
}

bool maximized()
{
	return get("maximized", !fullscreen());
}

bool fullscreen()
{
	return get("fullscreen", false);
}

void _set_resolution(const point& res)
{
	preferences::set("xresolution", std::to_string(res.x));
	preferences::set("yresolution", std::to_string(res.y));
}

void _set_maximized(bool ison)
{
	prefs["maximized"] = ison;
}

void _set_fullscreen(bool ison)
{
	prefs["fullscreen"] = ison;
}

bool turbo()
{
	if(CVideo::get_singleton().non_interactive()) {
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

int font_scaling()
{
	// Clip at 80 because if it's too low it'll cause crashes
	return std::max<int>(std::min<int>(prefs["font_scale"].to_int(100), max_font_scaling), min_font_scaling);
}

void set_font_scaling(int scale)
{
	prefs["font_scale"] = utils::clamp(scale, min_font_scaling, max_font_scaling);
}

int font_scaled(int size)
{
	return (size * font_scaling()) / 100;
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

std::string gui_theme()
{
	return prefs["gui2_theme"];
}

void set_gui_theme(const std::string& s)
{
	preferences::set("gui2_theme", s);
}

bool ellipses()
{
	return get("show_side_colors", false);
}

void set_ellipses(bool ison)
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

bool stop_music_in_background()
{
	return get("stop_music_in_background", false);
}

void set_stop_music_in_background(bool ison)
{
	preferences::set("stop_music_in_background", ison);
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
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_scroll_deadzone"), 1500), 0, 16000);
	return value;
}

int joystick_num_mouse_xaxis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_scroll_xaxis"), 0), -1, 3);
	return value;
}

int joystick_mouse_xaxis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_scroll_xaxis_num"), 0), 0, 7);
	return value;
}

int joystick_num_mouse_yaxis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_scroll_yaxis"), 0), -1, 3);
	return value;
}

int joystick_mouse_yaxis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_scroll_yaxis_num"), 1), 0, 7);
	return value;
}

int joystick_scroll_deadzone()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_scroll_deadzone"), 1500), 0, 16000);
	return value;
}

int joystick_cursor_deadzone()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_cursor_deadzone"), 1500), 0, 16000);
	return value;
}

int joystick_thrusta_deadzone()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_thrusta_deadzone"), 1500), 0, 16000);
	return value;
}

int joystick_thrustb_deadzone()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_thrustb_deadzone"), 1500), 0, 16000);
	return value;
}

int joystick_cursor_threshold()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_cursor_threshold"), 10000), 0, 16000);
	return value;
}

int joystick_num_scroll_xaxis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_scroll_xaxis"), 0), -1, 3);
	return value;
}

int joystick_scroll_xaxis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_scroll_xaxis_num"), 0), 0, 7);
	return value;
}

int joystick_num_scroll_yaxis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_scroll_yaxis"), 0), -1, 3);
	return value;
}

int joystick_scroll_yaxis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_scroll_yaxis_num"), 1), 0, 7);
	return value;
}

int joystick_num_cursor_xaxis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_cursor_xaxis"), 0), -1, 3);
	return value;
}

int joystick_cursor_xaxis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_cursor_xaxis_num"), 3), 0, 7);
	return value;
}

int joystick_num_cursor_yaxis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_cursor_yaxis"), 0), -1, 3);
	return value;
}

int joystick_cursor_yaxis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_cursor_yaxis_num"), 4), 0, 7);
	return value;
}

int joystick_num_thrusta_axis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_thrusta_axis"), 0), -1, 3);
	return value;
}

int joystick_thrusta_axis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_thrusta_axis_num"), 2), 0, 7);
	return value;
}

int joystick_num_thrustb_axis()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_num_thrustb_axis"), 0), -1, 3);
	return value;
}

int joystick_thrustb_axis_num()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("joystick_thrustb_axis_num"), 2), 0, 7);
	return value;
}


int scroll_speed()
{
	const int value = utils::clamp<int>(lexical_cast_default<int>(get("scroll"), 50), 1, 100);
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

bool animate_water()
{
	return preferences::get("animate_water", true);
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

void set_animate_water(bool value)
{
	set("animate_water", value);
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
	return get("color_cursors", true);
}

void _set_color_cursors(bool value)
{
	preferences::set("color_cursors", value);
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

bool disable_auto_moves()
{
	return get("disable_auto_moves", false);
}

void set_disable_auto_moves(bool value)
{
	preferences::set("disable_auto_moves", value);
}

bool disable_loadingscreen_animation()
{
	return get("disable_loadingscreen_animation", false);
}

void set_disable_loadingscreen_animation(bool value)
{
	set("disable_loadingscreen_animation", value);
}

bool damage_prediction_allow_monte_carlo_simulation()
{
	return get("damage_prediction_allow_monte_carlo_simulation", true);
}

void set_damage_prediction_allow_monte_carlo_simulation(bool value)
{
	set("damage_prediction_allow_monte_carlo_simulation", value);
}

} // end namespace preferences

