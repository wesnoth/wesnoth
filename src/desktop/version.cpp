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
#include "serialization/unicode.hpp"

#include <cstring>

#if defined(_X11) || defined(__APPLE__)

#include <cerrno>
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
	static const std::string base = "Microsoft Windows";

	OSVERSIONINFOEX v = { sizeof(OSVERSIONINFOEX) };

	if(!GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&v))) {
		ERR_DU << "os_version: GetVersionEx error ("
			   << GetLastError() << ")\n";
		return base;
	}

	const DWORD vnum = v.dwMajorVersion * 100 + v.dwMinorVersion;
	std::string version;

	switch(vnum)
	{
		case 500:
			version = "2000";
			break;
		case 501:
			version = "XP";
			break;
		case 502:
			// This will misidentify XP x64 but who really cares?
			version = "Server 2003";
			break;
		case 600:
			if(v.wProductType == VER_NT_WORKSTATION) {
				version = "Vista";
			} else {
				version = "Server 2008";
			}
			break;
		case 601:
			if(v.wProductType == VER_NT_WORKSTATION) {
				version = "7";
			} else {
				version = "Server 2008 R2";
			}
			break;
		case 602:
			// FIXME:
			// From Windows 8.1 onwards, applications must specifically declare
			// compatibility with the current version to get the real version
			// numbers, according to MSDN. So, until we get that built into the
			// executable's resources...
			if(v.wProductType == VER_NT_WORKSTATION) {
				version = "8 " + _("windows_version^(or later)");
			} else {
				version = "Server 2012 " + _("windows_version^(or later)");
			}
			break;
		case 603:
			if(v.wProductType == VER_NT_WORKSTATION) {
				version = "8.1";
			} else {
				version = "Server 2012 R2";
			}
			break;
		case 1000:
			if(v.wProductType == VER_NT_WORKSTATION) {
				version = "10";
				break;
			} // else fallback to default
		default:
			if(v.wProductType != VER_NT_WORKSTATION) {
				version = "Server";
			}
	}

	if(v.szCSDVersion && *v.szCSDVersion) {
		version += " ";
		version += unicode_cast<std::string>(std::wstring(v.szCSDVersion));
	}

	version += " (";
	// Add internal version numbers.
	version += (formatter()
			<< v.dwMajorVersion << '.'
			<< v.dwMinorVersion << '.'
			<< v.dwBuildNumber).str();
	version += ")";

	return base + " " + version;

#else

	ERR_DU << "os_version(): unsupported platform\n";
	return _("operating_system^<unknown>");

#endif
}

} // end namespace desktop

