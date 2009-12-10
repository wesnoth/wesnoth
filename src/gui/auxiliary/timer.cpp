/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/auxiliary/timer.hpp"

#include "events.hpp"
#include "gui/auxiliary/log.hpp"

#include <boost/static_assert.hpp>

#include <SDL_timer.h>

#include <map>

namespace gui2 {

struct ttimer
{
	ttimer()
		: sdl_id(NULL)
		, interval(0)
		, callback()
	{
	}

	SDL_TimerID sdl_id;
	Uint32 interval;
	boost::function<void(unsigned long id)> callback;
};

	/** Ids for the timers. */
	static unsigned long id = 0;

	/** The active timers. */
	static std::map<unsigned long, ttimer> timers;

static Uint32 timer_callback(Uint32, void* id)
{
	DBG_GUI_E << "Pushing timer event in queue.\n";

	std::map<unsigned long, ttimer>::iterator itor =
			timers.find(reinterpret_cast<unsigned long>(id));
	if(itor == timers.end()) {
		return 0;
	}

	SDL_Event event;
	SDL_UserEvent data;

	data.type = TIMER_EVENT;
	data.code = 0;
	data.data1 = id;
	data.data2 = NULL;

	event.type = TIMER_EVENT;
	event.user = data;

	SDL_PushEvent(&event);

	return itor->second.interval;
}

unsigned long
add_timer(const Uint32 interval
		, const boost::function<void(unsigned long id)>& callback
		, const bool repeat)
{
	BOOST_STATIC_ASSERT(sizeof(unsigned long) == sizeof(void*));

	DBG_GUI_E << "Adding timer.\n";

	do {
		++id;
	} while(id == 0 || timers.find(id) != timers.end());

	ttimer timer;
	timer.sdl_id = SDL_AddTimer(
			interval, timer_callback, reinterpret_cast<void*>(id));
	if(timer.sdl_id == NULL) {
		WRN_GUI_E << "Failed to create an sdl timer.\n";
		return 0;
	}

	if(repeat) {
		timer.interval = interval;
	}

	timer.callback = callback;

	timers.insert(std::make_pair(id, timer));

	DBG_GUI_E << "Added timer " << id << ".\n";
	return id;
}

bool
remove_timer(const unsigned long id)
{
	DBG_GUI_E << "Removing timer " << id << ".\n";

	std::map<unsigned long, ttimer>::iterator itor = timers.find(id);
	if(itor == timers.end()) {
		WRN_GUI_E << "Can't remove timer since it no longer exists.\n";
		return false;
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
		DBG_GUI_E << "The timer is already out of the SDL timer list.\n";
	}
	timers.erase(itor);
	return true;
}

bool
execute_timer(const unsigned long id)
{
	DBG_GUI_E << "Executing timer " << id << ".\n";

	std::map<unsigned long, ttimer>::iterator itor = timers.find(id);
	if(itor == timers.end()) {
		WRN_GUI_E << "Can't execute timer since it no longer exists.\n";
		return false;
	}

	itor->second.callback(id);
	if(itor->second.interval == 0) {
		timers.erase(itor);
	}
	return true;
}

} //namespace gui2

