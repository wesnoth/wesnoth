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

#include "SDL.h"
#include "animated.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"

namespace {
	int current_ticks = 0;
}

void new_animation_frame()
{
	current_ticks = SDL_GetTicks();
}

int get_current_animation_tick() 
{
	return current_ticks;
}

template<typename T, typename T_void_value>
const T animated<T,T_void_value>::void_value_ = T_void_value()();

template<typename T, typename T_void_value>
animated<T,T_void_value>::animated(int start_time) :
	starting_frame_time_(start_time),
	does_not_change_(true),
	started_(false),
	start_tick_(0),
	cycles_(false),
	acceleration_(1),
	last_update_tick_(0),
	current_frame_key_(0)
{
}

template<typename T,  typename T_void_value>
animated<T,T_void_value>::animated(const std::vector<std::pair<int,T> > &cfg, int start_time, bool force_change ):
	starting_frame_time_(start_time),
	does_not_change_(true),
	started_(false),
	start_tick_(0),
	cycles_(false),
	acceleration_(1),
	last_update_tick_(0),
	current_frame_key_(0)
{

	typename std::vector< std::pair<int,T> >::const_iterator itor = cfg.begin();
	for(; itor != cfg.end(); ++itor) {

		add_frame(itor->first,itor->second,force_change);
	}
}



template<typename T,  typename T_void_value>
void animated<T,T_void_value>::add_frame(int duration, const T& value,bool force_change)
{
	if(frames_.empty() ) {
		does_not_change_=!force_change;
		frames_.push_back( frame(duration,value,starting_frame_time_));
	} else {
		does_not_change_=false;
		frames_.push_back( frame(duration,value,frames_.back().start_time_+frames_.back().duration_));
	}
}

template<typename T,  typename T_void_value>
void animated<T,T_void_value>::start_animation(int start_time, bool cycles, double acceleration)
{
	started_ = true;
	last_update_tick_ = current_ticks;
	start_tick_ =  last_update_tick_ + ( starting_frame_time_ - start_time);
	cycles_ = cycles;
	acceleration_ = acceleration;
	//FIXME: allow negative acceleration -> reverse animation
	if(acceleration_ <=0) acceleration_ = 1;
	current_frame_key_= 0;
	update_last_draw_time();
}


template<typename T,  typename T_void_value>
void animated<T,T_void_value>::update_last_draw_time()
{
	last_update_tick_ = current_ticks;
	if(does_not_change_)
		return;

	// Always update last_update_tick_, for the animation_time functions to work.
	if(!started_) {
		return;
	}

	if(frames_.empty()) {
		does_not_change_ = true;
		return;
	}
	if(cycles_) {
		while(get_animation_time() > get_end_time()){  // cut extra time
			start_tick_ +=(int)(get_end_time()/acceleration_);
			current_frame_key_ =starting_frame_time_;
		}
	}
	while(get_current_frame_end_time() < get_animation_time() &&  // catch up
			get_current_frame_end_time() < get_end_time()) {// don't go after the end
		current_frame_key_++;
	}
}

template<typename T,  typename T_void_value>
bool animated<T,T_void_value>::need_update() const
{
	if(does_not_change_) {
		return false;
	}
	if(frames_.empty()) {
		return false;
	}
	if(!started_) {
		return false;
	}
	if(current_ticks >  (int)(get_current_frame_end_time()/acceleration_+start_tick_)){
		return true;
	}
	return false;
}

template<typename T,  typename T_void_value>
bool animated<T,T_void_value>::animation_would_finish() const
{
	if(frames_.empty())
		return true;
	if(!started_)
		return true;
	if(!cycles_ && (((double)(current_ticks - start_tick_)*acceleration_)+starting_frame_time_) > get_end_time())
		return true;

	return false;
}
template<typename T,  typename T_void_value>
bool animated<T,T_void_value>::animation_finished() const
{
	if(frames_.empty())
		return true;
	if(!started_)
		return true;
	if(!cycles_ && (get_animation_time() >  get_end_time()))
		return true;

	return false;
}

template<typename T,  typename T_void_value>
int animated<T,T_void_value>::get_animation_time() const
{
	if(!started_  ) return starting_frame_time_;

	return (int)(((double)(last_update_tick_ - start_tick_)*acceleration_)+starting_frame_time_);
}

template<typename T,  typename T_void_value>
const T& animated<T,T_void_value>::get_current_frame() const
{
	if(frames_.empty() )
		return void_value_;
	return frames_[current_frame_key_].value_;
}

template<typename T,  typename T_void_value>
const T& animated<T,T_void_value>::get_next_frame(int shift) const
{
	if(frames_.empty() )
		return void_value_;
	int next_frame_key = current_frame_key_ + shift;;
	if (!cycles_) {
		if (next_frame_key < 0)
			return get_first_frame();
		else if (next_frame_key >= get_frames_count())
			return get_last_frame();
	}
	return frames_[next_frame_key % get_frames_count()].value_;
}

template<typename T,  typename T_void_value>
const int animated<T,T_void_value>::get_current_frame_begin_time() const
{
	if(frames_.empty() )
		return starting_frame_time_;
	return frames_[current_frame_key_].start_time_;
}

template<typename T,  typename T_void_value>
const int animated<T,T_void_value>::get_current_frame_end_time() const
{
	if(frames_.empty() )
		return starting_frame_time_;
	return get_current_frame_begin_time() +get_current_frame_duration();
}

template<typename T,  typename T_void_value>
const int animated<T,T_void_value>::get_current_frame_duration() const
{
	if(frames_.empty() )
		return 0;
	return frames_[current_frame_key_].duration_;
}

template<typename T,  typename T_void_value>
const int animated<T,T_void_value>::get_current_frame_time() const
{
	if(frames_.empty() )
		return 0;
	return maximum<int>(0,get_animation_time() - get_current_frame_begin_time());
}

template<typename T,  typename T_void_value>
const T& animated<T,T_void_value>::get_first_frame() const
{
	if(frames_.empty() )
		return void_value_;
	return frames_[0].value_;
}

template<typename T,  typename T_void_value>
const T& animated<T,T_void_value>::get_last_frame() const
{
	if(frames_.empty() )
		return void_value_;
	return frames_.back().value_;
}

template<typename T, typename T_void_value>
int animated<T,T_void_value>::get_frames_count() const
{
	return frames_.size();
}

template<typename T,  typename T_void_value>
int animated<T,T_void_value>::get_begin_time() const
{
	return starting_frame_time_;
}

template<typename T,  typename T_void_value>
int animated<T,T_void_value>::get_end_time() const
{
	if(frames_.empty())
		return starting_frame_time_;
	return frames_.back().start_time_ + frames_.back().duration_;
}

// Force compilation of the following template instantiations

#include "unit_frame.hpp"
#include "image.hpp"

template class animated< image::locator >;
template class animated< std::string >;
template class animated< unit_frame >;
