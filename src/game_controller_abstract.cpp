/* $Id$ */
/*
   Copyright (C) 2011 - 2012 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_controller_abstract.hpp"

#include "game_display.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "language.hpp"
#include "preferences_display.hpp"

#include "preferences.hpp"

#include <boost/foreach.hpp>

#include <iostream>

game_controller_abstract::game_controller_abstract(const commandline_options &cmdline_opts) :
	cmdline_opts_(cmdline_opts),
	disp_(NULL),
	video_()
{

}

game_display& game_controller_abstract::disp()
{
	if(disp_.get() == NULL) {
		if(get_video_surface() == NULL) {
			throw CVideo::error();
		}
		disp_.assign(game_display::create_dummy_display(video_));
	}
	return *disp_.get();
}

bool game_controller_abstract::init_joystick()
{
	if (!preferences::joystick_support_enabled())
		return false;

	if(SDL_WasInit(SDL_INIT_JOYSTICK) == 0)
		if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
			return false;

	int joysticks = SDL_NumJoysticks();
	if (joysticks == 0) return false;

	SDL_JoystickEventState(SDL_ENABLE);

	SDL_Joystick* joystick;

	bool joystick_found = false;
	for (int i = 0; i<joysticks; i++)  {

		joystick = SDL_JoystickOpen(i);

		if (joystick)
			joystick_found = true;
	}
	return joystick_found;
}

bool game_controller_abstract::init_language()
{
	if(!::load_language_list())
		return false;

	language_def locale;
	if(cmdline_opts_.language) {
		std::vector<language_def> langs = get_languages();
		BOOST_FOREACH(const language_def & def, langs) {
			if(def.localename == *cmdline_opts_.language) {
				locale = def;
				break;
			}
		}
		if(locale.localename.empty()) {
			std::cerr << "Language symbol '" << *cmdline_opts_.language << "' not found.\n";
			return false;
		}
	} else {
		locale = get_locale();
	}
	::set_language(locale);

	if(!cmdline_opts_.nogui) {
		std::string wm_title_string = _("The Battle for Wesnoth");
		wm_title_string += " - " + game_config::revision;
		SDL_WM_SetCaption(wm_title_string.c_str(), NULL);
	}

	return true;
}

bool game_controller_abstract::init_video()
{
	if(cmdline_opts_.nogui) {
		if( !(cmdline_opts_.multiplayer || cmdline_opts_.screenshot) ) {
			std::cerr << "--nogui flag is only valid with --multiplayer flag or --screenshot flag\n";
			return false;
		}
		video_.make_fake();
		game_config::no_delay = true;
		return true;
	}

#if !(defined(__APPLE__))
	surface icon(image::get_image("game-icon.png", image::UNSCALED));
	if(icon != NULL) {
		///must be called after SDL_Init() and before setting video mode
		::SDL_WM_SetIcon(icon,NULL);
	}
#endif

	std::pair<int,int> resolution;
	int bpp = 0;
	int video_flags = 0;

	bool found_matching = preferences::detect_video_settings(video_, resolution, bpp, video_flags);

	if (cmdline_opts_.bpp) {
		bpp = *cmdline_opts_.bpp;
	} else if (cmdline_opts_.screenshot) {
		bpp = 32;
	}

	if(!found_matching) {
		std::cerr << "Video mode " << resolution.first << 'x'
			<< resolution.second << 'x' << bpp
			<< " is not supported.\n";

		if ((video_flags & FULL_SCREEN)) {
			std::cerr << "Try running the program with the --windowed option "
				<< "using a " << bpp << "bpp setting for your display adapter.\n";
		} else {
			std::cerr << "Try running the program with the --fullscreen option.\n";
		}

		return false;
	}

	std::cerr << "setting mode to " << resolution.first << "x" << resolution.second << "x" << bpp << "\n";
	const int res = video_.setMode(resolution.first,resolution.second,bpp,video_flags);
	video_.setBpp(bpp);
	if(res == 0) {
		std::cerr << "required video mode, " << resolution.first << "x"
		          << resolution.second << "x" << bpp << " is not supported\n";
		return false;
	}

	return true;
}
