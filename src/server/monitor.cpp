/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! Here is the implementation for server monitoring thread
//! Initial target is to have it useful in posix systems
//! and later add support for windows if possible.
//! If a system doesn't support monitoring it is not compiled.

#include "monitor.hpp"

#include "../util.hpp"

#include <iostream>

#include <glibtop.h>
#include <glibtop/proctime.h>

namespace nserver {

#ifdef SERVER_MONITOR


int monitor_thread_start(void *data)
{
	server_monitor &monitor = *(static_cast<server_monitor*>(data));
	size_t start_time = SDL_GetTicks();
	size_t delta;
	while(true)
	{
		monitor.check();
		if (monitor.is_ending())
		{
			break;
		}
		delta = minimum<size_t>(monitor.get_delay(), SDL_GetTicks() - start_time);
		SDL_Delay(monitor.get_delay()-delta);
		start_time = SDL_GetTicks();

	}
	return 0;
}

void server_monitor::check()
{
	size_t pid = getpid();
	glibtop_proc_time cpu;
	glibtop_get_proc_time(&cpu,pid);
	std::cout << "cpu time: real: " << cpu.rtime 
		<< " user: " << cpu.utime 
		<< " kernel: " << cpu.stime
		<< " frequency: " << cpu.frequency
		<< "\n";
}

#endif


server_monitor::server_monitor(size_t delay)
#ifdef SERVER_MONITOR
	: ending_(false), 
	delay_(delay),
	mutex_(new threading::mutex()),
	thread_(new threading::thread(&monitor_thread_start, static_cast<void*>(this)))
#endif
{
#ifdef SERVER_MONITOR
	glibtop_init();
#endif
}

server_monitor::~server_monitor()
{
#ifdef SERVER_MONITOR
	// atomic operation doesn't need locking :)
	ending_ = true;
	// Wait for thread to quit
	delete thread_;
	delete mutex_;
	glibtop_close();
#endif
}

}
