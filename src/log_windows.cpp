/*
	Copyright (C) 2014 - 2025
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

// For some reason, it became necessary to include this before the header
// after switching to c++11
#include <cstdio>

#include "log_windows.hpp"

#include "filesystem.hpp"
#include "log.hpp"
#include "serialization/unicode.hpp"

#include <iomanip>
#include <iostream>

#include <boost/algorithm/string/predicate.hpp>

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

static lg::log_domain log_setup("logsetup");
#define ERR_LS LOG_STREAM(err,   log_setup)
#define WRN_LS LOG_STREAM(warn,  log_setup)
#define LOG_LS LOG_STREAM(info,  log_setup)
#define DBG_LS LOG_STREAM(debug, log_setup)

namespace lg
{

namespace
{
/**
 * Singleton class that handles sending logging output to a console on windows.
 */
class console_handler
{
public:
	console_handler(const console_handler&) = delete;
	console_handler& operator=(const console_handler&) = delete;

	console_handler();

	/**
	 * Returns whether we own the console we are attached to, if any.
	 */
	bool owns_console() const;

private:
	bool created_wincon_;

	/**
	 * Switches to using a native console.
	 */
	void enable_native_console_output();
};

console_handler::console_handler()
	: created_wincon_(false)
{
	DBG_LS << "Early init message";

	if(GetConsoleWindow() != nullptr) {
		// Someone already attached a console to us. Assume we were compiled
		// with the console subsystem flag and that the standard streams are
		// already pointing to the console.
		LOG_LS << "Console already attached at startup (built with console subsystem flag?), log file disabled.";
	} else {
		enable_native_console_output();
	}

	DBG_LS << "Windows console init complete!";
}

bool console_handler::owns_console() const
{
	return created_wincon_;
}

void console_handler::enable_native_console_output()
{
	if(AttachConsole(ATTACH_PARENT_PROCESS)) {
		LOG_LS << "Attached parent process console.";
	} else if(AllocConsole()) {
		LOG_LS << "Allocated own console.";
		created_wincon_ = true;
	} else {
		// Wine as of version 4.21 just goes ERROR_ACCESS_DENIED when trying
		// to allocate a console for a GUI subsystem application. We can ignore
		// this since the user purportedly knows what they're doing and if they
		// get radio silence from Wesnoth and no log files they'll realize that
		// something went wrong.
		WRN_LS << "Cannot attach or allocate a console, continuing anyway (is this Wine?)";
	}

	DBG_LS << "stderr to console";
	fflush(stderr);
	std::cerr.flush();
	assert(freopen("CONOUT$", "wb", stderr) == stderr);

	DBG_LS << "stdout to console";
	fflush(stdout);
	std::cout.flush();
	assert(freopen("CONOUT$", "wb", stdout) == stdout);

	DBG_LS << "stdin from console";
	assert(freopen("CONIN$",  "rb", stdin) == stdin);

	LOG_LS << "Console streams handover complete!";
}

std::unique_ptr<console_handler> lfm;

} // end anonymous namespace

void do_console_redirect()
{
	if(!lfm) {
		lfm.reset(new console_handler());
	}
}

bool using_own_console()
{
	return lfm && lfm->owns_console();
}

} // end namespace lg
