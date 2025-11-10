/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "sound.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "random.hpp"
#include "serialization/string_utils.hpp"
#include "sound_music_track.hpp"
#include "utils/general.hpp"
#include "utils/rate_counter.hpp"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <list>
#include <utility>

static lg::log_domain log_audio("audio");
#define DBG_AUDIO LOG_STREAM(debug, log_audio)
#define LOG_AUDIO LOG_STREAM(info, log_audio)
#define ERR_AUDIO LOG_STREAM(err, log_audio)

// TODO: music track changes every time window loses and then regains focus
// TODO: MIX_Init and MIX_Quit handling - quit only quits if called the same number of times as init
// TODO: general MIX_ pointer cleanup
//       MIX_Audio already handled via destructor in shared pointer in audio_cache
// TODO: MIX_SetTrack3DPosition x and y positions
// TODO: PLAIN_LOG -> DBG_AUDIO
// TODO: removing MIX_Audio items from the cache
// TODO: test non-music functionality especially soundsources

namespace sound
{
// Channel-id mapping for use with sound sources (to check if given source
// is playing on a channel for fading/panning)
static std::vector<MIX_Track*> channels;
// filename, audio
static std::map<std::string, std::shared_ptr<MIX_Audio>> audio_cache;

MIX_Mixer* mixer;
std::map<int, sound_channels::type> channel_map;
}

using namespace std::chrono_literals;

namespace
{
bool mix_ok = false;
utils::optional<std::chrono::steady_clock::time_point> music_start_time;
utils::rate_counter music_refresh_rate{20};
bool want_new_music = false;
auto fade_out_time = 5000ms;
bool no_fading = false;

// number of allocated channels,
const std::size_t n_of_channels = 32;

// we need 2 channels, because we it for timer as well
const std::size_t music_channel = 0;
const std::size_t bell_channel = 1;
const std::size_t timer_channel = 2;

// number of channels reserved for sound sources
const std::size_t source_channel_start = 3;
const std::size_t source_channel_last = 10;

const std::size_t UI_sound_channel_start = 11;
const std::size_t UI_sound_channel_last = 12;

const std::size_t n_reserved_channels_start = 13;
const std::size_t n_reserved_channels_end = n_of_channels;

} // end anon namespace

namespace
{
std::vector<std::string> played_before;

//
// FIXME: the first music_track may be initialized before main()
// is reached. Using the logging facilities may lead to a SIGSEGV
// because it's not guaranteed that their objects are already alive.
//
// Use the music_track default constructor to avoid trying to
// invoke a log object while resolving paths.
//
std::vector<std::shared_ptr<sound::music_track>> current_track_list;
std::shared_ptr<sound::music_track> current_track;
unsigned int current_track_index = 0;
std::shared_ptr<sound::music_track> previous_track;

std::vector<std::shared_ptr<sound::music_track>>::const_iterator find_track(const sound::music_track& track)
{
	return utils::ranges::find(current_track_list, track,
		[](const std::shared_ptr<const sound::music_track>& ptr) { return *ptr; });
}

} // end anon namespace

namespace sound
{
utils::optional<unsigned int> get_current_track_index()
{
	if(current_track_index >= current_track_list.size()){
		return {};
	}
	return current_track_index;
}
std::shared_ptr<music_track> get_current_track()
{
	return current_track;
}
std::shared_ptr<music_track> get_previous_music_track()
{
	return previous_track;
}
void set_previous_track(std::shared_ptr<music_track> track)
{
	previous_track = std::move(track);
}

unsigned int get_num_tracks()
{
	return current_track_list.size();
}

std::shared_ptr<music_track> get_track(unsigned int i)
{
	if(i < current_track_list.size()) {
		return current_track_list[i];
	}

	if(i == current_track_list.size()) {
		return current_track;
	}

	return nullptr;
}

void set_track(unsigned int i, const std::shared_ptr<music_track>& to)
{
	if(i < current_track_list.size() && find_track(*to) != current_track_list.end()) {
		current_track_list[i] = std::make_shared<music_track>(*to);
	}
}

void remove_track(unsigned int i)
{
	if(i >= current_track_list.size()) {
		return;
	}

	if(i == current_track_index) {
		// Let the track finish playing
		if(current_track){
			current_track->set_play_once(true);
		}
		// Set current index to the new size of the list
		current_track_index = current_track_list.size() - 1;
	} else if(i < current_track_index) {
		current_track_index--;
	}

	current_track_list.erase(current_track_list.begin() + i);
}

} // end namespace sound

