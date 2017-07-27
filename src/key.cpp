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
		// Function keys
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
		// Keypad keys
		case SDLK_KP_0:
		case SDLK_KP_1:
		case SDLK_KP_2:
		case SDLK_KP_3:
		case SDLK_KP_4:
		case SDLK_KP_5:
		case SDLK_KP_6:
		case SDLK_KP_7:
		case SDLK_KP_8:
		case SDLK_KP_9:
		case SDLK_KP_00:
		case SDLK_KP_000:
		case SDLK_KP_A:
		case SDLK_KP_B:
		case SDLK_KP_C:
		case SDLK_KP_D:
		case SDLK_KP_E:
		case SDLK_KP_F:
		case SDLK_KP_AMPERSAND:
		case SDLK_KP_AT:
		case SDLK_KP_BACKSPACE:
		case SDLK_KP_BINARY:
		case SDLK_KP_CLEAR:
		case SDLK_KP_CLEARENTRY:
		case SDLK_KP_COLON:
		case SDLK_KP_COMMA:
		case SDLK_KP_DBLAMPERSAND:
		case SDLK_KP_DBLVERTICALBAR:
		case SDLK_KP_DECIMAL:
		case SDLK_KP_DIVIDE:
		case SDLK_KP_ENTER:
		case SDLK_KP_EQUALS:
		case SDLK_KP_EQUALSAS400:
		case SDLK_KP_EXCLAM:
		case SDLK_KP_GREATER:
		case SDLK_KP_HASH:
		case SDLK_KP_HEXADECIMAL:
		case SDLK_KP_LEFTBRACE:
		case SDLK_KP_LEFTPAREN:
		case SDLK_KP_LESS:
		case SDLK_KP_MEMADD:
		case SDLK_KP_MEMCLEAR:
		case SDLK_KP_MEMDIVIDE:
		case SDLK_KP_MEMMULTIPLY:
		case SDLK_KP_MEMRECALL:
		case SDLK_KP_MEMSTORE:
		case SDLK_KP_MEMSUBTRACT:
		case SDLK_KP_MINUS:
		case SDLK_KP_MULTIPLY:
		case SDLK_KP_OCTAL:
		case SDLK_KP_PERCENT:
		case SDLK_KP_PERIOD:
		case SDLK_KP_PLUS:
		case SDLK_KP_PLUSMINUS:
		case SDLK_KP_POWER:
		case SDLK_KP_RIGHTBRACE:
		case SDLK_KP_RIGHTPAREN:
		case SDLK_KP_SPACE:
		case SDLK_KP_TAB:
		case SDLK_KP_VERTICALBAR:
		case SDLK_KP_XOR:
		// Homepad keys
		case SDLK_INSERT:
		case SDLK_HOME:
		case SDLK_PAGEUP:
		case SDLK_PAGEDOWN:
		case SDLK_DELETE:
		case SDLK_END:
		// Arrow keys
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_SPACE:

			return true;
		default:
			return false;
	}
}