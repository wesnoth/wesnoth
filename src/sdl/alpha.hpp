/*
   Copyright (C) 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef SDL_ALPHA_HPP_INCLUDED
#define SDL_ALPHA_HPP_INCLUDED

/**
 * @file
 * Compatibility layer for using SDL 1.2 and 2.0.
 *
 * Emulates SDL_SetAlpha.
 */

#include <SDL.h>

#if SDL_VERSION_ATLEAST(2, 0, 0)

#define SDL_SRCALPHA 0x00010000

int SDL_SetAlpha(SDL_Surface* surface, Uint32 flag, Uint8 alpha);

#endif

#endif
