/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "sound.hpp"
#include "sound_music_track.hpp"
#include "util.hpp"

#include "SDL_mixer.h"

#include <boost/foreach.hpp>

#include <list>

static lg::log_domain log_audio("audio");
#define DBG_AUDIO LOG_STREAM(debug, log_audio)
#define LOG_AUDIO LOG_STREAM(info, log_audio)
#define ERR_AUDIO LOG_STREAM(err, log_audio)

namespace sound {
// Channel-chunk mapping lets us know, if we can safely free a given chunk
std::vector<Mix_Chunk*> channel_chunks;

// Channel-id mapping for use with sound sources (to check if given source
// is playing on a channel for fading/panning)
std::vector<int> channel_ids;

static void play_sound_internal(const std::string& files, channel_group group, unsigned int repeats=0,
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
const size_t n_of_channels = 32;

// we need 2 channels, because we it for timer as well
const size_t bell_channel = 0;
const size_t timer_channel = 1;

// number of channels reserved for sound sources
const size_t source_channels = 8;
const size_t source_channel_start = timer_channel + 1;
const size_t source_channel_last = source_channel_start + source_channels - 1;
const size_t UI_sound_channels = 2;
const size_t UI_sound_channel_start = source_channel_last + 1;
const size_t UI_sound_channel_last = UI_sound_channel_start + UI_sound_channels - 1;
const size_t n_reserved_channels = UI_sound_channel_last + 1; // sources, bell, timer and UI

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

std::vector<std::string> played_before;

//
// FIXME: the first music_track may be initialized before main()
// is reached. Using the logging facilities may lead to a SIGSEGV
// because it's not guaranteed that their objects are already alive.
//
// Use the music_track default constructor to avoid trying to
// invoke a log object while resolving paths.
//
std::vector<sound::music_track> current_track_list;
sound::music_track current_track;
sound::music_track last_track;

}

static bool track_ok(const std::string& id)
{
	LOG_AUDIO << "Considering " << id << "\n";

	// If they committed changes to list, we forget previous plays, but
	// still *never* repeat same track twice if we have an option.
	if (id == current_track.file_path())
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

	for (i = played_before.rbegin(); i != played_before.rend(); ++i) {
		if (*i == id) {
			++num_played;
			if (num_played == 2)
				break;
		} else {
			played.insert(*i);
		}
	}

	// If we've played this twice, must have played every other track.
	if (num_played == 2 && played.size() != current_track_list.size() - 1) {
		LOG_AUDIO << "Played twice with only " << played.size()
			<< " tracks between\n";
		return false;
	}

	// Check previous previous track not same.
	i = played_before.rbegin();
	if (i != played_before.rend()) {
		++i;
		if (i != played_before.rend()) {
			if (*i == id) {
				LOG_AUDIO << "Played just before previous\n";
				return false;
			}
		}
	}

	return true;
}


static const sound::music_track &choose_track()
{
	assert(!current_track_list.empty());

	unsigned int track = 0;

	if (current_track_list.size() > 1) {
		do {
			track = rand()%current_track_list.size();
		} while (!track_ok( current_track_list[track].file_path() ));
	}

	//LOG_AUDIO << "Next track will be " << current_track_list[track].file_path() << "\n";
	played_before.push_back( current_track_list[track].file_path() );
	return current_track_list[track];
}

static std::string pick_one(const std::string &files)
{
	std::vector<std::string> ids = utils::square_parenthetical_split(files,',',"[","]");

	if (ids.empty())
		return "";
	if (ids.size() == 1)
		return ids[0];

#ifdef LOW_MEM
	// We're memory constrained, so we shouldn't cache too many chunks
	return ids[0];
#endif

	// We avoid returning same choice twice if we can avoid it.
	static std::map<std::string,unsigned int> prev_choices;
	unsigned int choice;

	if (prev_choices.find(files) != prev_choices.end()) {
		choice = rand()%(ids.size()-1);
		if (choice >= prev_choices[files])
			++choice;
		prev_choices[files] = choice;
	} else {
		choice = rand()%ids.size();
		prev_choices.insert(std::pair<std::string,unsigned int>(files,choice));
	}

	return ids[choice];
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
		Mix_GroupChannels(UI_sound_channel_start, UI_sound_channel_last, SOUND_UI);
		Mix_GroupChannels(n_reserved_channels, n_of_channels - 1, SOUND_FX);

		set_sound_volume(preferences::sound_volume());
		set_UI_volume(preferences::UI_volume());
		set_music_volume(preferences::music_volume());
		set_bell_volume(preferences::bell_volume());

		Mix_ChannelFinished(channel_finished_hook);

		LOG_AUDIO << "Audio initialized.\n";

		DBG_AUDIO << "Channel layout: " << n_of_channels << " channels (" << n_reserved_channels << " reserved)\n"
				  << "    " << bell_channel << " - bell\n"
				  << "    " << timer_channel << " - timer\n"
				  << "    " << source_channel_start << ".." << source_channel_last << " - sound sources\n"
				  << "    " << UI_sound_channel_start << ".." << UI_sound_channel_last << " - UI\n"
				  << "    " << UI_sound_channel_last + 1 << ".." << n_of_channels - 1 << " - sound effects\n";

		play_music();
	}
	return true;
}

