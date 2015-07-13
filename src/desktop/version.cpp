/*
   Copyright (C) 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "desktop/version.hpp"

#include "formatter.hpp"
#include "gettext.hpp"
#include "log.hpp"

#if defined(_X11) || defined(__APPLE__)

#include <cerrno>
#include <cstring>

#include <sys/utsname.h>

#endif

#ifdef _WIN32

#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#endif

static lg::log_domain log_desktop("desktop");
#define ERR_DU LOG_STREAM(err, log_desktop)
#define LOG_DU LOG_STREAM(info, log_desktop)

namespace desktop
{

std::string os_version()
{
#if defined(_X11) || defined(__APPLE__)

	utsname u;

	if(uname(&u) != 0) {
		ERR_DU << "os_version: uname error (" << strerror(errno) << ")\n";
	}

	return (formatter() << u.sysname << ' '
						<< u.release << ' '
						<< u.version << ' '
						<< u.machine).str();

#elif defined(_WIN32)

	ERR_DU << "os_version: STUB!\n";
	return "Microsoft Windows";

#else

	ERR_DU << "os_version(): unsupported platform\n";
	return _("operating_system^<unknown>");

#endif
}

} // end namespace desktop

