/*
   Copyright (C) 2009 - 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * Storyscreen controller (wrapper interface).
 */

#ifndef STORYSCREEN_HPP_INCLUDED
#define STORYSCREEN_HPP_INCLUDED

#include <string>

#include "config.hpp"

class vconfig;
class display;
class t_string;

namespace storyscreen {

enum START_POSITION {
	START_BEGINNING,
	START_END
};

} /* storyscreen namespace */

/**
 * Function to show an introduction sequence segment using story WML.
 * The WML config data (story_cfg) has a format similar to:
 * @code
 * [part]
 *     id='id'
 *     story='story'
 *     image='img'
 * [/part]
 * @endcode
 * Where 'id' is a unique identifier, 'story' is text describing the
 * storyline,and 'img' is a background image. Each part of the sequence will
 * be displayed in turn, with the user able to go to the next part, or skip
 * it entirely.
 * Return is NEXT if the segment played to the end, BACK if the segment played to the beginning,
 * and QUIT if the story was quit
 */
void show_story(display &disp, const std::string &scenario_name,
	const config::const_child_itors &story);

/**
 * Displays a simple fading screen with any user-provided text.
 * Used after the end of single-player campaigns.
 *
 * @param text     Text to display, centered on the screen.
 *
 * @param duration In milliseconds, for how much time the text will
 *                 be displayed on screen.
 */
void show_endscreen(display& disp, const t_string& text, unsigned int duration);

#endif /* ! STORYSCREEN_HPP_INCLUDED */
