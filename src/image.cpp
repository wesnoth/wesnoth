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

#include "global.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "wesconfig.h"

#include "SDL_image.h"

#include <iostream>
#include <map>
#include <string>

#define ERR_DP LOG_STREAM(err, display)

namespace {

typedef std::map<image::locator::value, int> locator_finder_t;
typedef std::pair<image::locator::value, int> locator_finder_pair;
locator_finder_t locator_finder;

// Definition of all image maps
image::image_cache images_,scaled_images_,unmasked_images_,greyed_images_;
image::image_cache brightened_images_,semi_brightened_images_,darkened_images_;

image::locator_cache alternative_images_;

// const int cache_version_ = 0;

std::map<image::locator,bool> image_existance_map;

std::map<surface, surface> reversed_images_;

int red_adjust = 0, green_adjust = 0, blue_adjust = 0;

std::string image_mask;

#ifdef USE_TINY_GUI
const int tile_size = 36;
#else
const int tile_size = 72;
#endif
int zoom = tile_size;

//The "pointer to surfaces" vector is not cleared anymore (the surface are
//still freed, of course.) I do not think it is a problem, as the number of
//different surfaces the program may lookup has an upper limit, so its
//memory usage won't grow indefinitely over time
template<typename T>
void reset_cache(std::vector<image::cache_item<T> >& cache)
{
	typename std::vector<image::cache_item<T> >::iterator beg = cache.begin();
	typename std::vector<image::cache_item<T> >::iterator end = cache.end();

	for(; beg != end; ++beg) {
		beg->loaded = false;
		beg->item = T();
	}
}
}