void close_sound() {
	int frequency, channels;
	Uint16 format;

	if(mix_ok) {
		stop_bell();
		stop_UI_sound();
		stop_sound();
		sound_cache.clear();
		stop_music();
		mix_ok = false;

		int numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
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
		if (!sound::init_sound()) {
			ERR_AUDIO << "Error initializing audio device: " << Mix_GetError() << "\n";
		}
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
		Mix_HaltGroup(SOUND_SOURCES);
		Mix_HaltGroup(SOUND_FX);
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
	current_track_list.clear();
	current_track = music_track(file);
	play_music();
}

void empty_playlist()
{
	current_track_list.clear();
}

void play_music()
{
	music_start_time = 1; //immediate (same as effect as SDL_GetTicks())
	want_new_music=true;
	no_fading=false;
	fadingout_time=current_track.ms_after();
}

static void play_new_music()
{
	music_start_time = 0; //reset status: no start time
	want_new_music = true;

	if(!preferences::music_on() || !mix_ok || !current_track.valid()) {
		return;
	}

	const std::string& filename = current_track.file_path();

	std::map<std::string,Mix_Music*>::const_iterator itor = music_cache.find(filename);
	if(itor == music_cache.end()) {
		LOG_AUDIO << "attempting to insert track '" << filename << "' into cache\n";
		Mix_Music* const music = Mix_LoadMUS(filename.c_str());
		if(music == NULL) {
			ERR_AUDIO << "Could not load music file '" << filename << "': "
					  << Mix_GetError() << "\n";
			return;
		}
		itor = music_cache.insert(std::pair<std::string,Mix_Music*>(filename,music)).first;
		last_track=current_track;
	}

	LOG_AUDIO << "Playing track '" << filename << "'\n";
	int fading_time=current_track.ms_before();
	if(no_fading)
	{
		fading_time=0;
	}

	const int res = Mix_FadeInMusic(itor->second, 1, fading_time);
	if(res < 0)
	{
		ERR_AUDIO << "Could not play music: " << Mix_GetError() << " " << filename <<" \n";
	}

	want_new_music=false;
}

void play_music_repeatedly(const std::string &id)
{
	// Can happen if scenario doesn't specify.
	if (id.empty())
		return;

	current_track_list.clear();
	current_track_list.push_back(music_track(id));

	// If we're already playing it, don't interrupt.
	if (current_track != id) {
		current_track = music_track(id);
		play_music();
	}
}

void play_music_config(const config &music_node)
{
	music_track track( music_node );

	if (!track.valid() && !track.id().empty()) {
		ERR_AUDIO << "cannot open track '" << track.id() << "'; disabled in this playlist.\n";
	}

	// If they say play once, we don't alter playlist.
	if (track.play_once()) {
		current_track = track;
		play_music();
		return;
	}

	// Clear play list unless they specify append.
	if (!track.append()) {
		current_track_list.clear();
	}

	if(track.valid()) {
		// Avoid 2 tracks with the same name, since that can cause an infinite loop
		// in choose_track(), 2 tracks with the same name will always return the
		// current track and track_ok() doesn't allow that.
		std::vector<music_track>::const_iterator itor = current_track_list.begin();
		while(itor != current_track_list.end()) {
			if(track == *itor) break;
			++itor;
		}

		if(itor == current_track_list.end()) {
			current_track_list.push_back(track);
		} else {
			ERR_AUDIO << "tried to add duplicate track '" << track.file_path() << "'\n";
		}
	}

	// They can tell us to start playing this list immediately.
	if (track.immediate()) {
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
	played_before.clear();

	// Play-once is OK if still playing.
	if (current_track.play_once())
		return;

	// If current track no longer on playlist, change it.
	BOOST_FOREACH(const music_track &m, current_track_list) {
		if (current_track == m)
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
	// First entry clears playlist, others append to it.
	bool append = false;
	BOOST_FOREACH(music_track &m, current_track_list) {
		m.write(snapshot, append);
		append = true;
	}
}

void reposition_sound(int id, unsigned int distance)
{
	audio_lock lock;
	for (unsigned ch = 0; ch < channel_ids.size(); ++ch)
	{
		if (channel_ids[ch] != id) continue;
		if (distance >= DISTANCE_SILENT) {
			// Don't call Mix_FadeOutChannel if the channel's volume is set to
			// zero. It doesn't do anything in that case and the channel will
			// resume playing as soon as its volume is reset to a non-zero
			// value, which results in issues like sound sources deleted while
			// their volume is zero coming back to life and escaping Wesnoth's
			// sound source management code.
			if (Mix_Volume(ch, -1) == 0) {
				Mix_HaltChannel(ch);
			} else {
				Mix_FadeOutChannel(ch, 100);
			}
		} else {
			Mix_SetDistance(ch, distance);
		}
	}
}

bool is_sound_playing(int id)
{
	audio_lock lock;
	return std::find(channel_ids.begin(), channel_ids.end(), id) != channel_ids.end();
}

void stop_sound(int id)
{
	reposition_sound(id, DISTANCE_SILENT);
}

void play_sound_positioned(const std::string &files, int id, int repeats, unsigned int distance)
{
	if(preferences::sound_on()) {
		play_sound_internal(files, SOUND_SOURCES, repeats, distance, id);
	}
}

struct chunk_load_exception { };

static Mix_Chunk* load_chunk(const std::string& file, channel_group group)
{
	sound_cache_iterator it_bgn, it_end;
	sound_cache_iterator it;

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
			throw chunk_load_exception();
		}
		temp_chunk.group = group;
		std::string const &filename = filesystem::get_binary_file_location("sounds", file);

		if (!filename.empty()) {
			temp_chunk.set_data(Mix_LoadWAV(filename.c_str()));
		} else {
			ERR_AUDIO << "Could not load sound file '" << file << "'.\n";
			throw chunk_load_exception();
		}

		if (temp_chunk.get_data() == NULL) {
			ERR_AUDIO << "Could not load sound file '" << filename << "': "
				<< Mix_GetError() << "\n";
			throw chunk_load_exception();
		}

		sound_cache.push_front(temp_chunk);
	}

	return sound_cache.begin()->get_data();
}

void play_sound_internal(const std::string& files, channel_group group, unsigned int repeats,
			unsigned int distance, int id, int loop_ticks, int fadein_ticks)
{
	if(files.empty() || distance >= DISTANCE_SILENT || !mix_ok) {
		return;
	}

	audio_lock lock;

	// find a free channel in the desired group
	int channel = Mix_GroupAvailable(group);
	if(channel == -1) {
		LOG_AUDIO << "All channels dedicated to sound group(" << group << ") are busy, skipping.\n";
		return;
	}

	Mix_Chunk *chunk;
	std::string file = pick_one(files);

	try {
		chunk = load_chunk(file, group);
		assert(chunk);
	} catch(const chunk_load_exception&) {
		return;
	}

	/*
	 * This check prevents SDL_Mixer from blowing up on Windows when UI sound is played
	 * in response to toggling the checkbox which disables sound.
	 */
	if(group != SOUND_UI) {
		Mix_SetDistance(channel, distance);
	}

	int res;
	if(loop_ticks > 0) {
		if(fadein_ticks > 0) {
			res = Mix_FadeInChannelTimed(channel, chunk, -1, fadein_ticks, loop_ticks);
		} else {
			res = Mix_PlayChannel(channel, chunk, -1);
		}

		if(res >= 0) {
			Mix_ExpireChannel(channel, loop_ticks);
		}
	} else {
		if(fadein_ticks > 0) {
			res = Mix_FadeInChannel(channel, chunk, repeats, fadein_ticks);
		} else {
			res = Mix_PlayChannel(channel, chunk, repeats);
		}
	}

	if(res < 0) {
		ERR_AUDIO << "error playing sound effect: " << Mix_GetError() << "\n";
		//still keep it in the sound cache, in case we want to try again later
		return;
	}

	channel_ids[channel] = id;

	//reserve the channel's chunk from being freed, since it is playing
	channel_chunks[res] = chunk;
}

void play_sound(const std::string& files, channel_group group, unsigned int repeats)
{
	if(preferences::sound_on()) {
		play_sound_internal(files, group, repeats);
	}
}

// Play bell with separate volume setting
void play_bell(const std::string& files)
{
	if (preferences::turn_bell()) {
		play_sound_internal(files, SOUND_BELL);
	}
}

// Play timer with separate volume setting
void play_timer(const std::string& files, int loop_ticks, int fadein_ticks)
{
	if(preferences::sound_on()) {
		play_sound_internal(files, SOUND_TIMER, 0, 0, -1, loop_ticks, fadein_ticks);
	}
}

// Play UI sounds on separate volume than soundfx
void play_UI_sound(const std::string& files)
{
	if(preferences::UI_sound_on()) {
		play_sound_internal(files, SOUND_UI);
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

		// Bell, timer and UI have separate channels which we can't set up from this
		for (unsigned i = 0; i < n_of_channels; ++i){
			if(!(i >= UI_sound_channel_start && i <= UI_sound_channel_last)
			   && i != bell_channel && i != timer_channel)
			{
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

		for (unsigned i = UI_sound_channel_start; i <= UI_sound_channel_last; ++i) {
			Mix_Volume(i, vol);
		}
	}
}

} // end of sound namespace
