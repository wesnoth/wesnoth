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

/**
 * @file
 * Desktop environment interaction functions.
 */

#ifndef DESKTOP_UTIL_HPP_INCLUDED
#define DESKTOP_UTIL_HPP_INCLUDED

#include <string>

namespace desktop {

/**
 * Opens the specified object with the default application configured for its type.
 *
 * The default application for handling the object represented by
 * @a path_or_url is defined by the operating system and desktop environment
 * under which Wesnoth is running, and it is not under our control. Therefore,
 * <b>EXTREME CAUTION</b> is advised when using this function with URLs or
 * paths that are entirely or partially constructed from user-provided input
 * (e.g., WML from user-made add-ons, chat log messages).
 *
 * If the content pointed to by @a path_or_url cannot be trusted, you should
 * either refrain from using this function, or warn the user before calling
 * this function.
 */
bool open_object(const std::string& path_or_url);

}

#endif
