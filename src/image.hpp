/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "map_location.hpp"
#include "sdl_utils.hpp"
#include "terrain_translation.hpp"

#include "SDL.h"

#include <map>
#include <string>
#include <vector>

///this module manages the cache of images. With an image name, you can get
///the surface corresponding to that image.
//
///images come in a number of varieties:
/// - unscaled: no modifications have been done on the image.
/// - scaled_to_hex: images are scaled to the size of a tile
/// - scaled_to_zoom: images are scaled to the zoom factor, factor 1 is same as unscaled no other modifications are done
/// - unmasked: images are scaled, but have no time of day masking applied to them
/// - brightened: images are scaled and brighter than normal.
namespace image {
#ifdef USE_TINY_GUI
	// images in tiny-gui will be scaled at install time
	const int tile_size = 36;
#else
	const int tile_size = 72;
#endif

	template<typename T>
	class cache_type;

	//a generic image locator. Abstracts the location of an image.
	class locator
	{
	public:
		enum type { NONE, FILE, SUB_FILE };
	private:
		// Called by each constructor after actual construction to
		// initialize the index_ field
		void init_index();
		void parse_arguments();
		struct value {
			value();
			value(const value &a);
			value(const char *filename);
			value(const std::string& filename);
			value(const std::string& filename, const std::string& modifications);
			value(const std::string& filename, const map_location& loc, int center_x, int center_y, const std::string& modifications);

			bool operator==(const value& a) const;
			bool operator<(const value& a) const;

			type type_;
			std::string filename_;
			map_location loc_;
			std::string modifications_;
			int center_x_;
			int center_y_;
		};

		friend size_t hash_value(const value&);

	public:

		/**
		 * @todo replace this with std::unordered_map<value, int> or boost::unordered_map<value, int>
		 *       boost::unordered_map can almost just be dropped in as boost::hash<T>(T val) will return hash_value(val), but it
		 *       requires boost 1.35 (preferably 1.36 or later)
		 *
		 **/
		typedef std::map<size_t, std::map<value, int> > locator_finder_t;

		// Constructing locators is somewhat slow, accessing image
		// through locators is fast. The idea is that calling functions
		// should store locators, and not strings to construct locators
		// (the second will work, of course, but will be slower)
	        locator();
		locator(const locator &a, const std::string &mods ="");
		locator(const char *filename);
		locator(const std::string& filename);
		locator(const std::string& filename, const std::string& modifications);
		locator(const std::string& filename, const map_location& loc, int center_x, int center_y, const std::string& modifications="");

		locator& operator=(const locator &a);
		bool operator==(const locator &a) const { return index_ == a.index_; }
		bool operator!=(const locator &a) const { return index_ != a.index_; }
		bool operator<(const locator &a) const { return index_ < a.index_; }

		const std::string &get_filename() const { return val_.filename_; }
		const map_location& get_loc() const { return val_.loc_ ; }
		int get_center_x() const { return val_.center_x_; }
		int get_center_y() const { return val_.center_y_; }
		const std::string& get_modifications() const {return val_.modifications_;}
		type get_type() const { return val_.type_; };
		// const int get_index() const { return index_; };

		// returns true if the locator does not correspond to any
		// actual image
		bool is_void() const { return val_.type_ == NONE; }

		/**
		 * Tests whether the file the locater points at exists.
		 *
		 * is_void doesn't seem to work before the image is loaded and also in
		 * debug mode a placeholder is returned. So it's not possible to test
		 * for the existance of a file. So this function does that. (Note it
		 * tests for existance not whether or not it's a valid image.)
		 *
		 * @return                Whether or not the file exists.
		 */
		bool file_exists();

		// loads the image it is pointing to from the disk
		surface load_from_disk() const;

		template <typename T>
		bool in_cache(cache_type<T> &cache) const;
		template <typename T>
		const T &locate_in_cache(cache_type<T> &cache) const;
		template <typename T>
		void add_to_cache(cache_type<T> &cache, const T &data) const;

	private:

		surface load_image_file() const;
		surface load_image_sub_file() const;

		int index_;
		value val_;
	};

	size_t hash_value(const locator::value&);


	typedef cache_type<surface> image_cache;
	typedef cache_type<bool> bool_cache;
	typedef std::map<t_translation::t_terrain, surface> mini_terrain_cache_map;
	extern mini_terrain_cache_map mini_terrain_cache;
	extern mini_terrain_cache_map mini_fogged_terrain_cache;

	void flush_cache();

	///the image manager is responsible for setting up images, and destroying
	///all images when the program exits. It should probably
	///be created once for the life of the program
	struct manager
	{
		manager();
		~manager();
	};

	///will make all scaled images have these rgb values added to all
	///their pixels. i.e. add a certain color hint to images. useful
	///for representing day/night. Invalidates all scaled images.
	void set_color_adjustment(int r, int g, int b);

	class color_adjustment_resetter
	{
		public:
			color_adjustment_resetter();
			void reset();
		private:
			int r_, g_, b_;
	};

	///set the team colors used by the TC image modification
	///use a vector with one string for each team
	///using NULL will reset to default TC
	void set_team_colors(const std::vector<std::string>* colors = NULL);

	///sets the pixel format used by the images. Is called every time the
	///video mode changes. Invalidates all images.
	void set_pixel_format(SDL_PixelFormat* format);

	///sets the amount scaled images should be scaled. Invalidates all
	///scaled images.
	void set_zoom(int zoom);

	// unscaled : image will be drawn "as is" without changing size, even in case of redraw
	// hexed : the hex mask is applied on the image
	// unmasked : image will be scaled to fit into a hex, taking zoom into account
	// scaled_to_hex : same but ToD coloring is also applied
	enum TYPE { UNSCALED, HEXED, UNMASKED, SCALED_TO_HEX, SCALED_TO_ZOOM, BRIGHTENED, SEMI_BRIGHTENED };


	///function to get the surface corresponding to an image.
	///note that this surface must be freed by the user by calling
	///SDL_FreeSurface()
	surface get_image(const locator& i_locator, TYPE type=UNSCALED);

	///function to check if an image fit into an hex
	///return false if the image has not the standard size.
	bool is_in_hex(const locator& i_locator);

	///function to reverse an image. The image MUST have originally been returned from
	///an image:: function. Returned images have the same semantics as for get_image()
	///and must be freed using SDL_FreeSurface()
	surface reverse_image(const surface &surf);

	///returns true if the given image actually exists, without loading it.
	bool exists(const locator& i_locator);

	/// precache the existence of files in the subdir (ex: "terrain/")
	void precache_file_existence(const std::string& subdir = "");
	bool precached_file_exists(const std::string& file);
}

#endif

