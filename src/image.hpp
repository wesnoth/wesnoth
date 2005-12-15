/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "map.hpp"
#include "sdl_utils.hpp"

#include "SDL.h"
#include <string>
#include <vector>

///this module manages the cache of images. With an image name, you can get
///the surface corresponding to that image.
//
///images come in a number of varieties:
/// - unscaled: no modifications have been done on the image.
/// - scaled: images are scaled to the size of a tile
/// - unmasked: images are scaled, but have no time of day masking applied to them
/// - greyed: images are scaled and in greyscale
/// - brightened: images are scaled and brighter than normal.
namespace image {
	template<typename T>
	struct cache_item {
		cache_item() : loaded(false), item() {}
		cache_item(T item) : loaded(true), item(item) {}

		bool loaded;
		T item;
	};

	//a generic image locator. Abstracts the location of an image.
	class locator
	{
	private:
		// Called by each constructor after actual construction to
		// initialize the index_ field
		void init_index();
	public:
		enum type { NONE, FILE, SUB_FILE };

		struct value {
			value();
			value(const value &a);
			value(const char *filename);
			value(const char *filename, color_range new_rgb, std::vector<Uint32> swap_rgb);
			value(const std::string& filename);
		        value(const std::string& filename, color_range new_rgb, std::vector<Uint32> swap_rgb);;
			value(const std::string& filename, const gamemap::location& loc, color_range new_rgb, std::vector<Uint32> swap_rgb);

			bool operator==(const value& a) const;
			bool operator<(const value& a) const;

			type type_;
			std::string filename_;
			gamemap::location loc_;
		        color_range new_color;
		std::vector<Uint32> swap_colors;
		};

		// Constructing locators is somewhat slow, accessing image
		// through locators is fast. The idea is that calling functions
		// should store locators, and not strings to construct locators
		// (the second will work, of course, but will be slower)
	        locator();
		locator(const locator &a);
		locator(const char *filename);
		locator(const char *filename, color_range new_rgb, std::vector<Uint32> swap_rgb);
		locator(const std::string& filename);
		locator(const std::string& filename, color_range new_rgb, std::vector<Uint32> swap_rgb);
		locator(const std::string& filename, const gamemap::location& loc, color_range new_rgb = std::vector<Uint32>(), std::vector<Uint32> swap_rgb = std::vector<Uint32>());

		locator& operator=(const locator &a);
		bool operator==(const locator &a) const { return index_ == a.index_; }
		bool operator!=(const locator &a) const { return index_ != a.index_; }
		bool operator<(const locator &a) const { return index_ < a.index_; }

		const std::string &get_filename() const { return val_.filename_; }
		const gamemap::location& get_loc() const { return val_.loc_ ; }
		const color_range& get_new_color() const { return val_.new_color ; }
		const std::vector<Uint32>& get_swap_colors() const { return val_.swap_colors ; }
		const type get_type() const { return val_.type_; };
		// const int get_index() const { return index_; };

		// returns true if the locator does not correspond to any
		// actual image
		bool is_void() const { return val_.type_ == NONE; }
		// loads the image it is pointing to from the disk
		surface load_from_disk() const;

#if 0
		// returns true if the locator already was stored in the given
		// cache
		template<typename T>
		bool in_cache(const std::vector<cache_item<T> >& cache) const;
		// returns the image it is corresponding to in the given cache
		template<typename T>
		T locate_in_cache(const std::vector<cache_item<T> >& cache) const;
		// adds the given image to the given cache, indexed with the
		// current locator
		template<typename T>
		void add_to_cache(std::vector<cache_item<T> >& cache, const T &image) const;
#endif
		bool in_cache(const std::vector<cache_item<surface> >& cache) const;
		surface locate_in_cache(const std::vector<cache_item<surface> >& cache) const;
		void add_to_cache(std::vector<cache_item<surface> >& cache, const surface &image) const;
		bool in_cache(const std::vector<cache_item<locator> >& cache) const;
		locator locate_in_cache(const std::vector<cache_item<locator> >& cache) const;
		void add_to_cache(std::vector<cache_item<locator> >& cache, const locator &image) const;
	protected:
		static int last_index_;
	private:

		surface load_image_file() const;
		surface load_image_sub_file() const;

		int index_;
		value val_;
	};


	typedef std::vector<cache_item<surface> > image_cache;
	typedef std::vector<cache_item<locator> > locator_cache;
	typedef std::map<gamemap::TERRAIN, surface> mini_terrain_cache_map;
	extern mini_terrain_cache_map mini_terrain_cache;

	void flush_cache();

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

	extern SDL_PixelFormat* pixel_format;

	///sets the pixel format used by the images. Is called every time the
	///video mode changes. Invalidates all images.
	void set_pixel_format(SDL_PixelFormat* format);

	///sets the amount scaled images should be scaled. Invalidates all
	///scaled images.
	void set_zoom(int zoom);

	enum TYPE { UNSCALED, SCALED, UNMASKED, GREYED, DARKENED, BRIGHTENED, SEMI_BRIGHTENED };

	enum COLOUR_ADJUSTMENT { ADJUST_COLOUR, NO_ADJUST_COLOUR };

	///function to get the surface corresponding to an image.
	///note that this surface must be freed by the user by calling
	///SDL_FreeSurface()
	surface get_image(const locator& i_locator,TYPE type=SCALED, COLOUR_ADJUSTMENT adj=ADJUST_COLOUR);

	///function to get a scaled image, but scale it to specific dimensions.
	///if you later try to get the same image using get_image() the image will
	///have the dimensions specified here.
	///Note that this surface must be freed by the user by calling SDL_FreeSurface
	surface get_image_dim(const locator& i_locator, size_t x, size_t y);

	///function to reverse an image. The image MUST have originally been returned from
	///an image:: function. Returned images have the same semantics as for get_image()
	///and must be freed using SDL_FreeSurface()
	surface reverse_image(const surface &surf);


	locator get_alternative(const locator &i_locator, const std::string &alt);

	///function to register an image with the given id. Calls to get_image(id,UNSCALED) will
	///return this image. register_image() will take ownership of this image and free
	///it when the cache is cleared (change of video mode or colour adjustment).
	///If there is already an image registered with this id, that image will be freed
	///and replaced with this image.
	void register_image(const locator& i_locator, const surface& surf);

	//returns true if the given image actually exists, without loading it.
	bool exists(const locator& i_locator);
}

#endif

