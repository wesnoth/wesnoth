/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef INTRO_HPP_INCLUDED
#define INTRO_HPP_INCLUDED

#include "SDL.h"
#include "config.hpp"
#include "display.hpp"

#include <string>

void show_intro(display& screen, config& data);

void show_map_scene(display& screen, config& data);

#endif
