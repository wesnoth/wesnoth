/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file sdl_utils.cpp
 *  Support-routines for the SDL-graphics-library.
 */

#include "global.hpp"

#include "video.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

surface_lock::surface_lock(const surface &surf) : surface_(surf), locked_(false)
{
	if (SDL_MUSTLOCK(surface_))
		locked_ = SDL_LockSurface(surface_) == 0;
	// Check that the surface is neutral bpp 32, possibly with an empty alpha channel.
	assert((surface_->flags & SDL_RLEACCEL) == 0 &&
		surface_->format->BytesPerPixel == 4 &&
		surface_->format->Rmask == 0xFF0000u &&
		(surface_->format->Amask | 0xFF000000u) == 0xFF000000u);
}

surface_lock::~surface_lock()
{
	if (locked_)
		SDL_UnlockSurface(surface_);
}

SDL_Color int_to_color(const Uint32 rgb) {
	SDL_Color to_return = {
		(0x00FF0000 & rgb)>>16,
		(0x0000FF00 & rgb)>>8,
		(0x000000FF & rgb), 0
	};
	return to_return;
}

SDLKey sdl_keysym_from_name(std::string const &keyname)
{
	static bool initialized = false;
	typedef std::map<std::string const, SDLKey> keysym_map_t;
	static keysym_map_t keysym_map;

	if (!initialized) {
		for(SDLKey i = SDLK_FIRST; i < SDLK_LAST; i = SDLKey(int(i) + 1)) {
			std::string name = SDL_GetKeyName(i);
			if (!name.empty())
				keysym_map[name] = i;
		}
		initialized = true;
	}

	keysym_map_t::const_iterator it = keysym_map.find(keyname);
	if (it != keysym_map.end())
		return it->second;
	else
		return SDLK_UNKNOWN;
}

bool point_in_rect(int x, int y, const SDL_Rect& rect)
{
	return x >= rect.x && y >= rect.y && x < rect.x + rect.w && y < rect.y + rect.h;
}

bool rects_overlap(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	return (rect1.x < rect2.x+rect2.w && rect2.x < rect1.x+rect1.w &&
			rect1.y < rect2.y+rect2.h && rect2.y < rect1.y+rect1.h);
}

SDL_Rect intersect_rects(SDL_Rect const &rect1, SDL_Rect const &rect2)
{
	SDL_Rect res;
	res.x = std::max<int>(rect1.x, rect2.x);
	res.y = std::max<int>(rect1.y, rect2.y);
	res.w = std::max<int>(std::min<int>(rect1.x + rect1.w, rect2.x + rect2.w) - res.x, 0);
	res.h = std::max<int>(std::min<int>(rect1.y + rect1.h, rect2.y + rect2.h) - res.y, 0);
	return res;
}

SDL_Rect get_rect_union(SDL_Rect const &rect1, SDL_Rect const& rect2) {
	const int left_side = std::max(rect1.x, rect2.x);
	const int right_side = std::min(rect1.x + rect1.w, rect2.x + rect2.w);
	if(left_side > right_side) {
		return empty_rect;
	}

	const int top_side = std::max(rect1.y, rect2.y);
	const int bottom_side = std::min(rect1.y + rect1.h, rect2.y + rect2.h);
	if(top_side > bottom_side) {
		return empty_rect;
	}

	SDL_Rect result = {
			left_side,
			top_side,
			right_side - left_side,
			bottom_side - top_side};

	return result;
}
SDL_Rect create_rect(const int x, const int y, const int w, const int h)
{
	SDL_Rect rect = { x, y, w, h };
	return rect;
}

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}

static SDL_PixelFormat& get_neutral_pixel_format()
	{
		static bool first_time = true;
		static SDL_PixelFormat format;

		if(first_time) {
			first_time = false;
			surface surf(SDL_CreateRGBSurface(SDL_SWSURFACE,1,1,32,0xFF0000,0xFF00,0xFF,0xFF000000));
			format = *surf->format;
			format.palette = NULL;
		}

		return format;
	}

