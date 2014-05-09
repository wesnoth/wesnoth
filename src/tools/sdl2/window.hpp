/*
   Copyright (C) 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TOOLS_SDL2_WINDOW_HPP_INCLUDED
#define TOOLS_SDL2_WINDOW_HPP_INCLUDED

#include <string>

/**
 * Executes a window command.
 *
 * @param command                The command to be executed.
 * @param begin                  The beginning of the command's arguments.
 */
void execute_window(std::string& command, std::string::const_iterator begin);

#endif
