/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Pauli Nieminen <paniemin@cc.hut.fi>
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
			current_time_(SDL_GetTicks())
	{
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
		current_time_ = current_time;
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

	source& source::get_source()
	{
		return source::time_source_;
	}



}

