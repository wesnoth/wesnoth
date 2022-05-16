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
    if (music == NULL) {
        fprintf(stdout, "Cannot load music file: %s\\n", Mix_GetError());
        Mix_CloseAudio();
        return (EXIT_FAILURE);
    }

    fprintf(stdout, "Success\\n");
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    return (EXIT_SUCCESS);
}
