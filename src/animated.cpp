/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2005 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

//#include <string>
//#include <vector>
#include "SDL.h"
#include "animated.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"

template<typename T, typename T_void_value>
const T animated<T,T_void_value>::void_value_ = T_void_value()();

template<typename T, typename T_void_value>
animated<T,T_void_value>::animated() :
	starting_frame_time_(INT_MAX),
	ending_frame_time_(INT_MIN),
	started_(false),
	no_current_frame_(true),
	does_not_change_(false),
	real_start_ticks_(0),
	start_ticks_(0),
	acceleration_(1)
{}

template<typename T,  typename T_void_value>
animated<T,T_void_value>::animated(const std::string &cfg, const string_initializer& init):
	starting_frame_time_(INT_MAX),
	started_(false),
	no_current_frame_(true),
	does_not_change_(false),
	real_start_ticks_(0),
	start_ticks_(0),
	acceleration_(1)
{
	std::vector<std::string> items = utils::split(cfg);

	int current_time = 0;

	std::vector<std::string>::const_iterator itor = items.begin();
	for(; itor != items.end(); ++itor) {
		const std::vector<std::string>& items = utils::split(*itor, ':');
		std::string str;
		int time;

		if(items.size() > 1) {
			str = items.front();
			time = atoi(items.back().c_str());
		} else {
			str = *itor;
			time = 100;
		}

		frames_.push_back(frame(current_time, init(str)));
		current_time += time;
	}

	starting_frame_time_ = 0;
	ending_frame_time_ = current_time;
}


template<typename T,  typename T_void_value>
void animated<T,T_void_value>::add_frame(int start)
{
	frames_.push_back(frame(start));
	starting_frame_time_ = minimum<int>(starting_frame_time_, start);
	ending_frame_time_ = maximum<int>(ending_frame_time_, start);
}

template<typename T,  typename T_void_value>
void animated<T,T_void_value>::add_frame(int start, const T& value)
{
	frames_.push_back(frame(start, value));
	starting_frame_time_ = minimum<int>(starting_frame_time_, start);
	ending_frame_time_ = maximum<int>(ending_frame_time_, start);
}

template<typename T,  typename T_void_value>
void animated<T,T_void_value>::start_animation(int start_frame, int cycles, int acceleration)
{
	started_ = true;
	start_frame_ = start_frame;
	start_ticks_ = real_start_ticks_ = current_time_ = SDL_GetTicks() * acceleration;
	cycles_ = cycles;
	current_cycle_ = 0;
	acceleration_ = acceleration;
	// current_frame_ = frames_.begin();
	current_frame_ = 0;

	if (ending_frame_time_ >= start_frame_) {
		duration_ = ending_frame_time_ - start_frame_;
	} else {
		duration_ = 0;
	}
}


