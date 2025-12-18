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

	/** Creates an empty animation.
	 * @param start_time Logical start time of the animation timeline (default 0ms) */
	animated(const std::chrono::milliseconds& start_time = std::chrono::milliseconds{ 0 });

	/** Creates an animation from a sequence of frames.
	 * @param cfg Vector of (duration, value) pairs defining the animation
	 * @param start_time Logical start time of the animation timeline
	 * @param force_change If true, marks animation as changing even if all frames are identical */
	explicit animated(const anim_description& cfg,
		const std::chrono::milliseconds& start_time = std::chrono::milliseconds{ 0 },
		bool force_change = false);

	virtual ~animated() = default;

	/** Adds a frame to the animation.
	 * Frames are added sequentially; each new frame starts when the previous one ends.
	 * @param duration How long this frame should be displayed
	 * @param value The data/state for this frame
	 * @param force_change If true, marks the animation as changing (disables optimization) */
	void add_frame(const std::chrono::milliseconds& duration, const T& value, bool force_change = false);

	// ========================================================================
	// Playback Control
	// ========================================================================

	/** Starts playing the animation from a specific time offset.
	 * @param start_time_offset How far into the animation to start (0 = from beginning)
	 * @param cycles If true, animation loops; if false, plays once and stops */
	void start_animation(const std::chrono::milliseconds& start_time_offset, bool cycles = false);

	/** Pauses the animation, preserving current position.
	 * Use unpause_animation() to resume from where it was paused. */
	void pause_animation();

	/** Resumes a paused animation from where it left off.*/
	void unpause_animation();

	/** Advances to the correct frame based on elapsed time.
	 * Handles frame progression and looping logic.*/
	void advance_to_current_frame();

	// ========================================================================
	// Playback State Queries
	// ========================================================================

	/** Checks if it's time for animation to change frame/image. */
	bool need_update() const;

	/** Checks if the animation has finished playing. (If duration has run out.)
	 * Always returns true for looping animations. */
	bool animation_finished() const;

	/** Checks if this animation does infinite loops/cycles. */
	bool cycles() const { return cycles_; }

	// ========================================================================
	// Time Queries and Manipulation
	// ========================================================================

	/** Gets how much time has elapsed in the animation.
	 * Returns 0 if not started, or time since animation_start_tick_.
	 * Clamped by max_animation_time if set. */
	std::chrono::milliseconds get_elapsed_time() const;

	/** Offsets the animation timeline.
	 * Used to desynchronize animations for visual variety.
	 * @param time Offset amount (e.g., 300ms shifts animation 300ms forward in its cycle) */
	void apply_time_offset(const std::chrono::milliseconds& time);

	/** Sets a maximum duration for the animation.
	 * The animation will not progress beyond this time, and will be flagged as finished.
	 * @param time Maximum duration (0 = no limit) */
	void set_duration_limit(const std::chrono::milliseconds& time);

	/** Gets the total duration of one complete animation cycle. */
	std::chrono::milliseconds get_animation_duration() const;

	/** Gets the starting time offset for the animation timeline.
     * Default is 0ms; can be changed with set_begin_time(). */
	std::chrono::milliseconds get_begin_time() const;

	/** Gets the ending time offset of the animation timeline.
     * For a 3-second animation starting at 0ms, this returns 3000ms. */
	std::chrono::milliseconds get_end_time() const;

	/** Shifts the entire animation timeline by changing the begin time.
	 * All frame start times are adjusted accordingly. */
	void set_begin_time(const std::chrono::milliseconds& new_begin_time);

	/** Converts an animation time offset to an absolute timeline tick.
	 * @param animation_time Offset from animation start
	 * @return Absolute time point on the animation's timeline */
	std::chrono::steady_clock::time_point time_to_tick(const std::chrono::milliseconds& animation_time) const;

	// ========================================================================
	// Frame Queries
	// ========================================================================

	/** Gets the time offset from cycle/animation start where the current frame begins. */
	std::chrono::milliseconds get_current_frame_begin_time() const;

	/** Gets the time offset from cycle/animation start where the current frame ends. */
	std::chrono::milliseconds get_current_frame_end_time() const;

	/** Gets how long the current frame has been displayed. */
	std::chrono::milliseconds get_time_in_current_frame() const;

	const T& get_current_frame() const;

	const T& get_first_frame() const;

	const T& get_frame(std::size_t n) const;

	const T& get_last_frame() const;

	std::size_t get_frames_count() const;

	// ========================================================================
	// Optimization and Configuration
	// ========================================================================

	/** Forces the animation/image to be treated as a changing animation. */
	void force_change() { is_static_ = false; }

	/** Checks if this animation/image is optimized as static (single unchanging frame). */
	bool is_static() const { return is_static_; }

	/** Configures whether this animation uses the accelerated timeline.
	 * @param uses_accel True for accelerated (unit movement), false for normal (UI) */
	void set_uses_acceleration(bool uses_accel) { uses_acceleration_ = uses_accel; }

	/** Checks if this animation uses the accelerated timeline.*/
	bool uses_acceleration() const { return uses_acceleration_; }

	static const T void_value_; // MSVC: the frame constructor below requires this to be public

protected:
	friend class unit_animation;

	/** Removes frames before a given timeline offset, shortening the cycle/animation.
     * @param starting_time Time offset - frames before this are removed */
	void remove_frames_until(const std::chrono::milliseconds& starting_time);

	/** Truncates the cycle/animation at a specific timeline offset.
     * Frames after this time are removed, and the last frame is shortened if needed.
     * @param ending_time Time offset where the cycle/animation should end */
	void set_end_time(const std::chrono::milliseconds& ending_time);

private:
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
	bool paused_;               // Whether the animation is currently paused
	bool finished_;             // Whether the animation has finished, cycling animations never finish
	bool cycles_;               // Whether the animation loops/cycles
	bool uses_acceleration_;    // Which timeline to follow

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
