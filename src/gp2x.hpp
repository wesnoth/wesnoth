/* $Id: gp2x.hpp 13758 2006-09-30 19:09:45Z grzywacz $ */
/*
   Copyright (C) 2003-6 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef GP2X

#ifndef GP2X_HPP_INCLUDED
#define GP2X_HPP_INCLUDED

#include "SDL.h"

namespace gp2x {

int init_joystick();
void handle_joystick(SDL_JoyButtonEvent *);
void makeup_events();
Uint8 get_joystick_state(int *x, int *y);
void return_to_menu();
void mmu_hack();

} // namespace gp2x

#endif

#endif

/* vim: set ts=4 sw=4: */