static bool track_ok(const std::string& id)
{
	LOG_AUDIO << "Considering " << id;

	if(!current_track) {
		return true;
	}

	// If they committed changes to list, we forget previous plays, but
	// still *never* repeat same track twice if we have an option.
	if(id == current_track->file_path()) {
		return false;
	}

	if(current_track_list.size() <= 3) {
		return true;
	}

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

	for(i = played_before.rbegin(); i != played_before.rend(); ++i) {
		if(*i == id) {
			++num_played;
			if(num_played == 2) {
				break;
			}
		} else {
			played.insert(*i);
		}
	}

	// If we've played this twice, must have played every other track.
	if(num_played == 2 && played.size() != current_track_list.size() - 1) {
		LOG_AUDIO << "Played twice with only " << played.size() << " tracks between";
		return false;
	}

	// Check previous previous track not same.
	i = played_before.rbegin();
	if(i != played_before.rend()) {
		++i;
		if(i != played_before.rend()) {
			if(*i == id) {
				LOG_AUDIO << "Played just before previous";
				return false;
			}
		}
	}

	return true;
}

static std::shared_ptr<sound::music_track> choose_track()
{
	assert(!current_track_list.empty());

	if(current_track_index >= current_track_list.size()) {
		current_track_index = 0;
	}

	if(current_track_list[current_track_index]->shuffle()) {
		unsigned int track = 0;

		if(current_track_list.size() > 1) {
			do {
				track = randomness::rng::default_instance().get_random_int(0, current_track_list.size()-1);
			} while(!track_ok(current_track_list[track]->file_path()));
		}

		current_track_index = track;
	}

	DBG_AUDIO << "Next track will be " << current_track_list[current_track_index]->file_path();
	played_before.push_back(current_track_list[current_track_index]->file_path());
	return current_track_list[current_track_index];
}

static std::string pick_one(const std::string& files)
{
	std::vector<std::string> ids = utils::square_parenthetical_split(files, ',', "[", "]");

	if(ids.empty()) {
		return "";
	}

	if(ids.size() == 1) {
		return ids[0];
	}

	// We avoid returning same choice twice if we can avoid it.
	static std::map<std::string, unsigned int> prev_choices;
	unsigned int choice;

	if(prev_choices.find(files) != prev_choices.end()) {
		choice = randomness::rng::default_instance().get_random_int(0, ids.size()-1 - 1);
		if(choice >= prev_choices[files]) {
			++choice;
		}

		prev_choices[files] = choice;
	} else {
		choice = randomness::rng::default_instance().get_random_int(0, ids.size()-1);
		prev_choices.emplace(files, choice);
	}

	return ids[choice];
}

