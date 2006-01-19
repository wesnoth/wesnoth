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
std::map<std::string,Mix_Music*> music_cache;

struct music_track
{
	music_track(const std::string &tname);
	music_track(const std::string &tname,
				const std::string &ms_before_str,
				const std::string &ms_after_str);
	std::string name;
	unsigned int ms_before, ms_after;
};

music_track::music_track(const std::string &tname)
	: name(tname), ms_before(0), ms_after(0)
{
}
music_track::music_track(const std::string &tname,
						 const std::string &ms_before_str,
						 const std::string &ms_after_str)
	: name(tname)
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

std::vector<music_track> current_music_list;
struct music_track current_music("");

const struct music_track &random_track()
{
	wassert(!current_music_list.empty());
	return current_music_list[rand()%current_music_list.size()];
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

		LOG_AUDIO << "Audio initialized.\n";

		play_music();
	}
	return true;
}

void close_sound() {
	int numtimesopened, frequency, channels;
	Uint16 format;
	if(mix_ok) {
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
		Mix_HaltChannel(-1);

		std::map<std::string,Mix_Chunk*>::iterator i;
		for(i = sound_cache.begin(); i != sound_cache.end(); ++i)
			Mix_FreeChunk(i->second);
		sound_cache.clear();
	}
}

void think_about_music(void)
{
	if (!music_start_time) {
		if (!current_music_list.empty() && !Mix_PlayingMusic()) {
			// Pick next track, add ending time to its start time.
			unsigned end_time = current_music.ms_after;
			current_music = random_track();
			music_start_time = SDL_GetTicks() + end_time + current_music.ms_before;
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
	current_music_list = std::vector<music_track>();
	current_music = music_track(file);
	play_music();
}

void play_music()
{
	music_start_time = 0;

	if(!preferences::music_on() || !mix_ok || current_music_list.empty())
		return;

	std::map<std::string,Mix_Music*>::const_iterator itor = music_cache.find(current_music.name);
	if(itor == music_cache.end()) {
		const std::string& filename = get_binary_file_location("music", current_music.name);

		if(filename.empty()) {
			return;
		}

		Mix_Music* const music = Mix_LoadMUS(filename.c_str());
		if(music == NULL) {
			ERR_AUDIO << "Could not load music file '" << filename << "': "
					  << Mix_GetError() << "\n";
			return;
		}

		itor = music_cache.insert(std::pair<std::string,Mix_Music*>(current_music.name,music)).first;
	}

	if(Mix_PlayingMusic()) {
		Mix_FadeOutMusic(500);
	}

	const int res = Mix_FadeInMusic(itor->second, 1, 500);
	if(res < 0) {
		ERR_AUDIO << "Could not play music: " << Mix_GetError() << " " << current_music.name <<" \n";
	}
}

void play_music_file(const std::string &name)
{
	// Can happen if scenario doesn't specify.
	if (name.empty())
		return;

	current_music_list = std::vector<music_track>(1, music_track(name));

	// If we're already playing it, don't interrupt.
	if (current_music.name != name) {
		current_music = music_track(name);
		play_music();
	}
}

void play_music_list(const config::child_list &list)
{
	bool current_track_ok = false;

	current_music_list = std::vector<music_track>();
	for (config::const_child_iterator i = list.begin(); i != list.end(); i++) {
		const std::string &name = (*i)->get_attribute("file");
		current_music_list.push_back(music_track(name,
												 (*i)->get_attribute("ms_before"),
												 (*i)->get_attribute("ms_after")));

		// If we're playing something, that's OK if it's in new list, too.
		if (Mix_PlayingMusic() && name == current_music.name) {
			current_track_ok = true;
		}
	}

	// FIXME: if they want ms_before, don't start music immediately.
	if (!current_track_ok) {
		current_music = random_track();
		play_music();
	}
}

void play_sound(const std::string& file)
{
	if(preferences::sound_on() && mix_ok) {
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
		const int res = Mix_PlayChannel(-1, cache, 0);
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
		Mix_Volume(-1, vol);
	}
}

}