surface make_neutral_surface(const surface &surf)
{
	if(surf == NULL) {
		std::cerr << "null neutral surface...\n";
		return NULL;
	}

	surface const result = SDL_ConvertSurface(surf,&get_neutral_pixel_format(),SDL_SWSURFACE);
	if(result != NULL) {
		SDL_SetAlpha(result,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
	}

	return result;
}

surface create_neutral_surface(int w, int h)
{
	if (w < 0 || h < 0) {
		std::cerr << "error : neutral surface with negative dimensions\n";
		return NULL;
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

surface create_optimized_surface(const surface &surf)
{
	if(surf == NULL)
		return NULL;

	surface const result = display_format_alpha(surf);
	if(result == surf) {
		std::cerr << "resulting surface is the same as the source!!!\n";
	} else if(result == NULL) {
		return surf;
	}

	SDL_SetAlpha(result,SDL_SRCALPHA|SDL_RLEACCEL,SDL_ALPHA_OPAQUE);

	return result;
}

surface stretch_surface_horizontal(
		const surface& surf, const unsigned w, const bool optimize)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if(surf == NULL)
		return NULL;

	if(static_cast<int>(w) == surf->w) {
		return surf;
	}
	assert(w > 0);

	surface dst(create_neutral_surface(w, surf->h));

	surface src(make_neutral_surface(surf));
	// Now both surfaces are always in the "neutral" pixel format

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	{
		// Extra scoping used for the surface_lock.
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		for(unsigned y = 0; y < static_cast<unsigned>(src->h); ++y) {
			const Uint32 pixel = src_pixels [y * src->w];
			for(unsigned x = 0; x < w; ++x) {

				*dst_pixels++ = pixel;

			}
		}
	}

	return optimize ? create_optimized_surface(dst) : dst;
}

surface stretch_surface_vertical(
		const surface& surf, const unsigned h, const bool optimize)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if(surf == NULL)
		return NULL;

	if(static_cast<int>(h) == surf->h) {
		return surf;
	}
	assert(h > 0);

	surface dst(create_neutral_surface(surf->w, h));

	surface src(make_neutral_surface(surf));
	// Now both surfaces are always in the "neutral" pixel format

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	{
		// Extra scoping used for the surface_lock.
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		for(unsigned y = 0; y < static_cast<unsigned>(h); ++y) {
		  for(unsigned x = 0; x < static_cast<unsigned>(src->w); ++x) {

				*dst_pixels++ = src_pixels[x];
			}
		}
	}

	return optimize ? create_optimized_surface(dst) : dst;
}

// NOTE: Don't pass this function 0 scaling arguments.
surface scale_surface(const surface &surf, int w, int h, bool optimize)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if(surf == NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}
	assert(w >= 0);
	assert(h >= 0);

	surface dst(create_neutral_surface(w,h));

	if (w == 0 || h ==0) {
		std::cerr << "Create an empty image\n";
		return create_optimized_surface(dst);
	}

	surface src(make_neutral_surface(surf));
	// Now both surfaces are always in the "neutral" pixel format

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const fixed_t xratio = fxpdiv(surf->w,w);
	const fixed_t yratio = fxpdiv(surf->h,h);

	{
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* const dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		fixed_t ysrc = ftofxp(0.0);
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			fixed_t xsrc = ftofxp(0.0);
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				const int xsrcint = fxptoi(xsrc);
				const int ysrcint = fxptoi(ysrc);

				Uint32* const src_word = src_pixels + ysrcint*src->w + xsrcint;
				Uint32* const dst_word = dst_pixels +    ydst*dst->w + xdst;
				const int dx = (xsrcint + 1 < src->w) ? 1 : 0;
				const int dy = (ysrcint + 1 < src->h) ? src->w : 0;

				Uint8 r,g,b,a;
				Uint32 rr,gg,bb,aa;
				Uint16 avg_r, avg_g, avg_b, avg_a;
				Uint32 pix[4], bilin[4];

				// This next part is the fixed point
				// equivalent of "take everything to
				// the right of the decimal point."
				// These fundamental weights decide
				// the contributions from various
				// input pixels. The labels assume
				// that the upper left corner of the
				// screen ("northeast") is 0,0 but the
				// code should still be consistant if
				// the graphics origin is actually
				// somewhere else.

				const fixed_t e = 0x000000FF & xsrc;
				const fixed_t s = 0x000000FF & ysrc;
				const fixed_t n = 0xFF - s;
				const fixed_t w = 0xFF - e;

				pix[0] = *src_word;              // northwest
				pix[1] = *(src_word + dx);       // northeast
				pix[2] = *(src_word + dy);       // southwest
				pix[3] = *(src_word + dx + dy);  // southeast

				bilin[0] = n*w;
				bilin[1] = n*e;
				bilin[2] = s*w;
				bilin[3] = s*e;

				// Scope out the neighboorhood, see
				// what the pixel values are like.

				int count = 0;
				avg_r = avg_g = avg_b = avg_a = 0;
				int loc;
				for (loc=0; loc<4; loc++) {
				  a = pix[loc] >> 24;
				  r = pix[loc] >> 16;
				  g = pix[loc] >> 8;
				  b = pix[loc] >> 0;
				  if (a != 0) {
				    avg_r += r;
				    avg_g += g;
				    avg_b += b;
				    avg_a += a;
				    count++;
				  }
				}
				if (count>0) {
				  avg_r /= count;
				  avg_b /= count;
				  avg_g /= count;
				  avg_a /= count;
				}

				// Perform modified bilinear interpolation.
				// Don't trust any color information from
				// an RGBA sample when the alpha channel
				// is set to fully transparent.
				//
				// Some of the input images are hex tiles,
				// created using a hexagon shaped alpha channel
				// that is either set to full-on or full-off.
				//
				// If intermediate alpha values are introduced
				// along a hex edge, it produces a gametime artifact.
				// Moving the mouse around will leave behind
				// "hexagon halos" from the temporary highlighting.
				// In other words, the Wesnoth rendering engine
				// freaks out.
				//
				// The alpha thresholding step attempts
				// to accomodates this limitation.
				// There is a small loss of quality.
				// For example, skeleton bowstrings
				// are not as good as they could be.

				rr = gg = bb = aa = 0;
				for (loc=0; loc<4; loc++) {
				  a = pix[loc] >> 24;
				  r = pix[loc] >> 16;
				  g = pix[loc] >> 8;
				  b = pix[loc] >> 0;
				  if (a == 0) {
				    r = static_cast<Uint8>(avg_r);
				    g = static_cast<Uint8>(avg_g);
				    b = static_cast<Uint8>(avg_b);
				  }
				  rr += r * bilin[loc];
				  gg += g * bilin[loc];
				  bb += b * bilin[loc];
				  aa += a * bilin[loc];
				}
				r = rr >> 16;
				g = gg >> 16;
				b = bb >> 16;
				a = aa >> 16;
				a = (a < avg_a/2) ? 0 : avg_a;
				*dst_word = (a << 24) + (r << 16) + (g << 8) + b;
			}
		}
	}

	return optimize ? create_optimized_surface(dst) : dst;
}


