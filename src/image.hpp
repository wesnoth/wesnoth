#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "SDL.h"

#include "map.hpp"

#include <string>

namespace image {

	struct manager
	{
		manager();
		~manager();
	};

	void set_colour_adjustment(int r, int g, int b);
	void set_pixel_format(SDL_PixelFormat* format);
	void set_zoom(double zoom);

	enum TYPE { UNSCALED, SCALED, GREYED, BRIGHTENED };
	SDL_Surface* get_image(const std::string& filename,TYPE type=SCALED);
	SDL_Surface* get_image_dim(const std::string& filename, size_t x, size_t y);
}

#endif

