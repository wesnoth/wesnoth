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

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {

config prefs;

display* disp = NULL;

}

namespace preferences {

manager::manager()
{
	prefs.read(read_file(get_prefs_file()));

	set_music_volume(music_volume());
	set_sound_volume(sound_volume());
}

manager::~manager()
{
	write_file(get_prefs_file(),prefs.write());
}

display_manager::display_manager(display* d)
{
	disp = d;

	add_hotkeys(prefs);

	set_grid(grid());
	set_turbo(turbo());
	set_fullscreen(fullscreen());
}

display_manager::~display_manager()
{
	disp = NULL;
}

bool fullscreen()
{
	const string_map::const_iterator fullscreen =
	                                   prefs.values.find("fullscreen");
	return fullscreen == prefs.values.end() || fullscreen->second == "true";
}

void set_fullscreen(bool ison)
{
	prefs["fullscreen"] = (ison ? "true" : "false");

	if(disp != NULL) {
		const std::pair<int,int>& res = resolution();
		CVideo& video = disp->video();
		if(video.isFullScreen() != ison) {
			const int flags = ison ? FULL_SCREEN : 0;
			if(video.modePossible(res.first,res.second,16,flags)) {
				video.setMode(res.first,res.second,16,flags);
				disp->redraw_everything();
			} else {
				gui::show_dialog(*disp,NULL,"",string_table["video_mode_fail"],
				                 gui::MESSAGE);
			}
		}
	}
}

std::pair<int,int> resolution()
{
	const string_map::const_iterator x = prefs.values.find("xresolution");
	const string_map::const_iterator y = prefs.values.find("yresolution");
	if(x != prefs.values.end() && y != prefs.values.end() &&
	   x->second.empty() == false && y->second.empty() == false) {
		std::pair<int,int> res (maximum(atoi(x->second.c_str()),1024),
		                        maximum(atoi(y->second.c_str()),768));
		res.first &= ~1;
		res.second &= ~1;
		return res;
	} else {
		return std::pair<int,int>(1024,768);
	}
}

void set_resolution(const std::pair<int,int>& resolution)
{
	if(disp != NULL) {
		std::pair<int,int> res = resolution;

		//dimensions must be even
		res.first &= ~1;
		res.second &= ~1;

		CVideo& video = disp->video();
		const int flags = video.isFullScreen() ? FULL_SCREEN : 0;
		if(video.modePossible(res.first,res.second,16,flags)) {

			video.setMode(res.first,res.second,16,flags);

			disp->redraw_everything();

			char buf[50];
			sprintf(buf,"%d",res.first);
			prefs["xresolution"] = buf;
			sprintf(buf,"%d",res.second);
			prefs["yresolution"] = buf;
		} else {
			gui::show_dialog(*disp,NULL,"",string_table["video_mode_fail"],
			                 gui::MESSAGE);
		}
	}
}

bool turbo()
{
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

double music_volume()
{
	static const double default_value = 1.0;
	const string_map::const_iterator volume = prefs.values.find("music_volume");
	if(volume != prefs.values.end() && volume->second.empty() == false)
		return atof(volume->second.c_str());
	else
		return default_value;
}

void set_music_volume(double vol)
{
	std::stringstream stream;
	stream << vol;
	prefs["music_volume"] = stream.str();

	sound::set_music_volume(vol);
}

double sound_volume()
{
	static const double default_value = 1.0;
	const string_map::const_iterator volume = prefs.values.find("sound_volume");
	if(volume != prefs.values.end() && volume->second.empty() == false)
		return atof(volume->second.c_str());
	else
		return default_value;
}

void set_sound_volume(double vol)
{
	std::stringstream stream;
	stream << vol;
	prefs["sound_volume"] = stream.str();

	sound::set_sound_volume(vol);
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

const std::string& network_host()
{
	std::string& res = prefs["host"];
	if(res.empty())
		res = "kanetti1848.kanetti.com"; //"server.wesnoth.org";

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

double scroll_speed()
{
	return get_scroll_speed()*100.0 + 10.0;
}

double get_scroll_speed()
{
	static bool first_time = true;
	if(first_time) {
		first_time = false;
		const string_map::const_iterator itor = prefs.values.find("scroll");
		if(itor != prefs.values.end()) {
			scroll = minimum<double>(1.0,maximum<double>(0.0,
			                             atof(itor->second.c_str())));
		}
	}

	return scroll;
}

void set_scroll_speed(double new_speed)
{
	std::stringstream formatter;
	formatter << new_speed;
	prefs["scroll"] = formatter.str();
	scroll = new_speed;
}

bool turn_bell()
{
	return prefs["turn_bell"] == "yes";
}

void set_turn_bell(bool ison)
{
	prefs["turn_bell"] = (ison ? "yes" : "no");
}

bool turn_dialog()
{
	return prefs["turn_dialog"] == "yes";
}

void set_turn_dialog(bool ison)
{
	prefs["turn_dialog"] = (ison ? "yes" : "no");
}

void show_preferences_dialog(display& disp)
{
	const events::resize_lock prevent_resizing;
	
	log_scope("show_preferences_dialog");

	const int xpos = disp.x()/2 - 300;
	const int ypos = disp.y()/2 - 200;
	const int width = 600;
	const int height = 400;

	SDL_Rect clip_rect = {0,0,disp.x(),disp.y()};
	SDL_Rect title_rect = font::draw_text(NULL,clip_rect,16,font::NORMAL_COLOUR,
	                                      string_table["preferences"],0,0);

	gui::button close_button(disp,string_table["close_window"]);

	close_button.set_x(xpos + width/2 - close_button.width()/2);
	close_button.set_y(ypos + height - close_button.height()-14);

	const std::string& music_label = string_table["music_volume"];
	const std::string& sound_label = string_table["sound_volume"];
	const std::string& scroll_label = string_table["scroll_speed"];

	SDL_Rect music_rect = {0,0,0,0};
	music_rect = font::draw_text(NULL,clip_rect,14,font::NORMAL_COLOUR,
	                             music_label,0,0);

	SDL_Rect sound_rect = {0,0,0,0};
	sound_rect = font::draw_text(NULL,clip_rect,14,font::NORMAL_COLOUR,
	                             sound_label,0,0);

	SDL_Rect scroll_rect = {0,0,0,0};
	scroll_rect = font::draw_text(NULL,clip_rect,14,font::NORMAL_COLOUR,
	                              scroll_label,0,0);


	const int text_right = xpos + maximum(music_rect.w,sound_rect.w) + 5;

	const int music_pos = ypos + title_rect.h + 20;
	const int sound_pos = music_pos + 50;
	const int scroll_pos = sound_pos + 50;

	music_rect.x = text_right - music_rect.w;
	music_rect.y = music_pos;

	sound_rect.x = text_right - sound_rect.w;
	sound_rect.y = sound_pos;

	scroll_rect.x = text_right - scroll_rect.w;
	scroll_rect.y = scroll_pos;

	const int slider_left = text_right + 10;
	const int slider_right = xpos + width - 5;
	if(slider_left >= slider_right)
		return;

	SDL_Rect slider_rect = { slider_left,sound_pos,slider_right-slider_left,10};
	gui::slider sound_slider(disp,slider_rect,sound_volume());

	slider_rect.y = music_pos;
	gui::slider music_slider(disp,slider_rect,music_volume());

	slider_rect.y = scroll_pos;
	gui::slider scroll_slider(disp,slider_rect,get_scroll_speed());

	gui::button fullscreen_button(disp,string_table["full_screen"],
	                              gui::button::TYPE_CHECK);

	fullscreen_button.set_check(fullscreen());

	fullscreen_button.set_x(slider_left);
	fullscreen_button.set_y(sound_pos + 80);

	gui::button turbo_button(disp,string_table["speed_turbo"],
	                         gui::button::TYPE_CHECK);
	turbo_button.set_check(turbo());

	turbo_button.set_x(slider_left);
	turbo_button.set_y(sound_pos + 80 + 50);

	gui::button grid_button(disp,string_table["grid_button"],
	                        gui::button::TYPE_CHECK);
	grid_button.set_check(grid());

	grid_button.set_x(slider_left);
	grid_button.set_y(sound_pos + 80 + 100);

	gui::button resolution_button(disp,string_table["video_mode"]);
	resolution_button.set_x(slider_left);
	resolution_button.set_y(sound_pos + 80 + 150);

	gui::button turn_dialog_button(disp,string_table["turn_dialog_button"],
	                               gui::button::TYPE_CHECK);
	turn_dialog_button.set_check(turn_dialog());
	turn_dialog_button.set_x(slider_left+fullscreen_button.width()+30);
	turn_dialog_button.set_y(sound_pos + 80);

	gui::button turn_bell_button(disp,string_table["turn_bell_button"],
	                             gui::button::TYPE_CHECK);
	turn_bell_button.set_check(turn_bell());
	turn_bell_button.set_x(slider_left+fullscreen_button.width()+30);
	turn_bell_button.set_y(sound_pos + 80 + 50);

	bool redraw_all = true;

	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		if(close_button.process(mousex,mousey,left_button)) {
			break;
		}

		const double new_music=music_slider.process(mousex,mousey,left_button);
		const double new_sound=sound_slider.process(mousex,mousey,left_button);
		const double new_scroll =
		         scroll_slider.process(mousex,mousey,left_button);

		if(new_sound >= 0.0) {
			set_sound_volume(new_sound);
		}

		if(new_music >= 0.0) {
			set_music_volume(new_music);
		}

		if(new_scroll >= 0.0) {
			set_scroll_speed(new_scroll);
		}

		if(fullscreen_button.process(mousex,mousey,left_button)) {
			set_fullscreen(fullscreen_button.checked());
			redraw_all = true;
		}

		if(redraw_all) {
			gui::draw_dialog_frame(xpos,ypos,width,height,disp);
			sound_slider.background_changed();
			music_slider.background_changed();
			scroll_slider.background_changed();
			sound_slider.draw();
			music_slider.draw();
			scroll_slider.draw();
			fullscreen_button.draw();
			turbo_button.draw();
			grid_button.draw();
			close_button.draw();
			resolution_button.draw();
			turn_dialog_button.draw();
			turn_bell_button.draw();

			font::draw_text(&disp,clip_rect,14,font::NORMAL_COLOUR,music_label,
	                        music_rect.x,music_rect.y);

			font::draw_text(&disp,clip_rect,14,font::NORMAL_COLOUR,sound_label,
	    	                sound_rect.x,sound_rect.y);

			font::draw_text(&disp,clip_rect,14,font::NORMAL_COLOUR,scroll_label,
	    	                scroll_rect.x,scroll_rect.y);

			font::draw_text(&disp,clip_rect,18,font::NORMAL_COLOUR,
			                string_table["preferences"],
			                xpos+(width-title_rect.w)/2,ypos+10);

			redraw_all = false;
		}

		if(turbo_button.process(mousex,mousey,left_button)) {
			set_turbo(turbo_button.checked());
		}

		if(grid_button.process(mousex,mousey,left_button)) {
			set_grid(grid_button.checked());
		}

		if(resolution_button.process(mousex,mousey,left_button)) {
			show_video_mode_dialog(disp);
			break;
		}

		if(turn_bell_button.process(mousex,mousey,left_button)) {
			set_turn_bell(turn_bell_button.checked());
		}

		if(turn_dialog_button.process(mousex,mousey,left_button)) {
			set_turn_dialog(turn_dialog_button.checked());
		}

		disp.update_display();

		SDL_Delay(10);
		events::pump();
	}
}

void show_video_mode_dialog(display& disp)
{
	const events::resize_lock prevent_resizing;

	std::vector<std::pair<int,int> > resolutions;
	std::vector<std::string> options;

	CVideo& video = disp.video();
	SDL_Rect** modes = SDL_ListModes(video.getSurface()->format,FULL_SCREEN);

	//the SDL documentation says that a return value of -1 if no dimension
	//is available.
	if(modes == reinterpret_cast<SDL_Rect**>(-1) || modes == NULL) {
		if(modes != NULL)
			std::cerr << "Can support any video mode\n";
		else
			std::cerr << "No modes supported\n";
		gui::show_dialog(disp,NULL,"",string_table["video_mode_unavailable"]);
		return;
	}

	for(int i = 0; modes[i] != NULL; ++i) {
		if(modes[i]->w >= 1024 && modes[i]->h >= 768) {
			const std::pair<int,int> new_res(modes[i]->w,modes[i]->h);
			if(std::count(resolutions.begin(),resolutions.end(),new_res) > 0)
				continue;

			resolutions.push_back(new_res);

			std::stringstream option;
			option << modes[i]->w << "x" << modes[i]->h;
			options.push_back(option.str());
		}
	}

	if(resolutions.size() < 2) {
		gui::show_dialog(disp,NULL,"",string_table["video_mode_unavailable"]);
		return;
	}

	const int result = gui::show_dialog(disp,NULL,"",
	                                    string_table["choose_resolution"],
	                                    gui::MESSAGE,&options);
	if(size_t(result) < resolutions.size()) {
		set_resolution(resolutions[result]);
	}
}

}
