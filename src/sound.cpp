/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "random.hpp"
#include "sound.hpp"
#include "wesconfig.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include <cassert>
#include <iostream>
#include <map>
#include <list>

#define LOG_AUDIO LOG_STREAM(info, audio)
#define ERR_AUDIO LOG_STREAM(err, audio)

namespace sound {
// Channel-chunk mapping lets us know, if we can safely free a given chunk
std::vector<Mix_Chunk*> channel_chunks;

// Channel-id mapping for use with sound sources (to check if given source
// is playing on a channel for fading/panning)
std::vector<int> channel_ids;

static bool play_sound_internal(const std::string& files, channel_group group, bool sound_on,
						 unsigned int distance=0, int id=-1, int loop_ticks=0, int fadein_ticks=0);
}

namespace {

bool mix_ok = false;
int music_start_time = 0;
unsigned music_refresh = 0;
unsigned music_refresh_rate = 20;
bool want_new_music = false;
int fadingout_time=5000;
bool no_fading = false;

// number of allocated channels, 
const size_t n_of_channels = 16;

// we need 2 channels, because we it for timer as well
const size_t bell_channel = 0; 
const size_t timer_channel = 1;

// number of channels reserved for sound sources
const size_t source_channels = n_of_channels - 8;
const size_t source_channel_start = timer_channel + 1;
const size_t source_channel_last = source_channel_start + source_channels - 1;
const size_t UI_sound_channel = source_channel_last + 1;
const size_t n_reserved_channels = UI_sound_channel + 1; // sources, bell, timer and UI

// Max number of sound chunks that we want to cache
// Keep this above number of available channels to avoid busy-looping
#ifdef LOW_MEM
unsigned max_cached_chunks = 64;
#else
unsigned max_cached_chunks = 256;
#endif

std::map< Mix_Chunk*, int > chunk_usage;

}

static void increment_chunk_usage(Mix_Chunk* mcp) {
	++(chunk_usage[mcp]);
}

static void decrement_chunk_usage(Mix_Chunk* mcp) {
	if(mcp == NULL) return;
	std::map< Mix_Chunk*, int >::iterator this_usage = chunk_usage.find(mcp);
	assert(this_usage != chunk_usage.end());
	if(--(this_usage->second) == 0) {
		Mix_FreeChunk(mcp);
		chunk_usage.erase(this_usage);
	}
}

namespace {

class sound_cache_chunk {
public:
	sound_cache_chunk(const std::string& f) : group(sound::NULL_CHANNEL), file(f), data_(NULL) {}
	sound_cache_chunk(const sound_cache_chunk& scc)
		: group(scc.group), file(scc.file), data_(scc.data_)
	{
		increment_chunk_usage(data_);
	}

	~sound_cache_chunk()
	{
		decrement_chunk_usage(data_);
	}

	sound::channel_group group;
	std::string file;

	void set_data(Mix_Chunk* d) {
		increment_chunk_usage(d);
		decrement_chunk_usage(data_);
		data_ = d;
	}

	Mix_Chunk* get_data() const {
		return data_;
	}

	bool operator==(sound_cache_chunk const &scc) const {
		return file == scc.file;
	}

	bool operator!=(sound_cache_chunk const &scc) const { return !operator==(scc); }

	sound_cache_chunk& operator=(const sound_cache_chunk& scc) {
		file = scc.file;
		group = scc.group;
		set_data(scc.get_data());
		return *this;
	}

private:
	Mix_Chunk* data_;
};

std::list< sound_cache_chunk > sound_cache;
typedef std::list< sound_cache_chunk >::iterator sound_cache_iterator;
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
struct music_track last_track("");

}

static bool track_ok(const std::string &name)
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


