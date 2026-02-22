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

#ifdef __EMSCRIPTEN__

#include "sound_emscripten.hpp"
#include <emscripten.h>

// ---------------------------------------------------------------------------
// JS-side Web Audio state and functions, injected via EM_JS.
//
// State lives on Module._webAudio to survive across EM_JS calls.
// AudioContext is created lazily on first play().
// Decoded AudioBuffers are cached by filename in a Map.
// ---------------------------------------------------------------------------

// Initialize the Web Audio state object on Module (idempotent).
EM_JS(void, wa_ensure_init, (), {
	if (Module._webAudio) return;
	Module._webAudio = {
		ctx: null,
		masterGain: null,
		source: null,
		sourceGain: null,
		playing: false,
		fading: false,
		volume: 1.0,
		fadeTimer: 0,
		cache: new Map(),
		cacheOrder: [],
		playGen: 0
	};
});

EM_JS(void, wa_play, (const char* name_ptr, const uint8_t* data, int len, int fade_ms), {
	var wa = Module._webAudio;

	// Lazy-create AudioContext.
	if (!wa.ctx) {
		wa.ctx = new (window.AudioContext || window.webkitAudioContext)();
		wa.masterGain = wa.ctx.createGain();
		wa.masterGain.connect(wa.ctx.destination);
	}

	// Resume if suspended (browser autoplay policy).
	if (wa.ctx.state === "suspended") {
		wa.ctx.resume();
	}

	var name = UTF8ToString(name_ptr);
	var gen = ++wa.playGen;

	function startPlayback(decoded) {
		// Guard against stale decode callbacks for superseded tracks.
		if (wa.playGen !== gen) return;

		// Stop current source if any.
		if (wa.source) {
			try { wa.source.onended = null; wa.source.stop(); } catch(e) {}
		}
		if (wa.fadeTimer) { clearTimeout(wa.fadeTimer); wa.fadeTimer = 0; }

		var source = wa.ctx.createBufferSource();
		var gain = wa.ctx.createGain();
		source.buffer = decoded;
		source.connect(gain);
		gain.connect(wa.masterGain);

		// Fade in or immediate.  Volume is handled by masterGain;
		// per-source gain is only used for fade ramps (target 1.0).
		if (fade_ms > 0) {
			gain.gain.setValueAtTime(0, wa.ctx.currentTime);
			gain.gain.linearRampToValueAtTime(
				1.0, wa.ctx.currentTime + fade_ms / 1000.0
			);
			wa.fading = true;
			wa.fadeTimer = setTimeout(function() {
				wa.fading = false;
				wa.fadeTimer = 0;
			}, fade_ms);
		} else {
			gain.gain.setValueAtTime(1.0, wa.ctx.currentTime);
			wa.fading = false;
		}

		source.onended = function() {
			// Only mark not-playing if this is still the current source.
			if (wa.source === source) {
				wa.playing = false;
				wa.fading = false;
			}
		};

		source.start();
		wa.source = source;
		wa.sourceGain = gain;
		wa.playing = true;
	}

	// Check JS-side decoded AudioBuffer cache.
	if (wa.cache.has(name)) {
		// Touch LRU order so recently-played tracks aren't evicted.
		var idx = wa.cacheOrder.indexOf(name);
		if (idx !== -1) wa.cacheOrder.splice(idx, 1);
		wa.cacheOrder.push(name);
		startPlayback(wa.cache.get(name));
		return;
	}

	// Mark as playing before async decode so music_thinker doesn't
	// schedule another track during the decode gap.
	wa.playing = true;

	// Copy OGG bytes from WASM heap (heap may be detached after decode).
	var buf = new Uint8Array(HEAPU8.buffer, data, len).slice();

	wa.ctx.decodeAudioData(buf.buffer).then(function(decoded) {
		// LRU cache: keep at most 3 decoded AudioBuffers.
		wa.cache.set(name, decoded);
		var idx = wa.cacheOrder.indexOf(name);
		if (idx !== -1) wa.cacheOrder.splice(idx, 1);
		wa.cacheOrder.push(name);
		while (wa.cacheOrder.length > 3) {
			wa.cache["delete"](wa.cacheOrder.shift());
		}
		startPlayback(decoded);
	})["catch"](function(err) {
		console.error("[Web Audio] decode error for " + name + ":", err);
	});
});

