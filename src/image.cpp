#include "game_config.hpp"
#include "image.hpp"
#include "display.hpp"
#include "font.hpp"
#include "sdl_utils.hpp"
#include "team.hpp"
#include "util.hpp"

#include "SDL_image.h"

#include <iostream>
#include <map>
#include <string>

namespace {

typedef std::map<gamemap::TERRAIN,SDL_Surface*> mini_terrain_cache_map;
mini_terrain_cache_map mini_terrain_cache;

typedef std::map<std::string,SDL_Surface*> image_map;
image_map images_,scaledImages_,greyedImages_,brightenedImages_;

int red_adjust = 0, green_adjust = 0, blue_adjust = 0;

SDL_PixelFormat* pixel_format = NULL;

double zoom = 70.0;

//we have to go through all this trickery on clear_surfaces because
//some compilers don't support 'typename type::iterator'
template<typename Map,typename FwIt>
void clear_surfaces_internal(Map& surfaces, FwIt beg, FwIt end)
{
	for(; beg != end; ++beg) {
		SDL_FreeSurface(beg->second);
	}

	surfaces.clear();
}

template<typename Map>
void clear_surfaces(Map& surfaces)
{
	clear_surfaces_internal(surfaces,surfaces.begin(),surfaces.end());
}

enum TINT { GREY_IMAGE, BRIGHTEN_IMAGE };

SDL_Surface* get_tinted(const std::string& filename, TINT tint)
{
	image_map& images = tint == GREY_IMAGE ? greyedImages_ : brightenedImages_;

	const image_map::iterator itor = images.find(filename);

	if(itor != images.end()) {
		return itor->second;
	}

	scoped_sdl_surface base(image::get_image(filename,image::SCALED));
	SDL_Surface* const surface = (tint == GREY_IMAGE ? greyscale_image(base) : brighten_image(base,1.5));

	images.insert(std::pair<std::string,SDL_Surface*>(filename,surface));

	return surface;
}

void flush_cache()
{
	clear_surfaces(images_);
	clear_surfaces(scaledImages_);
	clear_surfaces(greyedImages_);
	clear_surfaces(brightenedImages_);
	clear_surfaces(mini_terrain_cache);
}

}

namespace image {

manager::manager() {}

manager::~manager()
{
	flush_cache();
}

void set_wm_icon()
{
	scoped_sdl_surface icon(get_image(game_config::game_icon,UNSCALED));
	if(icon != NULL) {
		std::cerr << "setting icon...\n";
		::SDL_WM_SetIcon(icon,NULL);
	}
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
		clear_surfaces(greyedImages_);
		clear_surfaces(brightenedImages_);
	}
}

void set_zoom(double amount)
{
	if(amount != zoom) {
		zoom = amount;
		clear_surfaces(scaledImages_);
		clear_surfaces(greyedImages_);
		clear_surfaces(brightenedImages_);
	}
}

SDL_Surface* get_image(const std::string& filename, TYPE type, COLOUR_ADJUSTMENT adjust_colour)
{
	SDL_Surface* result = NULL;
	if(type == GREYED) {
		result = get_tinted(filename,GREY_IMAGE);
	} else if(type == BRIGHTENED) {
		result = get_tinted(filename,BRIGHTEN_IMAGE);
	} else {

		image_map::iterator i;

		if(type == SCALED) {
			i = scaledImages_.find(filename);
			if(i != scaledImages_.end()) {
				result = i->second;
				sdl_add_ref(result);
				return result;
			}
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

			if(surf == NULL) {
				surf = IMG_Load(images_filename.c_str());
			}

			if(surf == NULL) {
				images_.insert(std::pair<std::string,SDL_Surface*>(filename,NULL));
				return NULL;
			}

			if(pixel_format != NULL) {
				SDL_Surface* const conv = display_format_alpha(surf);
				SDL_FreeSurface(surf);
				surf = conv;
			}

			i = images_.insert(std::pair<std::string,SDL_Surface*>(filename,surf)).first;
		}

		if(i->second == NULL)
			return NULL;

		if(type == UNSCALED) {
			result = i->second;
		} else {

			const int z = static_cast<int>(zoom);
			result = scale_surface(i->second,z,z);

			if(result == NULL)
				return NULL;

			if(adjust_colour == ADJUST_COLOUR) {
				const scoped_sdl_surface scoped_surface(result);
				result = adjust_surface_colour(result,red_adjust,green_adjust,blue_adjust);
			}

			scaledImages_.insert(std::pair<std::string,SDL_Surface*>(filename,result));
		}
	}

	if(result != NULL) {
		SDL_SetAlpha(result,SDL_SRCALPHA|SDL_RLEACCEL,SDL_ALPHA_OPAQUE);
	}

	sdl_add_ref(result);
	return result;
}

