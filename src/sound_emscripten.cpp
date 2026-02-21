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

#endif // __EMSCRIPTEN__
