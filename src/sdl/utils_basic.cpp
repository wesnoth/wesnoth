/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Basic support routines for the SDL-graphics-library.
 */

#include "sdl/utils.hpp"

#include <cassert>
#include <iostream>

surface_lock::surface_lock(surface &surf) : surface_(surf), locked_(false)
{
	if (SDL_MUSTLOCK(surface_))
		locked_ = SDL_LockSurface(surface_) == 0;
}

surface_lock::~surface_lock()
{
	if (locked_)
		SDL_UnlockSurface(surface_);
}

const_surface_lock::const_surface_lock(const surface &surf) : surface_(surf), locked_(false)
{
	if (SDL_MUSTLOCK(surface_))
		locked_ = SDL_LockSurface(surface_) == 0;
}

const_surface_lock::~const_surface_lock()
{
	if (locked_)
		SDL_UnlockSurface(surface_);
}

static SDL_PixelFormat& get_neutral_pixel_format()
{
	static bool first_time = true;
	static SDL_PixelFormat format;

	if(first_time) {
		first_time = false;
		surface surf(SDL_CreateRGBSurface(SDL_SWSURFACE,1,1,32,0xFF0000,0xFF00,0xFF,0xFF000000));
		format = *surf->format;
		format.palette = nullptr;
	}

	return format;
}

surface make_neutral_surface(const surface &surf)
{
	if(surf == nullptr) {
		std::cerr << "null neutral surface...\n";
		return nullptr;
	}

	surface result = SDL_ConvertSurface(surf,&get_neutral_pixel_format(),SDL_SWSURFACE);
	if(result != nullptr) {
		adjust_surface_alpha(result, SDL_ALPHA_OPAQUE);
	}

	return result;
}

surface create_neutral_surface(int w, int h)
{
	if (w < 0 || h < 0) {
		std::cerr << "error : neutral surface with negative dimensions\n";
		return nullptr;
	}

	SDL_PixelFormat format = get_neutral_pixel_format();
	surface result = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
			format.BitsPerPixel,
			format.Rmask,
			format.Gmask,
			format.Bmask,
			format.Amask);

	return result;
}

// NOTE: Don't pass this function 0 scaling arguments.
surface scale_surface(const surface &surf, int w, int h)
{
	return scale_surface(surf, w, h, true);
}

surface scale_surface(const surface &surf, int w, int h, bool optimize)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if(surf == nullptr)
		return nullptr;

	if(w == surf->w && h == surf->h) {
		return surf;
	}
	assert(w >= 0);
	assert(h >= 0);

	surface dst(create_neutral_surface(w,h));

	if (w == 0 || h ==0) {
		std::cerr << "Create an empty image\n";
		return dst;
	}

	surface src(make_neutral_surface(surf));
	// Now both surfaces are always in the "neutral" pixel format

	if(src == nullptr || dst == nullptr) {
		std::cerr << "Could not create surface to scale onto\n";
		return nullptr;
	}

	{
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		const Uint32* const src_pixels = src_lock.pixels();
		Uint32* const dst_pixels = dst_lock.pixels();

		fixed_t xratio = fxpdiv(surf->w,w);
		fixed_t yratio = fxpdiv(surf->h,h);

		fixed_t ysrc = ftofxp(0.0);
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			fixed_t xsrc = ftofxp(0.0);
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				const int xsrcint = fxptoi(xsrc);
				const int ysrcint = fxptoi(ysrc);

				const Uint32* const src_word = src_pixels + ysrcint*src->w + xsrcint;
				Uint32* const dst_word = dst_pixels +    ydst*dst->w + xdst;
				const int dx = (xsrcint + 1 < src->w) ? 1 : 0;
				const int dy = (ysrcint + 1 < src->h) ? src->w : 0;

				Uint8 r,g,b,a;
				Uint32 rr,gg,bb,aa, temp;

				Uint32 pix[4], bilin[4];

				// This next part is the fixed point
				// equivalent of "take everything to
				// the right of the decimal point."
				// These fundamental weights decide
				// the contributions from various
				// input pixels. The labels assume
				// that the upper left corner of the
				// screen ("northeast") is 0,0 but the
				// code should still be consistent if
				// the graphics origin is actually
				// somewhere else.
				//
				// That is, the bilin array holds the
				// "geometric" weights. I.E. If I'm scaling
				// a 2 x 2 block a 10 x 10 block, then for
				// pixel (2,2) of ouptut, the upper left
				// pixel should be 10:1 more influential than
				// the upper right, and also 10:1 more influential
				// than lower left, and 100:1 more influential
				// than lower right.

				const fixed_t e = 0x000000FF & xsrc;
				const fixed_t s = 0x000000FF & ysrc;
				const fixed_t n = 0xFF - s;
				// Not called "w" to avoid hiding a function parameter
				// (would cause a compiler warning in MSVC2015 with /W4)
				const fixed_t we = 0xFF - e;

				pix[0] = *src_word;              // northwest
				pix[1] = *(src_word + dx);       // northeast
				pix[2] = *(src_word + dy);       // southwest
				pix[3] = *(src_word + dx + dy);  // southeast

				bilin[0] = n*we;
				bilin[1] = n*e;
				bilin[2] = s*we;
				bilin[3] = s*e;

				int loc;
				rr = bb = gg = aa = 0;
				for (loc=0; loc<4; loc++) {
				  a = pix[loc] >> 24;
				  r = pix[loc] >> 16;
				  g = pix[loc] >> 8;
				  b = pix[loc] >> 0;

				  //We also have to implement weighting by alpha for the RGB components
				  //If a unit has some parts solid and some parts translucent,
				  //i.e. a red cloak but a dark shadow, then when we scale in
				  //the shadow shouldn't appear to become red at the edges.
				  //This part also smoothly interpolates between alpha=0 being
				  //transparent and having no contribution, vs being opaque.
				  temp = (a * bilin[loc]);
				  rr += r * temp;
				  gg += g * temp;
				  bb += b * temp;
				  aa += temp;
				}

				a = aa >> (16); // we average the alphas, they don't get weighted by any other factor besides bilin
				if (a != 0) {
					rr /= a;	// finish alpha weighting: divide by sum of alphas
					gg /= a;
					bb /= a;
				}
				r = rr >> (16); // now shift over by 16 for the bilin part
				g = gg >> (16);
				b = bb >> (16);
				*dst_word = (a << 24) + (r << 16) + (g << 8) + b;
			}
		}
	}

	if(optimize) {
		adjust_surface_alpha(dst, SDL_ALPHA_OPAQUE);
	}

	return dst;
}

