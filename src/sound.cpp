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

#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "random.hpp"
#include "sound.hpp"
#include "wassert.hpp"
#include "wesconfig.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include <iostream>
#include <map>

#define LOG_AUDIO LOG_STREAM(info, audio)
#define ERR_AUDIO LOG_STREAM(err, audio)

namespace {
bool mix_ok = false;
unsigned music_start_time = 0;
std::map<std::string,Mix_Chunk*> sound_cache;
std::map<std::string,Mix_Chunk*> bell_cache; // Bell sound use separate channel
std::map<std::string,Mix_Music*> music_cache;

struct music_track
{
	music_track(const std::string &tname);
	music_track(const std::string &tname,
				const std::string &ms_before_str,
				const std::string &ms_after_str);
	void write(config &snapshot, bool append);

	std::string name;
	unsigned int ms_before, ms_after;
	bool once;
};

std::vector<std::string> played_before;

music_track::music_track(const std::string &tname)
	: name(tname), ms_before(0), ms_after(0), once(false)
{
}
music_track::music_track(const std::string &tname,
						 const std::string &ms_before_str,
						 const std::string &ms_after_str)
	: name(tname), once(false)
{
	if (ms_before_str.empty())
		ms_before = 0;
	else
		ms_before = lexical_cast<int,std::string>(ms_before_str);

	if (ms_after_str.empty())
		ms_after = 0;
	else
		ms_after = lexical_cast<int,std::string>(ms_after_str);
}

void music_track::write(config &snapshot, bool append)
{
		config& m = snapshot.add_child("music");
		m["name"] = name;
		m["ms_before"] = lexical_cast<std::string>(ms_before);
		m["ms_after"] = lexical_cast<std::string>(ms_after);
		if (append)
			m["append"] = "yes";
}

std::vector<music_track> current_track_list;
struct music_track current_track("");

bool track_ok(const std::string &name)
{
	LOG_AUDIO << "Considering " << name << "\n";

	// If they committed changes to list, we forget previous plays, but
	// still *never* repeat same track twice if we have an option.
	if (name == current_track.name)
		return false;

	if (current_track_list.size() <= 3)
		return true;

	// Timothy Pinkham says:
	// 1) can't be repeated without 2 other pieces have already played
	// since A was played.
	// 2) cannot play more than 2 times without every other piece
	// having played at least 1 time.

	// Dammit, if our musicians keep coming up with algorithms, I'll
	// be out of a job!
	unsigned int num_played = 0;
	std::set<std::string> played;
	std::vector<std::string>::reverse_iterator i;

	for (i = played_before.rbegin(); i != played_before.rend(); i++) {
		if (*i == name) {
			num_played++;
			if (num_played == 2)
				break;
		} else {
			played.insert(*i);
		}
	}

	// If we've played this twice, must have played every other track.
	if (num_played == 2 && played.size() != current_track_list.size() - 1) {
		LOG_AUDIO << "Played twice with only "
				  << lexical_cast<std::string>(played.size())
				  << " tracks between\n";
		return false;
	}

	// Check previous previous track not same.
	i = played_before.rbegin();
	if (i != played_before.rend()) {
		i++;
		if (i != played_before.rend()) {
			if (*i == name) {
				LOG_AUDIO << "Played just before previous\n";
				return false;
			}
		}
	}

	return true;
}


const struct music_track &choose_track()
{
	wassert(!current_track_list.empty());

	std::string name;
	unsigned int track = 0;

	if (current_track_list.size() > 1) {
		do {
			track = rand()%current_track_list.size();
		} while (!track_ok(current_track_list[track].name));
	}

	//LOG_AUDIO << "Next track will be " << current_track_list[track].name << "\n";
	played_before.push_back(current_track_list[track].name);
	return current_track_list[track];
}

std::string pick_one(const std::string &files)
{
	std::vector<std::string> names = utils::split(files);

	if (names.size() == 0)
		return "";
	if (names.size() == 1)
		return names[0];

	// We avoid returning same choice twice if we can avoid it.
	static std::map<std::string,unsigned int> prev_choices;
	unsigned int choice;

	if (prev_choices.find(files) != prev_choices.end()) {
		choice = rand()%(names.size()-1);
		if (choice >= prev_choices[files])
			choice++;
		prev_choices[files] = choice;
	} else {
		choice = rand()%names.size();
		prev_choices.insert(std::pair<std::string,unsigned int>(files,choice));
	}

	return names[choice];
}
};

