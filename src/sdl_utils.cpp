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

#include <iostream>

#include "game.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"

SDL_Surface* scale_surface(SDL_Surface* surface, int w, int h)
{
	SDL_Surface* const dest = SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,
					                     surface->format->BitsPerPixel,
										 surface->format->Rmask,
										 surface->format->Gmask,
										 surface->format->Bmask,
										 surface->format->Amask);
	if(dest == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const double xratio = static_cast<double>(surface->w)/
			              static_cast<double>(w);
	const double yratio = static_cast<double>(surface->h)/
			              static_cast<double>(h);

	const int srcxpad = is_odd(surface->w);
	const int dstxpad = is_odd(dest->w);

	surface_lock dstlock(dest);
	surface_lock srclock(surface);

	double ysrc = 0.0;
	for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
		double xsrc = 0.0;
		for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
			int xsrcint = static_cast<int>(xsrc);
			const int ysrcint = static_cast<int>(ysrc);

			xsrcint += srcxpad*ysrcint;

			const int dstpad = dstxpad*ydst;

			srclock.pixels()[ydst*w + xdst + dstpad] =
			             dstlock.pixels()[ysrcint*surface->w + xsrcint];
		}
	}

	return dest;
}

SDL_Surface* get_surface_portion(SDL_Surface* src, SDL_Rect& area)
{
	if(area.x + area.w >= src->w || area.y + area.h >= src->h)
		return NULL;

	const SDL_PixelFormat* const fmt = src->format;
	SDL_Surface* const dst = SDL_CreateRGBSurface(0,area.w,area.h,
	                                              fmt->BitsPerPixel,fmt->Rmask,
	                                              fmt->Gmask,fmt->Bmask,
	                                              fmt->Amask);
	SDL_Rect dstarea = {0,0,0,0};

	SDL_BlitSurface(src,&area,dst,&dstarea);

	return dst;
}
