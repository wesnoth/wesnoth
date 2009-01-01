/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "time.hpp"
#include "global.hpp"
#include "SDL.h"

namespace ntime {

	source source::time_source_;

	source::source() : frame_time_(20), 
			current_time_(SDL_GetTicks()),
			index_(0), 
			mode_(REAL_TIME)
	{
		for (int i = 0; i < frames_to_remember; ++i)
		{
			time_[i] = 0;
		}
	}

	int source::next_index()
	{
		return (index_ + 1) % frames_to_remember;
	}

	size_t source::start_frame(const bool limit)
	{
		size_t previus_time = current_time_;
		size_t current_time = SDL_GetTicks();
		size_t frame_used = current_time - previus_time;
		if (limit && frame_time_ > frame_used)
		{
			// use delay to wait so we don't take all cpu
			size_t wait_time = frame_time_ - frame_used;
			SDL_Delay(wait_time);
			current_time += wait_time;
		}


		// Advance index only in begin of frame
		index_ = next_index();
		time_[index_] = current_time;
		if (mode_ == REAL_TIME)
		{
			current_time_ = current_time;
		}
		else
		{
			// Check if we have allready enought frames
			// for smooth time calculation
			if (time_[next_index()]) 
			{
				// smooth calcuations uses rounding to
				// keep time as near of real time as possible
				// @todo There should be error correction
				//		Maybe should use 3 bits shifted values 
				
				size_t average = ((current_time - time_[next_index()]))/((frames_to_remember - 1));


				current_time_ += average;
			}
			else
			{
				current_time_ = current_time;
			}
		}
		return frame_used;
	}

	void source::set_frame_rate(const size_t fps)
	{
		frame_time_ = 1000/fps;
	}

	void source::set_frame_time(const size_t ms)
	{
		frame_time_ = ms;
	}

	size_t source::get_time() const
	{
		return current_time_;
	}

	void source::set_time_mode(const time_mode& mode)
	{
		mode_ = mode;
	}

	source::time_mode source::get_time_mode() const
	{
		return mode_;
	}

	source& source::get_source()
	{
		return source::time_source_;
	}



}

