/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "key.hpp"

#define KEY_TEST 0

#if (KEY_TEST == 1)

#include "video.hpp"

int main( void )
{
	SDL_Init(SDL_INIT_VIDEO);
	CVideo video( 640, 480, 16, 0 );
	CKey key;
	printf( "press enter (escape exits)...\n" );
	for(;;) {
		if( key[KEY_RETURN] != 0 )
			printf( "key(ENTER) pressed\n" );
		if( key[SDLK_ESCAPE] != 0 )
			return 1;
	}
}

#endif

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
