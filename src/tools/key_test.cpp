#include "key.h"
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