EM_JS(void, wa_halt, (), {
	var wa = Module._webAudio;
	if (!wa) return;
	++wa.playGen; // invalidate any pending decode callbacks
	if (wa.source) {
		try { wa.source.onended = null; wa.source.stop(); } catch(e) {}
	}
	if (wa.fadeTimer) { clearTimeout(wa.fadeTimer); wa.fadeTimer = 0; }
	wa.playing = false;
	wa.fading = false;
});

EM_JS(void, wa_fade_out, (int ms), {
	var wa = Module._webAudio;
	if (!wa || !wa.source || !wa.sourceGain || !wa.playing) return;
	++wa.playGen; // invalidate any pending decode callbacks

	var now = wa.ctx.currentTime;
	wa.sourceGain.gain.cancelScheduledValues(now);
	wa.sourceGain.gain.setValueAtTime(wa.sourceGain.gain.value, now);
	wa.sourceGain.gain.linearRampToValueAtTime(0, now + ms / 1000.0);
	wa.fading = true;

	if (wa.fadeTimer) clearTimeout(wa.fadeTimer);
	// Stop the source after fade completes.
	var src = wa.source;
	wa.fadeTimer = setTimeout(function() {
		wa.fading = false;
		wa.fadeTimer = 0;
		if (wa.source === src) {
			try { src.onended = null; src.stop(); } catch(e) {}
			wa.playing = false;
		}
	}, ms);
});

EM_JS(int, wa_is_playing, (), {
	var wa = Module._webAudio;
	return (wa && wa.playing) ? 1 : 0;
});

EM_JS(int, wa_is_fading, (), {
	var wa = Module._webAudio;
	return (wa && wa.fading) ? 1 : 0;
});

EM_JS(void, wa_pause, (), {
	var wa = Module._webAudio;
	if (wa && wa.ctx && wa.ctx.state === "running") {
		wa.ctx.suspend();
	}
});

EM_JS(void, wa_resume, (), {
	var wa = Module._webAudio;
	if (wa && wa.ctx && wa.ctx.state === "suspended") {
		wa.ctx.resume();
	}
});

EM_JS(void, wa_set_volume, (double vol01), {
	var wa = Module._webAudio;
	if (!wa) return;
	wa.volume = vol01;
	if (wa.masterGain) {
		wa.masterGain.gain.setValueAtTime(vol01, wa.ctx.currentTime);
	}
});

EM_JS(double, wa_get_volume, (), {
	var wa = Module._webAudio;
	return wa ? wa.volume : 1.0;
});

// ---------------------------------------------------------------------------
// C++ wrapper implementations
// ---------------------------------------------------------------------------

namespace sound {
namespace emscripten {

void play(const char* filename, const uint8_t* data, std::size_t len, int fade_in_ms)
{
	wa_ensure_init();
	wa_play(filename, data, static_cast<int>(len), fade_in_ms);
}

void halt()
{
	wa_halt();
}

void fade_out(int ms)
{
	wa_fade_out(ms);
}

bool is_playing()
{
	return wa_is_playing() != 0;
}

bool is_fading()
{
	return wa_is_fading() != 0;
}

void pause()
{
	wa_pause();
}

void resume()
{
	wa_resume();
}

void set_volume(int vol)
{
	// SDL volume is 0-128 (MIX_MAX_VOLUME=128), map to 0.0-1.0
	double v = (vol < 0) ? 0.0 : (vol > 128) ? 1.0 : vol / 128.0;
	wa_set_volume(v);
}

int get_volume()
{
	return static_cast<int>(wa_get_volume() * 128.0);
}

} // namespace emscripten
} // namespace sound

// ---------------------------------------------------------------------------
// SFX bypass — plays sound effects via Web Audio AudioBufferSourceNode,
// immune to main-thread JSPI stalls that starve ScriptProcessorNode.
// ---------------------------------------------------------------------------

// Channel ranges matching C++ constants in sound.cpp anonymous namespace:
//   bell=0, timer=1, sources=2-9, UI=10-11, FX=12-31
// Groups: SOUND_SOURCES=0, SOUND_BELL=1, SOUND_TIMER=2, SOUND_UI=3, SOUND_FX=4

