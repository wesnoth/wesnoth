#include "game_config.hpp"
#include "image.hpp"
#include "sdl_utils.hpp"

#include "SDL_image.h"

#include <iostream>
#include <map>
#include <string>

namespace {

typedef std::map<std::string,SDL_Surface*> image_map;
image_map images_,scaledImages_,greyedImages_,brightenedImages_,foggedImages_;

int red_adjust = 0, green_adjust = 0, blue_adjust = 0;

SDL_PixelFormat* pixel_format = NULL;

double zoom = 70.0;

void clear_surfaces(image_map& surfaces)
{
	image_map::iterator beg = surfaces.begin();
	const image_map::iterator end = surfaces.end();
	for(; beg != end; ++beg) {
		SDL_FreeSurface(beg->second);
	}

	surfaces.clear();
}

enum TINT { GREY_IMAGE, BRIGHTEN_IMAGE };

SDL_Surface* get_tinted(const std::string& filename, TINT tint)
{
	image_map& images = tint == GREY_IMAGE ? greyedImages_ : brightenedImages_;

	const image_map::iterator itor = images.find(filename);

	if(itor != images.end())
		return itor->second;

	SDL_Surface* const base = image::get_image(filename,image::SCALED);
	if(base == NULL)
		return NULL;

	SDL_Surface* const surface =
		           SDL_CreateRGBSurface(SDL_SWSURFACE,base->w,base->h,
					                     base->format->BitsPerPixel,
										 base->format->Rmask,
										 base->format->Gmask,
										 base->format->Bmask,
										 base->format->Amask);

	images.insert(std::pair<std::string,SDL_Surface*>(filename,surface));

	surface_lock srclock(base);
	surface_lock dstlock(surface);
	short* begin = srclock.pixels();
	const short* const end = begin + base->h*(base->w + (base->w%2));
	short* dest = dstlock.pixels();

	const int rmax = 0xFF;
	const int gmax = 0xFF;
	const int bmax = 0xFF;

	while(begin != end) {
		Uint8 red, green, blue;
		SDL_GetRGB(*begin,base->format,&red,&green,&blue);
		int r = int(red), g = int(green), b = int(blue);

		if(tint == GREY_IMAGE) {
			const double greyscale = (double(r)/(double)rmax +
					                  double(g)/(double)gmax +
							          double(b)/(double)bmax)/3.0;

			r = int(rmax*greyscale);
			g = int(gmax*greyscale);
			b = int(bmax*greyscale);
		} else {
			r = int(double(r)*1.5);
			g = int(double(g)*1.5);
			b = int(double(b)*1.5);

			if(r > rmax)
				r = rmax;

			if(g > gmax)
				g = gmax;

			if(b > bmax)
				b = bmax;
		}

		*dest = SDL_MapRGB(base->format,r,g,b);

		++dest;
		++begin;
	}

	return surface;
}

void flush_cache()
{
	clear_surfaces(images_);
	clear_surfaces(scaledImages_);
	clear_surfaces(foggedImages_);
	clear_surfaces(greyedImages_);
	clear_surfaces(brightenedImages_);
}

}

namespace image {

manager::manager() {}

manager::~manager()
{
	flush_cache();
}

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
		clear_surfaces(scaledImages_);
		clear_surfaces(foggedImages_);
		clear_surfaces(greyedImages_);
		clear_surfaces(brightenedImages_);
	}
}

void set_zoom(double amount)
{
	if(amount != zoom) {
		zoom = amount;
		clear_surfaces(scaledImages_);
		clear_surfaces(foggedImages_);
		clear_surfaces(greyedImages_);
		clear_surfaces(brightenedImages_);
	}
}

SDL_Surface* get_image(const std::string& filename,TYPE type)
{
	if(pixel_format == NULL) {
		std::cerr << "pixel format uninitialized\n";
		return NULL;
	}

	if(type == GREYED) {
		return get_tinted(filename,GREY_IMAGE);
	} else if(type == BRIGHTENED) {
		return get_tinted(filename,BRIGHTEN_IMAGE);
	} else if(type == FOGGED) {
		const image_map::iterator i = foggedImages_.find(filename);
		if(i != foggedImages_.end())
			return i->second;

		SDL_Surface* const surf = get_image(filename,SCALED);
		if(surf == NULL)
			return NULL;

		SDL_Surface* const image = scale_surface(surf,surf->w,surf->h);
		adjust_surface_colour(image,-50,-50,-50);
		foggedImages_.insert(std::pair<std::string,SDL_Surface*>(filename,
		                                                         image));
		return image;
	}

	image_map::iterator i;

	if(type == SCALED) {
		i = scaledImages_.find(filename);
		if(i != scaledImages_.end())
			return i->second;
	}

	i = images_.find(filename);

	if(i == images_.end()) {
		const std::string images_path = "images/";
		const std::string images_filename = images_path + filename;
		SDL_Surface* surf = NULL;

		if(game_config::path.empty() == false) {
			const std::string& fullpath = game_config::path + "/" +
			                              images_filename;
			surf = IMG_Load(fullpath.c_str());
		}

		if(surf == NULL)
			surf = IMG_Load(images_filename.c_str());

		if(surf == NULL) {
			images_.insert(std::pair<std::string,SDL_Surface*>(filename,NULL));
			return NULL;
		}

		SDL_Surface* const conv = SDL_ConvertSurface(surf,
						                     pixel_format,SDL_SWSURFACE);
		SDL_FreeSurface(surf);

		i = images_.insert(std::pair<std::string,SDL_Surface*>(filename,conv))
				               .first;
	}

	if(i->second == NULL)
		return NULL;

	if(type == UNSCALED) {
		return i->second;
	}

	const int z = static_cast<int>(zoom);
	SDL_Surface* const new_surface = scale_surface(i->second,z,z);

	if(new_surface == NULL)
		return NULL;

	adjust_surface_colour(new_surface,red_adjust,green_adjust,blue_adjust);

	scaledImages_.insert(std::pair<std::string,SDL_Surface*>(filename,
								                             new_surface));
	return new_surface;
}

SDL_Surface* get_image_dim(const std::string& filename, size_t x, size_t y)
{
	SDL_Surface* const surf = get_image(filename,UNSCALED);
	if(surf != NULL && (surf->w != x || surf->h != y)) {
		SDL_Surface* const new_image = scale_surface(surf,x,y);
		images_.erase(filename);
		SDL_FreeSurface(surf);
		images_[filename] = new_image;
		return new_image;
	}

	return surf;
}


}