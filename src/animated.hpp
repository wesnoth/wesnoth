/*
	Copyright (C) 2004 - 2025
	by Philippe Plantier <ayin@anathas.org>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/** @file animated.hpp
 * Frame-based animation system with support for looping, acceleration, and time manipulation.
 * The animation system uses two parallel timelines:
 * - Normal timeline: Real wall-clock time for UI animations, effects, etc.
 * - Accelerated timeline: Can be sped up/slowed down for unit movement, attacks, etc. */

#pragma once

#include <algorithm>
#include <chrono>
#include <vector>

// ============================================================================
// Global Animation Timeline Management
// ============================================================================

/** Updates both animation timelines.
 * Should be called once per frame before advancing any animations.
 * @param acceleration Multiplier for the accelerated timeline (1.0 = normal speed) */
void update_animation_timers(double acceleration = 1.0);

/** Gets the current time point for animations.
 * @param uses_acceleration If true, returns accelerated timeline; otherwise normal timeline
 * @return Current time point on the selected timeline */
std::chrono::steady_clock::time_point get_current_animation_tick(bool uses_acceleration);


// ============================================================================
// Frame-Based Animation Template
// ============================================================================

template<typename T>
class animated
{
public:
	typedef std::pair<std::chrono::milliseconds, T> frame_description;
	typedef std::vector<frame_description> anim_description;

	// ========================================================================
	// Construction
	// ========================================================================

	/** Creates an animation with no frames; add_frame() populates it afterward.
	 * @param start_time Logical start time of the animation timeline (default 0ms) */
	animated(const std::chrono::milliseconds& start_time = std::chrono::milliseconds{0});

	/** Creates an animation with its frame list already built from cfg.
	 * @param cfg Sequence of (duration, value) pairs to build the frame list from
	 * @param start_time Logical start time of the animation timeline
	 * @param force_change See add_frame(). Applies to the whole sequence built from cfg. */
	explicit animated(const anim_description& cfg,
		const std::chrono::milliseconds& start_time = std::chrono::milliseconds{0},
		bool force_change = false);

	virtual ~animated() = default;

	/** Appends a frame, starting where the previous one ends (or at start_time, if first).
	 * @param duration How long this frame should be displayed
	 * @param value The data/state for this frame
	 * @param force_change Forces is_static() false even if this is the only frame added so
	 * far; use when the frame's own value can change over time despite the frame count. */
	void add_frame(const std::chrono::milliseconds& duration, const T& value, bool force_change = false);

	// ========================================================================
	// Playback Control
	// ========================================================================

	/** Anchors the timeline to the current tick, then begins playback.
	 * @param start_time_offset How far into the animation to start (0 = from beginning)
	 * @param cycles If true, loop back to the first frame on reaching the end; if false, stop there */
	void start_animation(const std::chrono::milliseconds& start_time_offset, bool cycles = false);

	/** Freezes the timeline in place. Playback continues from the same point via resume_animation(). */
	void pause_animation();

	/** Shifts the timeline forward by the time spent paused, then continues playback. */
	void resume_animation();

	/** Moves current_frame_index_ forward to match the current tick, one frame at a time
	 * (or in whole cycles, if far enough behind). Marks the animation finished() if it
	 * runs out of frames without cycles(). */
	void advance_to_current_frame();

	// ========================================================================
	// Playback State Queries
	// ========================================================================

	/** True once the current tick has reached the current frame's end, meaning
	 * advance_to_current_frame() has a new frame to move to. */
	bool need_update() const;

	/** True if playback has run past the last frame, is not yet started, or has no frames.
	 * Always true for a cycling animation, since it never runs out. */
	bool animation_finished() const;

	/** True if the animation loops back to the first frame instead of stopping at the last. */
	bool cycles() const { return cycles_; }

	// ========================================================================
	// Time Queries and Manipulation
	// ========================================================================

	/** Time since playback began, per get_playback_tick(). 0 if not started.
	 * Capped at the limit set by set_duration_limit(), if any. */
	std::chrono::milliseconds get_elapsed_time() const;

	/** Shifts the timeline so the current tick corresponds to the given animation time,
	 * then restarts frame tracking from the beginning to resync.
	 * Used to desynchronize otherwise-identical animations for visual variety.
	 * @param time e.g. 300ms starts the animation as if it had been playing for 300ms already */
	void apply_time_offset(const std::chrono::milliseconds& time);

	/** @param time Elapsed time past which animation_finished() reports true regardless of
	 * actual progress (0 = no limit). Does not affect frame playback, only that query. */
	void set_duration_limit(const std::chrono::milliseconds& time);

	/** get_end_time() minus get_begin_time(): how long one pass through all frames takes. */
	std::chrono::milliseconds get_animation_duration() const;

	/** Animation-time offset of the first frame's start. Default 0ms; see set_begin_time(). */
	std::chrono::milliseconds get_begin_time() const;

