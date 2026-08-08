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
#include "sdl/sdl3_properties_raii.hpp"
#include "serialization/string_utils.hpp"
#include "sound_music_track.hpp"
#include "utils/general.hpp"
#include "utils/rate_counter.hpp"

#include <SDL3_mixer/SDL_mixer.h>

#include <list>
#include <mutex>
#include <utility>

static lg::log_domain log_audio("audio");
#define DBG_AUDIO LOG_STREAM(debug, log_audio)
#define LOG_AUDIO LOG_STREAM(info, log_audio)
#define ERR_AUDIO LOG_STREAM(err, log_audio)

namespace sound
{
namespace
{
/** Lightweight lifetime wrapper for MIX_Track. */
class channel
{
public:
	/** Creates a new type-tagged track on @a mixer. */
	void allocate_channel(MIX_Mixer* mixer, const char* type_tag)
	{
		track_.reset(MIX_CreateTrack(mixer));
		MIX_TagTrack(*this, type_tag);
	}

	/** Implicit conversion for use with the SDL_Mixer API. */
	operator MIX_Track*() const
	{
		return track_.get();
	}

private:
	std::unique_ptr<MIX_Track, decltype(&MIX_DestroyTrack)> track_{nullptr, &MIX_DestroyTrack};
};

std::mutex soundsource_map_mutex;
std::map<unsigned int, MIX_Track*> soundsource_map;

std::array<channel, 32> channel_pool{};

// One of these can play at a time
const auto music_channels        = utils::span{channel_pool}.subspan<0, 1>();
const auto bell_channels         = utils::span{channel_pool}.subspan<1, 1>();
const auto timer_channels        = utils::span{channel_pool}.subspan<2, 1>();

// Several of these can play at a time
const auto positional_channels   = utils::span{channel_pool}.subspan<3, 8>();
const auto UI_channels           = utils::span{channel_pool}.subspan<11, 2>();
const auto SFX_channels          = utils::span{channel_pool}.subspan<13, 19>();

// filename, audio
std::map<std::string, std::shared_ptr<MIX_Audio>> music_cache;
std::vector<std::string> music_cache_insertion_order;

std::map<std::string, std::shared_ptr<MIX_Audio>> sound_cache;
std::vector<std::string> sound_cache_insertion_order;

MIX_Mixer* mixer = nullptr;
std::size_t mixer_init_counter = 0;

using namespace std::chrono_literals;

utils::optional<std::chrono::steady_clock::time_point> music_start_time;
utils::rate_counter music_refresh_rate{20};
bool want_new_music = false;
auto fade_out_time = 5000ms;
bool no_fading = false;

const std::size_t music_cache_limit = 30;
const std::size_t sound_cache_limit = 500;

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

namespace
{
bool track_ok(const std::string& id)
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

std::shared_ptr<sound::music_track> choose_track()
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

std::string pick_one(const std::string& files)
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

} // namespace

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
	driver_status res{mixer != nullptr, 0, SDL_AUDIO_UNKNOWN, 0, 0};

	if(mixer) {
		SDL_AudioSpec spec;
		if(MIX_GetMixerFormat(mixer, &spec)) {
			res.chunk_size = prefs::get().sound_buffer_size();
			res.frequency = spec.freq;
			res.format = spec.format;
			res.channels = spec.channels;
		}
	}

