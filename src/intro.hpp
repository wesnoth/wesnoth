/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file intro.hpp */

#ifndef INTRO_HPP_INCLUDED
#define INTRO_HPP_INCLUDED

class config;
class vconfig;
class display;
#include "SDL.h"

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
void the_end(display &disp, std::string text, unsigned int duration);

/**
 * Enables/disables the new (work in progress) story screen
 * code.
 */
void set_new_storyscreen(bool enabled);

bool get_new_storyscreen_status();

#endif /* ! INTRO_HPP_INCLUDED */