template<typename T,  typename T_void_value>
void animated<T,T_void_value>::update_current_frame_internal()
{
	// std::cerr << "--- updating frame ---\n";
	if(does_not_change_)
		return;

	frame_changed_ = false;
	// Always update current_time_, for the animation_time functions to work.
	current_time_ = SDL_GetTicks() * acceleration_;
	if(!started_)
		return;

	if(frames_.empty()) {
		no_current_frame_ = true;
		does_not_change_ = true;
		return;
	}

	if(frames_.size() == 1 && cycles_ == INFINITE_CYCLES) {
		does_not_change_ = true;
		frame_changed_ = false;
	}

	if (duration_ > 0) {
		// Ticks is the actual time since animation started.
		int ticks = current_time_ - start_ticks_;

		// Handles cycle overflow
		if(ticks > duration_) {
			int ncycles = ticks/duration_;
			current_cycle_ = minimum<int>(cycles_, current_cycle_ + ncycles);
			start_ticks_ += ncycles * duration_;
			ticks -= ncycles * duration_;
			// current_frame_ = frames_.begin();
			current_frame_ = 0;
			frame_changed_ = true;
		}
		// Checks if the animation is finished
		if(cycles_ != INFINITE_CYCLES && current_cycle_ >= cycles_) {
			// If the animation is finished, the current frame is the last
			// one
			current_frame_ = frames_.size() - 1;
			frame_changed_ = true;
			// std::cerr << "Animation finished\n";
			no_current_frame_ = false;
			started_ = false;
			return;
		}

		if(ticks < (frames_[current_frame_].milliseconds - start_frame_)) {
			// std::cerr << "Animation not yet started\n";
			frame_changed_ = true;
			no_current_frame_ = true;
			return;
		}

		// Looks for the current frame
		typename std::vector<frame>::size_type i = current_frame_ + 1;
		for(; i != frames_.size(); ++i) {
			if(ticks < (frames_[i].milliseconds - start_frame_))
				break;
			current_frame_ = i;
			frame_changed_ = true;
			// std::cerr << "Skipping to next frame\n";
		}
		no_current_frame_ = false;

	} else {
		// If the duration is void, the animation is automatically finished.
		// current_cycle_ = cycles_;
		if(cycles_ != -1)
			started_ = false;

		does_not_change_ = true;
		frame_changed_ = false;
		// current_frame_ = frames_.size() - 1;
		// frame_changed_ = false;
		// std::cerr << "Returning last frame\n";
		no_current_frame_ = false;
	}

}

template<typename T,  typename T_void_value>
bool animated<T,T_void_value>::frame_changed() const
{
	return frame_changed_;
}

template<typename T,  typename T_void_value>
bool animated<T,T_void_value>::animation_finished() const
{
	if(!started_)
		return true;
	//if(current_cycle_ == cycles_)
	//	return true;

	return false;
}

template<typename T,  typename T_void_value>
int animated<T,T_void_value>::get_animation_time() const
{
	if(does_not_change_)
		return SDL_GetTicks() * acceleration_ - real_start_ticks_ + start_frame_;

	return current_time_ - real_start_ticks_ + start_frame_;
}

template<typename T,  typename T_void_value>
int animated<T,T_void_value>::get_frame_time() const
{
	return current_time_ - start_ticks_ + start_frame_;
}

template<typename T,  typename T_void_value>
const T& animated<T,T_void_value>::get_current_frame() const
{
	if(no_current_frame_ == true)
		return void_value_;
	const frame& cur = frames_[current_frame_];
	if(!cur.has_value)
		return void_value_;
	return cur.value;
}

template<typename T,  typename T_void_value>
const T& animated<T,T_void_value>::get_first_frame() const
{
	typename std::vector<frame>::const_iterator frame_;
	for(frame_ = frames_.begin() ; frame_ != frames_.end(); frame_++ ) {
		if(frame_->has_value)
			return frame_->value;
	}
	return void_value_;
}

template<typename T,  typename T_void_value>
const T& animated<T,T_void_value>::get_last_frame() const
{
	typename std::vector<frame>::const_reverse_iterator frame_;
	for(frame_ = frames_.rbegin() ; frame_ != frames_.rend(); frame_++ ) {
		if(frame_->has_value)
			return frame_->value;
	}
	return void_value_;
}

template<typename T,  typename T_void_value>
int animated<T,T_void_value>::get_first_frame_time() const
{
	if (starting_frame_time_ != INT_MAX && starting_frame_time_ != INT_MIN)
		return starting_frame_time_;

	return 0;
}

template<typename T,  typename T_void_value>
int animated<T,T_void_value>::get_last_frame_time() const
{
	if (ending_frame_time_ != INT_MAX && ending_frame_time_ != INT_MIN)
		return ending_frame_time_;

	return 0;
}

// Force compilation of the following template instantiations

#include "image.hpp"
#include "unit_types.hpp"

template class animated< image::locator >;
template class animated< std::string >;
template class animated< unit_animation::frame >;