EM_JS(void, wa_sfx_ensure_init, (), {
	if (Module._webAudioSfx) return;

	// Share AudioContext with music bypass if available, else create new.
	var ctx;
	if (Module._webAudio && Module._webAudio.ctx) {
		ctx = Module._webAudio.ctx;
	} else {
		ctx = new (window.AudioContext || window.webkitAudioContext)();
	}

	// Per-group GainNodes (indexed by group enum value).
	var groupGain = [];
	for (var g = 0; g < 5; g++) {
		var gn = ctx.createGain();
		gn.connect(ctx.destination);
		groupGain.push(gn);
	}

	// Channel-to-group mapping (32 channels).
	var channelGroup = new Int32Array(32);
	channelGroup[0] = 1;  // bell -> SOUND_BELL
	channelGroup[1] = 2;  // timer -> SOUND_TIMER
	for (var i = 2; i <= 9; i++) channelGroup[i] = 0;   // sources -> SOUND_SOURCES
	for (var i = 10; i <= 11; i++) channelGroup[i] = 3;  // UI -> SOUND_UI
	for (var i = 12; i <= 31; i++) channelGroup[i] = 4;  // FX -> SOUND_FX

	// Channel range per group: [start, end] inclusive.
	var groupRange = [
		[2, 9],    // SOUND_SOURCES
		[0, 0],    // SOUND_BELL
		[1, 1],    // SOUND_TIMER
		[10, 11],  // SOUND_UI
		[12, 31]   // SOUND_FX
	];

	Module._webAudioSfx = {
		ctx: ctx,
		groupGain: groupGain,
		channelGroup: channelGroup,
		groupRange: groupRange,
		channels: new Array(32).fill(null),  // active source info per channel
		finishedQueue: [],
		cache: new Map(),
		cacheOrder: [],
		maxCache: 64
	};
});

EM_JS(void, wa_sfx_play, (const char* name_ptr, const uint8_t* data, int len,
                           int ch, int group, int distance, int repeats,
                           int fadein_ms, int loop_ms), {
	var sfx = Module._webAudioSfx;
	var ctx = sfx.ctx;

	// Resume if suspended (browser autoplay policy).
	if (ctx.state === "suspended") ctx.resume();

	// Stop any existing source on this channel.
	var old = sfx.channels[ch];
	if (old) {
		try { old.source.onended = null; old.source.stop(); } catch(e) {}
		sfx.channels[ch] = null;
	}

	var name = UTF8ToString(name_ptr);

	function startPlayback(decoded) {
		var source = ctx.createBufferSource();
		var fadeGain = ctx.createGain();
		var distGain = ctx.createGain();

		source.buffer = decoded;
		source.connect(fadeGain);
		fadeGain.connect(distGain);
		distGain.connect(sfx.groupGain[group]);

		// Distance attenuation: 0 = full volume, 255 = silent.
		var dist01 = 1.0 - (distance / 255.0);
		distGain.gain.setValueAtTime(dist01, ctx.currentTime);

		// Fade in.
		if (fadein_ms > 0) {
			fadeGain.gain.setValueAtTime(0, ctx.currentTime);
			fadeGain.gain.linearRampToValueAtTime(
				1.0, ctx.currentTime + fadein_ms / 1000.0);
		} else {
			fadeGain.gain.setValueAtTime(1.0, ctx.currentTime);
		}

		// Looping.
		if (repeats === -1) {
			source.loop = true;
		} else if (repeats > 0) {
			source.loop = true;
			// Play repeats+1 times total via scheduled stop.
			var totalDur = decoded.duration * (repeats + 1);
			source.stop(ctx.currentTime + totalDur);
		}

		// Auto-stop after loop_ms (only if we didn't already schedule a stop).
		if (loop_ms > 0 && repeats <= 0) {
			source.stop(ctx.currentTime + loop_ms / 1000.0);
		}

		var slot = { source: source, fadeGain: fadeGain, distGain: distGain };
		sfx.channels[ch] = slot;

		source.onended = function() {
			if (sfx.channels[ch] === slot) {
				sfx.channels[ch] = null;
				sfx.finishedQueue.push(ch);
			}
		};

		source.start();
	}

	// Check decoded AudioBuffer cache.
	if (sfx.cache.has(name)) {
		var idx = sfx.cacheOrder.indexOf(name);
		if (idx !== -1) sfx.cacheOrder.splice(idx, 1);
		sfx.cacheOrder.push(name);
		startPlayback(sfx.cache.get(name));
		return;
	}

	// Copy bytes from WASM heap (heap may move during async decode).
	var buf = new Uint8Array(HEAPU8.buffer, data, len).slice();

	ctx.decodeAudioData(buf.buffer).then(function(decoded) {
		// LRU cache.
		sfx.cache.set(name, decoded);
		var idx = sfx.cacheOrder.indexOf(name);
		if (idx !== -1) sfx.cacheOrder.splice(idx, 1);
		sfx.cacheOrder.push(name);
		while (sfx.cacheOrder.length > sfx.maxCache) {
			sfx.cache["delete"](sfx.cacheOrder.shift());
		}
		// Only play if channel hasn't been reassigned during decode.
		if (sfx.channels[ch] === null) {
			startPlayback(decoded);
		}
	})["catch"](function(err) {
		console.error("[Web Audio SFX] decode error for " + name + ":", err);
	});
});

