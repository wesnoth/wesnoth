/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "key.hpp"
#include "sdl/compat.hpp"

CKey::CKey() :
	key_list(SDL_GetKeyState(NULL))
{
}

bool CKey::operator[](int k) const
{
#if SDL_VERSION_ATLEAST(2,0,0)
	return key_list[SDL_GetScancodeFromKey(k)] > 0;
#else
	return key_list[k] > 0;
#endif
}
