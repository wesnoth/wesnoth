/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@comcast.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "key.hpp"

CKey::CKey() : is_enabled(true)
{
	static int num_keys = 300;
	key_list = SDL_GetKeyState( &num_keys );
}

int CKey::operator[]( int code )
{
	return (code == SDLK_ESCAPE || is_enabled) && int(key_list[code]);
}

void CKey::SetEnabled( bool enable )
{
	is_enabled = enable;
}
