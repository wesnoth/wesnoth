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

#ifndef TOOLS_SDL2_SDL2_HPP_INCLUDED
#define TOOLS_SDL2_SDL2_HPP_INCLUDED

#include <string>
#include <vector>

namespace sdl
{
class twindow;
} // namespace sdl

/**
 * Contains the windows created by the user.
 *
 * @note The pointers are leaked when the program terminates, leaving it to the
 * OS to do the cleanup.
 */
extern std::vector<sdl::twindow*> windows;

/**
 * Gets a token from a string.
 *
 * Tokens are separated by a single @p separator character.
 *
 * @param string                  The string to search for tokens.
 * @param[in,out] begin           Input:
 *                                - The beginning of the current token. The
 *                                  text until the next @p separator is matched.
 *                                Output:
 *                                - The position of the next token or the end
 *                                  of the @p string if the @p separator
 *                                  character was not found.
 * @param separator               The character marking the separation between
 *                                tokens.
 *
 * @returns                       The matched string. The match is either until
 *                                the @p separator character, or if the
 *                                character is not found until the end of the
 *                                @p string.
 */
std::string get_token(const std::string& string,
					  std::string::const_iterator& begin,
					  const char separator);

#endif
