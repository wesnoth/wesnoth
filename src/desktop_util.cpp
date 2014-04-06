/*
   Copyright (C) 2013 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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
#include "serialization/string_utils.hpp"

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

bool open_in_file_manager(const std::string& path)
{
#if defined(_X11) || defined(__APPLE__)

#ifndef __APPLE__
	LOG_DU << "open_in_file_manager(): on X11, will use xdg-open\n";
	const char launcher[] = "xdg-open";
#else
	LOG_DU << "open_in_file_manager(): on OS X, will use open\n";
	const char launcher[] = "open";
#endif

	int child_status = 0;
	const pid_t child = fork();

	if(child == -1) {
		ERR_DU << "open_in_file_manager(): fork() failed\n";
		return false;
	} else if(child == 0) {
		execlp(launcher, launcher, path.c_str(), reinterpret_cast<char*>(NULL));
		_exit(1); // This shouldn't happen.
	} else if(waitpid(child, &child_status, 0) == -1) {
		ERR_DU << "open_in_file_manager(): waitpid() failed\n";
		return false;
	}

	if(child_status) {
		if(WIFEXITED(child_status)) {
			ERR_DU << "open_in_file_manager(): " << launcher << " returned "
			       << WEXITSTATUS(child_status) << '\n';
		} else {
			ERR_DU << "open_in_file_manager(): " << launcher << " failed\n";
		}

		return false;
	}

	return true;

#elif defined(_WIN32)

	LOG_DU << "open_in_file_manager(): on Win32, will use ShellExecute()\n";

	wide_string wpath = utils::string_to_wstring(path);
	wpath.push_back(wchar_t(0)); // Make wpath NULL-terminated

	const ptrdiff_t res = reinterpret_cast<ptrdiff_t>(ShellExecute(NULL, L"open", &wpath.front(), NULL, NULL, SW_SHOW));
	if(res <= 32) {
		ERR_DU << "open_in_file_manager(): ShellExecute() failed (" << res << ")\n";
		return false;
	}

	return true;

#else

	ERR_DU << "open_in_file_manager(): unsupported platform\n";
	return false;

#endif
}

}
