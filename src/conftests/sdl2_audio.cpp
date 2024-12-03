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

#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

int main(int, char** argv)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stdout, "Cannot initialize SDL Audio: %s\\n", SDL_GetError());
        return (EXIT_FAILURE);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        fprintf(stdout, "Cannot initialize SDL Mixer: %s\\n", Mix_GetError());
        return (EXIT_FAILURE);
    }

    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        fprintf(stdout, "Cannot initialize OGG codec: %s\\n", Mix_GetError());
        Mix_CloseAudio();
        return (EXIT_FAILURE);
    }

    Mix_Music* music = Mix_LoadMUS(argv[1]);
    if (music == nullptr) {
        fprintf(stdout, "Cannot load music file: %s\\n", Mix_GetError());
        Mix_CloseAudio();
        return (EXIT_FAILURE);
    }

    fprintf(stdout, "Success\\n");
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    return (EXIT_SUCCESS);
}
