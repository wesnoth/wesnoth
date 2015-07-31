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

/**
 * @file
 * Platform identification and version information functions.
 */

#ifndef DESKTOP_VERSION_HPP_INCLUDED
#define DESKTOP_VERSION_HPP_INCLUDED

#include <string>

namespace desktop
{

/**
 * Returns a string with the running OS name and version information.
 *
 * On Unix-type platforms, this will be the uname information (which is rarely
 * useful). On Windows, this is a string we generate ourselves by processing
 * GetVersionEx's output data.
 *
 * Needless to say, this is a highly experimental and unreliable function and
 * odds are its output is not particularly useful most of the time.
 */
std::string os_version();

}

#endif
