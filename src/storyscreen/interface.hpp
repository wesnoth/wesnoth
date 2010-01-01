/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/interface.hpp
 * Storyscreen controller (wrapper interface).
 */

#ifndef STORYSCREEN_HPP_INCLUDED
#define STORYSCREEN_HPP_INCLUDED

class config;
class vconfig;
class display;
class t_string;

#include <string>

/**
 * Function to show an introduction sequence using story WML.
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
 */
void show_storyscreen(display& disp, const vconfig& story_cfg, const std::string& scenario_name);

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