// This is a copy-paste of the previous function scale_surface
// We only removed the alpha channel and big comments
// and change how we optimize the resulting surface
surface scale_opaque_surface(const surface &surf, int w, int h, bool optimize_format)
{
	if(surf == NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}
	assert(w >= 0);
	assert(h >= 0);

	surface dst(create_neutral_surface(w,h));

	if (w == 0 || h ==0) {
		std::cerr << "Create an empty image\n";
		return create_optimized_surface(dst);
	}

	surface src(make_neutral_surface(surf));
	// Now both surfaces are always in the "neutral" pixel format

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const fixed_t xratio = fxpdiv(surf->w,w);
	const fixed_t yratio = fxpdiv(surf->h,h);

	{
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* const dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		fixed_t ysrc = ftofxp(0.0);
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			fixed_t xsrc = ftofxp(0.0);
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				const int xsrcint = fxptoi(xsrc);
				const int ysrcint = fxptoi(ysrc);

				Uint32* const src_word = src_pixels + ysrcint*src->w + xsrcint;
				Uint32* const dst_word = dst_pixels +    ydst*dst->w + xdst;
				const int dx = (xsrcint + 1 < src->w) ? 1 : 0;
				const int dy = (ysrcint + 1 < src->h) ? src->w : 0;

				Uint8 r,g,b;
				Uint32 rr,gg,bb;
				Uint32 pix[4], bilin[4];

				const fixed_t e = 0x000000FF & xsrc;
				const fixed_t s = 0x000000FF & ysrc;
				const fixed_t n = 0xFF - s;
				const fixed_t w = 0xFF - e;

				pix[0] = *src_word;              // northwest
				pix[1] = *(src_word + dx);       // northeast
				pix[2] = *(src_word + dy);       // southwest
				pix[3] = *(src_word + dx + dy);  // southeast

				bilin[0] = n*w;
				bilin[1] = n*e;
				bilin[2] = s*w;
				bilin[3] = s*e;

				int loc;
				rr = gg = bb = 0;
				for (loc=0; loc<4; loc++) {
				  r = pix[loc] >> 16;
				  g = pix[loc] >> 8;
				  b = pix[loc] >> 0;
				  rr += r * bilin[loc];
				  gg += g * bilin[loc];
				  bb += b * bilin[loc];
				}
				r = rr >> 16;
				g = gg >> 16;
				b = bb >> 16;
				*dst_word = (255 << 24) + (r << 16) + (g << 8) + b;
			}
		}
	}

	return optimize_format ? display_format_alpha(dst) : dst;
}


surface scale_surface_blended(const surface &surf, int w, int h, bool optimize)
{
	if(surf== NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}
	assert(w >= 0);
	assert(h >= 0);

	surface dst(create_neutral_surface(w,h));

	if (w == 0 || h ==0) {
		std::cerr << "Create an empty image\n";
		return create_optimized_surface(dst);
	}

	surface src(make_neutral_surface(surf));

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const double xratio = static_cast<double>(surf->w)/
			              static_cast<double>(w);
	const double yratio = static_cast<double>(surf->h)/
			              static_cast<double>(h);

	{
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* const dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		double ysrc = 0.0;
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			double xsrc = 0.0;
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;

				double summation = 0.0;

				// We now have a rectangle, (xsrc,ysrc,xratio,yratio)
				// which we want to derive the pixel from
				for(double xloc = xsrc; xloc < xsrc+xratio; xloc += 1.0) {
					const double xsize = std::min<double>(std::floor(xloc+1.0)-xloc,xsrc+xratio-xloc);
					for(double yloc = ysrc; yloc < ysrc+yratio; yloc += 1.0) {
						const int xsrcint = std::max<int>(0,std::min<int>(src->w-1,static_cast<int>(xsrc)));
						const int ysrcint = std::max<int>(0,std::min<int>(src->h-1,static_cast<int>(ysrc)));

						const double ysize = std::min<double>(std::floor(yloc+1.0)-yloc,ysrc+yratio-yloc);

						Uint8 r,g,b,a;

						SDL_GetRGBA(src_pixels[ysrcint*src->w + xsrcint],src->format,&r,&g,&b,&a);
						const double value = xsize*ysize*double(a)/255.0;
						summation += value;

						red += r*value;
						green += g*value;
						blue += b*value;
						alpha += a*value;
					}
				}

				if(summation == 0.0)
					summation = 1.0;

				red /= summation;
				green /= summation;
				blue /= summation;
				alpha /= summation;

				dst_pixels[ydst*dst->w + xdst] = SDL_MapRGBA(dst->format,Uint8(red),Uint8(green),Uint8(blue),Uint8(alpha));
			}
		}
	}

	return optimize ? create_optimized_surface(dst) : dst;
}

