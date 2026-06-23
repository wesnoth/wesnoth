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

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <iostream>

int main(int, char** argv)
{
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stdout, "Cannot initialize SDL Audio: %s\n", SDL_GetError());
        return (EXIT_FAILURE);
    }

    if(!MIX_Init()) {
        fprintf(stdout, "Cannot initialize SDL Mixer: %s\n", SDL_GetError());
        return (EXIT_FAILURE);
    }

    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!mixer) {
        fprintf(stdout, "Cannot initialize audio device: %s\n", SDL_GetError());
        return (EXIT_FAILURE);
    }

    int n = MIX_GetNumAudioDecoders();
    bool found_vorbis = false;
    for(int i = 0; i < n; i++) {
        if(strcmp(MIX_GetAudioDecoder(i), "VORBIS") == 0) {
            found_vorbis = true;
	}
    }
    if(!found_vorbis) {
        fprintf(stdout, "VORBIS codec not available\n");
        return (EXIT_FAILURE);
    }

    MIX_Audio* music = MIX_LoadAudio(mixer, argv[1], false);
    if (music == nullptr) {
        fprintf(stdout, "Cannot load music file: %s\n", SDL_GetError());
        MIX_DestroyMixer(mixer);
        return (EXIT_FAILURE);
    }

    fprintf(stdout, "Success\n");
    MIX_DestroyAudio(music);
    MIX_DestroyMixer(mixer);
    MIX_Quit();
    return (EXIT_SUCCESS);
}