namespace sound
{
std::string current_driver()
{
	const char* const drvname = SDL_GetCurrentAudioDriver();
	return drvname ? drvname : "<not initialized>";
}

std::vector<std::string> enumerate_drivers()
{
	std::vector<std::string> res;
	int num_drivers = SDL_GetNumVideoDrivers();

	for(int n = 0; n < num_drivers; ++n) {
		const char* drvname = SDL_GetAudioDriver(n);
		res.emplace_back(drvname ? drvname : "<invalid driver>");
	}

	return res;
}

driver_status driver_status::query()
{
	driver_status res{mix_ok, 0, SDL_AUDIO_UNKNOWN, 0, 0};

	if(mix_ok) {
		SDL_AudioSpec* spec;
		MIX_GetMixerFormat(mixer, spec);
		res.chunk_size = prefs::get().sound_buffer_size();
		res.frequency = spec->freq;
		res.format = spec->format;
		res.channels = spec->channels;
	}

	return res;
}

bool init_sound()
{
	LOG_AUDIO << "Initializing audio...";
	if(SDL_WasInit(SDL_INIT_AUDIO) == 0) {
		if(!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
			return false;
		}
	}

	MIX_Init();

	if(!mix_ok) {
		SDL_AudioSpec spec;
		spec.freq = prefs::get().sample_rate();
		spec.format = SDL_AUDIO_S16;
		spec.channels = 2;
		mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
		if(!mixer) {
			mix_ok = false;
			ERR_AUDIO << "Could not initialize audio: " << SDL_GetError();
			return false;
		}

		mix_ok = true;

		MIX_Track* music_track = MIX_CreateTrack(mixer);
		MIX_TagTrack(music_track, sound_channels::music_tag);
		channels.push_back(music_track);
		channel_map.emplace(music_channel, sound_channels::type::music_tag);

		MIX_Track* bell_track = MIX_CreateTrack(mixer);
		MIX_TagTrack(bell_track, sound_channels::sound_bell_tag);
		channels.push_back(bell_track);
		channel_map.emplace(bell_channel, sound_channels::type::sound_bell_tag);
		
		MIX_Track* sound_timer_track = MIX_CreateTrack(mixer);
		MIX_TagTrack(sound_timer_track, sound_channels::sound_timer_tag);
		channels.push_back(sound_timer_track);
		channel_map.emplace(timer_channel, sound_channels::type::sound_timer_tag);

		for(std::size_t i = source_channel_start; i <= source_channel_last; i++) {
			MIX_Track* sound_source_track = MIX_CreateTrack(mixer);
			MIX_TagTrack(sound_source_track, sound_channels::sound_source_tag);
			channels.push_back(sound_source_track);
			channel_map.emplace(i, sound_channels::type::sound_source_tag);
		}

		for(std::size_t i = UI_sound_channel_start; i <= UI_sound_channel_last; i++) {
			MIX_Track* sound_ui_track = MIX_CreateTrack(mixer);
			MIX_TagTrack(sound_ui_track, sound_channels::sound_ui_tag);
			channels.push_back(sound_ui_track);
			channel_map.emplace(i, sound_channels::type::sound_ui_tag);
		}

		for(std::size_t i = n_reserved_channels_start; i <= n_reserved_channels_end; i++) {
			MIX_Track* sound_fx_track = MIX_CreateTrack(mixer);
			MIX_TagTrack(sound_fx_track, sound_channels::sound_fx_tag);
			channel_map.emplace(i, sound_channels::type::sound_fx_tag);
			channels.push_back(sound_fx_track);
		}

		set_sound_volume(prefs::get().sound_volume());
		set_UI_volume(prefs::get().ui_volume());
		set_music_volume(prefs::get().music_volume());
		set_bell_volume(prefs::get().bell_volume());

		LOG_AUDIO << "Audio initialized.";

		DBG_AUDIO << "Channel layout: " << n_of_channels << " channels\n"
		          << "    " << bell_channel << " - bell\n"
		          << "    " << timer_channel << " - timer\n"
		          << "    " << source_channel_start << ".." << source_channel_last << " - sound sources\n"
		          << "    " << UI_sound_channel_start << ".." << UI_sound_channel_last << " - UI\n"
		          << "    " << UI_sound_channel_last + 1 << ".." << n_of_channels - 1 << " - sound effects";

		play_music();
	}

	return true;
}

void close_sound()
{
	if(mix_ok) {
		stop_bell();
		stop_UI_sound();
		stop_sound();
		stop_music();
		mix_ok = false;

		MIX_DestroyMixer(mixer);
	}

	MIX_Quit();

	if(SDL_WasInit(SDL_INIT_AUDIO) != 0) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}

	LOG_AUDIO << "Audio device released.";
}

void reset_sound()
{
	bool music = prefs::get().music_on();
	bool sound = prefs::get().sound();
	bool UI_sound = prefs::get().ui_sound_on();
	bool bell = prefs::get().turn_bell();

	if(music || sound || bell || UI_sound) {
		sound::close_sound();
		if(!sound::init_sound()) {
			ERR_AUDIO << "Error initializing audio device: " << SDL_GetError();
		}

		if(!music) {
			sound::stop_music();
		}

		if(!sound) {
			sound::stop_sound();
		}

		if(!UI_sound) {
			sound::stop_UI_sound();
		}

		if(!bell) {
			sound::stop_bell();
		}
	}
}

void stop_music()
{
	if(mix_ok) {
		MIX_StopTrack(channels[0], MIX_TrackMSToFrames(channels[0], 500));
	}
}

void stop_sound()
{
	if(mix_ok) {
		MIX_StopTag(mixer, sound_channels::sound_source_tag, 0);
		MIX_StopTag(mixer, sound_channels::sound_fx_tag, 0);
	}
}

/*
 * For the purpose of channel manipulation, we treat turn timer the same as bell
 */
void stop_bell()
{
	if(mix_ok) {
		MIX_StopTag(mixer, sound_channels::sound_bell_tag, 0);
		MIX_StopTag(mixer, sound_channels::sound_timer_tag, 0);
	}
}

