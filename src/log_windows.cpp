/*
	Copyright (C) 2014 - 2022
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

#include <ctime>
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
 * Singleton class that deals with the intricacies of log file redirection.
 */
class log_file_manager
{
public:
	log_file_manager(const log_file_manager&) = delete;
	log_file_manager& operator=(const log_file_manager&) = delete;

	log_file_manager(bool native_console);

	/**
	 * Returns whether we own the console we are attached to, if any.
	 */
	bool owns_console() const;

private:
	bool created_wincon_;

	/**
	 * Switches to using a native console instead of log file redirection.
	 *
	 * This is an irreversible operation right now. This might change later if
	 * someone deems it useful.
	 */
	void enable_native_console_output();
};

log_file_manager::log_file_manager(bool native_console)
	: created_wincon_(false)
{
	DBG_LS << "Early init message";

	if(GetConsoleWindow() != nullptr) {
		// Someone already attached a console to us. Assume we were compiled
		// with the console subsystem flag and that the standard streams are
		// already pointing to the console.
		LOG_LS << "Console already attached at startup (built with console subsystem flag?), log file disabled.";
	} else if(native_console) {
		enable_native_console_output();
	}

	DBG_LS << "Windows console init complete!";
}

bool log_file_manager::owns_console() const
{
	return created_wincon_;
}

void log_file_manager::enable_native_console_output()
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

	// At this point the log file has been closed and it's no longer our
	// responsibility to clean up anything; Windows will figure out what to do
	// when the time comes for the process to exit.
	lg::set_log_file_path("");

	LOG_LS << "Console streams handover complete!";
}

std::unique_ptr<log_file_manager> lfm;

} // end anonymous namespace

void do_console_redirect(bool native_console)
{
	if(!lfm) {
		lfm.reset(new log_file_manager(native_console));
	}
}

bool using_own_console()
{
	return lfm && lfm->owns_console();
}

} // end namespace lg
