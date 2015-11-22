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

#ifndef DESKTOP_WINDOWS_LOG_HPP_INCLUDED
#define DESKTOP_WINDOWS_LOG_HPP_INCLUDED

#include <string>

/**
 * @file
 * Log file control routines for Windows.
 *
 * During static object initialization, stdout and stderr are redirected to a
 * uniquely-named log file located in the user's temporary directory as defined
 * by the platform (e.g. C:/Users/username/AppData/Local/Temp/wesnoth-XXXX.log).
 * Later, a request may be issued to relocate the log file to a more permanent
 * and user-accessible location (such as the Wesnoth user data directory).
 *
 * Because Wesnoth is normally built with the GUI subsystem option, there is no
 * console on startup and thus no way to see stdout/stderr output. Since
 * version 1.13.1, we can allocate a console during initialization when started
 * with the --wconsole option, but that is a somewhat clunky hack that does not
 * help with post mortem debugging.
 *
 * SDL 1.2 used to redirect stdout and stderr to stdout.txt and stderr.txt in
 * the process working directory automatically, but this approach too had its
 * own shortcomings by assuming the pwd was writable by the process (or in Vista
 * and later versions, requiring UAC virtualization to be enabled).
 */

namespace lg
{

/**
 * Returns the path to the current log file.
 *
 * An empty string is returned if the log file has not been set up yet or it
 * was disabled (e.g. by --wconsole).
 */
std::string log_file_path();

/**
 * Sets up the initial temporary log file.
 *
 * This has to be done on demand (preferably as early as possible) from a
 * function rather than during static initialization, otherwise things go
 * horribly wrong as soon as we try to use the logging facilities internally
 * for debug messages.
 */
void early_log_file_setup();

/**
 * Relocates the stdout+stderr log file to the user data directory.
 *
 * This function exits the process if something goes wrong (including calling
 * it when the user data directory isn't known yet).
 */
void finish_log_file_setup();

}

#endif