surface adjust_surface_colour(const surface &surf, int red, int green, int blue, bool optimize)
{
	if((red == 0 && green == 0 && blue == 0) || surf == NULL)
		return create_optimized_surface(surf);

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg) >> 0;

				r = std::max<int>(0,std::min<int>(255,int(r)+red));
				g = std::max<int>(0,std::min<int>(255,int(g)+green));
				b = std::max<int>(0,std::min<int>(255,int(b)+blue));

				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface greyscale_image(const surface &surf, bool optimize)
{
	if(surf == NULL)
		return NULL;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);
				//const Uint8 avg = (red+green+blue)/3;

				// Use the correct formula for RGB to grayscale conversion.
				// Ok, this is no big deal :)
				// The correct formula being:
				// gray=0.299red+0.587green+0.114blue
				const Uint8 avg = static_cast<Uint8>((
					77  * static_cast<Uint16>(r) +
					150 * static_cast<Uint16>(g) +
					29  * static_cast<Uint16>(b)  ) / 256);

				*beg = (alpha << 24) | (avg << 16) | (avg << 8) | avg;
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface shadow_image(const surface &surf, bool optimize)
{
	if(surf == NULL)
		return NULL;

	// we blur it, and reuse the neutral surface created by the blur function (optimized = false)
	surface nsurf (blur_alpha_surface(surf, 2, false));

	if(nsurf == NULL) {
		std::cerr << "failed to blur the shadow surface\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				// increase alpha and color in black (RGB=0)
				// with some stupid optimization for handling maximum values
				if (alpha < 255/4)
					*beg = (alpha*4) << 24;
				else
					*beg = 0xFF000000; // we hit the maximum
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}


surface recolor_image(surface surf, const std::map<Uint32, Uint32>& map_rgb, bool optimize){
	if(map_rgb.size()){
		if(surf == NULL)
		return NULL;

	     surface nsurf(make_neutral_surface(surf));
	     if(nsurf == NULL) {
			std::cerr << "failed to make neutral surface\n";
			return NULL;
	     }

		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		std::map<Uint32, Uint32>::const_iterator map_rgb_end = map_rgb.end();

		while(beg != end) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha){	// don't recolor invisible pixels.
				// palette use only RGB channels, so remove alpha
				Uint32 oldrgb = (*beg) & 0x00FFFFFF;
				std::map<Uint32, Uint32>::const_iterator i = map_rgb.find(oldrgb);
				if(i != map_rgb.end()){
					*beg = (alpha << 24) + i->second;
				}
			}
		++beg;
		}

		return optimize ? create_optimized_surface(nsurf) : nsurf;
	}
	return surf;
}

