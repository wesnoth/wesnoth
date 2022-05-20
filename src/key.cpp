/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

	switch (event.keysym.scancode) {
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_ESCAPE:
		case SDL_SCANCODE_BACKSPACE:
		case SDL_SCANCODE_TAB:
		// Function keys
		case SDL_SCANCODE_F1:
		case SDL_SCANCODE_F2:
		case SDL_SCANCODE_F3:
		case SDL_SCANCODE_F4:
		case SDL_SCANCODE_F5:
		case SDL_SCANCODE_F6:
		case SDL_SCANCODE_F7:
		case SDL_SCANCODE_F8:
		case SDL_SCANCODE_F9:
		case SDL_SCANCODE_F10:
		case SDL_SCANCODE_F11:
		case SDL_SCANCODE_F12:
		case SDL_SCANCODE_F13:
		case SDL_SCANCODE_F14:
		case SDL_SCANCODE_F15:
		case SDL_SCANCODE_F16:
		case SDL_SCANCODE_F17:
		case SDL_SCANCODE_F18:
		case SDL_SCANCODE_F19:
		case SDL_SCANCODE_F20:
		case SDL_SCANCODE_F21:
		case SDL_SCANCODE_F22:
		case SDL_SCANCODE_F23:
		case SDL_SCANCODE_F24:
		// Keypad keys
		case SDL_SCANCODE_KP_0:
		case SDL_SCANCODE_KP_1:
		case SDL_SCANCODE_KP_2:
		case SDL_SCANCODE_KP_3:
		case SDL_SCANCODE_KP_4:
		case SDL_SCANCODE_KP_5:
		case SDL_SCANCODE_KP_6:
		case SDL_SCANCODE_KP_7:
		case SDL_SCANCODE_KP_8:
		case SDL_SCANCODE_KP_9:
		case SDL_SCANCODE_KP_00:
		case SDL_SCANCODE_KP_000:
		case SDL_SCANCODE_KP_A:
		case SDL_SCANCODE_KP_B:
		case SDL_SCANCODE_KP_C:
		case SDL_SCANCODE_KP_D:
		case SDL_SCANCODE_KP_E:
		case SDL_SCANCODE_KP_F:
		case SDL_SCANCODE_KP_AMPERSAND:
		case SDL_SCANCODE_KP_AT:
		case SDL_SCANCODE_KP_BACKSPACE:
		case SDL_SCANCODE_KP_BINARY:
		case SDL_SCANCODE_KP_CLEAR:
		case SDL_SCANCODE_KP_CLEARENTRY:
		case SDL_SCANCODE_KP_COLON:
		case SDL_SCANCODE_KP_COMMA:
		case SDL_SCANCODE_KP_DBLAMPERSAND:
		case SDL_SCANCODE_KP_DBLVERTICALBAR:
		case SDL_SCANCODE_KP_DECIMAL:
		case SDL_SCANCODE_KP_DIVIDE:
		case SDL_SCANCODE_KP_ENTER:
		case SDL_SCANCODE_KP_EQUALS:
		case SDL_SCANCODE_KP_EQUALSAS400:
		case SDL_SCANCODE_KP_EXCLAM:
		case SDL_SCANCODE_KP_GREATER:
		case SDL_SCANCODE_KP_HASH:
		case SDL_SCANCODE_KP_HEXADECIMAL:
		case SDL_SCANCODE_KP_LEFTBRACE:
		case SDL_SCANCODE_KP_LEFTPAREN:
		case SDL_SCANCODE_KP_LESS:
		case SDL_SCANCODE_KP_MEMADD:
		case SDL_SCANCODE_KP_MEMCLEAR:
		case SDL_SCANCODE_KP_MEMDIVIDE:
		case SDL_SCANCODE_KP_MEMMULTIPLY:
		case SDL_SCANCODE_KP_MEMRECALL:
		case SDL_SCANCODE_KP_MEMSTORE:
		case SDL_SCANCODE_KP_MEMSUBTRACT:
		case SDL_SCANCODE_KP_MINUS:
		case SDL_SCANCODE_KP_MULTIPLY:
		case SDL_SCANCODE_KP_OCTAL:
		case SDL_SCANCODE_KP_PERCENT:
		case SDL_SCANCODE_KP_PERIOD:
		case SDL_SCANCODE_KP_PLUS:
		case SDL_SCANCODE_KP_PLUSMINUS:
		case SDL_SCANCODE_KP_POWER:
		case SDL_SCANCODE_KP_RIGHTBRACE:
		case SDL_SCANCODE_KP_RIGHTPAREN:
		case SDL_SCANCODE_KP_SPACE:
		case SDL_SCANCODE_KP_TAB:
		case SDL_SCANCODE_KP_VERTICALBAR:
		case SDL_SCANCODE_KP_XOR:
		// Homepad keys
		case SDL_SCANCODE_INSERT:
		case SDL_SCANCODE_HOME:
		case SDL_SCANCODE_PAGEUP:
		case SDL_SCANCODE_PAGEDOWN:
		case SDL_SCANCODE_DELETE:
		case SDL_SCANCODE_END:
		// Arrow keys
		case SDL_SCANCODE_UP:
		case SDL_SCANCODE_DOWN:
		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_RIGHT:
		case SDL_SCANCODE_SPACE:
		// Latin letters
		case SDL_SCANCODE_A:
		case SDL_SCANCODE_B:
		case SDL_SCANCODE_C:
		case SDL_SCANCODE_D:
		case SDL_SCANCODE_E:
		case SDL_SCANCODE_F:
		case SDL_SCANCODE_G:
		case SDL_SCANCODE_H:
		case SDL_SCANCODE_I:
		case SDL_SCANCODE_J:
		case SDL_SCANCODE_K:
		case SDL_SCANCODE_L:
		case SDL_SCANCODE_M:
		case SDL_SCANCODE_N:
		case SDL_SCANCODE_O:
		case SDL_SCANCODE_P:
		case SDL_SCANCODE_Q:
		case SDL_SCANCODE_R:
		case SDL_SCANCODE_S:
		case SDL_SCANCODE_T:
		case SDL_SCANCODE_U:
		case SDL_SCANCODE_V:
		case SDL_SCANCODE_W:
		case SDL_SCANCODE_X:
		case SDL_SCANCODE_Y:
		case SDL_SCANCODE_Z:

			return true;
		default:
			return false;
	}
}
