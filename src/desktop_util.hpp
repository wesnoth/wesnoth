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

/**
 * @file
 * Desktop environment interaction functions.
 */

#ifndef DESKTOP_UTIL_HPP_INCLUDED
#define DESKTOP_UTIL_HPP_INCLUDED

#include <string>

namespace desktop {

bool open_in_file_manager(const std::string& path);

}

#endif
