#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "map.hpp"
#include "unit.hpp"

#include "SDL.h"
#include <string>

class team;

//this module manages the cache of images. With an image name, you can get
//the surface corresponding to that image, and don't need to free the image.
//Note that surfaces returned from here are invalidated whenever events::pump()
//is called, and so shouldn't be kept, but should be regotten from here as
//needed.
//
//images come in a number of varieties:
// - unscaled: no modifications have been done on the image.
// - scaled: images are scaled to the size of a tile
// - greyed: images are scaled and in greyscale
// - brightened: images are scaled and brighter than normal.
namespace image {

	//the image manager is responsible for setting up images, and destroying
	//all images when the program exits. It should probably
	//be created once for the life of the program 
	struct manager
	{
		manager();
		~manager();
	};

	//function to set the program's icon to the window manager.
	//must be called after SDL_Init() is called, but before setting the
	//video mode
	void set_wm_icon();

	//will make all scaled images have these rgb values added to all
	//their pixels. i.e. add a certain colour hint to images. useful
	//for representing day/night. Invalidates all scaled images.
	void set_colour_adjustment(int r, int g, int b);

	//sets the pixel format used by the images. Is called every time the
	//video mode changes. Invalidates all images.
	void set_pixel_format(SDL_PixelFormat* format);

	//sets the amount scaled images should be scaled. Invalidates all
	//scaled images.
	void set_zoom(double zoom);

	enum TYPE { UNSCALED, SCALED, GREYED, BRIGHTENED };

	enum COLOUR_ADJUSTMENT { ADJUST_COLOUR, NO_ADJUST_COLOUR };

	//function to get the surface corresponding to an image.
	//note that this surface must be freed by the user by calling
	//SDL_FreeSurface
	SDL_Surface* get_image(const std::string& filename,TYPE type=SCALED, COLOUR_ADJUSTMENT adj=ADJUST_COLOUR);

	//function to get a scaled image, but scale it to specific dimensions.
	//if you later try to get the same image using get_image() the image will
	//have the dimensions specified here.
	//Note that this surface must be freed by the user by calling SDL_FreeSurface
	SDL_Surface* get_image_dim(const std::string& filename, size_t x, size_t y);

	//the surface returned must be freed by the user
	SDL_Surface* getMinimap(int w, int h, const gamemap& map_, int lawful_bonus,
			const team* tm=NULL);
}

#endif

