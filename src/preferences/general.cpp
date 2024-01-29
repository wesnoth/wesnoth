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
#include "video.hpp"
#include "game_config_view.hpp"

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
const int min_window_height = 540;

const int def_window_width  = 1280;
const int def_window_height = 720;

const int max_window_width = 1920;
const int max_window_height = 1080;

const int min_font_scaling  = 80;
const int max_font_scaling  = 150;

const int min_pixel_scale = 1;
const int max_pixel_scale = 4;

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

	preferences::load_base_prefs();
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
 * from the video function instead.
 */
void prefs_event_handler::handle_window_event(const SDL_Event& event)
{

	// Safety check to make sure this is a window event
	if (event.type != SDL_WINDOWEVENT) return;

	switch(event.window.event) {
	case SDL_WINDOWEVENT_RESIZED:
		_set_resolution(video::window_size());

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
	} catch(const filesystem::io_exception&) {
		ERR_FS << "error writing to preferences file '" << filesystem::get_prefs_file() << "'";
	}

	preferences::save_credentials();

#ifndef _WIN32
	if(!prefs_file_existed) {
		if(chmod(filesystem::get_prefs_file().c_str(), 0600) == -1) {
			ERR_FS << "error setting permissions of preferences file '" << filesystem::get_prefs_file() << "'";
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

optional_const_config get_child(const std::string& key)
{
	return prefs.optional_child(key);
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

void load_base_prefs() {
	try{
#ifdef DEFAULT_PREFS_PATH
		filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_default_prefs_file(),false);
		read(prefs, *stream);

		config user_prefs;
		stream = filesystem::istream_file(filesystem::get_prefs_file());
		read(user_prefs, *stream);

		prefs.merge_with(user_prefs);
#else
		prefs.clear();
		filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_prefs_file(),false);
		read(prefs, *stream);
#endif
	} catch(const config::error& e) {
		ERR_CFG << "Error loading preference, message: " << e.what();
	}
}


bool show_ally_orb() {
	return get("show_ally_orb", game_config::show_ally_orb);
}
void set_show_ally_orb(bool show_orb) {
	prefs["show_ally_orb"] = show_orb;
}

bool show_status_on_ally_orb() {
	return get("show_status_on_ally_orb", game_config::show_status_on_ally_orb);
}
void set_show_status_on_ally_orb(bool show_orb) {
	prefs["show_status_on_ally_orb"] = show_orb;
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

bool show_disengaged_orb() {
	return get("show_disengaged_orb", game_config::show_disengaged_orb);
}
void set_show_disengaged_orb(bool show_orb) {
	prefs["show_disengaged_orb"] = show_orb;
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
	const unsigned x_res = prefs["xresolution"].to_unsigned();
	const unsigned y_res = prefs["yresolution"].to_unsigned();

	// Either resolution was unspecified, return default.
	if(x_res == 0 || y_res == 0) {
		return point(def_window_width, def_window_height);
	}

	return point(
		std::max<unsigned>(x_res, min_window_width),
		std::max<unsigned>(y_res, min_window_height)
	);
}

int pixel_scale()
{
	// For now this has a minimum value of 1 and a maximum of 4.
	return std::max<int>(std::min<int>(prefs["pixel_scale"].to_int(1), max_pixel_scale), min_pixel_scale);
}

void set_pixel_scale(const int scale)
{
	prefs["pixel_scale"] = std::clamp(scale, min_pixel_scale, max_pixel_scale);
}

bool auto_pixel_scale()
{
	return get("auto_pixel_scale", true);
}

void set_auto_pixel_scale(bool choice)
{
	prefs["auto_pixel_scale"] = choice;
}

bool maximized()
{
	return get("maximized", !fullscreen());
}

bool fullscreen()
{
	return get("fullscreen", true);
}

bool vsync()
{
	return get("vsync", true);
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

void set_vsync(bool ison)
{
	prefs["vsync"] = ison;
}

bool turbo()
{
	if(video::headless()) {
		return true;
	}

	return get("turbo", false);
}

void set_turbo(bool ison)
{
	prefs["turbo"] = ison;
}

double turbo_speed()
{
	return prefs["turbo_speed"].to_double(2.0);
}

void set_turbo_speed(const double speed)
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
	prefs["font_scale"] = std::clamp(scale, min_font_scaling, max_font_scaling);
}

int font_scaled(int size)
{
	return (size * font_scaling()) / 100;
}

int keepalive_timeout()
{
	return prefs["keepalive_timeout"].to_int(10);
}

void keepalive_timeout(int seconds)
{
	prefs["keepalive_timeout"] = std::abs(seconds);
}

bool idle_anim()
{
	return  get("idle_anim", true);
}

void set_idle_anim(const bool ison)
{
	prefs["idle_anim"] = ison;
}

double idle_anim_rate()
{
	return prefs["idle_anim_rate"].to_double(1.0);
}

void set_idle_anim_rate(const int rate)
{
	prefs["idle_anim_rate"] = std::pow(2.0, -rate / 10.0);
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

void set_grid(bool ison)
{
	preferences::set("grid", ison);
}

std::size_t sound_buffer_size()
{
	// Sounds don't sound good on Windows unless the buffer size is 4k,
	// but this seems to cause crashes on other systems...
	#ifdef _WIN32
		const std::size_t buf_size = 4096;
	#else
		const std::size_t buf_size = 1024;
	#endif

	return prefs["sound_buffer_size"].to_int(buf_size);
}

void save_sound_buffer_size(const std::size_t size)
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

unsigned int tile_size()
{
	return prefs["tile_size"].to_unsigned();
}

void set_tile_size(const unsigned int size)
{
	prefs["tile_size"] = size;
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

int scroll_speed()
{
	return std::clamp<int>(lexical_cast_default<int>(get("scroll"), 50), 1, 100);
}

void set_scroll_speed(const int new_speed)
{
	prefs["scroll"] = new_speed;
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
	return prefs["draw_delay"].to_int(-1);
}

void set_draw_delay(int value)
{
	prefs["draw_delay"] = value;
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
	hotkey::load_custom_hotkeys(game_config_view::wrap(prefs));
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


optional_const_config get_alias()
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

	prefs["sample_rate"] = static_cast<int>(rate);

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

bool damage_prediction_allow_monte_carlo_simulation()
{
	return get("damage_prediction_allow_monte_carlo_simulation", true);
}

void set_damage_prediction_allow_monte_carlo_simulation(bool value)
{
	set("damage_prediction_allow_monte_carlo_simulation", value);
}

std::string addon_manager_saved_order_name()
{
	return get("addon_manager_saved_order_name");
}

void set_addon_manager_saved_order_name(const std::string& value)
{
	set("addon_manager_saved_order_name", value);
}

sort_order::type addon_manager_saved_order_direction()
{
	return sort_order::get_enum(get("addon_manager_saved_order_direction")).value_or(sort_order::type::none);
}

void set_addon_manager_saved_order_direction(sort_order::type value)
{
	set("addon_manager_saved_order_direction", sort_order::get_string(value));
}

std::string selected_achievement_group()
{
	return get("selected_achievement_group");
}

void set_selected_achievement_group(const std::string& content_for)
{
	set("selected_achievement_group", content_for);
}

bool achievement(const std::string& content_for, const std::string& id)
{
	for(config& ach : prefs.child_range("achievements"))
	{
		if(ach["content_for"].str() == content_for)
		{
			std::vector<std::string> ids = utils::split(ach["ids"]);
			return std::find(ids.begin(), ids.end(), id) != ids.end();
		}
	}
	return false;
}

void set_achievement(const std::string& content_for, const std::string& id)
{
	for(config& ach : prefs.child_range("achievements"))
	{
		// if achievements already exist for this content and the achievement has not already been set, add it
		if(ach["content_for"].str() == content_for)
		{
			std::vector<std::string> ids = utils::split(ach["ids"]);

			if(ids.empty())
			{
				ach["ids"] = id;
			}
			else if(std::find(ids.begin(), ids.end(), id) == ids.end())
			{
				ach["ids"] = ach["ids"].str() + "," + id;
			}
			ach.remove_children("in_progress", [&id](config cfg){return cfg["id"].str() == id;});
			return;
		}
	}

	// else no achievements have been set for this content yet
	config ach;
	ach["content_for"] = content_for;
	ach["ids"] = id;
	prefs.add_child("achievements", ach);
}

int progress_achievement(const std::string& content_for, const std::string& id, int limit, int max_progress, int amount)
{
	if(achievement(content_for, id))
	{
		return -1;
	}

	for(config& ach : prefs.child_range("achievements"))
	{
		// if achievements already exist for this content and the achievement has not already been set, add it
		if(ach["content_for"].str() == content_for)
		{
			// check if this achievement has progressed before - if so then increment it
			for(config& in_progress : ach.child_range("in_progress"))
			{
				if(in_progress["id"].str() == id)
				{
					// don't let using 'limit' decrease the achievement's current progress
					int starting_progress = in_progress["progress_at"].to_int();
					if(starting_progress >= limit) {
						return starting_progress;
					}

					in_progress["progress_at"] = std::clamp(starting_progress + amount, 0, std::min(limit, max_progress));
					return in_progress["progress_at"].to_int();
				}
			}

			// else this is the first time this achievement is progressing
			if(amount != 0)
			{
				config set_progress;
				set_progress["id"] = id;
				set_progress["progress_at"] = std::clamp(amount, 0, std::min(limit, max_progress));

				config& child = ach.add_child("in_progress", set_progress);
				return child["progress_at"].to_int();
			}
			return 0;
		}
	}

	// else not only has this achievement not progressed before, this is the first achievement for this achievement group to be added
	if(amount != 0)
	{
		config ach;
		config set_progress;

		set_progress["id"] = id;
		set_progress["progress_at"] = std::clamp(amount, 0, std::min(limit, max_progress));

		ach["content_for"] = content_for;
		ach["ids"] = "";

		config& child = ach.add_child("in_progress", set_progress);
		prefs.add_child("achievements", ach);
		return child["progress_at"].to_int();
	}
	return 0;
}

bool sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id)
{
	// this achievement is already completed
	if(achievement(content_for, id))
	{
		return true;
	}

	for(config& ach : prefs.child_range("achievements"))
	{
		if(ach["content_for"].str() == content_for)
		{
			// check if the specific sub-achievement has been completed but the overall achievement is not completed
			for(const auto& in_progress : ach.child_range("in_progress"))
			{
				if(in_progress["id"] == id)
				{
					std::vector<std::string> sub_ids = utils::split(in_progress["sub_ids"]);
					return std::find(sub_ids.begin(), sub_ids.end(), sub_id) != sub_ids.end();
				}
			}
		}
	}
	return false;
}

void set_sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id)
{
	// this achievement is already completed
	if(achievement(content_for, id))
	{
		return;
	}

	for(config& ach : prefs.child_range("achievements"))
	{
		// if achievements already exist for this content and the achievement has not already been set, add it
		if(ach["content_for"].str() == content_for)
		{
			// check if this achievement has had sub-achievements set before
			for(config& in_progress : ach.child_range("in_progress"))
			{
				if(in_progress["id"].str() == id)
				{
					std::vector<std::string> sub_ids = utils::split(ach["ids"]);

					if(std::find(sub_ids.begin(), sub_ids.end(), sub_id) == sub_ids.end())
					{
						in_progress["sub_ids"] = in_progress["sub_ids"].str() + "," + sub_id;
					}

					in_progress["progress_at"] = sub_ids.size()+1;
					return;
				}
			}

			// else if this is the first sub-achievement being set
			config set_progress;
			set_progress["id"] = id;
			set_progress["sub_ids"] = sub_id;
			set_progress["progress_at"] = 1;
			ach.add_child("in_progress", set_progress);
			return;
		}
	}

	// else not only has this achievement not had a sub-achievement completed before, this is the first achievement for this achievement group to be added
	config ach;
	config set_progress;

	set_progress["id"] = id;
	set_progress["sub_ids"] = sub_id;
	set_progress["progress_at"] = 1;

	ach["content_for"] = content_for;
	ach["ids"] = "";

	ach.add_child("in_progress", set_progress);
	prefs.add_child("achievements", ach);
}

void set_editor_chosen_addon(const std::string& addon_id)
{
	prefs["editor_chosen_addon"] = addon_id;
}

std::string editor_chosen_addon()
{
	return prefs["editor_chosen_addon"];
}

} // end namespace preferences