void stop_UI_sound()
{
	if(mix_ok) {
		MIX_StopTag(mixer, sound_channels::sound_ui_tag, 0);
	}
}

void play_music_once(const std::string& file)
{
	if(auto track = sound::music_track::create(file)) {
		set_previous_track(current_track);
		current_track = std::move(track);
		current_track->set_play_once(true);
		current_track_index = current_track_list.size();
		play_music();
	}
}

void empty_playlist()
{
	current_track_list.clear();
}

void play_music()
{
	if(!current_track) {
		return;
	}

	music_start_time = std::chrono::steady_clock::now(); // immediate
	want_new_music = true;
	no_fading = false;
	fade_out_time = previous_track != nullptr ? previous_track->ms_after() : 0ms;
}

void play_track(unsigned int i)
{
	set_previous_track(current_track);
	if(i >= current_track_list.size()) {
		current_track = choose_track();
	} else {
		current_track_index = i;
		current_track = current_track_list[i];
	}
	play_music();
}

static void play_new_music()
{
	music_start_time.reset(); // reset status: no start time
	want_new_music = true;

	if(!prefs::get().music_on() || !mix_ok || !current_track) {
		return;
	}

	std::string filename = current_track->file_path();
	if(auto localized = filesystem::get_localized_path(filename)) {
		filename = localized.value();
	}

	LOG_AUDIO << "Playing track '" << filename << "'";
	auto fading_time = current_track->ms_before();
	if(no_fading) {
		fading_time = 0ms;
	}

	// Halt any existing music.
	// If we don't do this SDL_Mixer blocks everything until fade out is complete.
	// Do not remove this without ensuring that it does not block.
	// If you don't want it to halt the music, ensure that fades are completed
	// before attempting to play new music.
	MIX_StopTrack(channels[0], 0);

	std::shared_ptr<MIX_Audio> music;
	if(audio_cache.count(filename) != 0) {
		music = audio_cache[filename];
		PLAIN_LOG << "cache hit for " << filename;
	} else {
		music.reset(MIX_LoadAudio_IO(mixer, SDL_IOFromFile(filename.c_str(), "rb"), true, false), &MIX_DestroyAudio);
		PLAIN_LOG << "cache miss for " << filename;
	}

	// Fade in the new music
	MIX_SetTrackAudio(channels[0], music.get());

	SDL_PropertiesID props;
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fading_time.count());
	
	if(!MIX_PlayTrack(channels[0], props)) {
		ERR_AUDIO << "Could not play music: " << SDL_GetError() << " " << filename << " ";
	} else if(audio_cache.count(filename) == 0) {
		audio_cache.emplace(filename, music);
	}

	want_new_music = false;
}

void play_music_config(const config& music_node, bool allow_interrupt_current_track, int i)
{
	//
	// FIXME: there is a memory leak somewhere in this function, seemingly related to the shared_ptrs
	// stored in current_track_list.
	//
	// vultraz 5/8/2017
	//

	auto track = sound::music_track::create(music_node);
	if(!track) {
		ERR_AUDIO << "cannot open track; disabled in this playlist.";
		return;
	}

	// If they say play once, we don't alter playlist.
	if(track->play_once()) {
		set_previous_track(current_track);
		current_track = std::move(track);
		current_track_index = current_track_list.size();
		play_music();
		return;
	}

	// Clear play list unless they specify append.
	if(!track->append()) {
		current_track_list.clear();
	}

	auto iter = find_track(*track);
	// Avoid 2 tracks with the same name, since that can cause an infinite loop
	// in choose_track(), 2 tracks with the same name will always return the
	// current track and track_ok() doesn't allow that.
	if(iter == current_track_list.end()) {
		auto insert_at = (i >= 0 && static_cast<std::size_t>(i) < current_track_list.size())
			? current_track_list.begin() + i
			: current_track_list.end();

		// Copy the track pointer so our local variable remains non-null.
		iter = current_track_list.insert(insert_at, track);
		auto new_track_index = std::distance(current_track_list.cbegin(), iter);

		// If we inserted the new track *before* the current track, adjust
		// cached index so it still points to the same element.
		if(new_track_index <= current_track_index) {
			++current_track_index;
		}
	} else {
		ERR_AUDIO << "tried to add duplicate track '" << track->file_path() << "'";
	}

	// They can tell us to start playing this list immediately.
	if(track->immediate()) {
		set_previous_track(current_track);
		current_track = *iter;
		current_track_index = std::distance(current_track_list.cbegin(), iter);
		play_music();
	} else if(!track->append() && !allow_interrupt_current_track && current_track) {
		// Make sure the current track will finish first
		current_track->set_play_once(true);
	}
}