surface brighten_image(const surface &surf, fixed_t amount, bool optimize)
{
	if(surf == nullptr) {
		return nullptr;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		if (amount < 0) amount = 0;
		while(beg != end) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				r = std::min<unsigned>(unsigned(fxpmult(r, amount)),255);
				g = std::min<unsigned>(unsigned(fxpmult(g, amount)),255);
				b = std::min<unsigned>(unsigned(fxpmult(b, amount)),255);

				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}

	if(optimize) {
		adjust_surface_alpha(nsurf, SDL_ALPHA_OPAQUE);
	}

	return nsurf;
}

void adjust_surface_alpha(surface& surf, fixed_t amount)
{
	if(surf == nullptr) {
		return;
	}

	SDL_SetSurfaceAlphaMod(surf, Uint8(amount));
}

surface cut_surface(const surface &surf, SDL_Rect const &r)
{
	if(surf == nullptr)
		return nullptr;

	surface res = create_compatible_surface(surf, r.w, r.h);

	if(res == nullptr) {
		std::cerr << "Could not create a new surface in cut_surface()\n";
		return nullptr;
	}

	size_t sbpp = surf->format->BytesPerPixel;
	size_t spitch = surf->pitch;
	size_t rbpp = res->format->BytesPerPixel;
	size_t rpitch = res->pitch;

	// compute the areas to copy
	SDL_Rect src_rect = r;
	SDL_Rect dst_rect = { 0, 0, r.w, r.h };

	if (src_rect.x < 0) {
		if (src_rect.x + src_rect.w <= 0)
			return res;
		dst_rect.x -= src_rect.x;
		dst_rect.w += src_rect.x;
		src_rect.w += src_rect.x;
		src_rect.x = 0;
	}
	if (src_rect.y < 0) {
		if (src_rect.y + src_rect.h <= 0)
			return res;
		dst_rect.y -= src_rect.y;
		dst_rect.h += src_rect.y;
		src_rect.h += src_rect.y;
		src_rect.y = 0;
	}

	if(src_rect.x >= surf->w || src_rect.y >= surf->h)
		return res;

	const_surface_lock slock(surf);
	surface_lock rlock(res);

	const Uint8* src = reinterpret_cast<const Uint8 *>(slock.pixels());
	Uint8* dest = reinterpret_cast<Uint8 *>(rlock.pixels());

	for(int y = 0; y < src_rect.h && (src_rect.y + y) < surf->h; ++y) {
		const Uint8* line_src  = src  + (src_rect.y + y) * spitch + src_rect.x * sbpp;
		Uint8* line_dest = dest + (dst_rect.y + y) * rpitch + dst_rect.x * rbpp;
		size_t size = src_rect.w + src_rect.x <= surf->w ? src_rect.w : surf->w - src_rect.x;

		assert(rpitch >= src_rect.w * rbpp);
		memcpy(line_dest, line_src, size * rbpp);
	}

	return res;
}

surface blend_surface(
		  const surface &surf
		, const double amount
		, const Uint32 color
		, const bool optimize)
{
	if(surf== nullptr) {
		return nullptr;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		Uint16 ratio = amount * 256;
		const Uint16 red   = ratio * static_cast<Uint8>(color >> 16);
		const Uint16 green = ratio * static_cast<Uint8>(color >> 8);
		const Uint16 blue  = ratio * static_cast<Uint8>(color);
		ratio = 256 - ratio;

		while(beg != end) {
			Uint8 a = static_cast<Uint8>(*beg >> 24);
			Uint8 r = (ratio * static_cast<Uint8>(*beg >> 16) + red)   >> 8;
			Uint8 g = (ratio * static_cast<Uint8>(*beg >> 8)  + green) >> 8;
			Uint8 b = (ratio * static_cast<Uint8>(*beg)       + blue)  >> 8;

			*beg = (a << 24) | (r << 16) | (g << 8) | b;

			++beg;
		}
	}

	if(optimize) {
		adjust_surface_alpha(nsurf, SDL_ALPHA_OPAQUE);
	}

	return nsurf;
}

surface create_compatible_surface(const surface &surf, int width, int height)
{
	if(surf == nullptr)
		return nullptr;

	if(width == -1)
		width = surf->w;

	if(height == -1)
		height = surf->h;

	surface s = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, surf->format->BitsPerPixel,
		surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask);
	if (surf->format->palette) {
		SDL_SetPaletteColors(s->format->palette, surf->format->palette->colors, 0, surf->format->palette->ncolors);
	}
	return s;
}
