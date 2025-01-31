/*
	Copyright (C) 2004 - 2024
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

/**
 * @file
 * Animate units.
 */

#pragma once

#include <chrono>
#include <vector>

void new_animation_frame();
std::chrono::steady_clock::time_point get_current_animation_tick();

template<typename T>
class animated
{
public:
	typedef std::pair<std::chrono::milliseconds, T> frame_description;
	typedef std::vector<frame_description> anim_description;

	animated(const std::chrono::milliseconds& start_time = std::chrono::milliseconds{0});
	explicit animated(const anim_description& cfg, const std::chrono::milliseconds& start_time = std::chrono::milliseconds{0}, bool force_change = false);

	virtual ~animated() = default;

	/** Adds a frame to an animation. */
	void add_frame(const std::chrono::milliseconds& duration, const T& value, bool force_change = false);

	bool not_started() const;

	/**
	 * Starts an animation cycle.
	 *
	 * The first frame of the animation to start may be set to any value by
	 * using a start_time different to 0.
	 */
	void start_animation(const std::chrono::milliseconds& start_time, bool cycles = false);
	void pause_animation()
	{
		started_ = false;
	}

	void restart_animation()
	{
		if(start_tick_ != std::chrono::steady_clock::time_point{}) {
			started_ = true;
		}
	}

	std::chrono::milliseconds get_begin_time() const;
	std::chrono::milliseconds get_end_time() const;
	void set_begin_time(const std::chrono::milliseconds& new_begin_time);

	std::chrono::steady_clock::time_point time_to_tick(const std::chrono::milliseconds& animation_time) const;
	std::chrono::milliseconds tick_to_time(const std::chrono::steady_clock::time_point& animation_tick) const;

	void update_last_draw_time(double acceleration = 0);
	bool need_update() const;

	bool cycles() const
	{
		return cycles_;
	}

	/** Returns true if the current animation was finished. */
	bool animation_finished() const;
	bool animation_finished_potential() const;
	std::chrono::milliseconds get_animation_time() const;
	std::chrono::milliseconds get_animation_time_potential() const;
	void set_animation_time(const std::chrono::milliseconds& time);
	void set_max_animation_time(const std::chrono::milliseconds& time);

	std::chrono::milliseconds get_animation_duration() const;
	const T& get_current_frame() const;
	std::chrono::milliseconds get_current_frame_begin_time() const;
	std::chrono::milliseconds get_current_frame_end_time() const;
	std::chrono::milliseconds get_current_frame_duration() const;
	std::chrono::milliseconds get_current_frame_time() const;
	const T& get_first_frame() const;
	const T& get_frame(std::size_t n) const;
	const T& get_last_frame() const;
	std::size_t get_frames_count() const;

	void force_change()
	{
		does_not_change_ = false;
	}

	bool does_not_change() const
	{
		return does_not_change_;
	}

	static const T void_value_; // MSVC: the frame constructor below requires this to be public

protected:
	friend class unit_animation;

	void remove_frames_until(const std::chrono::milliseconds& starting_time);
	void set_end_time(const std::chrono::milliseconds& ending_time);

	std::chrono::milliseconds starting_frame_time_;

private:
	struct frame
	{
		// Represents the timestamp of the frame start
		std::chrono::milliseconds duration_;
		T value_;
		std::chrono::milliseconds start_time_;
	};

	bool does_not_change_; // Optimization for 1-frame permanent animations
	bool started_;
	bool force_next_update_;
	std::vector<frame> frames_;

	// Can set a maximum animation time so that movement in particular does not exceed potential time
	// Ignored if has a value of 0
	std::chrono::milliseconds max_animation_time_;

	// These are only valid when anim is started
	std::chrono::steady_clock::time_point start_tick_; // time at which we started
	bool cycles_;
	double acceleration_;
	std::chrono::steady_clock::time_point last_update_tick_;
	int current_frame_key_;
};

// NOTE: this needs to be down here or the templates won't build.
#include "animated.tpp"
