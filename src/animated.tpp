/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Copyright (C) 2005 - 2016 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file animated.tpp
 * Templates related to animations.
 */

#include <climits>

#include <SDL.h>
#include "animated.hpp"

template<typename T, typename T_void_value>
const T animated<T,T_void_value>::void_value_ = T_void_value()();

template<typename T, typename T_void_value>
inline animated<T,T_void_value>::animated(int start_time) :
	starting_frame_time_(start_time),
	does_not_change_(true),
	started_(false),
	force_next_update_(false),
	frames_(),
	start_tick_(0),
	cycles_(false),
	acceleration_(1),
	last_update_tick_(0),
	current_frame_key_(0)
{
}

template<typename T, typename T_void_value>
inline animated<T,T_void_value>::animated(const std::vector<std::pair<int,T> > &cfg, int start_time, bool force_change ):
	starting_frame_time_(start_time),
	does_not_change_(true),
	started_(false),
	force_next_update_(false),
	frames_(),
	start_tick_(0),
	cycles_(false),
	acceleration_(1),
	last_update_tick_(0),
	current_frame_key_(0)
{
	typename std::vector< std::pair<int,T> >::const_iterator itor = cfg.begin();
	for (; itor != cfg.end(); ++itor) {
		add_frame(itor->first,itor->second,force_change);
	}
}

template<typename T, typename T_void_value>
inline void animated<T,T_void_value>::add_frame(int duration, const T& value,bool force_change)
{
	if (frames_.empty() ) {
		does_not_change_=!force_change;
		frames_.emplace_back(duration,value,starting_frame_time_);
	} else {
		does_not_change_=false;
		frames_.emplace_back(duration,value,frames_.back().start_time_+frames_.back().duration_);
	}
}

template<typename T, typename T_void_value>
inline void animated<T,T_void_value>::start_animation(int start_time, bool cycles)
{
	started_ = true;
	last_update_tick_ = get_current_animation_tick();
	acceleration_ = 1.0; //assume acceleration is 1, this will be fixed at first update_last_draw_time
	start_tick_ = last_update_tick_ +
		static_cast<int>(( starting_frame_time_ - start_time)/acceleration_);

	cycles_ = cycles;
	if (acceleration_ <= 0) {
		acceleration_ = 1;
	}
	current_frame_key_= 0;
	force_next_update_ = !frames_.empty();
}

template<typename T, typename T_void_value>
inline void animated<T,T_void_value>::update_last_draw_time(double acceleration)
{
	if (acceleration > 0 && acceleration_ != acceleration) {
		int tmp = tick_to_time(last_update_tick_);
		acceleration_ = acceleration;
		start_tick_ = last_update_tick_ +
			static_cast<int>(( starting_frame_time_ - tmp)/acceleration_);
	}
	if (!started_ && start_tick_ != 0) {
		// animation is paused
		start_tick_ += get_current_animation_tick() - last_update_tick_;
	}
	last_update_tick_ = get_current_animation_tick();
	if (force_next_update_) {
		force_next_update_ = false;
		return;
	}
	if (does_not_change_) {
		return;
	}

	// Always update last_update_tick_, for the animation_time functions to work.
	if (!started_) {
		return;
	}

	if (frames_.empty()) {
		does_not_change_ = true;
		return;
	}
	if (cycles_) {
		while(get_animation_time() > get_end_time()){  // cut extra time
			start_tick_ += std::max<int>(static_cast<int>(get_animation_duration()/acceleration_),1);
			current_frame_key_ = 0;
		}
	}
	if (get_current_frame_end_time() < get_animation_time() &&  // catch up
			get_current_frame_end_time() < get_end_time()) {// don't go after the end
		current_frame_key_++;
	}
}

template<typename T, typename T_void_value>
inline bool animated<T,T_void_value>::need_update() const
{
	if (force_next_update_) {
		return true;
	}
	if (does_not_change_) {
		return false;
	}
	if (frames_.empty()) {
		return false;
	}
	if (!started_ && start_tick_ == 0) {
		return false;
	}
	if (get_current_animation_tick() >
			static_cast<int>(get_current_frame_end_time() /
			acceleration_ + start_tick_)) {
		return true;
	}
	return false;
}

template<typename T, typename T_void_value>
inline bool animated<T,T_void_value>::animation_finished_potential() const
{
	if (frames_.empty()) {
		return true;
	}
	if (!started_ && start_tick_ == 0) {
		return true;
	}
	if (cycles_ ) {
		return true;
	}
	if (tick_to_time(get_current_animation_tick()) > get_end_time()) {
		return true;
	}

	return false;
}

