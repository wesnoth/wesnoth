/*
   Copyright (C) 2014 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "desktop/windows_console.hpp"

#include "log.hpp"

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 // XP and later

#include <windows.h>

#include <cstdio>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_desktop("desktop");
#define ERR_DU LOG_STREAM(err, log_desktop)
#define LOG_DU LOG_STREAM(info, log_desktop)

namespace {

class win32_console_manager
{
public:
	win32_console_manager()
	{
#if 0
		// Because this runs before cmdline processing, enable this block for
		// debugging purposes if necessary.
		lg::set_log_domain_severity("desktop", lg::debug);
#endif
		if(AttachConsole(ATTACH_PARENT_PROCESS)) {
			LOG_DU << "win32_console: attached to parent process console\n";
		} else if(AllocConsole()) {
			LOG_DU << "win32_console: attached to own console\n";
		} else {
			ERR_DU << "win32_console: failed to attach or allocate console!";
			return;
		}

		LOG_DU << "win32_console: stdin to console\n";
		freopen("CONIN$",  "rb", stdin);
		LOG_DU << "win32_console: stdout to console\n";
		std::cout.flush();
		freopen("CONOUT$", "wb", stdout);
		LOG_DU << "win32_console: stderr to console\n";
		std::cerr.flush();
		freopen("CONOUT$", "wb", stderr);

		LOG_DU << "win32_console: init complete\n";
	}

	~win32_console_manager()
	{
		FreeConsole();

		LOG_DU << "win32_console: uninit complete\n";
	}
};

boost::scoped_ptr<win32_console_manager> conman;

} // end anonymous namespace

namespace desktop {

void enable_win32_console()
{
	if(!conman) {
		conman.reset(new win32_console_manager());
	} else {
		ERR_DU << "win32_console: Console already enabled!\n";
	}
}

void disable_win32_console()
{
	conman.reset(NULL);
}

bool is_win32_console_enabled()
{
	return conman != NULL;
}

} // end namespace desktop
