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
#include <iostream>
#include <string>

//older versions of SDL don't define the
//mouse wheel macros, so define them ourselves
//if necessary.
#ifndef SDL_BUTTON_WHEELUP
#define SDL_BUTTON_WHEELUP 4
#endif

#ifndef SDL_BUTTON_WHEELDOWN
#define SDL_BUTTON_WHEELDOWN 5
#endif

struct free_sdl_surface {
	void operator()(SDL_Surface* surface) const { SDL_FreeSurface(surface); }
};

int sdl_add_ref(SDL_Surface* surface);

typedef util::scoped_resource<SDL_Surface*,free_sdl_surface> scoped_sdl_surface;

void draw_unit_ellipse(SDL_Surface* surf, short colour, const SDL_Rect& clip, int unitx, int unity,
                       SDL_Surface* behind, bool image_reverse);

SDL_Surface* clone_surface(SDL_Surface* surface);
SDL_Surface* scale_surface(SDL_Surface* surface, int w, int h);

void adjust_surface_colour(SDL_Surface* surface, int r, int g, int b);

SDL_Surface* get_surface_portion(SDL_Surface* src, SDL_Rect& rect);

SDL_Rect get_non_transperant_portion(const SDL_Surface* surf);

bool operator==(const SDL_Rect& a, const SDL_Rect& b);
bool operator!=(const SDL_Rect& a, const SDL_Rect& b);

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

	void read(const config& cfg) {
		const std::string& red = cfg["red"];
		const std::string& green = cfg["green"];
		const std::string& blue = cfg["blue"];

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

struct surface_lock
{
	surface_lock(SDL_Surface* surface) : surface_(surface), locked_(false)
	{
		if(SDL_MUSTLOCK(surface_)) {
			const int res = SDL_LockSurface(surface_);
			if(res == 0) {
				locked_ = true;
			}
		}
	}

	~surface_lock()
	{
		if(locked_) {
			SDL_UnlockSurface(surface_);
		}
	}

	short* pixels() { return reinterpret_cast<short*>(surface_->pixels); }
private:
	SDL_Surface* const surface_;
	bool locked_;
};

struct shared_sdl_surface
{
	explicit shared_sdl_surface(SDL_Surface* surf) : surface_(surf)
	{}

	explicit shared_sdl_surface(const shared_sdl_surface& o) : surface_(o.surface_.get())
	{
		sdl_add_ref(get());
	}

	shared_sdl_surface& operator=(const shared_sdl_surface& o)
	{
		surface_.assign(o.surface_.get());
		sdl_add_ref(get());
		return *this;
	}

	operator SDL_Surface*() const { return surface_; }

	SDL_Surface* get() const { return surface_.get(); }

	SDL_Surface* operator->() const { return surface_.get(); }

	void assign(SDL_Surface* surf) { surface_.assign(surf); }

private:
	scoped_sdl_surface surface_;
};

struct surface_restorer
{
	surface_restorer();
	surface_restorer(class CVideo* target, const SDL_Rect& rect);
	~surface_restorer();

	void restore();
	void update();
	void cancel();

private:
	class CVideo* target_;
	SDL_Rect rect_;
	shared_sdl_surface surface_;
};

#endif
