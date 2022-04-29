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