SDL_Surface* get_image_dim(const std::string& filename, size_t x, size_t y)
{
	SDL_Surface* const surf = get_image(filename,UNSCALED);
	if(surf != NULL && (size_t(surf->w) != x || size_t(surf->h) != y)) {
		SDL_Surface* const new_image = scale_surface(surf,x,y);
		images_.erase(filename);

		//free surf twice: once because calling get_image adds a reference.
		//again because we also want to remove the cache reference, since
		//we're removing it from the cache
		SDL_FreeSurface(surf);
		SDL_FreeSurface(surf);

		images_[filename] = new_image;
		sdl_add_ref(new_image);
		return new_image;
	}

	return surf;
}

SDL_Surface* getMinimap(int w, int h, const gamemap& map, 
		int lawful_bonus,
		const team* tm, const unit_map* units, const std::vector<team>* teams)
{
	SDL_Surface* minimap = NULL;
	if(minimap == NULL) {
		const int scale = 8;

		minimap = SDL_CreateRGBSurface(SDL_SWSURFACE,
		                               map.x()*scale*0.75,map.y()*scale,
		                               pixel_format->BitsPerPixel,
		                               pixel_format->Rmask,
		                               pixel_format->Gmask,
		                               pixel_format->Bmask,
		                               pixel_format->Amask);
		if(minimap == NULL)
			return NULL;

		typedef mini_terrain_cache_map cache_map;
		cache_map& cache = mini_terrain_cache;

		for(int y = 0; y != map.y(); ++y) {
			for(int x = 0; x != map.x(); ++x) {

				SDL_Surface* surf = NULL;
				scoped_sdl_surface scoped_surface(NULL);
				
				const gamemap::location loc(x,y);
				if(map.on_board(loc)) {
					const bool shrouded = tm != NULL && tm->shrouded(x,y);
					const bool fogged = tm != NULL && tm->fogged(x,y) && !shrouded;
					const gamemap::TERRAIN terrain = shrouded ? gamemap::VOID_TERRAIN : map[x][y];
					cache_map::iterator i = cache.find(terrain);

					if(i == cache.end()) {
						scoped_sdl_surface tile(get_image("terrain/" + map.get_terrain_info(terrain).default_image() + ".png",image::UNSCALED));

						if(tile == NULL) {
							std::cerr << "Could not get image for terrrain '"
							          << terrain << "'\n";
							continue;
						}

						surf = scale_surface_blended(tile,scale,scale);

						if(surf == NULL) {
							continue;
						}

						i = cache.insert(cache_map::value_type(terrain,surf)).first;
					} else {
						surf = i->second;
					}

					if(fogged) {
						scoped_surface.assign(adjust_surface_colour(surf,-50,-50,-50));
						surf = scoped_surface;
					}

					assert(surf != NULL);
					
					SDL_Rect maprect = {x*scale*0.75,y*scale + (is_odd(x) ? scale/2 : 0),0,0};
					SDL_BlitSurface(surf, NULL, minimap, &maprect);
				}
			}
		}
	}

	if(minimap->w != w || minimap->h != h) {
		SDL_Surface* const surf = minimap;
		minimap = scale_surface(surf,w,h);
		SDL_FreeSurface(surf);
	}

	return minimap;
}

}