	/** Animation-time offset where the last frame ends.
	 * For 3 seconds of frames starting at 500ms (see get_begin_time()), this returns 3500ms. */
	std::chrono::milliseconds get_end_time() const;

	/** Rebases the timeline onto a new begin time, offsetting every frame's start_time_
	 * by the difference so their durations and order are unchanged. */
	void set_begin_time(const std::chrono::milliseconds& new_begin_time);

	// ========================================================================
	// Frame Queries
	// ========================================================================

	/** Animation-time offset where the current frame starts. */
	std::chrono::milliseconds get_current_frame_begin_time() const;

	/** Animation-time offset where the current frame ends. */
	std::chrono::milliseconds get_current_frame_end_time() const;

	/** How far into its own duration the current frame is, clamped to [0, duration]. */
	std::chrono::milliseconds get_time_in_current_frame() const;

	const T& get_current_frame() const;
	const T& get_first_frame() const;
	const T& get_frame(std::size_t n) const;
	const T& get_last_frame() const;

	std::size_t get_frames_count() const;

	// ========================================================================
	// Optimization and Configuration
	// ========================================================================

	/** Clears the flag returned by is_static(). Needed because is_static_ is set purely from
	 * frame count: a single frame can still change visually over time through means this
	 * class doesn't track (e.g. unit_frame's interpolated offset/alpha/image parameters),
	 * in which case the owner must call this to keep need_update() reporting true. */
	void force_change() { is_static_ = false; }

	/** True when the single-unchanging-frame optimization is enabled: need_update() will
	 * always report false, since there is never a different frame to advance to.
	 * Set from add_frame()/the constructor when the animation has exactly one frame, unless
	 * force_change requests otherwise; cleared permanently by add_frame() past the first frame,
	 * or by force_change(). */
	bool is_static() const { return is_static_; }

	/** Selects which of the two global timelines get_current_animation_tick()
	 * reads for this animation: accelerated if true, normal if false. */
	void set_uses_acceleration(bool uses_accel) { uses_acceleration_ = uses_accel; }

	/** Which timeline this animation currently reads; see set_uses_acceleration(). */
	bool uses_acceleration() const { return uses_acceleration_; }

	static const T void_value_; // MSVC: the frame constructor below requires this to be public

protected:
	friend class unit_animation;

	/** Drops whole frames from the front while their end time is still before starting_time,
	 * then advances the begin time by their durations. May overshoot starting_time if it
	 * falls inside a remaining frame, rather than split that frame.
	 * @param starting_time Animation time the new begin time should reach */
	void remove_frames_until(const std::chrono::milliseconds& starting_time);

	/** Reduces or extends the frame list so get_end_time() becomes ending_time: drops frames
	 * after the cut and shortens the one it falls in, extends the last frame if ending_time is
	 * later, or (if ending_time is before the current begin time) collapses to a zero-length
	 * first frame.
	 * @param ending_time Animation time the new end time should reach */
	void set_end_time(const std::chrono::milliseconds& ending_time);

private:
	/** The tick get_elapsed_time() and get_time_in_current_frame() treat as "now": frozen at
	 * pause_tick_ while paused_, otherwise the live tick from get_current_animation_tick().
	 * Playback bookkeeping (start/pause/resume/advance) reads get_current_animation_tick()
	 * directly instead, since it needs the live clock even while paused_. */
	std::chrono::steady_clock::time_point get_playback_tick() const;

	// Internal frame representation
	struct frame
	{
		std::chrono::milliseconds duration_;    // How long this frame lasts
		T value_;                               // The frame data
		std::chrono::milliseconds start_time_;  // When this frame starts (in animation time)
	};

	// Animation timeline configuration
	std::chrono::milliseconds starting_frame_time_; // Logical start time of the animation
	std::chrono::milliseconds max_animation_time_;  // Optional duration cap (0 = unlimited)

	// Frame storage and tracking
	std::vector<frame> frames_;         // All frames in sequence
	std::size_t current_frame_index_;   // Which frame is currently displayed

	// Playback state
	bool started_;              // Whether values are initialized by start_animation()
	bool paused_;               // True while frozen at pause_tick_; see get_playback_tick()
	bool finished_;             // Set by advance_to_current_frame() when a non-cycling animation runs out of frames
	bool cycles_;               // Whether the animation loops/cycles
	bool uses_acceleration_;    // Which timeline to follow, normal or accelerated.

	// Timeline synchronization
	// The animation's position is tracked by relating animation time to real timeline ticks.
	std::chrono::steady_clock::time_point animation_start_tick_;    // Real tick corresponding to animation time 0 (can be adjusted).
	std::chrono::steady_clock::time_point next_frame_tick_;         // Real tick when next frame should display
	std::chrono::steady_clock::time_point pause_tick_;              // Real tick when pause_animation() was called

	// Optimization flag
	bool is_static_;   // True if all frames identical (skip updates)
};

// NOTE: this needs to be down here or the templates won't build.
#include "animated.tpp"