template<typename T, typename T_void_value>
inline bool animated<T,T_void_value>::animation_finished() const
{
	if (frames_.empty()) {
		return true;
	}
	if (!started_ && start_tick_ == 0) {
		return true;
	}
	if (cycles_) {
		return true;
	}
	if (get_animation_time() >  get_end_time()) {
		return true;
	}

	return false;
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_animation_time_potential() const
{
	if (!started_ && start_tick_ == 0 ) {
		return starting_frame_time_;
	}

	return tick_to_time(get_current_animation_tick());
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_animation_time() const
{
	if (!started_ && start_tick_ == 0 ) {
		return starting_frame_time_;
	}

	return tick_to_time(last_update_tick_);
}

template<typename T, typename T_void_value>
inline void animated<T,T_void_value>::set_animation_time(int time)
{
	start_tick_ = last_update_tick_ +
		static_cast<int>((starting_frame_time_ - time) / acceleration_);

	current_frame_key_= 0;
	force_next_update_ = true;
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_animation_duration() const
{
	return get_end_time() - get_begin_time();
}

template<typename T, typename T_void_value>
inline const T& animated<T,T_void_value>::get_current_frame() const
{
	if (frames_.empty()) {
		return void_value_;
	}
	return frames_[current_frame_key_].value_;
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_current_frame_begin_time() const
{
	if (frames_.empty()) {
		return starting_frame_time_;
	}
	return frames_[current_frame_key_].start_time_;
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_current_frame_end_time() const
{
	if (frames_.empty()) {
		return starting_frame_time_;
	}
	return get_current_frame_begin_time() +get_current_frame_duration();
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_current_frame_duration() const
{
	if (frames_.empty()) {
		return 0;
	}
	return frames_[current_frame_key_].duration_;
}

template<typename T,  typename T_void_value>
inline int animated<T,T_void_value>::get_current_frame_time() const
{
	if (frames_.empty()) {
		return 0;
	}
	//FIXME: get_animation_time() use acceleration but get_current_frame_begin_time() doesn't ?
	return std::max<int>(0,get_animation_time() - get_current_frame_begin_time());
}

template<typename T, typename T_void_value>
inline const T& animated<T,T_void_value>::get_first_frame() const
{
	if (frames_.empty()) {
		return void_value_;
	}
	return frames_[0].value_;
}

template<typename T, typename T_void_value>
inline const T& animated<T,T_void_value>::get_frame(size_t n) const
{
	if (n >= frames_.size()) {
		return void_value_;
	}
	return frames_[n].value_;
}

template<typename T, typename T_void_value>
inline const T& animated<T,T_void_value>::get_last_frame() const
{
	if (frames_.empty()) {
		return void_value_;
	}
	return frames_.back().value_;
}

template<typename T, typename T_void_value>
inline size_t animated<T,T_void_value>::get_frames_count() const
{
	return frames_.size();
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_begin_time() const
{
	return starting_frame_time_;
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::time_to_tick(int animation_time) const
{
	if (!started_ && start_tick_ == 0) {
		return 0;
	}
	return start_tick_ + static_cast<int>((animation_time-starting_frame_time_) / acceleration_);
}

template<typename T,  typename T_void_value>
inline int animated<T,T_void_value>::tick_to_time(int animation_tick) const
{
	if (!started_ && start_tick_ == 0) {
		return 0;
	}
	return static_cast<int>(
		(static_cast<double>(animation_tick - start_tick_) *
			acceleration_) + starting_frame_time_);
}

template<typename T, typename T_void_value>
inline int animated<T,T_void_value>::get_end_time() const
{
	if (frames_.empty()) {
		return starting_frame_time_;
	}
	return frames_.back().start_time_ + frames_.back().duration_;
}

template<typename T, typename T_void_value>
void animated<T,T_void_value>::remove_frames_until(int new_starting_time)
{
	while (starting_frame_time_	 < new_starting_time && !frames_.empty()) {
		starting_frame_time_ += frames_[0].duration_;
		frames_.erase(frames_.begin());
	}
}

template<typename T, typename T_void_value>
inline void animated<T,T_void_value>::set_end_time(int new_ending_time)
{
	int last_start_time = starting_frame_time_;
	typename std::vector<frame>::iterator current_frame = frames_.begin();
	while (last_start_time	< new_ending_time && current_frame != frames_.end()) {
		last_start_time += current_frame->duration_;
		++current_frame;
	}
	// at this point last_start_time is set to the beginning of the first frame past the end
	// or set to frames_.end()
	frames_.erase(current_frame,frames_.end());
	frames_.back().duration_ += new_ending_time - last_start_time;
}

template<typename T, typename T_void_value>
inline void animated<T,T_void_value>::set_begin_time(int new_begin_time)
{
	const int variation = new_begin_time - starting_frame_time_;
	starting_frame_time_ += variation;
	for (typename std::vector<frame>::iterator itor = frames_.begin(); itor != frames_.end() ; ++itor) {
		itor->start_time_ += variation;
	}
}

