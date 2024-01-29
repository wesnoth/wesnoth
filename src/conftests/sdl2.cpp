/*
	Copyright (C) 2022 - 2024
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
#include <stdlib.h>
#include <string>

int main(int, char** argv)
{
    int major = std::stoi(argv[1]);
    int minor = std::stoi(argv[2]);
    int patchlevel = std::stoi(argv[3]);

    if(!SDL_VERSION_ATLEAST(major, minor, patchlevel)) {
        exit(1);
    }

    SDL_Init(0);
    SDL_Quit();

    return 0;
}
