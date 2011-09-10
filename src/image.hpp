/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include "token.hpp"
#include "map_location.hpp"
#include "sdl_utils.hpp"
#include "terrain_translation.hpp"


///this module manages the cache of images. With an image name, you can get
///the surface corresponding to that image.
//
namespace image {
	const int tile_size = 72;

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
			explicit value(const n_token::t_token& filename);
			// value(const n_token::t_token& filename, const n_token::t_token& modifications);
			// value(const n_token::t_token& filename, const map_location& loc, int center_x, int center_y, const n_token::t_token& modifications);
			value(const n_token::t_token& filename, const n_token::t_token& modifications
				  , const map_location& loc = map_location::null_location, int center_x=0, int center_y=0);

			bool operator==(const value& a) const;
			bool operator<(const value& a) const;

			type type_;
			n_token::t_token filename_;
			map_location loc_;
			n_token::t_token modifications_;
			int center_x_;
			int center_y_;
		};

		friend size_t hash_value(const value&);
		friend size_t hash_value(const locator&);

	public:

		typedef boost::unordered_map<size_t, boost::unordered_map<value, int> > locator_finder_t;

		// Constructing locators is somewhat slow, accessing image
		// through locators is fast. The idea is that calling functions
		// should store locators, and not strings to construct locators
		// (the second will work, of course, but will be slower)
		locator();

		locator(const locator &a, const n_token::t_token &mods = n_token::t_token::z_empty());

		// locator(const locator &a, const std::string &mods ="");

		locator(const n_token::t_token& filename);
		locator(const n_token::t_token& filename, const n_token::t_token& modifications);
		locator(const n_token::t_token& filename, const map_location& loc, int center_x, int center_y
				, const n_token::t_token& modifications=n_token::t_token::z_empty());

		locator(const char *filename);
		locator(const std::string& filename);
		locator(const std::string& filename, const std::string& modifications);
		locator(const std::string& filename, const map_location& loc, int center_x, int center_y, const std::string& modifications="");

		locator& operator=(const locator &a);
		bool operator==(const locator &a) const { return index_ == a.index_; }
		bool operator!=(const locator &a) const { return index_ != a.index_; }
		bool operator<(const locator &a) const { return index_ < a.index_; }

		const n_token::t_token &get_filename() const { return val_.filename_; }
		const map_location& get_loc() const { return val_.loc_ ; }
		int get_center_x() const { return val_.center_x_; }
		int get_center_y() const { return val_.center_y_; }
		const n_token::t_token& get_modifications() const {return val_.modifications_;}
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
		 * for the existence of a file. So this function does that. (Note it
		 * tests for existence not whether or not it's a valid image.)
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
	size_t hash_value(const locator& );


	typedef cache_type<surface> image_cache;
	typedef cache_type<bool> bool_cache;
	typedef boost::unordered_map<t_translation::t_terrain, surface> mini_terrain_cache_map;
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
	void set_team_colors(const std::vector<n_token::t_token>* colors = NULL);

	const std::vector<n_token::t_token>& get_team_colors();

	///sets the pixel format used by the images. Is called every time the
	///video mode changes. Invalidates all images.
	void set_pixel_format(SDL_PixelFormat* format);

	///sets the amount scaled images should be scaled. Invalidates all
	///scaled images.
	void set_zoom(int zoom);

	/// UNSCALED : image will be drawn "as is" without changing size, even in case of redraw
	/// SCALED_TO_ZOOM : image will be scaled taking zoom into account
	/// HEXED : the hex mask is applied on the image
	/// SCALED_TO_HEX : image will be scaled to fit into a hex, taking zoom into account
	/// TOD_COLORED : same as SCALED_TO_HEX but ToD coloring is also applied
	/// BRIGHTENED  : same as TOD_COLORED but also brightened
	/// SEMI_BRIGHTENED  : same as TOD_COLORED but also semi-brightened
	enum TYPE { UNSCALED, SCALED_TO_ZOOM, HEXED, SCALED_TO_HEX, TOD_COLORED, BRIGHTENED, SEMI_BRIGHTENED};

	///function to get the surface corresponding to an image.
	///note that this surface must be freed by the user by calling
	///SDL_FreeSurface()
	surface get_image(const locator& i_locator, TYPE type=UNSCALED);

	///function to get the standard hex mask
	surface get_hexmask();

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
void precache_file_existence(const n_token::t_token& subdir = n_token::t_token::z_empty());
	bool precached_file_exists(const n_token::t_token& file);
}

#endif

