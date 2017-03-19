/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef INTRO_HPP_INCLUDED
#define INTRO_HPP_INCLUDED

class CVideo;

#include <string>

/**
 * Displays a simple fading screen with any user-provided text.
 * Used after the end of single-player campaigns.
 *
 * @param text     Text to display, centered on the screen.
 *
 * @param duration In milliseconds, for how much time the text will
 *                 be displayed on screen.
 */
void the_end(CVideo &video, std::string text, unsigned int duration);

#endif /* ! INTRO_HPP_INCLUDED */
