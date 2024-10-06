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

#include <SDL2/SDL_image.h>
#include <stdlib.h>

int main(int, char** argv)
{
    SDL_RWops *src = SDL_RWFromFile(argv[1], "rb");
    if (src == nullptr) {
        exit(2);
    }
    exit(!IMG_isWEBP(src));
}