surface brighten_image(const surface &surf, fixed_t amount, bool optimize)
{
	if(surf == NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
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

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface adjust_surface_alpha(const surface &surf, fixed_t amount, bool optimize)
{
	if(surf== NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
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

				alpha = std::min<unsigned>(unsigned(fxpmult(alpha,amount)),255);
				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface adjust_surface_alpha_add(const surface &surf, int amount, bool optimize)
{
	if(surf== NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		while(beg != end) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				alpha = Uint8(std::max<int>(0,std::min<int>(255,int(alpha) + amount)));
				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface mask_surface(const surface &surf, const surface &mask)
{
	if(surf == NULL) {
		return NULL;
	}

	surface nsurf = make_neutral_surface(surf);
	surface nmask(make_neutral_surface(mask));

	if(nsurf == NULL || nmask == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}
	if (nsurf->w !=  nmask->w) {
		// we don't support efficiently different width.
		// (different height is not a real problem)
		// This function is used on all hexes and usually only for that
		// so better keep it simple and efficient for the normal case
		std::cerr << "Detected an image with bad dimensions :" << nsurf->w << "x" << nsurf->h << "\n";
		std::cerr << "It will not be masked, please use :"<< nmask->w << "x" << nmask->h << "\n";
		return nsurf;
	}

	{
		surface_lock lock(nsurf);
		surface_lock mlock(nmask);

		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;
		Uint32* mbeg = mlock.pixels();
		Uint32* mend = mbeg + nmask->w*nmask->h;

		while(beg != end && mbeg != mend) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				Uint8 malpha = (*mbeg) >> 24;
				if (alpha > malpha) alpha = malpha;

				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
			++mbeg;
		}
	}

	return nsurf;
	//return create_optimized_surface(nsurf);
}

bool in_mask_surface(const surface &surf, const surface &mask)
{
	if(surf == NULL || mask == NULL) {
		return false;
	}

	if (surf->w != surf->w || surf->h != mask->h ) {
		// not same size, consider it doesn't fit
		return false;
	}

	surface nsurf = make_neutral_surface(surf);
	surface nmask(make_neutral_surface(mask));

	if(nsurf == NULL || nmask == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return false;
	}

	{
		surface_lock lock(nsurf);
		surface_lock mlock(nmask);

		Uint32* mbeg = mlock.pixels();
		Uint32* mend = mbeg + nmask->w*nmask->h;
		Uint32* beg = lock.pixels();
		// no need for 'end', because both surfaces have same size

		while(mbeg != mend) {
			Uint8 malpha = (*mbeg) >> 24;
			if(malpha == 0) {
				Uint8 alpha = (*beg) >> 24;
				if (alpha)
					return false;
			}
			++mbeg;
			++beg;
		}
	}

	return true;
}

surface blur_surface(const surface &surf, int depth, bool optimize)
{
	if(surf == NULL) {
		return NULL;
	}

	surface res = make_neutral_surface(surf);

	if(res == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	SDL_Rect rect = { 0, 0, surf->w, surf->h };
	blur_surface(res, rect, depth);

	return optimize ? create_optimized_surface(res) : res;
}

void blur_surface(surface& surf, SDL_Rect rect, unsigned depth)
{
	if(surf == NULL) {
		return;
	}

	assert((surf->flags & SDL_RLEACCEL) == 0);
	assert(surf->format->BitsPerPixel == 32);

	const unsigned max_blur = 256;
	if(depth > max_blur) {
		depth = max_blur;
	}

	Uint32 queue[max_blur];
	const Uint32* end_queue = queue + max_blur;

	const Uint32 ff = 0xff;

	const unsigned pixel_offset = rect.y * surf->w + rect.x;

	surface_lock lock(surf);
	for(unsigned y = 0; y < rect.h; ++y) {
		const Uint32* front = &queue[0];
		Uint32* back = &queue[0];
		Uint32 red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + pixel_offset + y * surf->w;
		for(unsigned x = 0; x <= depth && x < rect.w; ++x, ++p) {
			red += ((*p) >> 16)&0xFF;
			green += ((*p) >> 8)&0xFF;
			blue += (*p)&0xFF;
			++avg;
			*back++ = *p;
			if(back == end_queue) {
				back = &queue[0];
			}
		}

		p = lock.pixels() + pixel_offset + y * surf->w;
		for(unsigned x = 0; x < rect.w; ++x, ++p) {
			*p = 0xFF000000
					| (std::min(red/avg,ff) << 16)
					| (std::min(green/avg,ff) << 8)
					| std::min(blue/avg,ff);

			if(x >= depth) {
				red -= ((*front) >> 16)&0xFF;
				green -= ((*front) >> 8)&0xFF;
				blue -= *front&0xFF;
				--avg;
				++front;
				if(front == end_queue) {
					front = &queue[0];
				}
			}

			if(x + depth+1 < rect.w) {
				Uint32* q = p + depth+1;
				red += ((*q) >> 16)&0xFF;
				green += ((*q) >> 8)&0xFF;
				blue += (*q)&0xFF;
				++avg;
				*back++ = *q;
				if(back == end_queue) {
					back = &queue[0];
				}
			}
		}
	}

	for(unsigned x = 0; x < rect.w; ++x) {
		const Uint32* front = &queue[0];
		Uint32* back = &queue[0];
		Uint32 red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + pixel_offset + x;
		for(unsigned y = 0; y <= depth && y < rect.h; ++y, p += surf->w) {
			red += ((*p) >> 16)&0xFF;
			green += ((*p) >> 8)&0xFF;
			blue += *p&0xFF;
			++avg;
			*back++ = *p;
			if(back == end_queue) {
				back = &queue[0];
			}
		}

		p = lock.pixels() + pixel_offset + x;
		for(unsigned y = 0; y < rect.h; ++y, p += surf->w) {
			*p = 0xFF000000
					| (std::min(red/avg,ff) << 16)
					| (std::min(green/avg,ff) << 8)
					| std::min(blue/avg,ff);

			if(y >= depth) {
				red -= ((*front) >> 16)&0xFF;
				green -= ((*front) >> 8)&0xFF;
				blue -= *front&0xFF;
				--avg;
				++front;
				if(front == end_queue) {
					front = &queue[0];
				}
			}

			if(y + depth+1 < rect.h) {
				Uint32* q = p + (depth+1)*surf->w;
				red += ((*q) >> 16)&0xFF;
				green += ((*q) >> 8)&0xFF;
				blue += (*q)&0xFF;
				++avg;
				*back++ = *q;
				if(back == end_queue) {
					back = &queue[0];
				}
			}
		}
	}
}

surface blur_alpha_surface(const surface &surf, int depth, bool optimize)
{
	if(surf == NULL) {
		return NULL;
	}

	surface res = make_neutral_surface(surf);

	if(res == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	const int max_blur = 256;
	if(depth > max_blur) {
		depth = max_blur;
	}

	Uint32 queue[max_blur];
	const Uint32* end_queue = queue + max_blur;

	const Uint32 ff = 0xff;

	surface_lock lock(res);
	int x, y;
	for(y = 0; y < res->h; ++y) {
		const Uint32* front = &queue[0];
		Uint32* back = &queue[0];
		Uint32 alpha=0, red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + y*res->w;
		for(x = 0; x <= depth && x < res->w; ++x, ++p) {
			alpha += ((*p) >> 24)&0xFF;
			red += ((*p) >> 16)&0xFF;
			green += ((*p) >> 8)&0xFF;
			blue += (*p)&0xFF;
			++avg;
			*back++ = *p;
			if(back == end_queue) {
				back = &queue[0];
			}
		}

		p = lock.pixels() + y*res->w;
		for(x = 0; x < res->w; ++x, ++p) {
			*p = (std::min(alpha/avg,ff) << 24) | (std::min(red/avg,ff) << 16) | (std::min(green/avg,ff) << 8) | std::min(blue/avg,ff);
			if(x >= depth) {
				alpha -= ((*front) >> 24)&0xFF;
				red -= ((*front) >> 16)&0xFF;
				green -= ((*front) >> 8)&0xFF;
				blue -= *front&0xFF;
				--avg;
				++front;
				if(front == end_queue) {
					front = &queue[0];
				}
			}

			if(x + depth+1 < res->w) {
				Uint32* q = p + depth+1;
				alpha += ((*q) >> 24)&0xFF;
				red += ((*q) >> 16)&0xFF;
				green += ((*q) >> 8)&0xFF;
				blue += (*q)&0xFF;
				++avg;
				*back++ = *q;
				if(back == end_queue) {
					back = &queue[0];
				}
			}
		}
	}

	for(x = 0; x < res->w; ++x) {
		const Uint32* front = &queue[0];
		Uint32* back = &queue[0];
		Uint32 alpha=0, red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + x;
		for(y = 0; y <= depth && y < res->h; ++y, p += res->w) {
			alpha += ((*p) >> 24)&0xFF;
			red += ((*p) >> 16)&0xFF;
			green += ((*p) >> 8)&0xFF;
			blue += *p&0xFF;
			++avg;
			*back++ = *p;
			if(back == end_queue) {
				back = &queue[0];
			}
		}

		p = lock.pixels() + x;
		for(y = 0; y < res->h; ++y, p += res->w) {
			*p = (std::min(alpha/avg,ff) << 24) | (std::min(red/avg,ff) << 16) | (std::min(green/avg,ff) << 8) | std::min(blue/avg,ff);
			if(y >= depth) {
				alpha -= ((*front) >> 24)&0xFF;
				red -= ((*front) >> 16)&0xFF;
				green -= ((*front) >> 8)&0xFF;
				blue -= *front&0xFF;
				--avg;
				++front;
				if(front == end_queue) {
					front = &queue[0];
				}
			}

			if(y + depth+1 < res->h) {
				Uint32* q = p + (depth+1)*res->w;
				alpha += ((*q) >> 24)&0xFF;
				red += ((*q) >> 16)&0xFF;
				green += ((*q) >> 8)&0xFF;
				blue += (*q)&0xFF;
				++avg;
				*back++ = *q;
				if(back == end_queue) {
					back = &queue[0];
				}
			}
		}
	}

	return optimize ? create_optimized_surface(res) : res;
}

surface cut_surface(const surface &surf, SDL_Rect const &r)
{
	surface res = create_compatible_surface(surf, r.w, r.h);

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

	surface_lock slock(surf);
	surface_lock rlock(res);

	Uint8* src = reinterpret_cast<Uint8 *>(slock.pixels());
	Uint8* dest = reinterpret_cast<Uint8 *>(rlock.pixels());

	for(int y = 0; y < src_rect.h && (src_rect.y + y) < surf->h; ++y) {
		Uint8* line_src  = src  + (src_rect.y + y) * spitch + src_rect.x * sbpp;
		Uint8* line_dest = dest + (dst_rect.y + y) * rpitch + dst_rect.x * rbpp;
		size_t size = src_rect.w + src_rect.x <= surf->w ? src_rect.w : surf->w - src_rect.x;

		assert(rpitch >= src_rect.w * rbpp);
		memcpy(line_dest, line_src, size * rbpp);
	}

	return res;
}

surface blend_surface(const surface &surf, double amount, Uint32 colour, bool optimize)
{
	if(surf== NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

		Uint8 red, green, blue, alpha;
		SDL_GetRGBA(colour,nsurf->format,&red,&green,&blue,&alpha);

		red   = Uint8(red   * amount);
		green = Uint8(green * amount);
		blue  = Uint8(blue  * amount);

		amount = 1.0 - amount;

		while(beg != end) {
			Uint8 r, g, b, a;
			a = (*beg) >> 24;
			r = (*beg) >> 16;
			g = (*beg) >> 8;
			b = (*beg);

			r = Uint8(r * amount) + red;
			g = Uint8(g * amount) + green;
			b = Uint8(b * amount) + blue;

			*beg = (a << 24) | (r << 16) | (g << 8) | b;

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface flip_surface(const surface &surf, bool optimize)
{
	if(surf == NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* const pixels = lock.pixels();

		for(int y = 0; y != nsurf->h; ++y) {
			for(int x = 0; x != nsurf->w/2; ++x) {
				const int index1 = y*nsurf->w + x;
				const int index2 = (y+1)*nsurf->w - x - 1;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface flop_surface(const surface &surf, bool optimize)
{
	if(surf == NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(nsurf);
		Uint32* const pixels = lock.pixels();

		for(int x = 0; x != nsurf->w; ++x) {
			for(int y = 0; y != nsurf->h/2; ++y) {
				const int index1 = y*nsurf->w + x;
				const int index2 = (nsurf->h-y-1)*surf->w + x;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}


surface create_compatible_surface(const surface &surf, int width, int height)
{
	if(surf == NULL)
		return NULL;

	if(width == -1)
		width = surf->w;

	if(height == -1)
		height = surf->h;

	surface s = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, surf->format->BitsPerPixel,
		surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask);
	if (surf->format->palette) {
		SDL_SetPalette(s, SDL_LOGPAL, surf->format->palette->colors, 0, surf->format->palette->ncolors);
	}
	return s;
}

void blit_surface(const surface& src,
	const SDL_Rect* srcrect, surface& dst, const SDL_Rect* dstrect)
{
	assert(src);
	assert(dst);
	assert((src->flags & SDL_RLEACCEL) == 0);
	assert((dst->flags & SDL_RLEACCEL) == 0);

	// Get the areas to blit
	SDL_Rect dst_rect = { 0, 0, dst->w, dst->h };
	if(dstrect) {
		dst_rect.x = dstrect->x;
		dst_rect.w -= dstrect->x;

		dst_rect.y = dstrect->y;
		dst_rect.h -= dstrect->y;

	}

	SDL_Rect src_rect = { 0, 0, src->w, src->h };
	if(srcrect && srcrect->w && srcrect->h) {
		src_rect.x = srcrect->x;
		src_rect.y = srcrect->y;

		src_rect.w = srcrect->w;
		src_rect.h = srcrect->h;

		if (src_rect.x < 0) {
            if (src_rect.x + src_rect.w <= 0 || src_rect.x + dst_rect.w <= 0 )
                return;
			dst_rect.x -= src_rect.x;
			dst_rect.w += src_rect.x;
			src_rect.w += src_rect.x;
			src_rect.x = 0;
		}
		if (src_rect.y < 0) {
            if (src_rect.y + src_rect.h <= 0 || src_rect.y + dst_rect.h <= 0 )
                return;
			dst_rect.y -= src_rect.y;
			dst_rect.h += src_rect.y;
			src_rect.h += src_rect.y;
			src_rect.y = 0;
		}
		if (src_rect.x + src_rect.w > src->w) {
            if (src_rect.x >= src->w)
                return;
			src_rect.w = src->w - src_rect.x;
		}
		if (src_rect.y + src_rect.h > src->h) {
            if (src_rect.y >= src->h)
                return;
			src_rect.h = src->h - src_rect.y;
		}
	}

	assert(dst_rect.x >= 0);
	assert(dst_rect.y >= 0);

	// Get the blit size limits.
	const unsigned width = std::min(src_rect.w, dst_rect.w);
	const unsigned height = std::min(src_rect.h, dst_rect.h);
// 	std::cout << width << " -- " << height << "\n";
// 	std::cout << src->w << " -- " << src->h << "\n";
// 	std::cout << srcrect->x << "," << srcrect->y << " - " << srcrect->w << "x" << srcrect->h << " - " "\n";
// 	if (dstrect)
// 		std::cout << dstrect->x << "," << dstrect->y << " - " << dstrect->w << "x" << dstrect->h << " - " "\n";
// 	std::cout << src_rect.x << "," << src_rect.y << " - " << src_rect.w << "x" << src_rect.h << " - " "\n";
// 	std::cout << dst_rect.x << "," << dst_rect.y << " - " << dst_rect.w << "x" << dst_rect.h << " - " "\n";

	{
		// Extra scoping used for the surface_lock.
		surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		Uint32* const src_pixels = reinterpret_cast<Uint32*>(src_lock.pixels());
		Uint32* dst_pixels = reinterpret_cast<Uint32*>(dst_lock.pixels());

		for(unsigned y = 0; y < height; ++y) {
			for(unsigned x = 0; x < width; ++x) {

				// We need to do the blitting using some optimizations
				// if the src is fully transparent we can ignore this pixel
				// if the src is fully opaque we can overwrite the destination with this pixel
				// if the destination is fully transparent we replace us with the source
				//
				// We do these optimizations between the extraction of the variables
				// to avoid creating variables not used (it might save us some cycles).

				const Uint32 src_pixel = src_pixels[(y + src_rect.y) * src->w + (x + src_rect.x)];
				const Uint8 src_a = (src_pixel & 0xFF000000) >> 24;

				if(!src_a) {
					// Fully transparent source, ignore
					continue;
				}

				const ptrdiff_t dst_offset = (y + dst_rect.y) * dst->w + (x + dst_rect.x);
				if(src_a == 255) {
					// Fully opaque source, copy
					dst_pixels[dst_offset] = src_pixel;
					continue;
				}

				const Uint32 dst_pixel = dst_pixels[dst_offset];
				Uint8 dst_a = (dst_pixel & 0xFF000000) >> 24;

				if(!dst_a) {
					// Fully transparent destination, copy
					dst_pixels[dst_offset] = src_pixel;
					continue;
				}

				const Uint8 src_r = (src_pixel & 0x00FF0000) >> 16;
				const Uint8 src_g = (src_pixel & 0x0000FF00) >> 8;
				const Uint8 src_b = src_pixel & 0x000000FF;

				Uint8 dst_r = (dst_pixel & 0x00FF0000) >> 16;
				Uint8 dst_g = (dst_pixel & 0x0000FF00) >> 8;
				Uint8 dst_b = dst_pixel & 0x000000FF;

				if(dst_a == 255) {

					// Destination fully opaque blend the source.
					dst_r = (((src_r - dst_r) * src_a) >> 8 ) + dst_r;
					dst_g = (((src_g - dst_g) * src_a) >> 8 ) + dst_g;
					dst_b = (((src_b - dst_b) * src_a) >> 8 ) + dst_b;

				} else {

					// Destination and source party transparent.

					// aquired the data now do the blitting
					const unsigned tmp_a = 255 - src_a;

					const unsigned tmp_r = 1 + (src_r * src_a) + (dst_r * tmp_a);
					dst_r = (tmp_r + (tmp_r >> 8)) >> 8;

					const unsigned tmp_g = 1 + (src_g * src_a) + (dst_g * tmp_a);
					dst_g = (tmp_g + (tmp_g >> 8)) >> 8;

					const unsigned tmp_b = 1 + (src_b * src_a) + (dst_b * tmp_a);
					dst_b = (tmp_b + (tmp_b >> 8)) >> 8;

					dst_a += (((255 - dst_a) * src_a) >> 8);
				}

				dst_pixels[dst_offset] = (dst_a << 24) | (dst_r << 16) | (dst_g << 8) | (dst_b);

			}
		}
	}
}



void fill_rect_alpha(SDL_Rect &rect, Uint32 colour, Uint8 alpha, const surface &target)
{
	if(alpha == SDL_ALPHA_OPAQUE) {
		SDL_FillRect(target,&rect,colour);
		return;
	} else if(alpha == SDL_ALPHA_TRANSPARENT) {
		return;
	}

	surface tmp(create_compatible_surface(target,rect.w,rect.h));
	if(tmp == NULL) {
		return;
	}

	SDL_Rect r = {0,0,rect.w,rect.h};
	SDL_FillRect(tmp,&r,colour);
	SDL_SetAlpha(tmp,SDL_SRCALPHA,alpha);
	SDL_BlitSurface(tmp,NULL,target,&rect);
}

surface get_surface_portion(const surface &src, SDL_Rect &area, bool optimize_format)
{
	if (src == NULL) {
		return NULL;
	}

	// Check if there is something in the portion
	if(area.x >= src->w || area.y >= src->h || area.x + area.w < 0 || area.y + area.h < 0) {
		return NULL;
	}

	if(area.x + area.w > src->w) {
		area.w = src->w - area.x;
	}
	if(area.y + area.h > src->h) {
		area.h = src->h - area.y;
	}

	// use same format as the source (almost always the screen)
	surface dst = create_compatible_surface(src, area.w, area.h);

	if(dst == NULL) {
		std::cerr << "Could not create a new surface in get_surface_portion()\n";
		return NULL;
	}

	SDL_BlitSurface(src, &area, dst, NULL);

	return optimize_format ? display_format_alpha(dst) : dst;
}

namespace {

struct not_alpha
{
	not_alpha() {}

	// we assume neutral format
	bool operator()(Uint32 pixel) const {
		Uint8 alpha = pixel >> 24;
		return alpha != 0x00;
	}
};

}

SDL_Rect get_non_transparent_portion(const surface &surf)
{
	SDL_Rect res = {0,0,0,0};
	const surface nsurf(make_neutral_surface(surf));
	if(nsurf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return res;
	}

	const not_alpha calc;

	surface_lock lock(nsurf);
	const Uint32* const pixels = lock.pixels();

	int n;
	for(n = 0; n != nsurf->h; ++n) {
		const Uint32* const start_row = pixels + n*nsurf->w;
		const Uint32* const end_row = start_row + nsurf->w;

		if(std::find_if(start_row,end_row,calc) != end_row)
			break;
	}

	res.y = n;

	for(n = 0; n != nsurf->h-res.y; ++n) {
		const Uint32* const start_row = pixels + (nsurf->h-n-1)*surf->w;
		const Uint32* const end_row = start_row + nsurf->w;

		if(std::find_if(start_row,end_row,calc) != end_row)
			break;
	}

	// The height is the height of the surface,
	// minus the distance from the top and
	// the distance from the bottom.
	res.h = nsurf->h - res.y - n;

	for(n = 0; n != nsurf->w; ++n) {
		int y;
		for(y = 0; y != nsurf->h; ++y) {
			const Uint32 pixel = pixels[y*nsurf->w + n];
			if(calc(pixel))
				break;
		}

		if(y != nsurf->h)
			break;
	}

	res.x = n;

	for(n = 0; n != nsurf->w-res.x; ++n) {
		int y;
		for(y = 0; y != nsurf->h; ++y) {
			const Uint32 pixel = pixels[y*nsurf->w + surf->w - n - 1];
			if(calc(pixel))
				break;
		}

		if(y != nsurf->h)
			break;
	}

	res.w = nsurf->w - res.x - n;

	return res;
}

bool operator==(const SDL_Rect& a, const SDL_Rect& b)
{
	return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

bool operator!=(const SDL_Rect& a, const SDL_Rect& b)
{
	return !operator==(a,b);
}

bool operator==(const SDL_Color& a, const SDL_Color& b) {
	return a.r == b.r && a.g == b.g && a.b == b.b;
}

bool operator!=(const SDL_Color& a, const SDL_Color& b) {
	return !operator==(a,b);
}

SDL_Color inverse(const SDL_Color& colour) {
	SDL_Color inverse;
	inverse.r = 255 - colour.r;
	inverse.g = 255 - colour.g;
	inverse.b = 255 - colour.b;
	inverse.unused = 0;
	return inverse;
}

surface_restorer::surface_restorer() : target_(NULL), rect_(empty_rect), surface_(NULL)
{
}

surface_restorer::surface_restorer(CVideo* target, const SDL_Rect& rect)
: target_(target), rect_(rect), surface_(NULL)
{
	update();
}

surface_restorer::~surface_restorer()
{
	restore();
}

void surface_restorer::restore(SDL_Rect const &dst) const
{
	if (surface_.null())
		return;
	SDL_Rect dst2 = intersect_rects(dst, rect_);
	if (dst2.w == 0 || dst2.h == 0)
		return;
	SDL_Rect src = dst2;
	src.x -= rect_.x;
	src.y -= rect_.y;
	SDL_BlitSurface(surface_, &src, target_->getSurface(), &dst2);
	update_rect(dst2);
}

void surface_restorer::restore() const
{
	if (surface_.null())
		return;
	SDL_Rect dst = rect_;
	SDL_BlitSurface(surface_, NULL, target_->getSurface(), &dst);
	update_rect(rect_);
}

void surface_restorer::update()
{
	if(rect_.w == 0 || rect_.h == 0)
		surface_.assign(NULL);
	else
		surface_.assign(::get_surface_portion(target_->getSurface(),rect_));
}

void surface_restorer::cancel()
{
	surface_.assign(NULL);
}

void draw_rectangle(int x, int y, int w, int h, Uint32 colour,surface target)
{

	SDL_Rect top = {x,y,w,1};
	SDL_Rect bot = {x,y+h-1,w,1};
	SDL_Rect left = {x,y,1,h};
	SDL_Rect right = {x+w-1,y,1,h};

	SDL_FillRect(target,&top,colour);
	SDL_FillRect(target,&bot,colour);
	SDL_FillRect(target,&left,colour);
	SDL_FillRect(target,&right,colour);
}

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
                                 int r, int g, int b,
                                 double alpha, surface target)
{

	SDL_Rect rect = {x,y,w,h};
	fill_rect_alpha(rect,SDL_MapRGB(target->format,r,g,b),Uint8(alpha*255),target);
}

void draw_centered_on_background(surface surf, const SDL_Rect& rect, const SDL_Color& color, surface target)
{
	clip_rect_setter clip_setter(target, rect);

	Uint32 col = SDL_MapRGBA(target->format, color.r, color.g, color.b, color.unused);
	//TODO: only draw background outside the image
	SDL_Rect r = rect;
	SDL_FillRect(target, &r, col);

	if (surf != NULL) {
		r.x = rect.x + (rect.w-surf->w)/2;
		r.y = rect.y + (rect.h-surf->h)/2;
		SDL_BlitSurface(surf, NULL, target, &r);
	}
	update_rect(rect);
}

std::ostream& operator<<(std::ostream& s, const SDL_Rect& rect)
{
	s << rect.x << ',' << rect.y << " x "  << rect.w << ',' << rect.h;
	return s;
}

