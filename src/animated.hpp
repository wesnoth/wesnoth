/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ANIMATED_IMAGE_H_INCLUDED
#define ANIMATED_IMAGE_H_INCLUDED

#include <string>
#include <vector>

template<typename T>
class void_value
{
	public:
		const T operator()() { return T(); }
};

template<typename T, typename T_void_value=void_value<T> >
class animated
{
public:
	class string_initializer
	{
	public:
		virtual T operator()(const std::string& s) const { return T(s); }
		virtual ~string_initializer(){};
	};

	animated();
	virtual ~animated(){};

	//if T can be constructed from a string, you may use this constructor
	// animated(const std::string& cfg);
	//if T cannot, you may provide a custom (subclassed) string_initializer
	//to do the job

	animated(const std::string &cfg, const string_initializer& init=string_initializer());

	// Adds a void frame
	void add_frame(int start);

	// Adds a frame
	void add_frame(int start, const T& value);

	//Starts an animation cycle. The first frame of the animation to start
	//may be set to any value
	enum { INFINITE_CYCLES = -1 };
	void start_animation(int start_time=0, int cycles=1, int acceleration=1);

	int get_first_frame_time() const;
	int get_last_frame_time() const;

	//inlined for performance
	void update_current_frame();
	bool frame_changed() const;

	//True if the current animation was finished
	bool animation_finished() const;
	int get_animation_time() const;
	int get_cycle_time() const;
	const T& get_current_frame() const;
	const T& get_first_frame() const;
	const T& get_last_frame() const;


	static void synchronize_start(animated<T> &a, animated<T>& b,int acceleration);

private:
	struct frame
	{
		frame(int milliseconds) :
			milliseconds(milliseconds), has_value(false)
		{};

		frame(int milliseconds, const T& value) :
			milliseconds(milliseconds), has_value(true), value(value)
		{};

		// Represents the timestamp of the frame start
		int milliseconds;
		bool has_value;
		T value;
	};

	static const T void_value_;

	int starting_frame_time_;
	int ending_frame_time_;

	bool started_;
	bool no_current_frame_;
	bool does_not_change_;	// optimization for 1-frame permanent animations

	int real_start_ticks_;
	int start_ticks_;
	int current_cycle_;
	int current_time_;
	int cycles_;
	int acceleration_;
	bool frame_changed_;
	int start_frame_;
	int duration_;
	typename std::vector<frame>::size_type current_frame_;

	std::vector<frame> frames_;
};

#endif

