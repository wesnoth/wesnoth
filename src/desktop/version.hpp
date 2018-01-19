/*
   Copyright (C) 2015 - 2018 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Platform identification and version information functions.
 */

#pragma once

#include <string>

namespace desktop
{

/**
 * Returns a string with the running OS name and version information.
 *
 * On Windows, this is a string we generate ourselves by processing
 * GetVersionEx's output. On OS X and Linux, this is the output of a command
 * provided by the OS if available; failing that (and on other Unixes as well),
 * we use the uname system call, which is hardly ever useful.
 */
std::string os_version();

}