	return res;
}

bool init_sound()
{
	LOG_AUDIO << "Initializing audio...";
	if(SDL_WasInit(SDL_INIT_AUDIO) == 0) {
		if(!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
			ERR_AUDIO << "Could not initialize audio: " << SDL_GetError();
			return false;
		}
	}

	if(MIX_Init()) {
		mixer_init_counter++;
	} else {
		ERR_AUDIO << "Could not initialize mixer: " << SDL_GetError();
		return false;
	}

	if(!mixer) {
		SDL_AudioSpec spec;
		spec.freq = prefs::get().sample_rate();
		spec.format = SDL_AUDIO_S16;
		spec.channels = 2;
		mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
		if(!mixer) {
			ERR_AUDIO << "Could not initialize audio: " << SDL_GetError();
			return false;
		}

		for(channel& c : music_channels) {
			c.allocate_channel(mixer, sound_tracks::music);
		}

		for(channel& c : bell_channels) {
			c.allocate_channel(mixer, sound_tracks::sound_bell);
		}

		for(channel& c : timer_channels) {
			c.allocate_channel(mixer, sound_tracks::sound_timer);
		}

		for(channel& c : positional_channels) {
			c.allocate_channel(mixer, sound_tracks::sound_source);
		}

		for(channel& c : UI_channels) {
			c.allocate_channel(mixer, sound_tracks::sound_ui);
		}

		for(channel& c : SFX_channels) {
			c.allocate_channel(mixer, sound_tracks::sound_fx);
		}

		set_sound_volume(prefs::get().sound_volume());
		set_UI_volume(prefs::get().ui_volume());
		set_music_volume(prefs::get().music_volume());
		set_bell_volume(prefs::get().bell_volume());

		LOG_AUDIO << "Audio initialized.";
		play_music();
	}

	return true;
}

void close_sound()
{
	if(mixer) {
		stop_bell();
		stop_UI_sound();
		stop_sound();
		stop_music();
	}

	channel_pool = {};
	if(mixer) {
		MIX_DestroyMixer(mixer);
		mixer = nullptr;
	}

	flush_cache();

	// as per documentation, calling MIX_Init multiple times won't result in a failure
	// MIX_Quit then needs to be called the same number of times to make it de-initialize
	// so, make sure that always happens
	for(std::size_t i = 0; i < mixer_init_counter; i++) {
		MIX_Quit();
	}
	mixer_init_counter = 0;

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
		sound::init_sound();

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
	if(mixer) {
		MIX_StopTag(mixer, sound_tracks::music, 500);
	}
}

void stop_sound()
{
	if(mixer) {
		MIX_StopTag(mixer, sound_tracks::sound_source, 0);
		MIX_StopTag(mixer, sound_tracks::sound_fx, 0);
	}
}

/*
 * For the purpose of track manipulation, we treat turn timer the same as bell
 */
void stop_bell()
{
	if(mixer) {
		MIX_StopTag(mixer, sound_tracks::sound_bell, 0);
		MIX_StopTag(mixer, sound_tracks::sound_timer, 0);
	}
}

void stop_UI_sound()
{
	if(mixer) {
		MIX_StopTag(mixer, sound_tracks::sound_ui, 0);
	}
}

void restart_music()
{
	if(mixer) {
		MIX_ResumeTag(mixer, sound_tracks::music);
	}
}

void restart_sound()
{
	if(mixer) {
		MIX_ResumeTag(mixer, sound_tracks::sound_source);
		MIX_ResumeTag(mixer, sound_tracks::sound_fx);
	}
}

void restart_bell()
{
	if(mixer) {
		MIX_ResumeTag(mixer, sound_tracks::sound_bell);
		MIX_ResumeTag(mixer, sound_tracks::sound_timer);
	}
}

void restart_UI_sound()
{
	if(mixer) {
		MIX_ResumeTag(mixer, sound_tracks::sound_ui);
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

namespace
{
void play_new_music()
{
	music_start_time.reset(); // reset status: no start time
	want_new_music = true;

	if(!prefs::get().music_on() || !mixer || !current_track) {
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
	MIX_StopTrack(music_channels[0], 0);

	std::shared_ptr<MIX_Audio> music;
	if(music_cache.count(filename) != 0) {
		music = music_cache[filename];
		DBG_AUDIO << "cache hit for " << filename;
	} else {
		music.reset(MIX_LoadAudio(mixer, filename.c_str(), false), &MIX_DestroyAudio);
		DBG_AUDIO << "cache miss for " << filename;
	}

	// Fade in the new music
	MIX_SetTrackAudio(music_channels[0], music.get());

	sdl3_properties props;
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fading_time.count());

	if(!MIX_PlayTrack(music_channels[0], props)) {
		ERR_AUDIO << "Could not play music: " << SDL_GetError() << " " << filename << " ";
	} else if(music_cache.count(filename) == 0) {
		music_cache.emplace(filename, music);
		music_cache_insertion_order.emplace_back(filename);

		if(music_cache.size() > music_cache_limit) {
			std::string to_erase = music_cache_insertion_order[0];
			DBG_AUDIO << "Uncaching music file " << to_erase;
			music_cache_insertion_order.erase(music_cache_insertion_order.begin());
			music_cache.erase(to_erase);
		}
	}

	want_new_music = false;
}

MIX_Track* get_positional_channel(unsigned soundsource_id)
{
	std::scoped_lock lock{soundsource_map_mutex};
	const auto it = soundsource_map.find(soundsource_id);
	return it == soundsource_map.end() ? nullptr : it->second;
}

} // namespace

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
	if(MIX_GetTrackFadeFrames(music_channels[0]) != 0) {
		// Do not block everything while fading.
		return;
	}

	if(prefs::get().music_on()) {
		// TODO: rethink the music_thinker design, especially the use of fade_out_time
		auto now = std::chrono::steady_clock::now();

		bool is_playing = MIX_TrackPlaying(music_channels[0]);
		bool is_paused = MIX_TrackPaused(music_channels[0]);
		if(!music_start_time && !current_track_list.empty() && !is_playing && !is_paused) {
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
			if(MIX_TrackPlaying(music_channels[0])) {
				MIX_StopTrack(music_channels[0], MIX_TrackMSToFrames(music_channels[0], fade_out_time.count()));
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
			MIX_ResumeTrack(music_channels[0]);
			DBG_AUDIO << "resuming music";
		} else if(event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
			if(MIX_TrackPlaying(music_channels[0])) {
				MIX_PauseTrack(music_channels[0]);
				DBG_AUDIO << "pausing music";
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
	if(MIX_Track* track = get_positional_channel(id)) {
		if(distance == distance_silent) {
			MIX_StopTrack(track, 0);
		} else {
			MIX_Point3D pos;
			pos.x = 0;
			pos.y = distance;
			pos.z = 0;
			MIX_SetTrack3DPosition(track, &pos);
		}
	}
}

bool is_sound_playing(int id)
{
	MIX_Track* track = get_positional_channel(id);
	return track && MIX_TrackPlaying(track);
}

void stop_sound(unsigned id)
{
	reposition_sound(id, distance_silent);
}

namespace
{
MIX_Track* find_free_channel(sound_tracks::type group)
{
	const auto search = [](const auto& span) -> MIX_Track* {
		for(channel& c : span) {
			if(!MIX_TrackPlaying(c)) {
				return c;
			}
		}

		return nullptr;
	};

	switch(group) {
	case sound_tracks::type::music:
		return search(music_channels);
	case sound_tracks::type::sound_bell:
		return search(bell_channels);
	case sound_tracks::type::sound_timer:
		return search(timer_channels);
	case sound_tracks::type::sound_source:
		return search(positional_channels);
	case sound_tracks::type::sound_ui:
		return search(UI_channels);
	case sound_tracks::type::sound_fx:
		return search(SFX_channels);
	default:
		return nullptr;
	}
}

void play_sound_internal(const std::string& files,
		sound_tracks::type group,
		unsigned int repeats = 0,
		unsigned int distance = 0,
		unsigned int soundsource_id = UINT_MAX,
		const std::chrono::milliseconds& loop_ticks = 0ms,
		const std::chrono::milliseconds& fadein_ticks = 0ms)
{
	if(files.empty() || !mixer) {
		return;
	}

	if(group == sound_tracks::type::sound_source) {
		if(soundsource_id != UINT_MAX) {
			if(is_sound_playing(soundsource_id)) {
				return;
			}
		} else {
			return;
		}
	}

	// find a free track in the desired group
	MIX_Track* free_channel = find_free_channel(group);
	if(!free_channel) {
		LOG_AUDIO << "All tracks dedicated to sound group(" << sound_tracks::get_string(group) << ") are busy, skipping.";
		return;
	}

	std::string file = pick_one(files);
	const auto filename = filesystem::get_binary_file_location("sounds", file);
	if(!filename) {
		ERR_AUDIO << "Could not locate sound file '" << file << "'.";
		return;
	}
	const auto localized = filesystem::get_localized_path(filename.value_or(""));
	std::string real_path = localized.value_or(filename.value());

	MIX_Point3D pos;
	pos.x = 0;
	pos.y = distance;
	pos.z = 0;
	MIX_SetTrack3DPosition(free_channel, &pos);

	std::shared_ptr<MIX_Audio> sound;
	if(sound_cache.count(real_path) != 0) {
		sound = sound_cache[real_path];
		DBG_AUDIO << "cache hit for " << real_path;
	} else {
		sound.reset(MIX_LoadAudio(mixer, real_path.c_str(), false), &MIX_DestroyAudio);
		DBG_AUDIO << "cache miss for " << real_path;
	}

	MIX_SetTrackAudio(free_channel, sound.get());

	sdl3_properties props;

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

	res = MIX_PlayTrack(free_channel, props);

	if(!res) {
		ERR_AUDIO << "error playing sound effect " << real_path << " : " << SDL_GetError();
		// still keep it in the sound cache, in case we want to try again later
		return;
	} else if(group == sound_tracks::type::sound_source) {
		// first->first since emplace returns an iterator to a pair (what we actually want) and a boolean
		// const_cast since the callback signature only accepts void*, not const void*
		std::scoped_lock lock(soundsource_map_mutex);
		unsigned int* key = const_cast<unsigned int*>(&(soundsource_map.emplace(soundsource_id, free_channel).first->first));
		DBG_AUDIO << "adding callback for soundsource id " << *key;
		MIX_SetTrackStoppedCallback(free_channel, [](void* userdata, MIX_Track*){
			std::scoped_lock lock(soundsource_map_mutex);
			DBG_AUDIO << "in callback to erase soundsource mapping for id " << *static_cast<unsigned int*>(userdata);
			soundsource_map.erase(*static_cast<unsigned int*>(userdata));
		}, key);
	}

	if(res && sound_cache.count(real_path) == 0) {
		sound_cache.emplace(real_path, sound);
		sound_cache_insertion_order.emplace_back(real_path);

		if(sound_cache.size() > sound_cache_limit) {
			std::string to_erase = sound_cache_insertion_order[0];
			DBG_AUDIO << "Uncaching sound file " << to_erase;
			sound_cache_insertion_order.erase(sound_cache_insertion_order.begin());
			sound_cache.erase(to_erase);
		}
	}
}

/** Clamp gain value to a sensible (albeit arbitrary) range. */
volume clamp_gain(volume value)
{
	return std::clamp(value, sound::silence, sound::max_volume);
}

} // namespace

void play_sound(const std::string& files, sound_tracks::type group, unsigned int repeats)
{
	if(prefs::get().sound()) {
		sound::play_sound_internal(files, group, repeats);
	}
}

void play_sound_positioned(const std::string& files, int repeats, unsigned int distance, unsigned int id)
{
	if(prefs::get().sound()) {
		sound::play_sound_internal(files, sound_tracks::type::sound_source, repeats, distance, id);
	}
}

// Play bell with separate volume setting
void play_bell(const std::string& files)
{
	if(prefs::get().turn_bell()) {
		sound::play_sound_internal(files, sound_tracks::type::sound_bell);
	}
}

// Play timer with separate volume setting
void play_timer(const std::string& files, const std::chrono::milliseconds& loop_ticks, const std::chrono::milliseconds& fadein_ticks)
{
	if(prefs::get().sound()) {
		sound::play_sound_internal(files, sound_tracks::type::sound_timer, 0, distance_none, UINT_MAX, loop_ticks, fadein_ticks);
	}
}

// Play UI sounds on separate volume than soundfx
void play_UI_sound(const std::string& files)
{
	if(prefs::get().ui_sound_on()) {
		sound::play_sound_internal(files, sound_tracks::type::sound_ui);
	}
}

volume get_music_volume()
{
	if(mixer) {
		return volume{MIX_GetTrackGain(sound::music_channels[0])};
	}

	return sound::silence;
}

void set_music_volume(volume vol)
{
	if(mixer) {
		MIX_SetTrackGain(sound::music_channels[0], clamp_gain(vol));
	}
}

volume get_sound_volume()
{
	if(mixer) {
		// Since set_sound_volume sets all main tracks to the same, just return the volume of any main track
		return volume{MIX_GetTrackGain(sound::positional_channels[0])};
	}

	return sound::silence;
}

void set_sound_volume(volume vol)
{
	if(mixer) {
		vol = clamp_gain(vol);

		// Bell, timer and UI have separate tracks which we can't set up from this
		for(channel& c : positional_channels) {
			MIX_SetTrackGain(c, vol);
		}

		for(channel& c : SFX_channels) {
			MIX_SetTrackGain(c, vol);
		}
	}
}

/*
 * For the purpose of volume setting, we treat turn timer the same as bell
 */
void set_bell_volume(volume vol)
{
	if(mixer) {
		vol = clamp_gain(vol);

		MIX_SetTrackGain(sound::bell_channels[0], vol);
		MIX_SetTrackGain(sound::timer_channels[0], vol);
	}
}

void set_UI_volume(volume vol)
{
	if(mixer) {
		vol = clamp_gain(vol);

		for(channel& c : UI_channels) {
			MIX_SetTrackGain(c, vol);
		}
	}
}

void flush_cache()
{
	music_cache.clear();
	music_cache_insertion_order.clear();
	sound_cache.clear();
	sound_cache_insertion_order.clear();
}

} // end namespace sound
