/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file preferences.cpp 
//! Get and set user-preferences.

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "video.hpp" // non_interactive()
#include "wassert.hpp"
#include "wesconfig.h"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>

namespace {

bool colour_cursors = false;

bool no_preferences_save = false;

bool fps = false;

int draw_delay_ = 20;

config prefs;
}

namespace preferences {

base_manager::base_manager()
{
	scoped_istream stream = istream_file(get_prefs_file());
	read(prefs, *stream);
}

base_manager::~base_manager()
{
	if (no_preferences_save) return;
	try {
		scoped_ostream prefs_file = ostream_file(get_prefs_file());
		write(*prefs_file, prefs);
	} catch(io_exception&) {
		std::cerr << "error writing to preferences file '" << get_prefs_file() << "'\n";
	}
}

void set(const std::string key, std::string value) {
	prefs[key] = value;
}

void erase(const std::string key) {
	prefs.values.erase(key);
}

const std::string get(const std::string key) {
	return prefs[key];
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
	return utils::string_bool(get("fullscreen"), true);
}

void _set_fullscreen(bool ison)
{
	prefs["fullscreen"] = (ison ? "true" : "false");
}

std::pair<int,int> resolution()
{
	const std::string postfix = fullscreen() ? "resolution" : "windowsize";
	const string_map::const_iterator x = prefs.values.find('x' + postfix);
	const string_map::const_iterator y = prefs.values.find('y' + postfix);
	if(x != prefs.values.end() && y != prefs.values.end() &&
	   x->second.empty() == false && y->second.empty() == false) {
		std::pair<int,int> res (maximum(atoi(x->second.c_str()),min_allowed_width),
		                        maximum(atoi(y->second.c_str()),min_allowed_height));

		// Make sure resolutions are always divisible by 4
		//res.first &= ~3;
		//res.second &= ~3;
		return res;
	} else {
		return std::pair<int,int>(1024,768);
	}
}

bool turbo()
{
	if(non_interactive()) {
		return true;
	}

	return  utils::string_bool(get("turbo"), false);
}

void _set_turbo(bool ison)
{
	prefs["turbo"] = (ison ? "true" : "false");
}

double turbo_speed()
{
	return lexical_cast_default<double>(get("turbo_speed"), 2.0);
}

void save_turbo_speed(const double speed)
{
	preferences::set("turbo_speed", lexical_cast<std::string>(speed));
}

bool idle_anim()
{
	return  utils::string_bool(get("idle_anim"), true);
}

void _set_idle_anim(const bool ison)
{
	prefs["idle_anim"] = (ison ? "yes" : "no");
}

int idle_anim_rate()
{
	return lexical_cast_default<int>(get("idle_anim_rate"), 0);
}

void _set_idle_anim_rate(const int rate)
{
	preferences::set("idle_anim_rate", lexical_cast<std::string>(rate));
}

const std::string& language()
{
	return prefs["locale"];
}

void set_language(const std::string& s)
{
	preferences::set("locale", s);
}

bool adjust_gamma()
{
	return utils::string_bool(get("adjust_gamma"), false);
}

void _set_adjust_gamma(bool val)
{
	preferences::set("adjust_gamma", val ? "yes" : "no");
}

int gamma()
{
	static const int default_value = 100;

	if(!adjust_gamma()) {
		return default_value;
	}

	return lexical_cast_default<int>(get("gamma"), default_value);
}

void _set_gamma(int gamma)
{
	preferences::set("gamma", lexical_cast<std::string>(gamma));
}

bool grid()
{
	return utils::string_bool(get("grid"), false);
}

void _set_grid(bool ison)
{
	preferences::set("grid",  (ison ? "true" : "false"));
}

size_t sound_buffer_size()
{
	// Sounds don't sound good on Windows unless the buffer size is 4k,
	// but this seems to cause crashes on other systems...
	#ifdef WIN32
		const size_t buf_size = 4096;
	#else
		const size_t buf_size = 1024;
	#endif

	return lexical_cast_default<size_t>(get("sound_buffer_size"), buf_size);
}

void save_sound_buffer_size(const size_t size)
{
	#ifdef WIN32
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
	return lexical_cast_default<int>(get("music_volume"), 100);
}

void set_music_volume(int vol)
{
	if(music_volume() == vol) {
		return;
	}

	preferences::set("music_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_music_volume(music_volume());
}

int sound_volume()
{
	return lexical_cast_default<int>(get("sound_volume"), 100);
}

void set_sound_volume(int vol)
{
	if(sound_volume() == vol) {
		return;
	}

	preferences::set("sound_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_sound_volume(sound_volume());
}

int bell_volume()
{
	return lexical_cast_default<int>(get("bell_volume"), 100);
}

void set_bell_volume(int vol)
{
	if(bell_volume() == vol) {
		return;
	}

	preferences::set("bell_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_bell_volume(bell_volume());
}

int UI_volume()
{
	return lexical_cast_default<int>(get("UI_volume"), 100);
}

void set_UI_volume(int vol)
{
	if(UI_volume() == vol) {
		return;
	}

	preferences::set("UI_volume", lexical_cast_default<std::string>(vol, "100"));
	sound::set_UI_volume(UI_volume());
}


bool turn_bell()
{
	return get("turn_bell") == "yes";
}

bool set_turn_bell(bool ison)
{
	if(!turn_bell() && ison) {
		preferences::set("turn_bell", "yes");
		if(!music_on() && !sound_on() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("turn_bell", "no");
				return false;
			}
		}
	} else if(turn_bell() && !ison) {
		preferences::set("turn_bell", "no");
		sound::stop_bell();
		if(!music_on() && !sound_on() && !UI_sound_on())
			sound::close_sound();
	}
	return true;
}

bool UI_sound_on()
{
	return utils::string_bool(get("UI_sound"), true);
}

bool set_UI_sound(bool ison)
{
	if(!UI_sound_on() && ison) {
		preferences::set("UI_sound", "yes");
		if(!music_on() && !sound_on() && !turn_bell()) {
			if(!sound::init_sound()) {
				preferences::set("UI_sound", "no");
				return false;
			}
		}
	} else if(UI_sound_on() && !ison) {
		preferences::set("UI_sound", "no");
		sound::stop_UI_sound();
		if(!music_on() && !sound_on() && !turn_bell())
			sound::close_sound();
	}
	return true;
}

const std::string turn_cmd()
{
	return get("turn_cmd");
}

bool message_bell()
{
	return utils::string_bool(get("message_bell"), true);
}

bool sound_on()
{
	return utils::string_bool(get("sound"), true);
}

bool set_sound(bool ison) {
	if(!sound_on() && ison) {
		preferences::set("sound", "yes");
		if(!music_on() && !turn_bell() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("sound", "no");
				return false;
			}
		}
	} else if(sound_on() && !ison) {
		preferences::set("sound", "no");
		sound::stop_sound();
		if(!music_on() && !turn_bell() && !UI_sound_on())
			sound::close_sound();
	}
	return true;
}

bool music_on()
{
	return utils::string_bool(get("music"), true);
}

bool set_music(bool ison) {
	if(!music_on() && ison) {
		preferences::set("music", "yes");
		if(!sound_on() && !turn_bell() && !UI_sound_on()) {
			if(!sound::init_sound()) {
				preferences::set("music", "no");
				return false;
			}
		}
		else
			sound::play_music();
	} else if(music_on() && !ison) {
		preferences::set("music", "no");
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

int scroll_speed()
{
	static const int default_value = 50;
	int value = 0;
	const string_map::const_iterator i = prefs.values.find("scroll");
	if(i != prefs.values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	if(value < 1 || value > 100) {
		value = default_value;
	}

	scroll = value/100.0;

	return value;
}

void set_scroll_speed(int new_speed)
{
	std::stringstream stream;
	stream << new_speed;
	prefs["scroll"] = stream.str();
	scroll = new_speed / 100.0;
}

bool mouse_scroll_enabled()
{
	return utils::string_bool(get("mouse_scrolling"), true);
}

void enable_mouse_scroll(bool value)
{
	set("mouse_scrolling", value ? "yes" : "no");
}

bool animate_map()
{
	return utils::string_bool(preferences::get("animate_map"), true);
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

bool use_colour_cursors()
{
	return colour_cursors;
}

void _set_colour_cursors(bool value)
{
	preferences::set("colour_cursors", value ? "yes" : "no");
	colour_cursors = value;
}

void load_hotkeys() {
	hotkey::load_hotkeys(prefs);
}
void save_hotkeys() {
	hotkey::save_hotkeys(prefs);
}

unsigned int sample_rate()
{
	return lexical_cast_default<unsigned int>(preferences::get("sample_rate"), 44100);
}

void save_sample_rate(const unsigned int rate)
{
	if (sample_rate() == rate)
		return;

	preferences::set("sample_rate", lexical_cast<std::string>(rate));

	// If audio is open, we have to re set sample rate
	sound::reset_sound();
}

} // end namespace preferences