namespace sound {
manager::manager()
{
}
manager::~manager()
{
	close_sound();
}

bool init_sound() {
//sounds don't sound good on Windows unless the buffer size is 4k,
//but this seems to cause crashes on other systems...
#ifdef WIN32
	const size_t buf_size = 4096;
#else
	const size_t buf_size = 1024;
#endif
	if(SDL_WasInit(SDL_INIT_AUDIO) == 0)
		if(SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
			return false;

	if(!mix_ok) {
		if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,buf_size) == -1) {
			mix_ok = false;
			ERR_AUDIO << "Could not initialize audio: " << Mix_GetError() << "\n";
			return false;
		}

		mix_ok = true;
		Mix_AllocateChannels(16);
		set_sound_volume(preferences::sound_volume());
		set_music_volume(preferences::music_volume());
		set_bell_volume(preferences::bell_volume());

		LOG_AUDIO << "Audio initialized.\n";

		play_music();
	}
	return true;
}

void close_sound() {
	int numtimesopened, frequency, channels;
	Uint16 format;
	if(mix_ok) {
		stop_bell();
		stop_sound();
		stop_music();
		mix_ok = false;

		numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
		if(numtimesopened == 0) {
			ERR_AUDIO << "Error closing audio device: " << Mix_GetError() << "\n";
		}
		while (numtimesopened) {
			Mix_CloseAudio();
			--numtimesopened;
		}
	}
	if(SDL_WasInit(SDL_INIT_AUDIO) != 0)
		SDL_QuitSubSystem(SDL_INIT_AUDIO);

	LOG_AUDIO << "Audio device released.\n";
}

void stop_music() {
	if(mix_ok) {
		Mix_HaltMusic();

		std::map<std::string,Mix_Music*>::iterator i;
		for(i = music_cache.begin(); i != music_cache.end(); ++i)
			Mix_FreeMusic(i->second);
		music_cache.clear();
	}
}

void stop_sound() {
	if(mix_ok) {
		for (int i = 0; i < 15; i++)
			Mix_HaltChannel(i);

		std::map<std::string,Mix_Chunk*>::iterator i;
		for(i = sound_cache.begin(); i != sound_cache.end(); ++i)
			Mix_FreeChunk(i->second);
		sound_cache.clear();
	}
}

void stop_bell() {
	if(mix_ok) {
		Mix_HaltChannel(15);

		std::map<std::string,Mix_Chunk*>::iterator i;
		for(i = bell_cache.begin(); i != bell_cache.end(); ++i)
			Mix_FreeChunk(i->second);
		bell_cache.clear();
	}
}

void think_about_music(void)
{
	if (!music_start_time) {
		if (!current_track_list.empty() && !Mix_PlayingMusic()) {
			// Pick next track, add ending time to its start time.
			unsigned end_time = current_track.ms_after;
			current_track = choose_track();
			music_start_time = SDL_GetTicks() + end_time + current_track.ms_before;
		}
	} else {
		if (SDL_GetTicks() >= music_start_time) {
			play_music();
		}
	}
}

void play_music_once(const std::string &file)
{
	// Clear list so it's not replayed.
	current_track_list = std::vector<music_track>();
	current_track = music_track(file);
	play_music();
}

void play_no_music()
{
	current_track_list = std::vector<music_track>();
	current_track = music_track("");

	if (preferences::music_on() && mix_ok && Mix_PlayingMusic()) {
		Mix_FadeOutMusic(500);
	}
}

void play_music()
{
	music_start_time = 0;

	if(!preferences::music_on() || !mix_ok || current_track.name.empty())
		return;

	std::map<std::string,Mix_Music*>::const_iterator itor = music_cache.find(current_track.name);
	if(itor == music_cache.end()) {
		const std::string& filename = get_binary_file_location("music", current_track.name);

		if(filename.empty()) {
			ERR_AUDIO << "Could not open track '" << current_track.name << "'\n";
			return;
		}

		Mix_Music* const music = Mix_LoadMUS(filename.c_str());
		if(music == NULL) {
			ERR_AUDIO << "Could not load music file '" << filename << "': "
					  << Mix_GetError() << "\n";
			return;
		}
		itor = music_cache.insert(std::pair<std::string,Mix_Music*>(current_track.name,music)).first;
	}
	LOG_AUDIO << "Playing track '" << current_track.name << "'\n";

	if(Mix_PlayingMusic()) {
		Mix_FadeOutMusic(500);
	}

	const int res = Mix_FadeInMusic(itor->second, 1, 500);
	if(res < 0) {
		ERR_AUDIO << "Could not play music: " << Mix_GetError() << " " << current_track.name <<" \n";
	}
}

void play_music_repeatedly(const std::string &name)
{
	// Can happen if scenario doesn't specify.
	if (name.empty())
		return;

	current_track_list = std::vector<music_track>(1, music_track(name));

	// If we're already playing it, don't interrupt.
	if (current_track.name != name) {
		current_track = music_track(name);
		play_music();
	}
}

