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

CKey::CKey() :
	key_list(SDL_GetKeyboardState(nullptr))
{
}

bool CKey::operator[](int k) const
{
	return key_list[SDL_GetScancodeFromKey(k)] > 0;
}

bool CKey::is_uncomposable(const SDL_KeyboardEvent &event) {

	switch (event.keysym.sym) {
		case SDLK_RETURN:
		case SDLK_ESCAPE:
		case SDLK_BACKSPACE:
		case SDLK_TAB:
		case SDLK_F1:
		case SDLK_F2:
		case SDLK_F3:
		case SDLK_F4:
		case SDLK_F5:
		case SDLK_F6:
		case SDLK_F7:
		case SDLK_F8:
		case SDLK_F9:
		case SDLK_F10:
		case SDLK_F11:
		case SDLK_F12:
		case SDLK_F13:
		case SDLK_F14:
		case SDLK_F15:
		case SDLK_F16:
		case SDLK_F17:
		case SDLK_F18:
		case SDLK_F19:
		case SDLK_F20:
		case SDLK_F21:
		case SDLK_F22:
		case SDLK_F23:
		case SDLK_F24:
		case SDLK_INSERT:
		case SDLK_HOME:
		case SDLK_PAGEUP:
		case SDLK_PAGEDOWN:
		case SDLK_DELETE:
		case SDLK_END:
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_LEFT:
		case SDLK_RIGHT:

			return true;
		default:
			return false;
	}
}