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

bool init_sound();
void close_sound();
void reset_sound();

void stop_music();
void stop_sound();
void stop_UI_sound();
void stop_bell();

// Read config entry, alter track list accordingly.
void play_music_config(const config &music);
// Act on any track list changes from above.
void commit_music_changes();

// Play this particular music file over and over and over.
void play_music_repeatedly(const std::string &name);
// Play this particular music file once, then silence.
void play_music_once(const std::string &name);
// Empty the playlist.
void play_no_music();
// Start playing current music.
void play_music();

// Change parameters of a playing sound, given its id
void reposition_sound(int id, unsigned int distance);

// Check if there's a sound associated with given id playing
bool is_sound_playing(int id);

// Stop sound associated with a given id
void stop_sound(int id);

// Play sound, or random one of comma-separated sounds.
void play_sound(const std::string& files, int channel = -1);

// Play sound, or random one of comma-separated sounds. Use specified
// distance and associate it with specified id (of a sound source).
void play_sound_positioned(const std::string &files, int id, unsigned int distance);

// Play sound, or random one of comma-separated sounds in bell channel
void play_bell(const std::string& files);

// Play user-interface sound, or random one of comma-separated sounds.
void play_UI_sound(const std::string& files);

// Called from event loop to see if we need new music track.
void think_about_music(void);

// Save music playlist for snapshot
void write_music_play_list(config& snapshot);

void set_music_volume(int vol);
void set_sound_volume(int vol);
void set_bell_volume(int vol);
void set_UI_volume(int vol);

}

#endif
