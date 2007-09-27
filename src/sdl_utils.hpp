/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file sdl_utils.hpp

#ifndef SDL_UTILS_INCLUDED
#define SDL_UTILS_INCLUDED

#include "scoped_resource.hpp"
#include "util.hpp"

#include "SDL.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

//older versions of SDL don't define the
//mouse wheel macros, so define them ourselves
//if necessary.
#ifndef SDL_BUTTON_WHEELUP
#define SDL_BUTTON_WHEELUP 4
#endif

#ifndef SDL_BUTTON_WHEELDOWN
#define SDL_BUTTON_WHEELDOWN 5
#endif

#ifndef SDL_BUTTON_WHEELLEFT
#define SDL_BUTTON_WHEELLEFT 6
#endif

#ifndef SDL_BUTTON_WHEELRIGHT
#define SDL_BUTTON_WHEELRIGHT 7
#endif

namespace {
const SDL_Rect empty_rect = { 0, 0, 0, 0 };
}

SDLKey sdl_keysym_from_name(std::string const &keyname);

bool point_in_rect(int x, int y, const SDL_Rect& rect);
bool rects_overlap(const SDL_Rect& rect1, const SDL_Rect& rect2);
SDL_Rect intersect_rects(SDL_Rect const &rect1, SDL_Rect const &rect2);

struct surface
{
private:
	static void sdl_add_ref(SDL_Surface *surf)
	{
		if (surf != NULL)
			++surf->refcount;
	}

	struct free_sdl_surface {
		void operator()(SDL_Surface *surf) const
		{
			if (surf != NULL)
				 SDL_FreeSurface(surf);
		}
	};

	typedef util::scoped_resource<SDL_Surface*,free_sdl_surface> scoped_sdl_surface;
public:
	surface() : surface_(NULL)
	{}

	surface(SDL_Surface *surf) : surface_(surf)
	{}

	surface(const surface& o) : surface_(o.surface_.get())
	{
		sdl_add_ref(surface_.get());
	}

	void assign(const surface& o)
	{
		SDL_Surface *surf = o.surface_.get();
		sdl_add_ref(surf); // need to be done before assign to avoid corruption on "a=a;"
		surface_.assign(surf);
	}

	surface& operator=(const surface& o)
	{
		assign(o);
		return *this;
	}

	operator SDL_Surface*() const { return surface_.get(); }

	SDL_Surface* get() const { return surface_.get(); }

	SDL_Surface* operator->() const { return surface_.get(); }

	void assign(SDL_Surface* surf) { surface_.assign(surf); }

	bool null() const { return surface_.get() == NULL; }

private:
	scoped_sdl_surface surface_;
};

bool operator<(const surface& a, const surface& b);

surface make_neutral_surface(surface const &surf);
surface create_optimized_surface(surface const &surf);
surface scale_surface(surface const &surf, int w, int h);
surface scale_surface_blended(surface const &surf, int w, int h);
surface adjust_surface_colour(surface const &surf, int r, int g, int b);
surface greyscale_image(surface const &surf);
surface darken_image(surface const &surf);
surface recolor_image(surface surf, const std::map<Uint32, Uint32>& map_rgb);

surface brighten_image(surface const &surf, fixed_t amount);
// send NULL if the portion is outside of the surface
surface get_surface_portion(surface const &surf, SDL_Rect &rect);
surface adjust_surface_alpha(surface const &surf, fixed_t amount, bool optimize=true);
surface adjust_surface_alpha_add(surface const &surf, int amount);
surface mask_surface(surface const &surf, surface const &mask);
surface blur_surface(surface const &surf, int depth = 1);
surface blur_alpha_surface(surface const &surf, int depth = 1);
surface cut_surface(surface const &surf, SDL_Rect const &r);
surface blend_surface(surface const &surf, double amount, Uint32 colour);
surface flip_surface(surface const &surf);
surface flop_surface(surface const &surf);
surface create_compatible_surface(surface const &surf, int width = -1, int height = -1);

void fill_rect_alpha(SDL_Rect &rect, Uint32 colour, Uint8 alpha, surface const &target);

SDL_Rect get_non_transparent_portion(surface const &surf);

bool operator==(const SDL_Rect& a, const SDL_Rect& b);
bool operator!=(const SDL_Rect& a, const SDL_Rect& b);
bool operator==(const SDL_Color& a, const SDL_Color& b);
bool operator!=(const SDL_Color& a, const SDL_Color& b);
SDL_Color inverse(const SDL_Color& colour);
SDL_Color int_to_color(const Uint32 rgb);
Uint32 color_to_int(const SDL_Color& rgb);

class config; // no need to include config.hpp

struct pixel_data
{
	pixel_data() : r(0), g(0), b(0)
	{}

	pixel_data(int red, int green, int blue) : r(red), g(green), b(blue)
	{}

	pixel_data(int pixel, SDL_PixelFormat* fmt) : r(0), g(0), b(0) {
		unformat(pixel, fmt);
	}

	pixel_data(config& cfg) : r(0), g(0), b(0) {
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

	void read(const config& cfg);

	int r, g, b;
};

struct surface_lock
{
	surface_lock(surface const &surf) : surface_(surf), locked_(false)
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

	Uint32* pixels() { return reinterpret_cast<Uint32*>(surface_->pixels); }
private:
	surface const surface_;
	bool locked_;
};

struct surface_restorer
{
	surface_restorer();
	surface_restorer(class CVideo* target, const SDL_Rect& rect);
	~surface_restorer();

	void restore() const;
	void restore(SDL_Rect const &dst) const;
	void update();
	void cancel();

	const SDL_Rect& area() const { return rect_; }

private:
	class CVideo* target_;
	SDL_Rect rect_;
	surface surface_;
};

struct clip_rect_setter
{
	clip_rect_setter(surface const &surf, const SDL_Rect& r) : surface_(surf), rect()
	{
		SDL_GetClipRect(surface_,&rect);
		SDL_SetClipRect(surface_,&r);
	}

	~clip_rect_setter() { SDL_SetClipRect(surface_,&rect); }

private:
	surface surface_;
	SDL_Rect rect;
};


void draw_rectangle(int x, int y, int w, int h, Uint32 colour, surface tg);

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
                                 int r, int g, int b,
				 double alpha, surface target);

// blit the image on the center of the rectangle
// and a add a colored background
void draw_centered_on_background(surface surf, const SDL_Rect& rect,
	const SDL_Color& color, surface target);

#endif
