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
#ifndef SOUND_HPP_INCLUDED
#define SOUND_HPP_INCLUDED

#include <string>
#include <vector>
#include "config.hpp"

namespace sound {

struct manager {
	manager();
	~manager();
};

bool init_sound();
void close_sound();

void stop_music();
void stop_sound();

void play_music_list(const config::child_list &list);
void play_music_file(const std::string &name);
void play_music_once(const std::string &name);
void play_music();
void play_sound(const std::string& file);

// Called from event loop to see if we need new music track.
void think_about_music(void);

void set_music_volume(int vol);
void set_sound_volume(int vol);

}

#endif