static const music_track &choose_track()
{
	assert(!current_track_list.empty());

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

static std::string pick_one(const std::string &files)
{
	std::vector<std::string> names = utils::split(files);

	if (names.size() == 0)
		return "";
	if (names.size() == 1)
		return names[0];

#ifdef LOW_MEM
	// We're memory constrained, so we shouldn't cache too many chunks
	return names[0];
#endif

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

namespace {

struct audio_lock
{
	audio_lock()
	{
		SDL_LockAudio();
	}

	~audio_lock()
	{
		SDL_UnlockAudio();
	}
};

} // end of anonymous namespace


namespace sound {

// Removes channel-chunk and channel-id mapping
static void channel_finished_hook(int channel)
{
	channel_chunks[channel] = NULL;
	channel_ids[channel] = -1;
}

bool init_sound() {
	LOG_AUDIO << "Initializing audio...\n";
	if(SDL_WasInit(SDL_INIT_AUDIO) == 0)
		if(SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
			return false;

	if(!mix_ok) {
		if(Mix_OpenAudio(preferences::sample_rate(), MIX_DEFAULT_FORMAT, 2, preferences::sound_buffer_size()) == -1) {
			mix_ok = false;
			ERR_AUDIO << "Could not initialize audio: " << Mix_GetError() << "\n";
			return false;
		}

		mix_ok = true;
		Mix_AllocateChannels(n_of_channels);
		Mix_ReserveChannels(n_reserved_channels);

		channel_chunks.clear();
		channel_chunks.resize(n_of_channels, NULL);
		channel_ids.resize(n_of_channels, -1);

		Mix_GroupChannel(bell_channel, SOUND_BELL);
		Mix_GroupChannel(timer_channel, SOUND_TIMER);
		Mix_GroupChannels(source_channel_start, source_channel_last, SOUND_SOURCES);
		Mix_GroupChannel(UI_sound_channel, SOUND_UI);
		Mix_GroupChannels(n_reserved_channels, n_of_channels - 1, SOUND_FX);

		set_sound_volume(preferences::sound_volume());
		set_UI_volume(preferences::UI_volume());
		set_music_volume(preferences::music_volume());
		set_bell_volume(preferences::bell_volume());

		Mix_ChannelFinished(channel_finished_hook);

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
		stop_UI_sound();
		stop_sound();
		sound_cache.clear();
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

void reset_sound() {
	bool music = preferences::music_on();
	bool sound = preferences::sound_on();
	bool UI_sound = preferences::UI_sound_on();
	bool bell = preferences::turn_bell();

	if (music || sound || bell || UI_sound) {
		sound::close_sound();
		if (!sound::init_sound())
			ERR_AUDIO << "Error initializing audio device: " << Mix_GetError() << "\n";
		if (!music)
			sound::stop_music();
		if (!sound)
			sound::stop_sound();
		if (!UI_sound)
			sound::stop_UI_sound();
		if (!bell)
			sound::stop_bell();
	}
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
		{
			Mix_HaltGroup(SOUND_SOURCES);
			Mix_HaltGroup(SOUND_FX);
		}
		sound_cache_iterator itor = sound_cache.begin();
		while(itor != sound_cache.end())
		{
			if(itor->group == SOUND_SOURCES || itor->group == SOUND_FX) {
				itor = sound_cache.erase(itor);
			} else {
				++itor;
			}
		}
	}
}

/*
 * For the purpose of channel manipulation, we treat turn timer the same as bell
 */
void stop_bell() {
	if(mix_ok) {
		Mix_HaltGroup(SOUND_BELL);
		Mix_HaltGroup(SOUND_TIMER);
		sound_cache_iterator itor = sound_cache.begin();
		while(itor != sound_cache.end())
		{
			if(itor->group == SOUND_BELL || itor->group == SOUND_TIMER) {
				itor = sound_cache.erase(itor);
			} else {
				++itor;
			}
		}
	}
}

void stop_UI_sound() {
	if(mix_ok) {
		Mix_HaltGroup(SOUND_UI);
		sound_cache_iterator itor = sound_cache.begin();
		while(itor != sound_cache.end())
		{
			if(itor->group == SOUND_UI) {
				itor = sound_cache.erase(itor);
			} else {
				++itor;
			}
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
	want_new_music=true;
	no_fading=false;
	fadingout_time=current_track.ms_after;
}

void play_new_music()
{
	music_start_time = 0;
	want_new_music=true;
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
		last_track=current_track;
	}
	if(!Mix_PlayingMusic())
	{
		LOG_AUDIO << "Playing track '" << current_track.name << "'\n";
		int fading_time=current_track.ms_before;
		if(no_fading)
		{
			fading_time=0;
		}
		const int res = Mix_FadeInMusic(itor->second, 1, fading_time);
		if(res < 0)
		{
			ERR_AUDIO << "Could not play music: " << Mix_GetError() << " " << current_track.name <<" \n";
		}
		want_new_music=false;
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

void music_thinker::process(events::pump_info &info) {
	if(preferences::music_on()) {
		if(!music_start_time && !current_track_list.empty() && !Mix_PlayingMusic()) {
			// Pick next track, add ending time to its start time.
			current_track = choose_track();
			music_start_time = info.ticks();
			no_fading=true;
			fadingout_time=0;
		}
		if(music_start_time && info.ticks(&music_refresh, music_refresh_rate) >= music_start_time - fadingout_time) {
			want_new_music=true;
		}
		if(want_new_music) {
			if(Mix_PlayingMusic()) {
				Mix_FadeOutMusic(fadingout_time);
			}
			play_new_music();
		}
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

void reposition_sound(int id, unsigned int distance)
{
	audio_lock lock();
	for(unsigned int ch = 0; ch < channel_ids.size(); ++ch) {
		int& ch_id = channel_ids[ch];
		if(ch_id == id) {
			if(distance >= DISTANCE_SILENT) {
				Mix_FadeOutChannel(ch, 100);
				ch_id = -1;
			}
			else {
				Mix_SetDistance(ch, distance);
			}
		}
	}
}

bool is_sound_playing(int id)
{
	audio_lock lock();
	return std::find(channel_ids.begin(), channel_ids.end(), id) != channel_ids.end();
}

void stop_sound(int id)
{
	reposition_sound(id, DISTANCE_SILENT);
}

void play_sound_positioned(const std::string &files, int id, unsigned int distance)
{
	play_sound_internal(files, SOUND_SOURCES, preferences::sound_on(), distance, id);
}

bool play_sound_internal(const std::string& files, channel_group group, bool sound_on, unsigned int distance, int id, int loop_ticks, int fadein_ticks)
{
	if(files.empty() || distance >= DISTANCE_SILENT || !sound_on || !mix_ok) {
		return false;
	}

	std::string file = pick_one(files);
	sound_cache_iterator it_bgn, it_end;
	sound_cache_iterator it;
	int channel;

	audio_lock lock();

	// find a free channel in the desired group
	channel = Mix_GroupAvailable(group);
	if(channel == -1) {
		LOG_AUDIO << "All channels dedicated to sound group(" << group << ") are busy, skipping.\n";
		return false;
	}
	channel_ids[channel] = id;
	Mix_SetDistance(channel, distance);

	sound_cache_chunk temp_chunk(file); // search the sound cache on this key
	it_bgn = sound_cache.begin(), it_end = sound_cache.end();
	it = std::find(it_bgn, it_end, temp_chunk);

	if (it != it_end) {
		if(it->group != group) {
			// cached item has been used in multiple sound groups
			it->group = NULL_CHANNEL;
		}
		//splice the most recently used chunk to the front of the cache
		sound_cache.splice(it_bgn, sound_cache, it);
	} else {
		// remove the least recently used chunk from cache if it's full
		bool cache_full = (sound_cache.size() == max_cached_chunks);
		while( cache_full && it != it_bgn ) {
			// make sure this chunk is not being played before freeing it
			std::vector<Mix_Chunk*>::iterator ch_end = channel_chunks.end();
			if(std::find(channel_chunks.begin(), ch_end, (--it)->get_data()) == ch_end) {
				sound_cache.erase(it);
				cache_full = false;
			}
		}
		if(cache_full) {
			LOG_AUDIO << "Maximum sound cache size reached and all are busy, skipping.\n";
			return false;
		}
		temp_chunk.group = group;
		std::string const &filename = get_binary_file_location("sounds", file);
		if (!filename.empty()) {
			temp_chunk.set_data(Mix_LoadWAV(filename.c_str()));
		}
		if (temp_chunk.get_data() == NULL) {
			ERR_AUDIO << "Could not load sound file '" << filename << "': "
				<< Mix_GetError() << "\n";
			return false;
		}
		sound_cache.push_front(temp_chunk);
		it = sound_cache.begin();
	}

	int res;
	if(loop_ticks > 0) {
		if(fadein_ticks > 0) {
			res = Mix_FadeInChannelTimed(channel, it->get_data(), -1, fadein_ticks, loop_ticks);
		} else {
			res = Mix_PlayChannel(channel, it->get_data(), -1);
		}

		if(res >= 0) {
			Mix_ExpireChannel(channel, loop_ticks);
		}
	} else {
		if(fadein_ticks > 0) {
			res = Mix_FadeInChannel(channel, it->get_data(), -1, fadein_ticks);
		} else {
			res = Mix_PlayChannel(channel, it->get_data(), 0);
		}
	}
	if(res < 0) {
		ERR_AUDIO << "error playing sound effect: " << Mix_GetError() << "\n";
		//still keep it in the sound cache, in case we want to try again later
		return false;
	}

	//reserve the channel's chunk from being freed, since it is playing
	channel_chunks[res] = it->get_data();
	return true;
}

void play_sound(const std::string& files, channel_group group)
{
	play_sound_internal(files, group, preferences::sound_on());
}

// Play bell with separate volume setting
void play_bell(const std::string& files)
{
	play_sound_internal(files, SOUND_BELL, preferences::turn_bell());
}

// Play timer with separate volume setting
void play_timer(const std::string& files, int loop_ticks, int fadein_ticks)
{
	play_sound_internal(files, SOUND_TIMER, preferences::turn_bell(), 0, -1, loop_ticks, fadein_ticks);
}

// Play UI sounds on separate volume than soundfx
void play_UI_sound(const std::string& files)
{
	play_sound_internal(files, SOUND_UI, preferences::UI_sound_on());
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

		// Bell, timer and UI have separate channels which we can't set up from this
		for (unsigned i = 0; i < n_of_channels; i++){
			if(i != UI_sound_channel && i != bell_channel && i != timer_channel) {
				Mix_Volume(i, vol);
			}
		}
	}
}

/*
 * For the purpose of volume setting, we treat turn timer the same as bell
 */
void set_bell_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > MIX_MAX_VOLUME)
			vol = MIX_MAX_VOLUME;

		Mix_Volume(bell_channel, vol);
		Mix_Volume(timer_channel, vol);
	}
}

void set_UI_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > MIX_MAX_VOLUME)
			vol = MIX_MAX_VOLUME;

		Mix_Volume(UI_sound_channel, vol);
	}
}

} // end of sound namespace