void music_thinker::process()
{
	if(prefs::get().music_on()) {
		// TODO: rethink the music_thinker design, especially the use of fade_out_time
		auto now = std::chrono::steady_clock::now();

		if(!music_start_time && !current_track_list.empty() && !MIX_TrackPlaying(channels[0])) {
			// Pick next track, add ending time to its start time.
			set_previous_track(current_track);
			current_track = choose_track();
			music_start_time = now;
			no_fading = true;
			fade_out_time = 0ms;
		}

		if(music_start_time && music_refresh_rate.poll()) {
			want_new_music = now >= *music_start_time - fade_out_time;
		}

		if(want_new_music) {
			if(MIX_TrackPlaying(channels[0])) {
				MIX_StopTrack(channels[0], MIX_TrackMSToFrames(channels[0], fade_out_time.count()));
				return;
			}

			play_new_music();
		}
	}
}

music_muter::music_muter()
	: events::sdl_handler(false)
{
	join_global();
}

void music_muter::handle_window_event(const SDL_Event& event)
{
	if(prefs::get().stop_music_in_background() && prefs::get().music_on()) {
		if(event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
			MIX_ResumeTrack(channels[0]);
		} else if(event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
			if(MIX_TrackPlaying(channels[0])) {
				MIX_PauseTrack(channels[0]);
			}
		}
	}
}

void commit_music_changes()
{
	played_before.clear();

	// Play-once is OK if still playing.
	if(current_track) {
		if(current_track->play_once()) {
			return;
		}

		// If current track no longer on playlist, change it.
		for(auto m : current_track_list) {
			if(*current_track == *m) {
				return;
			}
		}
	}

	// Victory empties playlist: if next scenario doesn't specify one...
	if(current_track_list.empty()) {
		return;
	}

	// FIXME: we don't pause ms_before on this first track.  Should we?
	set_previous_track(current_track);
	current_track = choose_track();
	play_music();
}

void write_music_play_list(config& snapshot)
{
	// First entry clears playlist, others append to it.
	bool append = false;
	for(auto m : current_track_list) {
		m->write(snapshot, append);
		append = true;
	}
}

void reposition_sound(unsigned id, unsigned int distance)
{
	for(unsigned ch = 0; ch < channels.size(); ++ch) {
		if(ch != id) {
			continue;
		}

		if(distance >= DISTANCE_SILENT) {
			MIX_StopTrack(channels[ch], 0);
		} else {
			MIX_Point3D pos;
			pos.x = distance;
			pos.y = distance;
			pos.z = 0;
			MIX_SetTrack3DPosition(channels[ch], &pos);
		}
	}
}

bool is_sound_playing(int id)
{
	return MIX_TrackPlaying(channels[id]);
}

void stop_sound(unsigned id)
{
	reposition_sound(id, DISTANCE_SILENT);
}

using namespace std::chrono_literals;

