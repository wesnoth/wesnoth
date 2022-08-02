#include <SDL2/SDL_image.h>
#include <stdlib.h>

int main(int, char** argv)
{
    SDL_RWops *src = SDL_RWFromFile(argv[1], "rb");
    if (src == NULL) {
        exit(2);
    }
    exit(!IMG_isJPG(src));
}
