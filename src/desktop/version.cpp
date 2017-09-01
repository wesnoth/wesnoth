/*
   Copyright (C) 2015 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "filesystem.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/unicode.hpp"
#ifdef __APPLE__
#include "../version.hpp"
#endif

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

namespace
{

#ifdef _WIN32
/**
 * Detects whether we are running on Wine or not.
 *
 * This is for informational purposes only and all Windows code should assume
 * we are running on the real thing instead.
 */
bool on_wine()
{
	HMODULE ntdll = GetModuleHandle(L"ntdll.dll");
	if(!ntdll) {
		return false;
	}

	return GetProcAddress(ntdll, "wine_get_version") != nullptr;
}
#endif

#if defined(_X11) || defined(__APPLE__)
/**
 * Release policy for POSIX pipe streams opened with popen(3).
 */
struct posix_pipe_release_policy
{
	void operator()(std::FILE* f) const { if(f != nullptr) { pclose(f); } }
};

/**
 * Scoped POSIX pipe stream.
 *
 * The stream object type is the same as a regular file stream, but the release
 * policy is different, as required by popen(3).
 */
typedef std::unique_ptr<std::FILE, posix_pipe_release_policy> scoped_posix_pipe;

/**
 * Read a single line from the specified pipe.
 *
 * @returns An empty string if the pipe is invalid or nothing could be read.
 */
std::string read_pipe_line(scoped_posix_pipe& p)
{
	if(!p.get()) {
		return "";
	}

	std::string ver;
	int c;

	ver.reserve(64);

	// We only want the first line.
	while((c = std::fgetc(p.get())) && c != EOF && c != '\n' && c != '\r') {
		ver.push_back(static_cast<char>(c));
	}

	return ver;
}
#endif

} // end anonymous namespace

std::string os_version()
{
#if defined(_X11) || defined(__APPLE__)

#ifdef __APPLE__

	//
	// Standard Mac OS X version
	//

	static const std::string version_plist = "/System/Library/CoreServices/SystemVersion.plist";
	static const std::string defaults_bin = "/usr/bin/defaults";

	if(filesystem::file_exists(defaults_bin) && filesystem::file_exists(version_plist)) {
		static const std::string cmdline
				= defaults_bin + " read " + version_plist + " ProductUserVisibleVersion";

		scoped_posix_pipe p(popen(cmdline.c_str(), "r"));
		const std::string& ver = read_pipe_line(p);

		if(!ver.empty()) {
			const version_info version(ver);
			if (version.minor_version() < 12) {
				return "Apple OS X " + ver;
			} else {
				return "Apple macOS " + ver;
			}
 		}
	}

#else

	//
	// Linux Standard Base version.
	//

	static const std::string lsb_release_bin = "/usr/bin/lsb_release";

	if(filesystem::file_exists(lsb_release_bin)) {
		static const std::string cmdline = lsb_release_bin + " -s -d";

		scoped_posix_pipe p(popen(cmdline.c_str(), "r"));
		std::string ver = read_pipe_line(p);

		if(ver.length() >= 2 && ver[0] == '"' && ver[ver.length() - 1] == '"') {
			ver.erase(ver.length() - 1, 1);
			ver.erase(0, 1);
		}

		// Check this again in case we got "" above for some weird reason.
		if(!ver.empty()) {
			return ver;
		}
	}
#endif

	//
	// POSIX uname version.
	//

	utsname u;

	if(uname(&u) != 0) {
		ERR_DU << "os_version: uname error (" << strerror(errno) << ")\n";
	}

	return formatter() << u.sysname << ' '
						<< u.release << ' '
						<< u.version << ' '
						<< u.machine;

#elif defined(_WIN32)

	//
	// Windows version.
	//

	static const std::string base
			= !on_wine() ? "Microsoft Windows" : "Wine/Microsoft Windows";

	OSVERSIONINFOEX v { sizeof(OSVERSIONINFOEX) };

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
			if(v.wProductType == VER_NT_WORKSTATION) {
				version = "8";
			} else {
				version = "Server 2012";
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
	version += formatter()
			<< v.dwMajorVersion << '.'
			<< v.dwMinorVersion << '.'
			<< v.dwBuildNumber;
	version += ")";

	return base + " " + version;

#else

	//
	// "I don't know where I am" version.
	//

	ERR_DU << "os_version(): unsupported platform\n";
	return _("operating_system^<unknown>");

#endif
}

} // end namespace desktop

