#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "map.hpp"
#include "unit.hpp"

#include "SDL.h"
#include <string>

class team;

///this module manages the cache of images. With an image name, you can get
///the surface corresponding to that image, and don't need to free the image.
///Note that surfaces returned from here are invalidated whenever events::pump()
///is called, and so shouldn't be kept, but should be regotten from here as
///needed.
//
///images come in a number of varieties:
/// - unscaled: no modifications have been done on the image.
/// - scaled: images are scaled to the size of a tile
/// - unmasked: images are scaled, but have no time of day masking applied to them
/// - greyed: images are scaled and in greyscale
/// - brightened: images are scaled and brighter than normal.
namespace image {
	///a generic image locator. Abstracts the location of an image.
	///used as a key for a std::map
	struct locator
	{
		locator(const char *filename) : filename(filename) { type = FILE; }
		locator(const std::string& filename) : filename(filename) { type = FILE; }
		locator(const std::string& filename, const gamemap::location& loc) : filename(filename), loc(loc)
			{ type = SUB_FILE; }

		bool operator==(const locator &a) const;
		bool operator<(const locator &a) const;

		enum { FILE, SUB_FILE } type;
		std::string filename;
		gamemap::location loc;
	};

	///the image manager is responsible for setting up images, and destroying
	///all images when the program exits. It should probably
	///be created once for the life of the program 
	struct manager
	{
		manager();
		~manager();
	};

	///function to set the program's icon to the window manager.
	///must be called after SDL_Init() is called, but before setting the
	///video mode
	void set_wm_icon();

	///will make all scaled images have these rgb values added to all
	///their pixels. i.e. add a certain colour hint to images. useful
	///for representing day/night. Invalidates all scaled images.
	void set_colour_adjustment(int r, int g, int b);

	///function to get back the current colour adjustment values
	void get_colour_adjustment(int *r, int *g, int *b);

	///function which sets a certain image as a 'mask' for all scaled images.
	///the 'mask' is blitted onto all scaled images.
	void set_image_mask(const std::string& image_name);

	///sets the pixel format used by the images. Is called every time the
	///video mode changes. Invalidates all images.
	void set_pixel_format(SDL_PixelFormat* format);

	///sets the amount scaled images should be scaled. Invalidates all
	///scaled images.
	void set_zoom(int zoom);

	enum TYPE { UNSCALED, SCALED, UNMASKED, GREYED, BRIGHTENED, SEMI_BRIGHTENED };

	enum COLOUR_ADJUSTMENT { ADJUST_COLOUR, NO_ADJUST_COLOUR };

	///function to get the surface corresponding to an image.
	///note that this surface must be freed by the user by calling
	///SDL_FreeSurface()
	SDL_Surface* get_image(const locator& i_locator,TYPE type=SCALED, COLOUR_ADJUSTMENT adj=ADJUST_COLOUR);

	///function to get a scaled image, but scale it to specific dimensions.
	///if you later try to get the same image using get_image() the image will
	///have the dimensions specified here.
	///Note that this surface must be freed by the user by calling SDL_FreeSurface
	SDL_Surface* get_image_dim(const locator& i_locator, size_t x, size_t y);

	///function to reverse an image. The image MUST have originally been returned from
	///an image:: function. Returned images have the same semantics as for get_image()
	///and must be freed using SDL_FreeSurface()
	SDL_Surface* reverse_image(SDL_Surface* surf);

	///function to register an image with the given id. Calls to get_image(id,UNSCALED) will
	///return this image. register_image() will take ownership of this image and free
	///it when the cache is cleared (change of video mode or colour adjustment).
	///If there is already an image registered with this id, that image will be freed
	///and replaced with this image.
	void register_image(const locator& i_locator, SDL_Surface* surf);

	///function to create the minimap for a given map
	///the surface returned must be freed by the user
	SDL_Surface* getMinimap(int w, int h, const gamemap& map_, int lawful_bonus, const team* tm=NULL);
}

#endif

