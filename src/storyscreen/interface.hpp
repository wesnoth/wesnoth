/*
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
class CVideo;
class t_string;

namespace storyscreen {

enum START_POSITION {
	START_BEGINNING,
	START_END
};

} /* storyscreen namespace */

/**
 * Shows an introduction sequence using story WML.
 *
 * Each part of the sequence will be displayed in turn, with the user
 * able to go to the next part, previous part, or skip it entirely.
 */
void show_story(CVideo& video, const std::string &scenario_name,
	const config::const_child_itors &story);

#endif /* ! STORYSCREEN_HPP_INCLUDED */
