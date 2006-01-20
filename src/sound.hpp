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

// Read config entry, alter track list accordingly.
void play_music_config(const config &music);
// Act on any track list changes from above.
void commit_music_changes();

// Play this particular music file over and over and over.
void play_music_repeatedly(const std::string &name);
// Play this particular music file once, then silence.
void play_music_once(const std::string &name);
// Start playing current music.
void play_music();

void play_sound(const std::string& file);

// Called from event loop to see if we need new music track.
void think_about_music(void);

// Save music playlist for snapshot
void write_music_play_list(config& snapshot);

void set_music_volume(int vol);
void set_sound_volume(int vol);

}

#endif
