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

#include "sdl/alpha.hpp"

#if SDL_VERSION_ATLEAST(2, 0, 0)

int SDL_SetAlpha(SDL_Surface* surface, Uint32 flag, Uint8 alpha)
{
	if(flag & SDL_SRCALPHA) {
		return SDL_SetSurfaceAlphaMod(surface, alpha);
	} else {
		return SDL_SetSurfaceAlphaMod(surface, SDL_ALPHA_OPAQUE);
	}
}

#endif