void play_sound_internal(const std::string& files,
		sound_channels::type group,
		unsigned int repeats = 0,
		unsigned int distance = 0,
		const std::chrono::milliseconds& loop_ticks = 0ms,
		const std::chrono::milliseconds& fadein_ticks = 0ms)
{
	if(files.empty() || distance >= DISTANCE_SILENT || !mix_ok) {
		return;
	}

	int free_channel = -1;
	// find a free channel in the desired group
	for(const auto& channel : sound::channel_map) {
		if(channel.second == group && !MIX_TrackPlaying(channels[channel.first])) {
			free_channel = channel.first;
		}
	}

	if(free_channel == -1) {
		LOG_AUDIO << "All channels dedicated to sound group(" << sound_channels::get_string(group) << ") are busy, skipping.";
		return;
	}

	std::string file = pick_one(files);
	const auto filename = filesystem::get_binary_file_location("sounds", file);
	const auto localized = filesystem::get_localized_path(filename.value_or(""));
	std::string real_path = localized.value_or(filename.value());

	/*
	 * This check prevents SDL_Mixer from blowing up on Windows when UI sound is played
	 * in response to toggling the checkbox which disables sound.
	 */
	if(group != sound_channels::type::sound_timer_tag) {
		MIX_Point3D pos;
		pos.x = distance;
		pos.y = distance;
		pos.z = 0;
		MIX_SetTrack3DPosition(channels[free_channel], &pos);
	}

	std::shared_ptr<MIX_Audio> sound;
	if(audio_cache.count(real_path) != 0) {
		sound = audio_cache[real_path];
		PLAIN_LOG << "cache hit for " << real_path;
	} else {
		sound.reset(MIX_LoadAudio_IO(mixer, SDL_IOFromFile(real_path.c_str(), "rb"), true, false), &MIX_DestroyAudio);
		PLAIN_LOG << "cache miss for " << real_path;
	}

	MIX_SetTrackAudio(channels[free_channel], sound.get());

	SDL_PropertiesID props;

	bool res;
	if(loop_ticks > 0ms) {
		if(fadein_ticks > 0ms) {
			SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fadein_ticks.count());
			SDL_SetNumberProperty(props, MIX_PROP_PLAY_MAX_MILLISECONDS_NUMBER, loop_ticks.count());
		} else {
			SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
		}
	} else {
		if(fadein_ticks > 0ms) {
			SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fadein_ticks.count());
		} else {
			SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, repeats);
		}
	}

	res = MIX_PlayTrack(channels[free_channel], props);

	if(!res) {
		ERR_AUDIO << "error playing sound effect " << real_path << " : " << SDL_GetError();
		// still keep it in the sound cache, in case we want to try again later
		return;
	} else if(audio_cache.count(real_path) == 0) {
		audio_cache.emplace(real_path, sound);
	}
}

} // end anon namespace

namespace sound {
void play_sound(const std::string& files, sound_channels::type group, unsigned int repeats)
{
	if(prefs::get().sound()) {
		sound::play_sound_internal(files, group, repeats);
	}
}

void play_sound_positioned(const std::string& files, int repeats, unsigned int distance)
{
	if(prefs::get().sound()) {
		sound::play_sound_internal(files, sound_channels::type::sound_source_tag, repeats, distance);
	}
}

// Play bell with separate volume setting
void play_bell(const std::string& files)
{
	if(prefs::get().turn_bell()) {
		sound::play_sound_internal(files, sound_channels::type::sound_bell_tag);
	}
}

// Play timer with separate volume setting
void play_timer(const std::string& files, const std::chrono::milliseconds& loop_ticks, const std::chrono::milliseconds& fadein_ticks)
{
	if(prefs::get().sound()) {
		sound::play_sound_internal(files, sound_channels::type::sound_timer_tag, 0, 0, loop_ticks, fadein_ticks);
	}
}

// Play UI sounds on separate volume than soundfx
void play_UI_sound(const std::string& files)
{
	if(prefs::get().ui_sound_on()) {
		sound::play_sound_internal(files, sound_channels::type::sound_ui_tag);
	}
}

int get_music_volume()
{
	if(mix_ok) {
		return MIX_SetTrackGain(sound::channels[music_channel], -1);
	}

	return 0;
}

void set_music_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > 1.0f) {
			vol = 1.0f;
		}

		MIX_SetTrackGain(sound::channels[music_channel], vol);
	}
}

int get_sound_volume()
{
	if(mix_ok) {
		// Since set_sound_volume sets all main channels to the same, just return the volume of any main channel
		return MIX_SetTrackGain(sound::channels[source_channel_start], -1);
	}
	return 0;
}

void set_sound_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > 1.0f) {
			vol = 1.0f;
		}

		// Bell, timer and UI have separate channels which we can't set up from this
		for(unsigned i = 0; i < n_of_channels; ++i) {
			if(!(i >= UI_sound_channel_start && i <= UI_sound_channel_last) && i != bell_channel
					&& i != timer_channel) {
				MIX_SetTrackGain(sound::channels[i], vol);
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
		if(vol > 1.0f) {
			vol = 1.0f;
		}

		MIX_SetTrackGain(sound::channels[bell_channel], vol);
		MIX_SetTrackGain(sound::channels[timer_channel], vol);
	}
}

void set_UI_volume(int vol)
{
	if(mix_ok && vol >= 0) {
		if(vol > 1.0f) {
			vol = 1.0f;
		}

		for(unsigned i = UI_sound_channel_start; i <= UI_sound_channel_last; ++i) {
			MIX_SetTrackGain(sound::channels[i], vol);
		}
	}
}

void flush_cache()
{
	// TODO
}

} // end namespace sound
