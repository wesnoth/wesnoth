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

#pragma once

#ifdef __EMSCRIPTEN__

#include <cstddef>
#include <cstdint>

namespace sound {
namespace emscripten {

/**
 * Decode OGG bytes and start playback via Web Audio API.
 * Decoding is asynchronous (fire-and-forget); playback starts when decode completes.
 * @param filename  Cache key for decoded AudioBuffer (resolved file path)
 * @param data      Raw OGG file bytes (pointer into WASM heap)
 * @param len       Length of OGG data in bytes
 * @param fade_in_ms  Fade-in duration in milliseconds (0 = no fade)
 */
void play(const char* filename, const uint8_t* data, std::size_t len, int fade_in_ms);

/** Stop playback immediately. */
void halt();

/** Fade out current track over @a ms milliseconds, then stop. */
void fade_out(int ms);

/** True if a track is currently playing (includes fading). */
bool is_playing();

/** True if currently fading in or out. */
bool is_fading();

/** Suspend the AudioContext (pause playback). */
void pause();

/** Resume a suspended AudioContext. */
void resume();

/**
 * Set music volume.
 * @param vol  SDL-style volume 0-128 (MIX_MAX_VOLUME)
 */
void set_volume(int vol);

/**
 * Get current music volume.
 * @return SDL-style volume 0-128
 */
int get_volume();

namespace sfx {

/**
 * Play a sound effect via Web Audio API (browser audio thread).
 * @param filename  Cache key (resolved file path)
 * @param data      Raw audio file bytes (pointer into WASM heap)
 * @param len       Length in bytes
 * @param channel   SDL_mixer channel index (0-31)
 * @param group     Channel group (SOUND_SOURCES=0 .. SOUND_FX=4)
 * @param distance  Distance attenuation 0-255 (255 = silent)
 * @param repeats   -1 = loop forever, 0 = play once, N = play N+1 times
 * @param fadein_ms Fade-in duration in milliseconds (0 = no fade)
 * @param loop_ms   Auto-stop after this many ms (0 = play to end)
 */
void play(const char* filename, const uint8_t* data, std::size_t len,
          int channel, int group, int distance, int repeats,
          int fadein_ms, int loop_ms);

/** Stop a single channel immediately. */
void halt_channel(int channel);

/** Stop all channels in a group. */
void halt_group(int group);

/** Update distance attenuation on a playing channel. */
void set_distance(int channel, int distance);

/** Set volume for an entire group. @param vol SDL 0-128 */
void set_group_volume(int group, int vol);

/** True if the channel has an active AudioBufferSourceNode. */
bool is_channel_playing(int channel);

/** Pop the next finished channel from the queue. @return channel or -1 */
int drain_finished();

/** Find a free (idle) channel in the given group. @return channel or -1 */
int find_free_channel(int group);

} // namespace sfx

} // namespace emscripten
} // namespace sound

#endif // __EMSCRIPTEN__
