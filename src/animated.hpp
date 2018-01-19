/*
   Copyright (C) 2004 - 2018 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

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

#include <map>
#include <string>
#include <vector>

void new_animation_frame();
int get_current_animation_tick();

template<typename T>
class void_value
{
public:
	const T operator()() const
	{
		return T();
	}
};

template<typename T, typename T_void_value = void_value<T>>
class animated
{
public:
	animated(int start_time = 0);
	virtual ~animated() {}

	typedef std::pair<int, T> frame_description;
	typedef std::vector<frame_description> anim_description;
	animated(const std::vector<frame_description>& cfg, int start_time = 0, bool force_change = false);

	/** Adds a frame to an animation. */
	void add_frame(int duration, const T& value, bool force_change = false);

	/**
	 * Starts an animation cycle.
	 *
	 * The first frame of the animation to start may be set to any value by
	 * using a start_time different to 0.
	 */
	void start_animation(int start_time, bool cycles = false);
	void pause_animation()
	{
		started_ = false;
	}

	void restart_animation()
	{
		if(start_tick_)
			started_ = true;
	}

	int get_begin_time() const;
	int get_end_time() const;
	void set_begin_time(int new_begin_time);

	int time_to_tick(int animation_time) const;
	int tick_to_time(int animation_tick) const;

	void update_last_draw_time(double acceleration = 0);
	bool need_update() const;

	bool cycles() const
	{
		return cycles_;
	}

	/** Returns true if the current animation was finished. */
	bool animation_finished() const;
	bool animation_finished_potential() const;
	int get_animation_time() const;
	int get_animation_time_potential() const;
	void set_animation_time(int time);
	void set_max_animation_time(int time);

	int get_animation_duration() const;
	const T& get_current_frame() const;
	int get_current_frame_begin_time() const;
	int get_current_frame_end_time() const;
	int get_current_frame_duration() const;
	int get_current_frame_time() const;
	const T& get_first_frame() const;
	const T& get_frame(size_t n) const;
	const T& get_last_frame() const;
	size_t get_frames_count() const;

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
	int starting_frame_time_;
	void remove_frames_until(int starting_time);
	void set_end_time(int ending_time);

private:
	struct frame
	{
		frame(int duration, const T& value, int start_time)
			: duration_(duration)
			, value_(value)
			, start_time_(start_time)
		{
		}

		frame()
			: duration_(0)
			, value_(void_value_)
			, start_time_(0)
		{
		}

		// Represents the timestamp of the frame start
		int duration_;
		T value_;
		int start_time_;
	};

	bool does_not_change_; // Optimization for 1-frame permanent animations
	bool started_;
	bool force_next_update_;
	std::vector<frame> frames_;

	// Can set a maximum animation time so that movement in particular does not exceed potential time
	// Ignored if has a value of 0
	int max_animation_time_;

	// These are only valid when anim is started
	int start_tick_; // time at which we started
	bool cycles_;
	double acceleration_;
	int last_update_tick_;
	int current_frame_key_;
};

// NOTE: this needs to be down here or the templates won't build.
#include "animated.tpp"
