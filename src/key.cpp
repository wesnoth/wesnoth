/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "key.hpp"

#include <SDL_keyboard.h>

CKey::CKey() :
	key_list(SDL_GetKeyboardState(nullptr))
{
}

bool CKey::operator[](int k) const
{
	return key_list[SDL_GetScancodeFromKey(k)] > 0;
}
