/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef SDL_UTILS_INCLUDED
#define SDL_UTILS_INCLUDED

#include "config.hpp"
#include "scoped_resource.hpp"

#include "SDL.h"

#include <cstdlib>
#include <string>

struct free_sdl_surface {
	void operator()(SDL_Surface* surface) const { SDL_FreeSurface(surface); }
};

typedef util::scoped_resource<SDL_Surface*,free_sdl_surface> scoped_sdl_surface;

SDL_Surface* scale_surface(SDL_Surface* surface, int w, int h);

SDL_Surface* get_surface_portion(SDL_Surface* src, SDL_Rect& rect);

struct pixel_data
{
	pixel_data() : r(0), g(0), b(0)
	{}

	pixel_data(int red, int green, int blue) : r(red), g(green), b(blue)
	{}

	pixel_data(int pixel, SDL_PixelFormat* fmt) {
		unformat(pixel, fmt);
	}

	pixel_data(config& cfg) {
		read(cfg);
	}
	
	int format(SDL_PixelFormat* fmt) const {
		return SDL_MapRGB(fmt,r,g,b);
	}

	void unformat(int pixel, SDL_PixelFormat* fmt) {
		r = ((pixel&fmt->Rmask) >> fmt->Rshift);
		g = ((pixel&fmt->Gmask) >> fmt->Gshift);
		b = ((pixel&fmt->Bmask) >> fmt->Bshift);
	}
	
	void read(config& cfg) {
		const std::string& red = cfg.values["red"];
		const std::string& green = cfg.values["green"];
		const std::string& blue = cfg.values["blue"];

		if(red.empty())
			r = 0;
		else
			r = atoi(red.c_str());

		if(green.empty())
			g = 0;
		else
			g = atoi(green.c_str());

		if(blue.empty())
			b = 0;
		else
			b = atoi(blue.c_str());
	}
	
	int r, g, b;
};

#endif
