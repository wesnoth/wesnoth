#include "cursor.hpp"
#include "image.hpp"
#include "preferences.hpp"
#include "scoped_resource.hpp"
#include "sdl_utils.hpp"

#include "SDL.h"

#include <vector>

namespace
{

SDL_Cursor* create_cursor(SDL_Surface* surface)
{
	const scoped_sdl_surface surf(make_neutral_surface(surface));
	if(surf == NULL) {
		return NULL;
	}

	//the width must be a multiple of 8 (SDL requirement)
	size_t cursor_width = surf->w;
	if((cursor_width%8) != 0) {
		cursor_width += 8 - (cursor_width%8);
	}

	std::vector<Uint8> data((cursor_width*surf->h)/8,0);
	std::vector<Uint8> mask(data.size(),0);

	//see http://sdldoc.csn.ul.ie/sdlcreatecursor.php for documentation on
	//the format that data has to be in to pass to SDL_CreateCursor
	surface_lock lock(surf);
	const Uint32* const pixels = reinterpret_cast<Uint32*>(lock.pixels());
	for(size_t y = 0; y != surf->h; ++y) {
		for(size_t x = 0; x != surf->w; ++x) {
			Uint8 r,g,b,a;
			SDL_GetRGBA(pixels[y*surf->w + x],surf->format,&r,&g,&b,&a);

			const size_t index = y*cursor_width + x;
			
			const size_t shift = 7 - (index%8);

			const Uint8 trans = (a < 128 ? 0 : 1) << shift;
			const Uint8 black = (trans == 0 || (r+g+b)/3 > 128 ? 0 : 1) << shift;

			data[index/8] |= black;
			mask[index/8] |= trans;
		}
	}

	return SDL_CreateCursor(&data[0],&mask[0],cursor_width,surf->h,0,0);
}

SDL_Cursor* cache[cursor::NUM_CURSORS] = { NULL, NULL, NULL, NULL };

//this array must have members corresponding to cursor::CURSOR_TYPE enum members
const std::string images[cursor::NUM_CURSORS] = { "normal.png", "wait.png", "move.png", "attack.png" };

cursor::CURSOR_TYPE current_cursor = cursor::NUM_CURSORS;

SDL_Cursor* get_cursor(cursor::CURSOR_TYPE type)
{
	if(cache[type] == NULL) {
		static const std::string prefix = "cursors-bw/";
		const scoped_sdl_surface surf(image::get_image(prefix + images[type],image::UNSCALED));
		cache[type] = create_cursor(surf);
	}

	return cache[type];
}

void clear_cache()
{
	for(size_t n = 0; n != cursor::NUM_CURSORS; ++n) {
		if(cache[n] != NULL) {
			SDL_FreeCursor(cache[n]);
			cache[n] = NULL;
		}
	}
}

}

namespace cursor
{

manager::manager()
{
}

manager::~manager()
{
	clear_cache();
}

void set(CURSOR_TYPE type)
{
	SDL_Cursor* const cursor = get_cursor(type);
	if(cursor != NULL) {
		SDL_SetCursor(cursor);
		current_cursor = type;
	}
}

setter::setter(CURSOR_TYPE type) : old_(current_cursor)
{
	set(type);
}

setter::~setter()
{
	set(old_);
}

}