/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "cursor.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "video.hpp" // non_interactive()
#include "wesconfig.h"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>

namespace {

config prefs;

bool muted_ = false;
bool colour_cursors = false;

bool message_private_on = true;

bool haloes = true;

bool unit_genders = true;

bool fps = false;

std::set<std::string> encountered_units_set;
std::set<std::string> encountered_terrains_set;

}

namespace preferences {

manager::manager()
{
	scoped_istream stream = istream_file(get_prefs_file());
	read(prefs, *stream);
	set_music_volume(music_volume());
	set_sound_volume(sound_volume());

	set_colour_cursors(prefs["colour_cursors"] == "yes");
	set_show_haloes(prefs["show_haloes"] != "no");

	std::vector<std::string> v;
	v = utils::split(prefs["encountered_units"]);
	std::copy(v.begin(), v.end(),
			  std::inserter(encountered_units_set, encountered_units_set.begin()));
	v = utils::split(prefs["encountered_terrains"]);
	std::copy(v.begin(), v.end(),
			  std::inserter(encountered_terrains_set, encountered_terrains_set.begin()));
}

manager::~manager()
{

	std::vector<std::string> v;
	std::copy(encountered_units_set.begin(), encountered_units_set.end(), std::back_inserter(v));
	prefs["encountered_units"] = utils::join(v);
	v.clear();
	std::copy(encountered_terrains_set.begin(), encountered_terrains_set.end(),
			  std::back_inserter(v));
	prefs["encountered_terrains"] = utils::join(v);
	encountered_units_set.clear();
	encountered_terrains_set.clear();
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

std::string get(const std::string key) {
	return prefs[key];
}

namespace {
	bool is_fullscreen = false;
}

bool fullscreen()
{
	static bool first_time = true;
	if(first_time) {
		const string_map::const_iterator fullscreen =
	                                   prefs.values.find("fullscreen");
		is_fullscreen = fullscreen == prefs.values.end() || fullscreen->second == "true";
	}

	return is_fullscreen;
}

void _set_fullscreen(bool ison)
{
	is_fullscreen = ison;
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

		//make sure resolutions are always divisible by 4
		//res.first &= ~3;
		//res.second &= ~3;
		return res;
	} else {
		return std::pair<int,int>(1024,768);
	}
}
bool turbo()
{
	if(non_interactive())
		return true;

	const string_map::const_iterator turbo = prefs.values.find("turbo");
	return turbo != prefs.values.end() && turbo->second == "true";
}

void _set_turbo(bool ison)
{
	prefs["turbo"] = (ison ? "true" : "false");
}

const std::string& language()
{
	return prefs["locale"];
}

void set_language(const std::string& s)
{
	prefs["locale"] = s;
}

int music_volume()
{
	return lexical_cast_default<int>(prefs["music_volume"], 100);
}

void set_music_volume(int vol)
{
	prefs["music_volume"] = lexical_cast_default<std::string>(vol, "100");
	sound::set_music_volume(music_volume());
}

int sound_volume()
{
	return lexical_cast_default<int>(prefs["sound_volume"], 100);
}

void set_sound_volume(int vol)
{
	prefs["sound_volume"] = lexical_cast_default<std::string>(vol, "100");
	sound::set_sound_volume(sound_volume());
}

void mute(bool muted)
{
	sound::set_music_volume(muted ? 0 : music_volume());
	sound::set_sound_volume(muted ? 0 : sound_volume());
	muted_ = muted;
}

bool adjust_gamma()
{
	return prefs["adjust_gamma"] == "yes";
}

void _set_adjust_gamma(bool val)
{
	prefs["adjust_gamma"] = val ? "yes" : "no";
}

int gamma()
{
	static const int default_value = 100;
	const string_map::const_iterator gamma = prefs.values.find("gamma");
	if(adjust_gamma() && gamma != prefs.values.end() && gamma->second.empty() == false)
		return atoi(gamma->second.c_str());
	else
		return default_value;
}

void _set_gamma(int gamma)
{
	std::stringstream stream;
	stream << gamma;
	prefs["gamma"] = stream.str();
}

bool is_muted()
{
	return muted_;
}

bool grid()
{
	const string_map::const_iterator turbo = prefs.values.find("grid");
	return turbo != prefs.values.end() && turbo->second == "true";
}

void _set_grid(bool ison)
{
	prefs["grid"] = (ison ? "true" : "false");
}

const std::string& official_network_host()
{
	static const std::string host = WESNOTH_DEFAULT_SERVER;
	return host;
}

const std::string& network_host()
{
	t_string& res = prefs["host"];
	if(res.empty())
		res = WESNOTH_DEFAULT_SERVER;

	return res;
}

void set_network_host(const std::string& host)
{
	prefs["host"] = host;
}

const std::string& campaign_server()
{
	t_string& res = prefs["campaign_server"];
	if(res.empty())
		res = "campaigns.wesnoth.org";

	return res;
}

void set_campaign_server(const std::string& host)
{
	prefs["campaign_server"] = host;
}

const std::string& login()
{
	t_string& res = prefs["login"];
	if(res.empty()) {
		char* const login = getenv("USER");
		if(login != NULL) {
			res = login;
		}

		if(res.empty()) {
			res = _("player");
		}
	}

	return res;
}

void set_login(const std::string& username)
{
	prefs["login"] = username;
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

bool turn_bell()
{
	return prefs["turn_bell"] == "yes";
}

void set_turn_bell(bool ison)
{
	prefs["turn_bell"] = (ison ? "yes" : "no");
}

const std::string& turn_cmd()
{
	return prefs["turn_cmd"];
}

void set_turn_cmd(const std::string& cmd)
{
	prefs["turn_cmd"] = cmd;
}

bool message_bell()
{
	return prefs["message_bell"] != "no";
}

void set_message_bell(bool ison)
{
	prefs["message_bell"] = (ison ? "yes" : "no");
}

bool sound_on() {
	return prefs["sound"] != "no";
}

bool set_sound(bool ison) {
	if(!sound_on() && ison) {
		prefs["sound"] = "yes";
		if(!music_on()) {
			if(!sound::init_sound()) {
				prefs["sound"] = "no";
				return false;
			}
		}
	} else if(sound_on() && !ison) {
		prefs["sound"] = "no";
		sound::stop_sound();
		if(!music_on())
			sound::close_sound();
	}
	return true;
}

bool music_on() {
	return prefs["music"] != "no";
}

bool set_music(bool ison) {
	if(!music_on() && ison) {
		prefs["music"] = "yes";
		if(!sound_on()) {
			if(!sound::init_sound()) {
				prefs["music"] = "no";
				return false;
			}
		}
		else
			sound::play_music("");
	} else if(music_on() && !ison) {
		prefs["music"] = "no";
		if(!sound_on())
			sound::close_sound();
		else
			sound::stop_music();
	}
	return true;
}

bool turn_dialog()
{
	return prefs["turn_dialog"] == "yes";
}

void set_turn_dialog(bool ison)
{
	prefs["turn_dialog"] = (ison ? "yes" : "no");
}

bool show_combat()
{
	return prefs["show_combat"] != "no";
}

bool allow_observers()
{
	return prefs["allow_observers"] != "no";
}

void set_allow_observers(bool value)
{
	prefs["allow_observers"] = value ? "yes" : "no";
}

bool use_map_settings()
{
	return prefs["mp_use_map_settings"] == "yes";
}

void set_use_map_settings(bool value)
{
	prefs["mp_use_map_settings"] = value ? "yes" : "no";
}

bool fog()
{
	return prefs["mp_fog"] == "yes";
}

void set_fog(bool value)
{
	prefs["mp_fog"] = value ? "yes" : "no";
}

bool shroud()
{
	return prefs["mp_shroud"] == "yes";
}

void set_shroud(bool value)
{
	prefs["mp_shroud"] = value ? "yes" : "no";
}

int turns()
{
	static const int default_value = 50;
	int value = 0;
	const string_map::const_iterator i = prefs.values.find("mp_turns");
	if(i != prefs.values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	if(value < 20 || value > 100) {
		value = default_value;
	}

	return value;
}

void set_turns(int value)
{
	std::stringstream stream;
	stream << value;
	prefs["mp_turns"] = stream.str();
}

int village_gold()
{
	static const int default_value = 2;
	int value = 0;
	const string_map::const_iterator i = prefs.values.find("mp_village_gold");
	if(i != prefs.values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	if(value < 1 || value > 5) {
		value = default_value;
	}

	return value;
}

void set_village_gold(int value)
{
	std::stringstream stream;
	stream << value;
	prefs["mp_village_gold"] = stream.str();
}

int xp_modifier()
{
	static const int default_value = 70;
	int value = 0;
	const string_map::const_iterator i = prefs.values.find("mp_xp_modifier");
	if(i != prefs.values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	if(value < 30 || value > 200) {
		value = default_value;
	}

	return value;
}

void set_xp_modifier(int value)
{
	std::stringstream stream;
	stream << value;
	prefs["mp_xp_modifier"] = stream.str();
}

int era()
{
	int value = 0;
	const string_map::const_iterator i = prefs.values.find("mp_era");
	if(i != prefs.values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}

	return value;
}

void set_era(int value)
{
	std::stringstream stream;
	stream << value;
	prefs["mp_era"] = stream.str();
}

int map()
{
	int value = 0;
	const string_map::const_iterator i = prefs.values.find("mp_map");
	if(i != prefs.values.end() && i->second.empty() == false) {
		value = atoi(i->second.c_str());
	}
	return value;
}

void set_map(int value)
{
	std::stringstream stream;
	stream << value;
	prefs["mp_map"] = stream.str();
}

bool show_ai_moves()
{
	return prefs["show_ai_moves"] != "no";
}

void set_show_ai_moves(bool value)
{
	prefs["show_ai_moves"] = value ? "yes" : "no";
}

void set_show_side_colours(bool value)
{
	prefs["show_side_colours"] = value ? "yes" : "no";
}

bool show_side_colours()
{
	return prefs["show_side_colours"] != "no";
}

void set_ask_delete_saves(bool value)
{
	prefs["ask_delete"] = value ? "yes" : "no";
}

bool ask_delete_saves()
{
	return prefs["ask_delete"] != "no";
}

std::string client_type()
{
	if(prefs["client_type"] == "ai")
		return "ai";
	else
		return "human";
}

std::string clock_format()
{
	if(prefs["clock_format"].size())
		return prefs["clock_format"];
	else
	  prefs["clock_format"]="%H:%M:%S";
		return "%H:%M:%S";
}

const std::string& theme()
{
	if(non_interactive()) {
		static const std::string null_theme = "null";
		return null_theme;
	}

	t_string& res = prefs["theme"];
	if(res.empty()) {
		res = "Default";
	}

	return res;
}

void set_theme(const std::string& theme)
{
	if(theme != "null") {
		prefs["theme"] = theme;
	}
}

bool use_colour_cursors()
{
	return colour_cursors;
}

void set_colour_cursors(bool value)
{
	prefs["colour_cursors"] = value ? "yes" : "no";
	colour_cursors = value;

	cursor::use_colour(value);
}

bool show_floating_labels()
{
	return prefs["floating_labels"] != "no";
}

void set_show_floating_labels(bool value)
{
	prefs["floating_labels"] = value ? "yes" : "no";
}

bool message_private()
{
	return message_private_on;
}

void set_message_private(bool value)
{
	message_private_on = value;
}

bool show_tip_of_day()
{
	return prefs["tip_of_day"] != "no";
}

void set_show_tip_of_day(bool value)
{
	prefs["tip_of_day"] = value ? "yes" : "no";
}

bool show_haloes()
{
	return haloes;
}

void set_show_haloes(bool value)
{
	haloes = value;
	prefs["show_haloes"] = value ? "yes" : "no";
}

bool flip_time()
{
	return prefs["flip_time"] == "yes";
}

void set_flip_time(bool value)
{
	prefs["flip_time"] = value ? "yes" : "no";
}

bool show_fps()
{
	return fps;
}

void set_show_fps(bool value)
{
	fps = value;
}

bool compress_saves()
{
	return prefs["compress_saves"] != "no";
}

std::set<std::string> &encountered_units() {
	return encountered_units_set;
}

std::set<std::string> &encountered_terrains() {
	return encountered_terrains_set;
}

CACHE_SAVES_METHOD cache_saves()
{
	if(prefs["cache_saves"] == "always") {
		return CACHE_SAVES_ALWAYS;
	} else if(prefs["cache_saves"] == "never") {
		return CACHE_SAVES_NEVER;
	} else {
		return CACHE_SAVES_ASK;
	}
}

void set_cache_saves(CACHE_SAVES_METHOD method)
{
	switch(method) {
	case CACHE_SAVES_ALWAYS:
		prefs["cache_saves"] = "always";
		break;
	case CACHE_SAVES_NEVER:
		prefs["cache_saves"] = "never";
		break;
	case CACHE_SAVES_ASK:
		prefs["cache_saves"] = "ask";
		break;
	}
}

bool compare_resolutions(const std::pair<int,int>& lhs, const std::pair<int,int>& rhs)
{
	return lhs.first*lhs.second < rhs.first*rhs.second;
}

bool green_confirm()
{
	std::string confirmation = prefs["confirm_end_turn"];

	if (confirmation == "green" || confirmation == "yes")
		return true;
	return false;
}

bool yellow_confirm()
{
	return prefs["confirm_end_turn"] == "yellow";
}

bool confirm_no_moves()
{
	//This is very non-intrusive so it is on by default
	const std::string confirmation = prefs["confirm_end_turn"];
	return confirmation == "no_moves" || confirmation.empty();
}


void load_hotkeys() {
	hotkey::load_hotkeys(prefs);
}
void save_hotkeys() {
	hotkey::save_hotkeys(prefs);
}

}
