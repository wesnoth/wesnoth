/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "cursor.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "widgets/button.hpp"
#include "widgets/slider.hpp"
#include "widgets/menu.hpp"
#include "wesconfig.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iterator>

namespace {

config prefs;

display* disp = NULL;

bool muted_ = false;
bool colour_cursors = false;

bool message_private_on = true;

bool haloes = true;

std::set<std::string> encountered_units_set;
std::set<std::string> encountered_terrains_set;

}

namespace preferences {

manager::manager()
{
	prefs.read(read_file(get_prefs_file()));
	set_music_volume(music_volume());
	set_sound_volume(sound_volume());

	set_colour_cursors(prefs["colour_cursors"] == "yes");
	set_show_haloes(prefs["show_haloes"] != "no");

	std::vector<std::string> v;
	v = config::split(prefs["encountered_units"]);
	std::copy(v.begin(), v.end(),
			  std::inserter(encountered_units_set, encountered_units_set.begin()));
	v = config::split(prefs["encountered_terrains"]);
	std::copy(v.begin(), v.end(),
			  std::inserter(encountered_terrains_set, encountered_terrains_set.begin()));
}

manager::~manager()
{
	
	std::vector<std::string> v;
	std::copy(encountered_units_set.begin(), encountered_units_set.end(), std::back_inserter(v));
	prefs["encountered_units"] = config::join(v);
	v.clear();
	std::copy(encountered_terrains_set.begin(), encountered_terrains_set.end(),
			  std::back_inserter(v));
	prefs["encountered_terrains"] = config::join(v);
	encountered_units_set.clear();
	encountered_terrains_set.clear();
	try {
		write_file(get_prefs_file(),prefs.write());
	} catch(io_exception&) {
		std::cerr << "error writing to preferences file '" << get_prefs_file() << "'\n";
	}
}

display_manager::display_manager(display* d)
{
	disp = d;

	hotkey::add_hotkeys(prefs,true);

	set_grid(grid());
	set_turbo(turbo());
	set_fullscreen(fullscreen());
	set_gamma(gamma());
}

display_manager::~display_manager()
{
	disp = NULL;
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

void set_fullscreen(bool ison)
{
	is_fullscreen = ison;
	prefs["fullscreen"] = (ison ? "true" : "false");

	if(disp != NULL) {
		const std::pair<int,int>& res = resolution();
		CVideo& video = disp->video();
		if(video.isFullScreen() != ison) {
			const int flags = ison ? FULL_SCREEN : 0;
			const int bpp = video.modePossible(res.first,res.second,16,flags);
			if(bpp > 0) {
				video.setMode(res.first,res.second,bpp,flags);
				disp->redraw_everything();
			} else if(video.modePossible(1024,768,16,flags)) {
				set_resolution(std::pair<int,int>(1024,768));
			} else {
				gui::show_dialog(*disp,NULL,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen."),
				                 gui::MESSAGE);
			}
		}
	}
}

std::pair<int,int> resolution()
{
	const std::string postfix = fullscreen() ? "resolution" : "windowsize";
	const string_map::const_iterator x = prefs.values.find('x' + postfix);
	const string_map::const_iterator y = prefs.values.find('y' + postfix);
	if(x != prefs.values.end() && y != prefs.values.end() &&
	   x->second.empty() == false && y->second.empty() == false) {
		std::pair<int,int> res (maximum(atoi(x->second.c_str()),800),
		                        maximum(atoi(y->second.c_str()),600));

		//make sure resolutions are always divisible by 4
		res.first &= ~3;
		res.second &= ~3;
		return res;
	} else {
		return std::pair<int,int>(1024,768);
	}
}

void set_resolution(const std::pair<int,int>& resolution)
{
	std::pair<int,int> res = resolution;

	//make sure resolutions are always divisible by 4
	res.first &= ~3;
	res.second &= ~3;

	bool write_resolution = true;

	if(disp != NULL) {
		CVideo& video = disp->video();
		const int flags = fullscreen() ? FULL_SCREEN : 0;
		const int bpp = video.modePossible(res.first,res.second,16,flags);
		if(bpp != 0) {
			video.setMode(res.first,res.second,bpp,flags);
			disp->redraw_everything();

		} else {
			write_resolution = false;
			gui::show_dialog(*disp,NULL,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen."),gui::MESSAGE);
		}
	}

	if(write_resolution) {
	const std::string postfix = fullscreen() ? "resolution" : "windowsize";
		prefs['x' + postfix] = lexical_cast<std::string>(res.first);
		prefs['y' + postfix] = lexical_cast<std::string>(res.second);
	}
}

bool turbo()
{
	if(non_interactive())
		return true;

	const string_map::const_iterator turbo = prefs.values.find("turbo");
	return turbo != prefs.values.end() && turbo->second == "true";
}

void set_turbo(bool ison)
{
	prefs["turbo"] = (ison ? "true" : "false");

	if(disp != NULL) {
		disp->set_turbo(ison);
	}
}

const std::string& locale()
{
	return prefs["locale"];
}

void set_locale(const std::string& s)
{
	prefs["locale"] = s;
}

int music_volume()
{
	static const int default_value = 100;
	const string_map::const_iterator volume = prefs.values.find("music_volume");
	if(volume != prefs.values.end() && volume->second.empty() == false)
		return atoi(volume->second.c_str());
	else
		return default_value;
}

void set_music_volume(int vol)
{
	std::stringstream stream;
	stream << vol;
	prefs["music_volume"] = stream.str();

	sound::set_music_volume(vol / 100.0);
}

int sound_volume()
{
	static const int default_value = 100;
	const string_map::const_iterator volume = prefs.values.find("sound_volume");
	if(volume != prefs.values.end() && volume->second.empty() == false)
		return atoi(volume->second.c_str());
	else
		return default_value;
}

void set_sound_volume(int vol)
{
	std::stringstream stream;
	stream << vol;
	prefs["sound_volume"] = stream.str();

	sound::set_sound_volume(vol / 100.0);
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

void set_adjust_gamma(bool val)
{
	//if we are turning gamma adjustment off, then set it to '1.0'
	if(val == false && adjust_gamma()) {
		CVideo& video = disp->video();
		video.setGamma(1.0);
	}

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

void set_gamma(int gamma)
{
	std::stringstream stream;
	stream << gamma;
	prefs["gamma"] = stream.str();

	if(adjust_gamma()) {
		CVideo& video = disp->video();
		video.setGamma((float)gamma / 100);
	}
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

void set_grid(bool ison)
{
	prefs["grid"] = (ison ? "true" : "false");

	if(disp != NULL) {
		disp->set_grid(ison);
	}
}

const std::string& official_network_host()
{
	static const std::string host = WESNOTH_DEFAULT_SERVER;
	return host;
}

const std::string& network_host()
{
	std::string& res = prefs["host"];
	if(res.empty())
		res = WESNOTH_DEFAULT_SERVER;

	return res;
}

void set_network_host(const std::string& host)
{
	prefs["host"] = host;
}

const std::string& login()
{
	std::string& res = prefs["login"];
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
	return prefs["show_side_colours"] == "yes";
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

const std::string& theme()
{
	if(non_interactive()) {
		static const std::string null_theme = "null";
		return null_theme;
	}

	std::string& res = prefs["theme"];
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

namespace {

const SDL_Rect empty_rect = {0,0,0,0};
const SDL_Rect preferences_dialog_area = {-400,-400,400,400};

class preferences_dialog : public gui::preview_pane
{
public:
	preferences_dialog(display& disp);

	struct video_mode_change_exception
	{
		enum TYPE { CHANGE_RESOLUTION, MAKE_FULLSCREEN, MAKE_WINDOWED };

		video_mode_change_exception(TYPE type) : type(type)
		{}

		TYPE type;
	};

private:

	void draw();
	void process();
	bool left_side() const { return false; }
	void set_selection(int index);

	gui::slider music_slider_, sound_slider_, scroll_slider_, gamma_slider_;
	gui::button fullscreen_button_, turbo_button_, show_ai_moves_button_,
	            show_grid_button_, show_floating_labels_button_, turn_dialog_button_,
				turn_bell_button_, show_team_colours_button_, show_colour_cursors_button_,
				show_haloing_button_, video_mode_button_, hotkeys_button_, gamma_button_;
	std::string music_label_, sound_label_, scroll_label_, gamma_label_;
	size_t slider_label_width_;

	enum TAB { GENERAL_TAB, DISPLAY_TAB, SOUND_TAB };
	TAB tab_;
};

preferences_dialog::preferences_dialog(display& disp) : gui::preview_pane(disp),
music_slider_(disp,empty_rect), sound_slider_(disp,empty_rect),
scroll_slider_(disp,empty_rect), gamma_slider_(disp,empty_rect),
fullscreen_button_(disp,_("Full Screen"),gui::button::TYPE_CHECK),
turbo_button_(disp,_("Accelerated Speed"),gui::button::TYPE_CHECK),
show_ai_moves_button_(disp,_("Skip AI Moves"),gui::button::TYPE_CHECK),
show_grid_button_(disp,_("Show Grid"),gui::button::TYPE_CHECK),
show_floating_labels_button_(disp,_("Show Floating Labels"),gui::button::TYPE_CHECK),
turn_dialog_button_(disp,_("Turn Dialog"),gui::button::TYPE_CHECK),
turn_bell_button_(disp,_("Turn Bell"),gui::button::TYPE_CHECK),
show_team_colours_button_(disp,_("Show Team Colors"),gui::button::TYPE_CHECK),
show_colour_cursors_button_(disp,_("Show Color Cursors"),gui::button::TYPE_CHECK),
show_haloing_button_(disp,_("Show Haloing Effects"),gui::button::TYPE_CHECK),
video_mode_button_(disp,_("Video Mode")),
hotkeys_button_(disp,_("Hotkeys")),
gamma_button_(disp,_("Adjust Gamma"),gui::button::TYPE_CHECK),
music_label_(_("Music Volume:")), sound_label_(_("SFX Volume:")),
scroll_label_(_("Scroll Speed:")), gamma_label_(_("Gamma:")),
slider_label_width_(0), tab_(GENERAL_TAB)
{
	set_location(preferences_dialog_area);

	slider_label_width_ = maximum<size_t>(font::text_area(music_label_,14).w,
	                      maximum<size_t>(font::text_area(sound_label_,14).w,
						  maximum<size_t>(font::text_area(scroll_label_,14).w,font::text_area(gamma_label_,14).w)));

	sound_slider_.set_min(1);
	sound_slider_.set_max(100);
	sound_slider_.set_value(sound_volume());
	sound_slider_.set_help_string(_("Change the sound effects volume"));

	music_slider_.set_min(1);
	music_slider_.set_max(100);
	music_slider_.set_value(music_volume());
	music_slider_.set_help_string(_("Change the music volume"));

	scroll_slider_.set_min(1);
	scroll_slider_.set_max(100);
	scroll_slider_.set_value(scroll_speed());
	scroll_slider_.set_help_string(_("Change the speed of scrolling around the map"));

	gamma_button_.set_check(adjust_gamma());
	gamma_button_.set_help_string(_("Change the brightness of the display"));

	gamma_slider_.set_min(50);
	gamma_slider_.set_max(200);
	gamma_slider_.set_value(gamma());
	gamma_slider_.set_help_string(_("Change the brightness of the display"));

	fullscreen_button_.set_check(fullscreen());
	fullscreen_button_.set_help_string(_("Choose whether the game should run full screen or in a window"));

	turbo_button_.set_check(turbo());
	turbo_button_.set_help_string(_("Make units move and fight faster"));

	show_ai_moves_button_.set_check(!show_ai_moves());
	show_ai_moves_button_.set_help_string(_("Do not animate AI units moving"));

	show_grid_button_.set_check(grid());
	show_grid_button_.set_help_string(_("Overlay a grid onto the map"));

	show_floating_labels_button_.set_check(show_floating_labels());
	show_floating_labels_button_.set_help_string(_("Show text above a unit when it is hit to display damage inflicted"));

	video_mode_button_.set_help_string(_("Change the resolution the game runs at"));

	turn_dialog_button_.set_check(turn_dialog());
	turn_dialog_button_.set_help_string(_("Display a dialog at the beginning of your turn"));

	turn_bell_button_.set_check(turn_bell());
	turn_bell_button_.set_help_string(_("Play a bell sound at the beginning of your turn"));

	show_team_colours_button_.set_check(show_side_colours());
	show_team_colours_button_.set_help_string(_("Show a colored circle around the base of each unit to show which side it is on"));

	show_colour_cursors_button_.set_check(use_colour_cursors());
	show_colour_cursors_button_.set_help_string(_("Use colored mouse cursors (may be slower)"));

	show_haloing_button_.set_check(show_haloes());
	show_haloing_button_.set_help_string(_("Use graphical special effects (may be slower)"));

	hotkeys_button_.set_help_string(_("View and configure keyboard shortcuts"));
}

void preferences_dialog::process()
{
	if(turbo_button_.pressed()) {
		set_turbo(turbo_button_.checked());
	}

	if(show_ai_moves_button_.pressed()) {
		set_show_ai_moves(!show_ai_moves_button_.checked());
	}

	if(show_grid_button_.pressed()) {
		set_grid(show_grid_button_.checked());
	}

	if(show_floating_labels_button_.pressed()) {
		set_show_floating_labels(show_floating_labels_button_.checked());
	}

	if(video_mode_button_.pressed()) {
		throw video_mode_change_exception(video_mode_change_exception::CHANGE_RESOLUTION);
	}

	if(fullscreen_button_.pressed()) {
		throw video_mode_change_exception(fullscreen_button_.checked() ? video_mode_change_exception::MAKE_FULLSCREEN
		                                                               : video_mode_change_exception::MAKE_WINDOWED);
	}

	if(turn_bell_button_.pressed()) {
		set_turn_bell(turn_bell_button_.checked());
	}

	if(turn_dialog_button_.pressed()) {
		set_turn_dialog(turn_dialog_button_.checked());
	}

	if(show_team_colours_button_.pressed()) {
		set_show_side_colours(show_team_colours_button_.checked());
	}

	if(hotkeys_button_.pressed()) {
		show_hotkeys_dialog(disp());
	}

	if(show_colour_cursors_button_.pressed()) {
		set_colour_cursors(show_colour_cursors_button_.checked());
	}

	if(show_haloing_button_.pressed()) {
		set_show_haloes(show_haloing_button_.checked());
	}

	if(gamma_button_.pressed()) {
		set_adjust_gamma(gamma_button_.checked());
		set_selection(int(tab_));
	}

	set_sound_volume(sound_slider_.value());
	set_music_volume(music_slider_.value());
	set_scroll_speed(scroll_slider_.value());
	set_gamma(gamma_slider_.value());
}

void preferences_dialog::draw()
{
	if(!dirty()) {
		return;
	}

	bg_restore();

	music_slider_.set_dirty();
	sound_slider_.set_dirty();
	scroll_slider_.set_dirty();
	gamma_slider_.set_dirty();
	fullscreen_button_.set_dirty();
	turbo_button_.set_dirty();
	show_ai_moves_button_.set_dirty();
	show_grid_button_.set_dirty();
	show_floating_labels_button_.set_dirty();
	turn_dialog_button_.set_dirty();
	turn_bell_button_.set_dirty();
	show_team_colours_button_.set_dirty();
	show_colour_cursors_button_.set_dirty();
	show_haloing_button_.set_dirty();
	video_mode_button_.set_dirty();
	hotkeys_button_.set_dirty();
	gamma_button_.set_dirty();

	const int border = 10;

	int ypos = location().y;
	if(tab_ == GENERAL_TAB) {
		font::draw_text(&disp(),location(),14,font::NORMAL_COLOUR,scroll_label_,location().x,ypos);
		const SDL_Rect scroll_rect = {location().x + slider_label_width_,ypos,location().w - slider_label_width_ - border,scroll_slider_.location().h};
		scroll_slider_.set_location(scroll_rect);

		ypos += 50;
		turbo_button_.set_location(location().x,ypos);

		ypos += 50;
		show_ai_moves_button_.set_location(location().x,ypos);

		ypos += 50;
		turn_dialog_button_.set_location(location().x,ypos);

		ypos += 50;
		turn_bell_button_.set_location(location().x,ypos);

		ypos += 50;
		show_team_colours_button_.set_location(location().x,ypos);

		ypos += 50;
		show_grid_button_.set_location(location().x,ypos);

		ypos += 50;
		hotkeys_button_.set_location(location().x,ypos);

	} else if(tab_ == DISPLAY_TAB) {
		gamma_button_.set_location(location().x,ypos);

		ypos += 50;

		if(adjust_gamma()) {
			font::draw_text(&disp(),location(),14,font::NORMAL_COLOUR,gamma_label_,location().x,ypos);
		}

		const SDL_Rect gamma_rect = {location().x + slider_label_width_,ypos,location().w - slider_label_width_ - border,gamma_slider_.location().h};
		gamma_slider_.set_location(gamma_rect);

		ypos += 50;
		show_floating_labels_button_.set_location(location().x,ypos);

		ypos += 50;
		show_colour_cursors_button_.set_location(location().x,ypos);

		ypos += 50;
		show_haloing_button_.set_location(location().x,ypos);

		ypos += 50;
		fullscreen_button_.set_location(location().x,ypos);

		ypos += 50;
		video_mode_button_.set_location(location().x,ypos);

	} else if(tab_ == SOUND_TAB) {
		font::draw_text(&disp(),location(),14,font::NORMAL_COLOUR,music_label_,location().x,ypos);
		const SDL_Rect music_rect = {location().x + slider_label_width_,ypos,location().w - slider_label_width_ - border,music_slider_.location().h};
		music_slider_.set_location(music_rect);

		ypos += 50;

		font::draw_text(&disp(),location(),14,font::NORMAL_COLOUR,sound_label_,location().x,ypos);
		const SDL_Rect sound_rect = {location().x + slider_label_width_,ypos,location().w - slider_label_width_ - border,sound_slider_.location().h};
		sound_slider_.set_location(sound_rect);
	}

	set_dirty(false);
}

void preferences_dialog::set_selection(int index)
{
	tab_ = TAB(index);
	set_dirty();

	scroll_slider_.hide(tab_ != GENERAL_TAB);
	gamma_slider_.hide(tab_ != DISPLAY_TAB || !adjust_gamma());
	music_slider_.hide(tab_ != SOUND_TAB);
	sound_slider_.hide(tab_ != SOUND_TAB);
	turbo_button_.hide(tab_ != GENERAL_TAB);
	show_ai_moves_button_.hide(tab_ != GENERAL_TAB);
	turn_dialog_button_.hide(tab_ != GENERAL_TAB);
	turn_bell_button_.hide(tab_ != GENERAL_TAB);
	hotkeys_button_.hide(tab_ != GENERAL_TAB);
	show_team_colours_button_.hide(tab_ != GENERAL_TAB);
	show_grid_button_.hide(tab_ != GENERAL_TAB);

	gamma_button_.hide(tab_ != DISPLAY_TAB);
	show_floating_labels_button_.hide(tab_ != DISPLAY_TAB);
	show_colour_cursors_button_.hide(tab_ != DISPLAY_TAB);
	show_haloing_button_.hide(tab_ != DISPLAY_TAB);
	fullscreen_button_.hide(tab_ != DISPLAY_TAB);
	video_mode_button_.hide(tab_ != DISPLAY_TAB);
}                                          

}

void show_preferences_dialog(display& disp)
{
	std::vector<std::string> items;

	items.push_back(std::string("&icons/icon-general.png,") + dsgettext(GETTEXT_DOMAIN,"Prefs section^General"));
	items.push_back(std::string("&icons/icon-display.png,") + dsgettext(GETTEXT_DOMAIN,"Prefs section^Display"));
	items.push_back(std::string("&icons/icon-music.png,") + dsgettext(GETTEXT_DOMAIN,"Prefs section^Sound"));
	
	for(;;) {
		try {
			const events::event_context dialog_events_context;

			preferences_dialog dialog(disp);
			std::vector<gui::preview_pane*> panes;
			panes.push_back(&dialog);

			gui::show_dialog(disp,NULL,_("Preferences"),"",gui::CLOSE_ONLY,&items,&panes);
			return;
		} catch(preferences_dialog::video_mode_change_exception& e) {
			switch(e.type) {
			case preferences_dialog::video_mode_change_exception::CHANGE_RESOLUTION:
				show_video_mode_dialog(disp);
				break;
			case preferences_dialog::video_mode_change_exception::MAKE_FULLSCREEN:
				set_fullscreen(true);
				break;
			case preferences_dialog::video_mode_change_exception::MAKE_WINDOWED:
				set_fullscreen(false);
				break;
			}

			if(items[1].empty() || items[1][0] != '*') {
				items[1] = "*" + items[1];
			}
		}
	}
}

bool compare_resolutions(const std::pair<int,int>& lhs, const std::pair<int,int>& rhs)
{
	return lhs.first*lhs.second < rhs.first*rhs.second;
}

bool show_video_mode_dialog(display& disp)
{
	const events::resize_lock prevent_resizing;
	const events::event_context dialog_events_context;

	CVideo& video = disp.video();

	SDL_PixelFormat format = *video.getSurface()->format;
	format.BitsPerPixel = video.getBpp();

	const SDL_Rect* const * const modes = SDL_ListModes(&format,FULL_SCREEN);

	//the SDL documentation says that a return value of -1 if no dimension
	//is available.
	if(modes == reinterpret_cast<SDL_Rect**>(-1) || modes == NULL) {
		if(modes != NULL)
			std::cerr << "Can support any video mode\n";
		else
			std::cerr << "No modes supported\n";
		gui::show_dialog(disp,NULL,"",_("There are no alternative video modes available"));
		return false;
	}

	std::vector<std::pair<int,int> > resolutions;

	for(int i = 0; modes[i] != NULL; ++i) {
		if(modes[i]->w >= 800 && modes[i]->h >= 600) {
			resolutions.push_back(std::pair<int,int>(modes[i]->w,modes[i]->h));
		}
	}

	const std::pair<int,int> current_res(video.getSurface()->w,video.getSurface()->h);
	resolutions.push_back(current_res);

	std::sort(resolutions.begin(),resolutions.end(),compare_resolutions);
	resolutions.erase(std::unique(resolutions.begin(),resolutions.end()),resolutions.end());

	std::vector<std::string> options;
	for(std::vector<std::pair<int,int> >::const_iterator j = resolutions.begin(); j != resolutions.end(); ++j) {
		std::ostringstream option;
		if(*j == current_res) {
			option << char(gui::menu::DEFAULT_ITEM);
		}

		option << j->first << "x" << j->second;
		options.push_back(option.str());
	}

	const int result = gui::show_dialog(disp,NULL,"",
	                                    _("Choose Resolution"),
	                                    gui::OK_CANCEL,&options);
	if(size_t(result) < resolutions.size() && resolutions[result] != current_res) {
		set_resolution(resolutions[result]);
		return true;
	} else {
		return false;
	}
}

void show_hotkeys_dialog (display & disp, config *save_config)
{
	log_scope ("show_hotkeys_dialog");

	const events::event_context dialog_events_context;

	const int centerx = disp.x()/2;
	const int centery = disp.y()/2;
	const int xpos = centerx  - 300;
	const int ypos = centery  - 250;
	const int width = 600;
	const int height = 500;

	gui::button close_button (disp, _("Close Window"));
	std::vector<gui::button*> buttons;
	buttons.push_back(&close_button);

	surface_restorer restorer;	
	gui::draw_dialog(xpos,ypos,width,height,disp,_("Hotkey Settings"),NULL,&buttons,&restorer);
	
	SDL_Rect clip_rect = { 0, 0, disp.x (), disp.y () };
	SDL_Rect text_size = font::draw_text(NULL, clip_rect, 16,
			           font::NORMAL_COLOUR,_("Press desired HotKey"),
						0, 0);

	std::vector<std::string> menu_items;

	std::vector<hotkey::hotkey_item> hotkeys = hotkey::get_hotkeys();
	for(std::vector<hotkey::hotkey_item>::iterator i = hotkeys.begin(); i != hotkeys.end(); ++i) {
		std::stringstream str,name;
		name<< hotkey::command_to_description(i->action);
		str << name.str();
		str << ",  :  ,";
		str << hotkey::get_hotkey_name(*i); 
		menu_items.push_back (str.str ());
	}

	gui::menu menu_ (disp, menu_items, false, height);
	menu_.set_width(400);	
	menu_.set_loc (xpos + 20, ypos);
	
	gui::button change_button (disp, _("Change Hotkey"));
	change_button.set_location(xpos + width - change_button.width () -30,ypos + 80);

	gui::button save_button (disp, _("Save Hotkeys"));
	save_button.set_location(xpos + width - save_button.width () - 30,ypos + 130);

	bool redraw_all = true;

	for(;;) {

		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState (&mousex, &mousey);
		const bool left_button = mouse_flags & SDL_BUTTON_LMASK;

		if(redraw_all) {
			menu_.redraw();
			close_button.draw ();
			change_button.draw();
			save_button.draw();
						
			redraw_all = false;
		};

		if(close_button.process (mousex, mousey, left_button)) {
			break;
		}

		if(change_button.process (mousex, mousey, left_button)) {
			// Lets change this hotkey......
			SDL_Rect dlgr = {centerx-text_size.w/2-30,
								centery-text_size.h/2 - 16,
									text_size.w+60,
									text_size.h+32};
			surface_restorer restorer(&disp.video(),dlgr);										
		 	gui::draw_dialog_frame (centerx-text_size.w/2 - 20, 
									centery-text_size.h/2 - 6,
									text_size.w+40,
									text_size.h+12,disp);
			font::draw_text (&disp, clip_rect, 18,font::NORMAL_COLOUR,
				 _("Press desired HotKey"),centerx-text_size.w/2-10,
				 centery-text_size.h/2-3);
			disp.update_display();
			SDL_Event event;
			event.type = 0;
			int key=0; //just to avoid warning
			int mod=0;
			bool used = false;
			while (event.type!=SDL_KEYDOWN) SDL_PollEvent(&event);
			do {
				if (event.type==SDL_KEYDOWN)
				{
				 	key=event.key.keysym.sym;
				 	mod=event.key.keysym.mod;
				};			
				SDL_PollEvent(&event);
				disp.video().flip();
				SDL_Delay(10);
			} while (event.type!=SDL_KEYUP);
			restorer.restore();
			disp.update_display();
			for (std::vector < hotkey::hotkey_item >::iterator i =
	     		hotkeys.begin (); i != hotkeys.end (); i++)
			{ 
				if((i->keycode == key) 
					&& (i->alt == ((mod&KMOD_ALT) != 0))
					&& (i->ctrl == ((mod&KMOD_CTRL) != 0))
					&& (i->shift == ((mod&KMOD_SHIFT) != 0))
					&& (i->command == ((mod&KMOD_LMETA) != 0))) {
					used = true;
				}
			}
			if(used) {
				gui::show_dialog(disp,NULL,"",_("This HotKey is already in use."),gui::MESSAGE);
			} else {
				hotkeys[menu_.selection()].alt =  ((mod&KMOD_ALT) != 0);
				hotkeys[menu_.selection()].ctrl = ((mod&KMOD_CTRL) != 0);
				hotkeys[menu_.selection()].shift = ((mod&KMOD_SHIFT) != 0);
				hotkeys[menu_.selection()].command = ((mod&KMOD_LMETA) != 0);
				hotkeys[menu_.selection()].keycode = key;
				hotkey::change_hotkey(hotkeys[menu_.selection()]);
				menu_.change_item(menu_.selection(),2,
							hotkey::get_hotkey_name(hotkeys[menu_.selection()]));
			};
			redraw_all = true;
		}
		if (save_button.process (mousex, mousey, left_button))
		{
			if (save_config == NULL) {
				hotkey::save_hotkeys(prefs);
			}
			else {
				hotkey::save_hotkeys(*save_config);
			}
			redraw_all = true;
		}

		menu_.process (mousex, mousey, left_button, false,
			       false, false, false);

		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		
		disp.update_display();

		SDL_Delay(10);
	}
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

}