void play_music_config(const config &music)
{
	struct music_track track(music["name"],
							 music["ms_before"],
							 music["ms_after"]);

	// If they say play once, we don't alter playlist.
	if (utils::string_bool(music["play_once"])) {
		current_track = track;
		current_track.once = true;
		play_music();
		return;
	}

	// Clear play list unless they specify append.
	if (!utils::string_bool(music["append"])) {
		current_track_list = std::vector<music_track>();
	}

	current_track_list.push_back(track);

	// They can tell us to start playing this list immediately.
	if (utils::string_bool(music["immediate"])) {
		current_track = track;
		play_music();
	}
}

void commit_music_changes()
{
	std::vector<music_track>::iterator i;

	// Clear list of tracks played before.
	played_before = std::vector<std::string>();

	// Play-once is OK if still playing.
	if (current_track.once)
		return;

	// If current track no longer on playlist, change it.
	for (i = current_track_list.begin(); i != current_track_list.end(); i++) {
		if (current_track.name == i->name)
			return;
	}

	// Victory empties playlist: if next scenario doesn't specify one...
	if (current_track_list.empty())
		return;

	// FIXME: we don't pause ms_before on this first track.  Should we?
	current_track = choose_track();
	play_music();
}

void write_music_play_list(config& snapshot)
{
	std::vector<music_track>::iterator i;
	bool append = false;

	// First entry clears playlist, others append to it.
	for (i = current_track_list.begin(); i != current_track_list.end(); i++) {
		i->write(snapshot, append);
		append = true;
	}
}

void play_sound(const std::string& files)
{

	if(files.empty()) return;
	if(preferences::sound_on() && mix_ok) {
		std::string file = pick_one(files);
		// the insertion will fail if there is already an element in the cache
		std::pair< std::map< std::string, Mix_Chunk * >::iterator, bool >
			it = sound_cache.insert(std::make_pair(file, (Mix_Chunk *)0));
		Mix_Chunk *&cache = it.first->second;
		if (it.second) {
			std::string const &filename = get_binary_file_location("sounds", file);
			if (!filename.empty()) {
#ifdef USE_ZIPIOS
				std::string const &s = read_file(filename);
				if (!s.empty()) {
					SDL_RWops* ops = SDL_RWFromMem((void*)s.c_str(), s.size());
					cache = Mix_LoadWAV_RW(ops,0);
				}
#else
				cache = Mix_LoadWAV(filename.c_str());
#endif
			}

			if (cache == NULL) {
				ERR_AUDIO << "Could not load sound file '" << filename << "': "
					<< Mix_GetError() << "\n";
				return;
			}
		}

		//play on the first available channel
		//FIXME: in worst case it can play on bell channel(15), nothing happend
		// only sound can have another volume than others sounds
		const int res = Mix_PlayChannel(-1, cache, 0);
		if(res < 0) {
			ERR_AUDIO << "error playing sound effect: " << Mix_GetError() << "\n";
		}
	}
}

// Play bell on separate volume than sound
void play_bell(const std::string& files)
{
   if(files.empty()) return;
	
   if(preferences::turn_bell() && mix_ok) {
      std::string file = pick_one(files);
      // the insertion will fail if there is already an element in the cache
      std::pair< std::map< std::string, Mix_Chunk * >::iterator, bool >
         it = bell_cache.insert(std::make_pair(file, (Mix_Chunk *)0));
      Mix_Chunk *&cache = it.first->second;
      if (it.second) {
         std::string const &filename = get_binary_file_location("sounds", file);         if (!filename.empty()) {
#ifdef USE_ZIPIOS
            std::string const &s = read_file(filename);
            if (!s.empty()) {
               SDL_RWops* ops = SDL_RWFromMem((void*)s.c_str(), s.size());
               cache = Mix_LoadWAV_RW(ops,0);
            }
#else
            cache = Mix_LoadWAV(filename.c_str());
#endif
         }

         if (cache == NULL) {
            ERR_AUDIO << "Could not load sound file '" << filename << "': "
               << Mix_GetError() << "\n";
            return;
         }
      }

      //play on the last (bell) channel 
      const int res = Mix_PlayChannel(15, cache, 0);
      if(res < 0) {
         ERR_AUDIO << "error playing sound effect: " << Mix_GetError() << "\n";
      }

	
   }
}

void set_music_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > MIX_MAX_VOLUME)
			vol = MIX_MAX_VOLUME;
		Mix_VolumeMusic(vol);
	}
}

void set_sound_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > MIX_MAX_VOLUME)
			vol = MIX_MAX_VOLUME;
		// Bell has separate channel which we can't set up from this
		for (int i = 0; i < 15; i++){
			Mix_Volume(i, vol);
		}
	}
}

void set_bell_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > MIX_MAX_VOLUME)
			vol = MIX_MAX_VOLUME;
		Mix_Volume(15, vol);
	}
}


}
