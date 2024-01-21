/*
	Copyright (C) 2015 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <cstring>

#if defined(__APPLE__) || defined(_X11)
#include <sys/utsname.h>
#endif

#if defined(__APPLE__)

#include "apple_version.hpp"
#include "serialization/string_utils.hpp"

#include <map>
#include <boost/algorithm/string/trim.hpp>

#elif defined(_X11)

#include "serialization/string_utils.hpp"

#include <cerrno>
#include <map>
#include <boost/algorithm/string/trim.hpp>

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

/**
 * Tries to find out the Windows 10 release number.
 *
 * This depends on the registry having special information in it. This may or
 * may not break in the future or be faked by the application compatibility
 * layer. Take with a grain of salt.
 */
std::string windows_release_id()
{
	char buf[256]{""};
	DWORD size = sizeof(buf);

	auto res = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "DisplayVersion", RRF_RT_REG_SZ, nullptr, buf, &size);
	if(res != ERROR_SUCCESS) {
		res = RegGetValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ReleaseId", RRF_RT_REG_SZ, nullptr, buf, &size);
	}

	return std::string{res == ERROR_SUCCESS ? buf : ""};
}

std::string windows_runtime_arch()
{
	SYSTEM_INFO si;
	SecureZeroMemory(&si, sizeof(SYSTEM_INFO));
	GetNativeSystemInfo(&si);

	switch(si.wProcessorArchitecture) {
		case PROCESSOR_ARCHITECTURE_INTEL:
			return "x86";
		case PROCESSOR_ARCHITECTURE_AMD64:
			return "x86_64";
		case PROCESSOR_ARCHITECTURE_ARM:
			return "arm";
		case PROCESSOR_ARCHITECTURE_ARM64:
			return "arm64";
		case PROCESSOR_ARCHITECTURE_IA64:
			return "ia64";
		default:
			return _("cpu_architecture^<unknown>");
	}
}

#endif

#if defined(_X11)
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

std::map<std::string, std::string> parse_fdo_osrelease(const std::string& path)
{
	auto in = filesystem::istream_file(path);
	if(!in->good()) {
		return {};
	}

	std::map<std::string, std::string> res;

	// NOTE:  intentionally basic "parsing" here. We are not supposed to see
	//        more complex shell syntax anyway.
	//        <https://www.freedesktop.org/software/systemd/man/os-release.html>
	for(std::string s; std::getline(*in, s);) {
		if(s.empty() || s.front() == '#') {
			continue;
		}

		auto eqsign_pos = s.find('=');
		if(!eqsign_pos || eqsign_pos == std::string::npos) {
			continue;
		}

		auto lhs = s.substr(0, eqsign_pos),
			 rhs = eqsign_pos + 1 < s.length() ? utils::unescape(s.substr(eqsign_pos + 1)) : "";

		boost::algorithm::trim(lhs);
		boost::algorithm::trim(rhs);

		// Unquote if the quotes match on both sides
		if(rhs.length() >= 2 && rhs.front() == '"' && rhs.back() == '"') {
			rhs.pop_back();
			rhs.erase(0, 1);
		}

		res.emplace(std::move(lhs), std::move(rhs));
	}

	return res;
}

#endif

} // end anonymous namespace

std::string os_version()
{
#if defined(__APPLE__) || defined(_X11)
	// Some systems, e.g. SunOS, need "struct" here
	struct utsname u;

	if(uname(&u) != 0) {
		ERR_DU << "os_version: uname error (" << strerror(errno) << ")";
	}
#endif

#if defined(__APPLE__)

	//
	// Standard Mac OS X version
	//

	return desktop::apple::os_version() + " " + u.machine;

#elif defined(_X11)

	//
	// systemd/freedesktop.org method.
	//

	std::map<std::string, std::string> osrel;

	static const std::string fdo_osrel_etc = "/etc/os-release";
	static const std::string fdo_osrel_usr = "/usr/lib/os-release";

	if(filesystem::file_exists(fdo_osrel_etc)) {
		osrel = parse_fdo_osrelease(fdo_osrel_etc);
	} else if(filesystem::file_exists(fdo_osrel_usr)) {
		osrel = parse_fdo_osrelease(fdo_osrel_usr);
	}

	// Check both existence and emptiness in case some vendor sets PRETTY_NAME=""
	auto osrel_distname = osrel["PRETTY_NAME"];
	if(osrel_distname.empty()) {
		osrel_distname = osrel["NAME"];
	}

	if(!osrel_distname.empty()) {
		return osrel_distname + " " + u.machine;
	}

	//
	// Linux Standard Base fallback.
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
			return ver + " " + u.machine;
		}
	}

	//
	// POSIX uname version fallback.
	//

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

	OSVERSIONINFOEX v;

	SecureZeroMemory(&v, sizeof(OSVERSIONINFOEX));
	v.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

#ifdef _MSC_VER
// GetVersionEx is rather problematic, but it works for our usecase.
// See https://msdn.microsoft.com/en-us/library/windows/desktop/ms724451(v=vs.85).aspx
// for more info.
#pragma warning(push)
#pragma warning(disable:4996)
#endif
	if(!GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&v))) {
		ERR_DU << "os_version: GetVersionEx error (" << GetLastError() << ')';
		return base;
	}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
				version = v.dwBuildNumber < 22000 ? "10" : "11";
				const auto& release_id = windows_release_id();
				if(!release_id.empty()) {
					version += ' ';
					version += release_id;
				}
				break;
			} // else fallback to default
			[[fallthrough]];
		default:
			if(v.wProductType != VER_NT_WORKSTATION) {
				version = "Server";
			}
	}

	if(*v.szCSDVersion) {
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

	return base + " " + version + " " + windows_runtime_arch();

#else

	//
	// "I don't know where I am" version.
	//

	ERR_DU << "os_version(): unsupported platform";
	return _("operating_system^<unknown>");

#endif
}

} // end namespace desktop