EM_JS(void, wa_sfx_halt_channel, (int ch), {
	var sfx = Module._webAudioSfx;
	if (!sfx) return;
	var slot = sfx.channels[ch];
	if (slot) {
		try { slot.source.onended = null; slot.source.stop(); } catch(e) {}
		sfx.channels[ch] = null;
		sfx.finishedQueue.push(ch);
	}
});

EM_JS(void, wa_sfx_halt_group, (int group), {
	var sfx = Module._webAudioSfx;
	if (!sfx) return;
	var range = sfx.groupRange[group];
	if (!range) return;
	for (var ch = range[0]; ch <= range[1]; ch++) {
		var slot = sfx.channels[ch];
		if (slot) {
			try { slot.source.onended = null; slot.source.stop(); } catch(e) {}
			sfx.channels[ch] = null;
			sfx.finishedQueue.push(ch);
		}
	}
});

EM_JS(void, wa_sfx_set_distance, (int ch, int distance), {
	var sfx = Module._webAudioSfx;
	if (!sfx) return;
	var slot = sfx.channels[ch];
	if (slot) {
		var dist01 = 1.0 - (distance / 255.0);
		slot.distGain.gain.setValueAtTime(dist01, sfx.ctx.currentTime);
	}
});

EM_JS(void, wa_sfx_set_group_volume, (int group, double vol01), {
	var sfx = Module._webAudioSfx;
	if (!sfx) return;
	sfx.groupGain[group].gain.setValueAtTime(vol01, sfx.ctx.currentTime);
});

EM_JS(int, wa_sfx_is_channel_playing, (int ch), {
	var sfx = Module._webAudioSfx;
	return (sfx && sfx.channels[ch] !== null) ? 1 : 0;
});

EM_JS(int, wa_sfx_drain_finished, (), {
	var sfx = Module._webAudioSfx;
	if (!sfx || sfx.finishedQueue.length === 0) return -1;
	return sfx.finishedQueue.shift();
});

EM_JS(int, wa_sfx_find_free_channel, (int group), {
	var sfx = Module._webAudioSfx;
	if (!sfx) return -1;
	var range = sfx.groupRange[group];
	if (!range) return -1;
	for (var ch = range[0]; ch <= range[1]; ch++) {
		if (sfx.channels[ch] === null) return ch;
	}
	return -1;
});

// ---------------------------------------------------------------------------
// C++ wrappers for SFX bypass
// ---------------------------------------------------------------------------

namespace sound {
namespace emscripten {
namespace sfx {

void play(const char* filename, const uint8_t* data, std::size_t len,
          int channel, int group, int distance, int repeats,
          int fadein_ms, int loop_ms)
{
	wa_sfx_ensure_init();
	wa_sfx_play(filename, data, static_cast<int>(len),
	             channel, group, distance, repeats, fadein_ms, loop_ms);
}

void halt_channel(int channel)
{
	wa_sfx_ensure_init();
	wa_sfx_halt_channel(channel);
}

void halt_group(int group)
{
	wa_sfx_ensure_init();
	wa_sfx_halt_group(group);
}

void set_distance(int channel, int distance)
{
	wa_sfx_ensure_init();
	wa_sfx_set_distance(channel, distance);
}

void set_group_volume(int group, int vol)
{
	wa_sfx_ensure_init();
	double v = (vol < 0) ? 0.0 : (vol > 128) ? 1.0 : vol / 128.0;
	wa_sfx_set_group_volume(group, v);
}

bool is_channel_playing(int channel)
{
	return wa_sfx_is_channel_playing(channel) != 0;
}

int drain_finished()
{
	return wa_sfx_drain_finished();
}

int find_free_channel(int group)
{
	wa_sfx_ensure_init();
	return wa_sfx_find_free_channel(group);
}

} // namespace sfx
} // namespace emscripten
} // namespace sound

#endif // __EMSCRIPTEN__
