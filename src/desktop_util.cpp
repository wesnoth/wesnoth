/*
   Copyright (C) 2013 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "desktop_util.hpp"

#include "log.hpp"
#include "serialization/unicode.hpp"

#if defined(_X11) || defined(__APPLE__)

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> // fork(), exec family

#elif defined(_WIN32)

#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h> // ShellExecute()

#endif

static lg::log_domain log_desktop("desktop");
#define ERR_DU LOG_STREAM(err, log_desktop)
#define LOG_DU LOG_STREAM(info, log_desktop)

namespace desktop {

bool open_object_is_supported()
{
#if defined(_X11) || defined(__APPLE__) || defined(_WIN32)
	return true;
#else
	return false;
#endif
}

bool open_object(const std::string& path_or_url)
{
#if defined(_X11) || defined(__APPLE__)

#ifndef __APPLE__
	LOG_DU << "open_object(): on X11, will use xdg-open\n";
	const char launcher[] = "xdg-open";
#else
	LOG_DU << "open_object(): on OS X, will use open\n";
	const char launcher[] = "open";
#endif

	int child_status = 0;
	const pid_t child = fork();

	if(child == -1) {
		ERR_DU << "open_object(): fork() failed\n";
		return false;
	} else if(child == 0) {
		execlp(launcher, launcher, path_or_url.c_str(), reinterpret_cast<char*>(NULL));
		_exit(1); // This shouldn't happen.
	} else if(waitpid(child, &child_status, 0) == -1) {
		ERR_DU << "open_object(): waitpid() failed\n";
		return false;
	}

	if(child_status) {
		if(WIFEXITED(child_status)) {
			ERR_DU << "open_object(): " << launcher << " returned "
			       << WEXITSTATUS(child_status) << '\n';
		} else {
			ERR_DU << "open_object(): " << launcher << " failed\n";
		}

		return false;
	}

	return true;

#elif defined(_WIN32)

	LOG_DU << "open_object(): on Win32, will use ShellExecute()\n";

	utf16::string u16path = unicode_cast<utf16::string>(path_or_url);
	u16path.push_back(wchar_t(0)); // Make wpath NULL-terminated

	const ptrdiff_t res = reinterpret_cast<ptrdiff_t>(ShellExecute(NULL, L"open", &u16path.front(), NULL, NULL, SW_SHOW));
	if(res <= 32) {
		ERR_DU << "open_object(): ShellExecute() failed (" << res << ")\n";
		return false;
	}

	return true;

#else

	ERR_DU << "open_object(): unsupported platform\n";
	return false;

#endif
}

}
