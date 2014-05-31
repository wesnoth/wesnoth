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

#ifndef SDL_RECT_HPP_INCLUDED
#define SDL_RECT_HPP_INCLUDED

/**
 * @file
 * Constains the SDL_Rect helper code.
 */

#include <SDL_version.h>

#if SDL_VERSION_ATLEAST(2, 0, 0)
#include <SDL_rect.h>
#else
#include <SDL_video.h>
#endif

namespace sdl
{

/**
 * Creates an empty SDL_Rect.
 *
 * Since SDL_Rect doesn't have a constructor it's not possible to create it as
 * a temporary for a function parameter. This functions overcomes this limit.
 */
SDL_Rect create_rect(const int x, const int y, const int w, const int h);

} // namespace sdl

#endif
