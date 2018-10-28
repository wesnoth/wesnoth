/*
   Copyright (C) 2009 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/core/timer.hpp"

#include "events.hpp"
#include "gui/core/log.hpp"

#include <SDL_timer.h>

#include <unordered_map>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace gui2
{

struct timer
{
	timer() : sdl_id(0), interval(0), callback()
	{
	}

	SDL_TimerID sdl_id;
	uint32_t interval;
	std::function<void(size_t id)> callback;
};

/** Ids for the timers. */
static size_t next_timer_id = 0;

/** The active timers. */
static std::unordered_map<size_t, timer>& get_timers()
{
	static auto ptimers = new std::unordered_map<size_t, timer>();
	return *ptimers;
}

// R/W lock can be converted from Boost to std in C++17.
boost::shared_mutex timers_mutex;

typedef boost::shared_lock<boost::shared_mutex> read_lock_guard;
typedef boost::unique_lock<boost::shared_mutex> write_lock_guard;


extern "C" {

static uint32_t timer_callback(uint32_t, void* id)
{
	DBG_GUI_E << "Pushing timer event in queue.\n";

	Uint32 result;
	{
		// iTunes reported several crashes here, when it was a lock_guard<mutex>. 
		// This means timer_callback is called from a thread that already holds timers_mutex. How can this be?..
		read_lock_guard lock(timers_mutex);

		auto itor = get_timers().find(reinterpret_cast<size_t>(id));
		if(itor == get_timers().end()) {
			return 0;
		}
		result = itor->second.interval;
	}

	SDL_Event event;

	event.type = TIMER_EVENT;
	event.user.code = 0;
	event.user.data1 = id;
	event.user.data2 = nullptr;

	SDL_PushEvent(&event);

	return result;
}

} // extern "C"

static size_t get_next_timer_id()
{
	size_t next_id;
	write_lock_guard lock(timers_mutex);

	do {
		next_id = ++next_timer_id;
	} while(next_id == 0 || get_timers().count(next_id) > 0);

	return next_id;
}

size_t add_timer(const uint32_t interval,
				 const std::function<void(size_t id)>& callback,
				 const bool repeat)
{
	static_assert(sizeof(size_t) == sizeof(void*), "Pointer and size_t are not the same size");

	DBG_GUI_E << "Adding timer.\n";

	timer timer;
	size_t next_id = get_next_timer_id();

	timer.sdl_id = SDL_AddTimer(
			interval, timer_callback, reinterpret_cast<void*>(next_id));

	if(timer.sdl_id == 0) {
		WRN_GUI_E << "Failed to create an sdl timer." << std::endl;
		return 0;
	}

	if(repeat) {
		timer.interval = interval;
	}

	timer.callback = callback;

	{
		write_lock_guard lock(timers_mutex);

		get_timers().emplace(next_id, timer);
	}

	DBG_GUI_E << "Added timer " << next_id << ".\n";
	return next_id;
}

bool remove_timer(const size_t id)
{
	DBG_GUI_E << "Removing timer " << id << ".\n";

	SDL_TimerID sdl_id;
	{
		write_lock_guard lock(timers_mutex);

		auto itor = get_timers().find(id);
		if(itor == get_timers().end()) {
			LOG_GUI_E << "Can't remove timer since it no longer exists.\n";
			return false;
		}

		sdl_id = itor->second.sdl_id;

		get_timers().erase(itor);
	}

	if(!SDL_RemoveTimer(sdl_id)) {
		/*
		 * This can happen if the caller of the timer didn't get the event yet
		 * but the timer has already been fired. This due to the fact that a
		 * timer pushes an event in the queue, which allows the following
		 * condition:
		 * - Timer fires
		 * - Push event in queue
		 * - Another event is processed and tries to remove the event.
		 */
		DBG_GUI_E << "The timer is already out of the SDL timer list.\n";
	}
	return true;
}

bool execute_timer(const size_t id)
{
	DBG_GUI_E << "Executing timer " << id << ".\n";

	std::function<void(size_t)> callback = nullptr;
	{
		write_lock_guard lock(timers_mutex);

		auto itor = get_timers().find(id);
		if(itor == get_timers().end()) {
			LOG_GUI_E << "Can't execute timer since it no longer exists.\n";
			return false;
		}

		callback = itor->second.callback;

		if(itor->second.interval == 0) {
			get_timers().erase(itor);
		}
	}

	callback(id);

	return true;
}

} // namespace gui2
