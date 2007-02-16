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
#include "team.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "wesconfig.h"
#include "serialization/string_utils.hpp"

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
image::image_cache images_,scaled_images_,unmasked_images_;
image::image_cache brightened_images_,semi_brightened_images_;

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
		brightened_images_.push_back(cache_item<surface>());
		semi_brightened_images_.push_back(cache_item<surface>());

		alternative_images_.push_back(cache_item<locator>());
	} else {
		index_ = i->second;
	}
}

void locator::parse_arguments()
{
	std::string& fn = val_.filename_;
	if(fn.empty()) {
		return;
	}
	size_t markup_field = fn.find('~');
 
	if(markup_field != std::string::npos) {
		val_.type_ = SUB_FILE;
		val_.modifications_ = fn.substr(markup_field, fn.size() - markup_field);
		fn = fn.substr(0,markup_field);
	}
}

locator::locator() :
	index_(-1)
{
}

locator::locator(const locator &a, const std::string& mods):
	 val_(a.val_)
{
	if(mods.size()){
			val_.modifications_ += mods;
			val_.type_=SUB_FILE;
			init_index();
	}
	else index_=a.index_;
}

locator::locator(const char *filename) :
	val_(filename)
{
	parse_arguments();
	init_index();
}

locator::locator(const std::string &filename) :
	val_(filename)
{
	parse_arguments();
	init_index();
}

locator::locator(const char *filename, const std::string& modifications) :
	val_(filename, modifications)
{
	init_index();
}

locator::locator(const std::string &filename, const std::string& modifications) :
	val_(filename, modifications)
{
	init_index();
}

locator::locator(const std::string &filename, const gamemap::location &loc, const std::string& modifications) :
	val_(filename, loc, modifications)
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
  modifications_(a.modifications_)
{
}

locator::value::value() :
	type_(NONE)
{}

locator::value::value(const char *filename) :
  type_(FILE), filename_(filename)
{
}


locator::value::value(const char *filename, const std::string& modifications) :
  type_(SUB_FILE), filename_(filename), modifications_(modifications)
{
}

locator::value::value(const std::string& filename) :
  type_(FILE), filename_(filename)
{
}

locator::value::value(const std::string& filename, const std::string& modifications) :
  type_(SUB_FILE), filename_(filename), modifications_(modifications)
{
}

locator::value::value(const std::string& filename, const gamemap::location& loc, const std::string& modifications) :
  type_(SUB_FILE), filename_(filename), loc_(loc), modifications_(modifications)
{
}

bool locator::value::operator==(const value& a) const
{
	if(a.type_ != type_) {
		return false;
	} else if(type_ == FILE) {
		return filename_ == a.filename_;
	} else if(type_ == SUB_FILE) {
	  return filename_ == a.filename_ && loc_ == a.loc_ && modifications_ == a.modifications_; 
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
		return(modifications_ < a.modifications_);
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

	if(val_.modifications_.size()){
		bool xflip = false;
		bool yflip = false;
		std::map<Uint32, Uint32> recolor_map;
		std::vector<std::string> modlist = utils::paranthetical_split(val_.modifications_,'~');
		for(std::vector<std::string>::const_iterator i=modlist.begin();
			i!= modlist.end();i++){
			std::vector<std::string> tmpmod = utils::paranthetical_split(*i);
			std::vector<std::string>::const_iterator j=tmpmod.begin();
			while(j!= tmpmod.end()){
				std::string function=*j++;
				if(j==tmpmod.end()){
					if(function.size()){
						ERR_DP << "error parsing image modifications: " 
							<< val_.modifications_<< "\n";
					}
					break;
				}
				std::string field = *j++;
				if("TC" == function){//deprecated team coloring syntax
					//replace with proper RC syntax
					std::string::size_type pos = 0;
					pos = field.find(',');
					if (pos == std::string::npos)
						break;
					std::string f1,f2;
					int side_n = lexical_cast_default<int>(field.substr(0,pos),-1);
					if (side_n < 0)
						break;
					f1 = team::get_side_colour_index(side_n);
					f2 = field.substr(pos+1);
					if(game_config::tc_info(f2).size()){
						function="RC";
						field= f2 + ">" + f1;
					}						
				}
				if("RC" == function){ //re-color function
					std::vector<std::string> recolor=utils::split(field,'>');
					if(recolor.size()>1){
						std::map<std::string, color_range>::const_iterator nc = game_config::team_rgb_range.find(recolor[1]);
						if(nc != game_config::team_rgb_range.end()){
							color_range new_color = nc->second;
							std::vector<Uint32> old_color = game_config::tc_info(recolor[0]);
							std::map<Uint32, Uint32> tmp_map = recolor_range(new_color,old_color);
							for(std::map<Uint32, Uint32>::const_iterator tmp = tmp_map.begin(); tmp!= tmp_map.end(); tmp++){	
								recolor_map[tmp->first] = tmp->second;
							}
						}	 
					}
				}									
				if("FL" == function){ //flip layer
					if(field.empty() || field.find("horiz") != std::string::npos) {
						xflip = !xflip;
					}
					if(field.find("vert") != std::string::npos) {
						yflip = !yflip;
					}
				}									
			} 			
		}
		surf = recolor_image(surf,recolor_map);
		if(xflip) {
			surf = flip_surface(surf);
		}
		if(yflip) {
			surf = flop_surface(surf);
		}
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
		reset_cache(brightened_images_);
		reset_cache(semi_brightened_images_);
		reset_cache(alternative_images_);
		reversed_images_.clear();
	}
}

void set_image_mask(const std::string& image)
{
	if(image_mask != image) {
		image_mask = image;
		reset_cache(scaled_images_);
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

surface get_image(const image::locator& i_locator, TYPE type, COLOUR_ADJUSTMENT adj,bool add_to_cache )
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
			if(add_to_cache) i_locator.add_to_cache(*imap, surface(NULL));
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
	if(add_to_cache) i_locator.add_to_cache(*imap, res);
	return res;
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