namespace image {

mini_terrain_cache_map mini_terrain_cache;

void flush_cache()
{
	reset_cache(images_);
	reset_cache(scaled_images_);
	reset_cache(unmasked_images_);
	reset_cache(greyed_images_);
	reset_cache(darkened_images_);
	reset_cache(brightened_images_);
	reset_cache(semi_brightened_images_);
	reset_cache(alternative_images_);
	mini_terrain_cache.clear();
	reversed_images_.clear();
}

int locator::last_index_ = 0;

void locator::init_index()
{
	locator_finder_t::iterator i = locator_finder.find(val_);

	if(i == locator_finder.end()) {
		index_ = last_index_++;
		locator_finder.insert(locator_finder_pair(val_, index_));

		images_.push_back(cache_item<surface>());
		scaled_images_.push_back(cache_item<surface>());
		unmasked_images_.push_back(cache_item<surface>());
		greyed_images_.push_back(cache_item<surface>());
		darkened_images_.push_back(cache_item<surface>());
		brightened_images_.push_back(cache_item<surface>());
		semi_brightened_images_.push_back(cache_item<surface>());

		alternative_images_.push_back(cache_item<locator>());
	} else {
		index_ = i->second;
	}
}

locator::locator() :
	index_(-1)
{
}

locator::locator(const locator &a):
	index_(a.index_), val_(a.val_)
{
}

locator::locator(const char *filename) :
	val_(filename)
{
	init_index();
}

locator::locator(const std::string &filename) :
	val_(filename)
{
	init_index();
}

locator::locator(const char *filename, color_range new_rgb, std::vector<Uint32> swap_rgb) :
	val_(filename, new_rgb, swap_rgb)
{
	init_index();
}

locator::locator(const std::string &filename, color_range new_rgb, std::vector<Uint32> swap_rgb) :
	val_(filename, new_rgb, swap_rgb)
{
	init_index();
}

locator::locator(const std::string &filename, const gamemap::location &loc, color_range new_rgb, std::vector<Uint32> swap_rgb) :
	val_(filename, loc, new_rgb, swap_rgb)
{
	init_index();
}

locator& locator::operator=(const locator &a)
{
	index_ = a.index_;
	val_ = a.val_;

	return *this;
}

locator::value::value(const locator::value& a) :
  type_(a.type_), filename_(a.filename_), loc_(a.loc_),
  new_color(a.new_color), swap_colors(a.swap_colors)
{
}

locator::value::value() :
	type_(NONE)
{}

locator::value::value(const char *filename) :
  type_(FILE), filename_(filename)
{
}


locator::value::value(const char *filename, color_range new_rgb, std::vector<Uint32> swap_rgb) :
  type_(SUB_FILE), filename_(filename), new_color(new_rgb), swap_colors(swap_rgb)
{
}

locator::value::value(const std::string& filename) :
  type_(FILE), filename_(filename)
{
}

locator::value::value(const std::string& filename, color_range new_rgb, std::vector<Uint32> swap_rgb) :
  type_(SUB_FILE), filename_(filename), new_color(new_rgb), swap_colors(swap_rgb)
{
}

locator::value::value(const std::string& filename, const gamemap::location& loc, color_range new_rgb, std::vector<Uint32> swap_rgb) :
  type_(SUB_FILE), filename_(filename), loc_(loc), new_color(new_rgb), swap_colors(swap_rgb)
{
}

bool locator::value::operator==(const value& a) const
{
	if(a.type_ != type_) {
		return false;
	} else if(type_ == FILE) {
		return filename_ == a.filename_;
	} else if(type_ == SUB_FILE) {
	  return filename_ == a.filename_ && loc_ == a.loc_ && new_color == a.new_color; //note not checking swap_colors purposely
	} else {
		return false;
	}
}

bool locator::value::operator<(const value& a) const
{
	if(type_ != a.type_) {
		return type_ < a.type_;
	} else if(type_ == FILE) {
		return filename_ < a.filename_;
	} else if(type_ == SUB_FILE) {
		if(filename_ != a.filename_)
			return filename_ < a.filename_;
		if(loc_ != a.loc_)
		        return loc_ < a.loc_;
		return(new_color < a.new_color);
	} else {
		return false;
	}
}

surface locator::load_image_file() const
{
	surface res;

	std::string location = get_binary_file_location("images", val_.filename_);

	bool try_units = false;

	do {
		if (!location.empty()) {
#ifdef USE_ZIPIOS
			std::string const &s = read_file(location);
			if (!s.empty()) {
				SDL_RWops* ops = SDL_RWFromMem((void*)s.c_str(), s.size());
				res = IMG_Load_RW(ops, 0);
				SDL_FreeRW(ops);
			}
#else
			res = IMG_Load(location.c_str());
#endif
		}
		if (res.null() && (!try_units)) {
			try_units = true;
			location = get_binary_file_location("images", "units/" + val_.filename_);
		} else {
			try_units = false;
		}
	} while (try_units);

	if (res.null()) {
		ERR_DP << "could not open image '" << val_.filename_ << "'\n";
	}

	return res;
}

surface locator::load_image_sub_file() const
{
	const surface mother_surface(get_image(val_.filename_, UNSCALED, NO_ADJUST_COLOUR));
	const surface mask(get_image(game_config::terrain_mask_image, UNSCALED, NO_ADJUST_COLOUR));

	if(mother_surface == NULL)
		return surface(NULL);
	if(mask == NULL)
		return surface(NULL);

	surface surf=mother_surface;
	if(val_.loc_.x>-1 && val_.loc_.y>-1){
	  SDL_Rect srcrect = {
	    ((tile_size*3) / 4) * val_.loc_.x,
	    tile_size * val_.loc_.y + (tile_size/2) * (val_.loc_.x % 2),
	    tile_size, tile_size
	  };

	  surface tmp(cut_surface(mother_surface, srcrect));
	  surf=mask_surface(tmp, mask);
	}
	if(val_.swap_colors.size()){
	  surf=recolor_image(surf,get_new_color(),get_swap_colors());
	}
	return surf;
}

surface locator::load_from_disk() const
{
	switch(val_.type_) {
		case FILE:
			return load_image_file();
		case SUB_FILE:
			return load_image_sub_file();
		default:
			return surface(NULL);
	}
	wassert(false);
}

#if 0
template<typename T>
bool locator::in_cache(const std::vector<cache_item<T> >& cache) const
{
	if(index_ == -1)
		return false;

	return cache[index_].loaded;
}

template<typename T>
T locator::locate_in_cache(const std::vector<cache_item<T> >& cache) const
{
	if(index_ == -1)
		return T();

	return cache[index_].item;
}

template<typename T>
void locator::add_to_cache(std::vector<cache_item<T> >& cache, const T& item) const
{
	if(index_ == -1)
		return;

	cache[index_] = cache_item<T>(item);
}
#endif

bool locator::in_cache(const std::vector<cache_item<surface> >& cache) const
{
	if(index_ == -1)
		return false;

	return cache[index_].loaded;
}
surface locator::locate_in_cache(const std::vector<cache_item<surface> >& cache) const
{
	if(index_ == -1)
		return surface();

	return cache[index_].item;
}
void locator::add_to_cache(std::vector<cache_item<surface> >& cache, const surface& item) const
{
	if(index_ == -1)
		return;

	cache[index_] = cache_item<surface>(item);
}
bool locator::in_cache(const std::vector<cache_item<locator> >& cache) const
{
	if(index_ == -1)
		return false;

	return cache[index_].loaded;
}
locator locator::locate_in_cache(const std::vector<cache_item<locator> >& cache) const
{
	if(index_ == -1)
		return locator();

	return cache[index_].item;
}
void locator::add_to_cache(std::vector<cache_item<locator> >& cache, const locator& item) const
{
	if(index_ == -1)
		return;

	cache[index_] = cache_item<locator>(item);
}

manager::manager() {}

manager::~manager()
{
	flush_cache();
}

void set_wm_icon()
{
#if !(defined(__APPLE__))
	surface icon(get_image(game_config::game_icon,UNSCALED));
	if(icon != NULL) {
		::SDL_WM_SetIcon(icon,NULL);
	}
#endif
}

SDL_PixelFormat* pixel_format = NULL;

void set_pixel_format(SDL_PixelFormat* format)
{
	pixel_format = format;
	flush_cache();
}

void set_colour_adjustment(int r, int g, int b)
{
	if(r != red_adjust || g != green_adjust || b != blue_adjust) {
		red_adjust = r;
		green_adjust = g;
		blue_adjust = b;
		reset_cache(scaled_images_);
		reset_cache(greyed_images_);
		reset_cache(darkened_images_);
		reset_cache(brightened_images_);
		reset_cache(semi_brightened_images_);
		reset_cache(alternative_images_);
		reversed_images_.clear();
	}
}


void get_colour_adjustment(int *r, int *g, int *b)
{
	if(r != NULL) {
		*r = red_adjust;
	}

	if(g != NULL) {
		*g = green_adjust;
	}

	if(b != NULL) {
		*b = blue_adjust;
	}
}


void set_image_mask(const std::string& image)
{
	if(image_mask != image) {
		image_mask = image;
		reset_cache(scaled_images_);
		reset_cache(greyed_images_);
		reset_cache(darkened_images_);
		reset_cache(brightened_images_);
		reset_cache(semi_brightened_images_);
		reset_cache(alternative_images_);
		reversed_images_.clear();
	}

}

void set_zoom(int amount)
{
	if(amount != zoom) {
		zoom = amount;
		reset_cache(scaled_images_);
		reset_cache(greyed_images_);
		reset_cache(darkened_images_);
		reset_cache(brightened_images_);
		reset_cache(semi_brightened_images_);
		reset_cache(unmasked_images_);
		reset_cache(alternative_images_);
		reversed_images_.clear();
	}
}

surface get_unmasked(const locator i_locator)
{
	surface image(get_image(i_locator, UNSCALED));

	// Re-cut scaled tiles according to a mask. Check if the surface we try
	// to get is not the mask itself, to avoid an infinite loop.
	surface res;
	if(i_locator != locator(game_config::terrain_mask_image)) {
		const surface hex(get_image(game_config::terrain_mask_image,
					UNMASKED, NO_ADJUST_COLOUR));
		res = mask_surface(scale_surface(image, zoom, zoom), hex);
	} else {
		res = scale_surface(image, zoom, zoom);
	}

	return res;
}

surface get_scaled(const locator i_locator, COLOUR_ADJUSTMENT adj)
{
	surface res(get_image(i_locator, UNMASKED, adj));

	// Adjusts colour if necessary.
	if(adj == ADJUST_COLOUR && (red_adjust != 0 ||
				green_adjust != 0 || blue_adjust != 0)) {
		res = surface(adjust_surface_colour(res,
					red_adjust, green_adjust, blue_adjust));
	}

	const surface mask(get_image(image_mask,UNMASKED,NO_ADJUST_COLOUR));
	if(mask != NULL) {
		SDL_SetAlpha(mask,SDL_SRCALPHA|SDL_RLEACCEL,SDL_ALPHA_OPAQUE);
		SDL_SetAlpha(res,SDL_SRCALPHA|SDL_RLEACCEL,SDL_ALPHA_OPAQUE);

		//commented out pending reply from SDL team about bug report
		//SDL_BlitSurface(mask,NULL,result,NULL);
	}
	return res;
}

surface get_greyed(const locator i_locator, COLOUR_ADJUSTMENT adj)
{
	surface image(get_image(i_locator, SCALED, adj));

	return surface(greyscale_image(image));
}

surface get_darkened(const locator i_locator, COLOUR_ADJUSTMENT adj)
{
	surface image(get_image(i_locator, SCALED, adj));

	return surface(darken_image(image));
}

surface get_brightened(const locator i_locator, COLOUR_ADJUSTMENT adj)
{
	surface image(get_image(i_locator, SCALED, adj));
	return surface(brighten_image(image, ftofxp(1.5)));
}

surface get_semi_brightened(const locator i_locator, COLOUR_ADJUSTMENT adj)
{
	surface image(get_image(i_locator, SCALED, adj));
	return surface(brighten_image(image, ftofxp(1.25)));
}

surface get_image(const image::locator& i_locator, TYPE type, COLOUR_ADJUSTMENT adj)
{
	surface res(NULL);
	image_cache *imap;

	if(i_locator.is_void())
		return surface(NULL);

	switch(type) {
	case UNSCALED:
		imap = &images_;
		break;
	case SCALED:
		imap = &scaled_images_;
		break;
	case UNMASKED:
		imap = &unmasked_images_;
		break;
	case GREYED:
		imap = &greyed_images_;
		break;
	case DARKENED:
		imap = &darkened_images_;
		break;
	case BRIGHTENED:
		imap = &brightened_images_;
		break;
	case SEMI_BRIGHTENED:
		imap = &semi_brightened_images_;
		break;
	default:
		return surface(NULL);
	}

	if(i_locator.in_cache(*imap))
		return i_locator.locate_in_cache(*imap);

	// If type is unscaled, directly load the image from the disk. Else,
	// create it from the unscaled image
	if(type == UNSCALED) {
		res = i_locator.load_from_disk();

		if(res == NULL) {
			i_locator.add_to_cache(*imap, surface(NULL));
			return surface(NULL);
		}
	} else {

		// surface base_image(get_image(i_locator, UNSCALED));

		switch(type) {
		case SCALED:
			res = get_scaled(i_locator, adj);
			break;
		case UNMASKED:
			res = get_unmasked(i_locator);
			break;
		case GREYED:
			res = get_greyed(i_locator, adj);
			break;
		case DARKENED:
			res = get_darkened(i_locator, adj);
			break;
		case BRIGHTENED:
			res = get_brightened(i_locator, adj);
			break;
		case SEMI_BRIGHTENED:
			res = get_semi_brightened(i_locator, adj);
			break;
		default:
			return surface(NULL);
		}
	}

	// optimizes surface before storing it
	res = create_optimized_surface(res);
	i_locator.add_to_cache(*imap, res);
	return res;
}

surface get_image_dim(const image::locator& i_locator, size_t x, size_t y)
{
	const surface surf(get_image(i_locator,UNSCALED));

	if(surf != NULL && (size_t(surf->w) != x || size_t(surf->h) != y)) {
		const surface new_image(scale_surface(surf,x,y));

		i_locator.add_to_cache(images_, new_image);
		return new_image;
	}

	return surf;
}

surface reverse_image(const surface& surf)
{
	if(surf == NULL) {
		return surface(NULL);
	}

	const std::map<surface,surface>::iterator itor = reversed_images_.find(surf);
	if(itor != reversed_images_.end()) {
		// sdl_add_ref(itor->second);
		return itor->second;
	}

	const surface rev(flip_surface(surf));
	if(rev == NULL) {
		return surface(NULL);
	}

	reversed_images_.insert(std::pair<surface,surface>(surf,rev));
	// sdl_add_ref(rev);
	return rev;
}

locator get_alternative(const image::locator &i_locator, const std::string &alt)
{
	if(i_locator.is_void())
		return locator();

	if(i_locator.in_cache(alternative_images_))
		return i_locator.locate_in_cache(alternative_images_);

	const std::string &name = i_locator.get_filename();
	const std::string::size_type pos = name.rfind('.');
	const std::string alternative = (pos != 0 ? name.substr(0, pos) : "") +
		alt + (pos != std::string::npos ? name.substr(pos) : "");
	locator res;

	switch (i_locator.get_type()) {
	case locator::FILE:
		res = locator(alternative);
		break;
	case locator::SUB_FILE:
		res = locator(alternative, i_locator.get_loc(), i_locator.get_new_color(), i_locator.get_swap_colors());
		break;
	default:
		wassert(false);
	}

	i_locator.add_to_cache(alternative_images_, res);

	return res;
}



void register_image(const image::locator& id, const surface& surf)
{
	id.add_to_cache(images_, surf);
}

bool exists(const image::locator& i_locator)
{
	typedef image::locator loc;
	loc::type type = i_locator.get_type();
	if (type != loc::FILE && type != loc::SUB_FILE)
		return false;

	// the insertion will fail if there is already an element in the cache
	std::pair< std::map< image::locator, bool >::iterator, bool >
		it = image_existance_map.insert(std::make_pair(i_locator, false));
	bool &cache = it.first->second;
	if (it.second)
		cache = !get_binary_file_location("images", i_locator.get_filename()).empty();
	return cache;
}


}
