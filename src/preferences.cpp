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

std::string get(const std::string key) {
	return prefs[key];
}

void disable_preferences_save() {
	no_preferences_save = true;
}

config* get_prefs(){
	config* pointer = &prefs;
	return pointer;
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

double turbo_speed()
{
	return lexical_cast_default<double>(prefs["turbo_speed"], 2);
}

void save_turbo_speed(const double speed)
{
	prefs["turbo_speed"] = lexical_cast_default<std::string>(speed, "2");
}
const std::string& language()
{
	return prefs["locale"];
}

void set_language(const std::string& s)
{
	prefs["locale"] = s;
}

bool grid()
{
	const string_map::const_iterator it = prefs.values.find("grid");
	return it != prefs.values.end() && it->second == "true";
}

void _set_grid(bool ison)
{
	prefs["grid"] = (ison ? "true" : "false");
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

void load_hotkeys() {
	hotkey::load_hotkeys(prefs);
}
void save_hotkeys() {
	hotkey::save_hotkeys(prefs);
}

}
