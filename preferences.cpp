/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "filesystem.hpp"
#include "font.hpp"
#include "language.hpp"
#include "menu.hpp"
#include "preferences.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "widgets/button.hpp"
#include "widgets/slider.hpp"

#include <cstdlib>
#include <sstream>

namespace {

config prefs;

display* disp;

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
	prefs.values["fullscreen"] = (ison ? "true" : "false");

	if(disp != NULL) {
		CVideo& video = disp->video();
		if(video.isFullScreen() != ison) {
			const int flags = ison ? FULL_SCREEN : 0;
			if(video.modePossible(1024,768,16,flags)) {
				video.setMode(1024,768,16,flags);
				disp->redraw_everything();
			} else {
				gui::show_dialog(*disp,NULL,"",string_table["video_mode_fail"],
				                 gui::MESSAGE);
			}
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
	prefs.values["turbo"] = (ison ? "true" : "false");

	if(disp != NULL) {
		disp->set_turbo(ison);
	}
}

const std::string& locale()
{
	return prefs.values["locale"];
}

void set_locale(const std::string& s)
{
	prefs.values["locale"] = s;
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
	prefs.values["music_volume"] = stream.str();

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
	prefs.values["sound_volume"] = stream.str();

	sound::set_sound_volume(vol);
}

bool grid()
{
	const string_map::const_iterator turbo = prefs.values.find("grid");
	return turbo != prefs.values.end() && turbo->second == "true";
}

void set_grid(bool ison)
{
	prefs.values["grid"] = (ison ? "true" : "false");

	if(disp != NULL) {
		disp->set_grid(ison);
	}
}

void show_preferences_dialog(display& disp)
{
	const int border_size = 6;
	const int xpos = disp.x()/2 - 300;
	const int ypos = disp.y()/2 - 200;
	const int width = 600;
	const int height = 400;

	disp.invalidate_all();
	disp.draw();
	
	SDL_Rect clip_rect = {0,0,disp.x(),disp.y()};
	SDL_Rect title_rect = font::draw_text(NULL,clip_rect,16,font::NORMAL_COLOUR,
	                                      string_table["preferences"],0,0);

	gui::button close_button(disp,string_table["close_window"]);

	close_button.set_x(xpos + width/2 - close_button.width()/2);
	close_button.set_y(ypos + height - close_button.height()-14);

	const std::string& music_label = string_table["music_volume"];
	const std::string& sound_label = string_table["sound_volume"];

	SDL_Rect music_rect = {0,0,0,0};
	music_rect = font::draw_text(NULL,clip_rect,14,font::NORMAL_COLOUR,
	                             music_label,0,0);
	
	SDL_Rect sound_rect = {0,0,0,0};
	sound_rect = font::draw_text(NULL,clip_rect,14,font::NORMAL_COLOUR,
	                             sound_label,0,0);
	
	const int text_right = xpos + maximum(music_rect.w,sound_rect.w) + 5;

	const int music_pos = ypos + title_rect.h + 20;
	const int sound_pos = music_pos + 50;
	
	music_rect.x = text_right - music_rect.w;
	music_rect.y = music_pos;

	sound_rect.x = text_right - sound_rect.w;
	sound_rect.y = sound_pos;

	const int slider_left = text_right + 10;
	const int slider_right = xpos + width - 5;
	if(slider_left >= slider_right)
		return;

	SDL_Rect slider_rect = { slider_left,sound_pos,slider_right-slider_left,10};
	gui::slider sound_slider(disp,slider_rect,sound_volume());

	slider_rect.y = music_pos;
	gui::slider music_slider(disp,slider_rect,music_volume());

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

	bool redraw_all = true;

	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;
		const bool right_button = mouse_flags&SDL_BUTTON_RMASK;

		if(close_button.process(mousex,mousey,left_button)) {
			break;
		}

		const double new_music=music_slider.process(mousex,mousey,left_button);
		const double new_sound=sound_slider.process(mousex,mousey,left_button);

		if(new_sound >= 0.0) {
			set_sound_volume(new_sound);
		}

		if(new_music >= 0.0) {
			set_music_volume(new_music);
		}

		if(fullscreen_button.process(mousex,mousey,left_button)) {
			set_fullscreen(fullscreen_button.checked());
			redraw_all = true;
		}

		if(redraw_all) {
			gui::draw_dialog_frame(xpos,ypos,width,height,disp);
			sound_slider.background_changed();
			music_slider.background_changed();
			sound_slider.draw();
			music_slider.draw();
			fullscreen_button.draw();
			turbo_button.draw();
			grid_button.draw();
			close_button.draw();

			font::draw_text(&disp,clip_rect,14,font::NORMAL_COLOUR,music_label,
	                        music_rect.x,music_rect.y);

			font::draw_text(&disp,clip_rect,14,font::NORMAL_COLOUR,sound_label,
	    	                sound_rect.x,sound_rect.y);

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

		disp.update_display();

		SDL_Delay(10);
		SDL_PumpEvents();
	}

	disp.invalidate_all();
	disp.draw();
}

}
