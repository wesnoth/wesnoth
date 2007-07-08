/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "cursor.hpp"
#include "display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "marked-up_text.hpp"
#include "preferences_display.hpp"
#include "construct_dialog.hpp"
#include "show_dialog.hpp"
#include "video.hpp"
#include "wml_separators.hpp"
#include "widgets/button.hpp"
#include "widgets/label.hpp"
#include "widgets/menu.hpp"
#include "widgets/slider.hpp"
#include "widgets/textbox.hpp"
#include "theme.hpp"

#include <vector>
#include <string>

namespace preferences {

display* disp = NULL;

display_manager::display_manager(display* d)
{
	disp = d;

	load_hotkeys();

	set_grid(grid());
	set_turbo(turbo());
	set_turbo_speed(turbo_speed());
	set_fullscreen(fullscreen());
	set_gamma(gamma());
	set_colour_cursors(preferences::get("colour_cursors") == "yes");
}

display_manager::~display_manager()
{
	disp = NULL;
}

void set_fullscreen(bool ison)
{
	_set_fullscreen(ison);

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
				gui::message_dialog(*disp,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen.")).show();
			}
			// we reinit color cursors, because SDL on Mac seems to forget the SDL_Cursor
			set_colour_cursors(preferences::get("colour_cursors") == "yes");
		}
	}
}

void set_resolution(const std::pair<int,int>& resolution)
{
	std::pair<int,int> res = resolution;

	// - Ayin: disabled the following code. Why would one want to enforce that?
	// Some 16:9, or laptop screens, may have resolutions which do not
	// comply to this rule (see bug 10630). I'm commenting this until it
	// proves absolutely necessary.
	//
	//make sure resolutions are always divisible by 4
	//res.first &= ~3;
	//res.second &= ~3;

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
			gui::message_dialog(*disp,"",_("The video mode could not be changed. Your window manager must be set to 16 bits per pixel to run the game in windowed mode. Your display must support 1024x768x16 to run the game full screen.")).show();
		}
	}

	if(write_resolution) {
		const std::string postfix = fullscreen() ? "resolution" : "windowsize";
		preferences::set('x' + postfix, lexical_cast<std::string>(res.first));
		preferences::set('y' + postfix, lexical_cast<std::string>(res.second));
	}
}

void set_turbo(bool ison)
{
	_set_turbo(ison);

	if(disp != NULL) {
		disp->set_turbo(ison);
	}
}

void set_turbo_speed(double speed)
{
	save_turbo_speed(speed);

	if(disp != NULL) {
		disp->set_turbo_speed(speed);
	}
}

void set_adjust_gamma(bool val)
{
	//if we are turning gamma adjustment off, then set it to '1.0'
	if(val == false && adjust_gamma()) {
		CVideo& video = disp->video();
		video.setGamma(1.0);
	}

	_set_adjust_gamma(val);
}

void set_gamma(int gamma)
{
	_set_gamma(gamma);

	if(adjust_gamma()) {
		CVideo& video = disp->video();
		video.setGamma((float)gamma / 100);
	}
}

void set_grid(bool ison)
{
	_set_grid(ison);

	if(disp != NULL) {
		disp->set_grid(ison);
	}
}

void set_colour_cursors(bool value)
{
	_set_colour_cursors(value);

	cursor::set();
}

}
