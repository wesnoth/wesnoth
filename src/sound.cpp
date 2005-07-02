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
#include "sound.hpp"
#include "wesconfig.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include <iostream>
#include <map>

namespace {

bool mix_ok = false;
std::map<std::string,Mix_Chunk*> sound_cache;
std::map<std::string,Mix_Music*> music_cache;

std::string current_music = "";

bool music_off = false;
bool sound_off = false;

}

namespace sound {

manager::manager(bool sound_on)
{
	if(!sound_on) {
		return;
	}

	SDL_Init(SDL_INIT_AUDIO);

//sounds don't sound good on Windows unless the buffer size is 4k,
//but this seems to cause crashes on other systems...
#ifdef WIN32
	const size_t buf_size = 4096;
#else
	const size_t buf_size = 1024;
#endif

	const int res = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,MIX_DEFAULT_FORMAT,2,buf_size);
	if(res >= 0) {
		mix_ok = true;
		Mix_AllocateChannels(16);
	} else {
		mix_ok = false;
		std::cerr << "Could not initialize audio: " << SDL_GetError() << "\n";
	}
}

manager::~manager()
{
	std::cerr << "closing audio...\n";
	if(!mix_ok)
		return;

	Mix_HaltMusic();
	Mix_HaltChannel(-1);

	for(std::map<std::string,Mix_Chunk*>::iterator i = sound_cache.begin();
	    i != sound_cache.end(); ++i) {
		Mix_FreeChunk(i->second);
	}

	for(std::map<std::string,Mix_Music*>::iterator j = music_cache.begin();
	    j != music_cache.end(); ++j) {
		Mix_FreeMusic(j->second);
	}

	std::cerr << "final closing audio...\n";
	Mix_CloseAudio();
	std::cerr << "done closing audio...\n";
}

void play_music(const std::string& file)
{
	if(!mix_ok || current_music == file)
		return;

	if(music_off) {
		current_music = file;
		return;
	}

	std::map<std::string,Mix_Music*>::const_iterator itor = music_cache.find(file);
	if(itor == music_cache.end()) {
		const std::string& filename = get_binary_file_location("music",file);

		if(filename.empty()) {
			return;
		}

		Mix_Music* const music = Mix_LoadMUS(filename.c_str());
		if(music == NULL) {
			std::cerr << "Could not load music file '" << filename << "': "
			          << SDL_GetError() << "\n";
			return;
		}

		itor = music_cache.insert(std::pair<std::string,Mix_Music*>(file,music)).first;
	}

	if(Mix_PlayingMusic()) {
		Mix_FadeOutMusic(500);
	}

	const int res = Mix_FadeInMusic(itor->second,-1,500);
	if(res < 0) {
		std::cerr << "Could not play music: " << SDL_GetError() << "\n";
	}

	current_music = file;
}

void play_sound(const std::string& file)
{
	if(!mix_ok || sound_off)
		return;

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
			std::cerr << "Could not load sound file '" << filename << "': "
			          << SDL_GetError() << "\n";
			return;
		}
	}

	//play on the first available channel
	const int res = Mix_PlayChannel(-1, cache, 0);
	if(res < 0) {
		std::cerr << "error playing sound effect: " << SDL_GetError() << "\n";
	}
}

void set_music_volume(double vol)
{
	if(!mix_ok)
		return;

	if(vol < 0.05) {
		Mix_HaltMusic();
		music_off = true;
		return;
	}

	Mix_VolumeMusic(int(vol*double(MIX_MAX_VOLUME)));

	//if the music was off completely, start playing it again now
	if(music_off) {
		music_off = false;
		const std::string music = current_music;
		current_music = "";
		if(!music.empty()) {
			play_music(music);
		}
	}
}

void set_sound_volume(double vol)
{
	if(!mix_ok)
		return;

	if(vol < 0.05) {
		sound_off = true;
		return;
	} else {
		sound_off = false;
		Mix_Volume(-1,int(vol*double(MIX_MAX_VOLUME)));
	}
}

}
