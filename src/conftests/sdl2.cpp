/*
	Copyright (C) 2022 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <SDL2/SDL.h>

#define STR(x) STR_(x)
#define STR_(x) #x

#if ! SDL_VERSION_ATLEAST(REQ_MAJOR, REQ_MINOR, REQ_PATCH)
#pragma message "SDL version " STR(SDL_MAJOR_VERSION.SDL_MINOR_VERSION.SDL_PATCHLEVEL) " is older than required version " STR(REQ_MAJOR.REQ_MINOR.REQ_PATCH)
#error SDL is too old!
#endif

int main(int, char**)
{
    SDL_Init(0);
    SDL_Quit();

    return 0;
}
