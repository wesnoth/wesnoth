/*
   Copyright (C) 2007 - 2017 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
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
 * Template instantiations for wesnoth-game.
 */

#include "animated.hpp"

#include <SDL_timer.h>

// Put these here to ensure that there's only
// one instance of the current_ticks variable
namespace {
	int current_ticks = 0;
}

void new_animation_frame()
{
	current_ticks = SDL_GetTicks();
}

int get_current_animation_tick()
{
	return current_ticks;
}
