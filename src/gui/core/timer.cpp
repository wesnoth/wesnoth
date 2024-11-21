/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include <SDL2/SDL_timer.h>

#include <map>
#include <mutex>

namespace gui2
{

struct timer
{
	SDL_TimerID sdl_id{0};
	std::chrono::milliseconds interval{0};
	std::function<void(std::size_t id)> callback{};
};

/** Ids for the timers. */
static std::size_t next_timer_id = 0;

/** The active timers. */
static std::map<std::size_t, timer>& get_timers()
{
	static std::map<std::size_t, timer>* ptimers = new std::map<std::size_t, timer>();
	return *ptimers;
}
/**
	The id of the event being executed, 0 if none.
	NOTE: it is possible that multiple timers are executed at the same time
	      if one of the timer starts an event loop for example if its handler
		  shows a dialog. In that case code that relies on this breaks. This
		  could probably fixed my making this a list/stack of ids.
*/
static std::size_t executing_id = 0;

std::mutex timers_mutex;

/** Did somebody try to remove the timer during its execution? */
static bool executing_id_removed = false;

/**
 * Helper to make removing a timer in a callback safe.
 *
 * Upon creation it sets the executing id and clears the remove request flag.
 *
 * If an remove_timer() is called for the id being executed it requests a
 * remove the timer and exits remove_timer().
 *
 * Upon destruction it tests whether there was a request to remove the id and
 * does so. It also clears the executing id. It leaves the remove request flag
 * since the execution function needs to know whether or not the event was
 * removed.
 */
class executor
{
public:
	executor(std::size_t id)
	{
		executing_id = id;
		executing_id_removed = false;
	}

	~executor()
	{
		const std::size_t id = executing_id;
		executing_id = 0;
		if(executing_id_removed) {
			remove_timer(id);
		}
	}
};

extern "C" {

static uint32_t timer_callback(uint32_t, void* id)
{
	DBG_GUI_E << "Pushing timer event in queue.";
	// iTunes still reports a couple of crashes here. Cannot see a problem yet.

	uint32_t result;
	{
		std::scoped_lock lock(timers_mutex);

		auto itor = get_timers().find(reinterpret_cast<std::size_t>(id));
		if(itor == get_timers().end()) {
			return 0;
		}
		result = itor->second.interval.count();
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

std::size_t add_timer(const std::chrono::milliseconds& interval,
				 const std::function<void(std::size_t id)>& callback,
				 const bool repeat)
{
	static_assert(sizeof(std::size_t) == sizeof(void*), "Pointer and std::size_t are not the same size");

	DBG_GUI_E << "Adding timer.";

	timer timer;
	{
		std::scoped_lock lock(timers_mutex);

		do {
			++next_timer_id;
		} while(next_timer_id == 0 || get_timers().count(next_timer_id) > 0);

		timer.sdl_id = SDL_AddTimer(
				interval.count(), timer_callback, reinterpret_cast<void*>(next_timer_id));
	}

	if(timer.sdl_id == 0) {
		WRN_GUI_E << "Failed to create an sdl timer.";
		return 0;
	}

	if(repeat) {
		timer.interval = interval;
	}

	timer.callback = callback;

	{
		std::scoped_lock lock(timers_mutex);

		get_timers().emplace(next_timer_id, timer);
	}

	DBG_GUI_E << "Added timer " << next_timer_id << ".";
	return next_timer_id;
}

bool remove_timer(const std::size_t id)
{
	DBG_GUI_E << "Removing timer " << id << ".";

	std::scoped_lock lock(timers_mutex);

	auto itor = get_timers().find(id);
	if(itor == get_timers().end()) {
		LOG_GUI_E << "Can't remove timer since it no longer exists.";
		return false;
	}

	if(id == executing_id) {
		executing_id_removed = true;
		return true;
	}

	if(!SDL_RemoveTimer(itor->second.sdl_id)) {
		/*
		 * This can happen if the caller of the timer didn't get the event yet
		 * but the timer has already been fired. This due to the fact that a
		 * timer pushes an event in the queue, which allows the following
		 * condition:
		 * - Timer fires
		 * - Push event in queue
		 * - Another event is processed and tries to remove the event.
		 */
		DBG_GUI_E << "The timer is already out of the SDL timer list.";
	}
	get_timers().erase(itor);
	return true;
}

bool execute_timer(const std::size_t id)
{
	DBG_GUI_E << "Executing timer " << id << ".";

	std::function<void(size_t)> callback = nullptr;
	{
		std::scoped_lock lock(timers_mutex);

		auto itor = get_timers().find(id);
		if(itor == get_timers().end()) {
			LOG_GUI_E << "Can't execute timer since it no longer exists.";
			return false;
		}

		callback = itor->second.callback;

		if(itor->second.interval == std::chrono::milliseconds{0}) {
			get_timers().erase(itor);
		}
	}

	callback(id);

	return true;
}

} // namespace gui2
