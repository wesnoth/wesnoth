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

//! @file sdl_utils.cpp
//! Support-routines for the SDL-graphics-library.

#include "global.hpp"

#include "config.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>

SDL_Color int_to_color(const Uint32 rgb) {
	SDL_Color to_return = {
		(0x00FF0000 & rgb)>>16,
		(0x0000FF00 & rgb)>>8,
		(0x000000FF & rgb), 0
	};
	return to_return;
}

Uint32 color_to_int(const SDL_Color& color) {
	Uint32 to_return = color.r;
	to_return = (to_return << 8) & color.g;
	to_return = (to_return << 8) & color.b;
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
	res.x = maximum<int>(rect1.x, rect2.x);
	res.y = maximum<int>(rect1.y, rect2.y);
	res.w = maximum<int>(minimum<int>(rect1.x + rect1.w, rect2.x + rect2.w) - res.x, 0);
	res.h = maximum<int>(minimum<int>(rect1.y + rect1.h, rect2.y + rect2.h) - res.y, 0);
	return res;
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

surface make_neutral_surface(surface const &surf)
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


surface create_optimized_surface(surface const &surf)
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

// NOTE: Don't pass this function 0 scaling arguments.
surface scale_surface(surface const &surf, int w, int h)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if(surf == NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}
	wassert(w >= 0);
	wassert(h >= 0);

	surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
		
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
				    r = avg_r;
				    g = avg_g;
				    b = avg_b;
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

	return create_optimized_surface(dst);
}

surface scale_surface_blended(surface const &surf, int w, int h)
{
	if(surf== NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}
	wassert(w >= 0);
	wassert(h >= 0);

	surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));

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
					const double xsize = minimum<double>(std::floor(xloc+1.0)-xloc,xsrc+xratio-xloc);
					for(double yloc = ysrc; yloc < ysrc+yratio; yloc += 1.0) {
						const int xsrcint = maximum<int>(0,minimum<int>(src->w-1,static_cast<int>(xsrc)));
						const int ysrcint = maximum<int>(0,minimum<int>(src->h-1,static_cast<int>(ysrc)));

						const double ysize = minimum<double>(std::floor(yloc+1.0)-yloc,ysrc+yratio-yloc);

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

	return create_optimized_surface(dst);
}

surface adjust_surface_colour(surface const &surf, int r, int g, int b)
{
	if(r == 0 && g == 0 && b == 0 || surf == NULL)
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
			Uint8 red, green, blue, alpha;
			alpha = (*beg) >> 24;
			red   = (*beg) >> 16;
			green = (*beg) >> 8;
			blue  = (*beg) >> 0;


			red = maximum<int>(8,minimum<int>(255,int(red)+r));
			green = maximum<int>(0,minimum<int>(255,int(green)+g));
			blue  = maximum<int>(0,minimum<int>(255,int(blue)+b));

			*beg = (alpha << 24) + (red << 16) + (green << 8) + blue;

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface greyscale_image(surface const &surf)
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
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			//const Uint8 avg = (red+green+blue)/3;

			// Use the correct formula for RGB to grayscale conversion.
			// Ok, this is no big deal :)
			// The correct formula being:
			// gray=0.299red+0.587green+0.114blue
			const Uint8 avg = static_cast<Uint8>((
				77 * static_cast<Uint16>(red) +
				150 * static_cast<Uint16>(green) +
				29 * static_cast<Uint16>(blue)) / 256);


			*beg = SDL_MapRGBA(nsurf->format,avg,avg,avg,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface darken_image(surface const &surf)
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
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			//const Uint8 avg = (red+green+blue)/3;

			// Use the correct formula for RGB to grayscale conversion.
			// Ok, this is no big deal :)
			// The correct formula being:
			// gray=0.299red+0.587green+0.114blue
			const Uint8 avg = static_cast<Uint8>((
				77 * static_cast<Uint16>(red) +
				150 * static_cast<Uint16>(green) +
				29 * static_cast<Uint16>(blue)) / 256);
			// then tint 77%, 67%, 72%
			const Uint8 r= ((avg * 196) >> 8);
			const Uint8 g= ((avg * 171) >> 8);
			const Uint8 b= ((avg * 184) >> 8);


			*beg = SDL_MapRGBA(nsurf->format,r,g,b,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface recolor_image(surface surf, const std::map<Uint32, Uint32>& map_rgb){
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

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			if(alpha){	// don't recolor invisible pixels.
				Uint32 oldrgb = (red<<16) + (green<<8) + (blue);
				for(std::map<Uint32, Uint32>::const_iterator i=map_rgb.begin(); i!= map_rgb.end(); i++){
					if(oldrgb==i->first){
						Uint32 new_rgb = i->second;
						Uint8 new_r = (new_rgb & 0x00FF0000)>>16;
						Uint8 new_g = (new_rgb & 0x0000FF00)>>8;
						Uint8 new_b = (new_rgb & 0x000000FF);
					*beg = SDL_MapRGBA(nsurf->format,new_r,new_g,new_b,alpha);
					break; // no double replacements.
					}
				}
			}
		++beg;
		}
		return create_optimized_surface(nsurf);
	}
	return surf;
}

surface brighten_image(surface const &surf, fixed_t amount)
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
			Uint8 red, green, blue, alpha;
			alpha = (*beg) >> 24;
			red   = (*beg) >> 16;
			green = (*beg) >> 8;
			blue  = (*beg) >> 0;

			red = minimum<unsigned>(unsigned(fxpmult(red,amount)),255);
			green = minimum<unsigned>(unsigned(fxpmult(green,amount)),255);
			blue = minimum<unsigned>(unsigned(fxpmult(blue,amount)),255);

			*beg = (alpha << 24) + (red << 16) + (green << 8) + blue;

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface adjust_surface_alpha(surface const &surf, fixed_t amount, bool optimize)
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
			Uint8 red, green, blue, alpha;
			alpha = (*beg) >> 24;
			red   = (*beg) >> 16;
			green = (*beg) >> 8;
			blue  = (*beg) >> 0;

			alpha = minimum<unsigned>(unsigned(fxpmult(alpha,amount)),255);

			*beg = (alpha << 24) + (red << 16) + (green << 8) + blue;

			++beg;
		}
	}

	if (optimize) {
		return create_optimized_surface(nsurf);
	} else {
		return nsurf;
	}
}

surface adjust_surface_alpha_add(surface const &surf, int amount)
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
			Uint8 red, green, blue, alpha;
			alpha = (*beg) >> 24;
			red   = (*beg) >> 16;
			green = (*beg) >> 8;
			blue  = (*beg) >> 0;

			alpha = Uint8(maximum<int>(0,minimum<int>(255,int(alpha) + amount)));

			*beg = (alpha << 24) + (red << 16) + (green << 8) + blue;

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

//! Applies a mask on a surface.
surface mask_surface(surface const &surf, surface const &mask)
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
			Uint8 red, green, blue, alpha;
			Uint8 malpha;

			alpha = (*beg) >> 24;
			red   = (*beg) >> 16;
			green = (*beg) >> 8;
			blue  = (*beg) >> 0;

			malpha = (*mbeg) >> 24;

			alpha = Uint8(minimum<int>(malpha, alpha));

			*beg = (alpha << 24) + (red << 16) + (green << 8) + blue;

			++beg;
			++mbeg;
		}
	}

	return nsurf;
	//return create_optimized_surface(nsurf);
}

//! Cross-fades a surface.
surface blur_surface(surface const &surf, int depth)
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
		Uint32 red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + y*res->w;
		for(x = 0; x <= depth && x < res->w; ++x, ++p) {
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
			*p = 0xFF000000 | (minimum(red/avg,ff) << 16) | (minimum(green/avg,ff) << 8) | minimum(blue/avg,ff);
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

			if(x + depth+1 < res->w) {
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

	for(x = 0; x < res->w; ++x) {
		const Uint32* front = &queue[0];
		Uint32* back = &queue[0];
		Uint32 red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + x;
		for(y = 0; y <= depth && y < res->h; ++y, p += res->w) {
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
			*p = 0xFF000000 | (minimum(red/avg,ff) << 16) | (minimum(green/avg,ff) << 8) | minimum(blue/avg,ff);
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

			if(y + depth+1 < res->h) {
				Uint32* q = p + (depth+1)*res->w;
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

	return create_optimized_surface(res);
}

//! Cross-fades a surface with alpha channel.
//! @todo FIXME: This is just an adapted copy-paste
//! of the normal blur but with blur alpha channel too
surface blur_alpha_surface(surface const &surf, int depth)
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
			*p = (minimum(alpha/avg,ff) << 24) | (minimum(red/avg,ff) << 16) | (minimum(green/avg,ff) << 8) | minimum(blue/avg,ff);
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
			*p = (minimum(alpha/avg,ff) << 24) | (minimum(red/avg,ff) << 16) | (minimum(green/avg,ff) << 8) | minimum(blue/avg,ff);
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

	return create_optimized_surface(res);
}

//! Cuts a rectangle from a surface.
surface cut_surface(surface const &surf, SDL_Rect const &r)
{
	surface res = create_compatible_surface(surf, r.w, r.h);

	size_t sbpp = surf->format->BytesPerPixel;
	size_t spitch = surf->pitch;
	size_t rbpp = res->format->BytesPerPixel;
	size_t rpitch = res->pitch;

	surface_lock slock(surf);
	surface_lock rlock(res);

	Uint8* src = reinterpret_cast<Uint8 *>(slock.pixels());
	Uint8* dest = reinterpret_cast<Uint8 *>(rlock.pixels());

	if(r.x >= surf->w)
		return res;

	for(int y = 0; y < r.h && (r.y + y) < surf->h; ++y) {
		Uint8* line_src = src + (r.y + y) * spitch + r.x * sbpp;
		Uint8* line_dest = dest + y * rpitch;
		size_t size = r.w + r.x <= surf->w ? r.w : surf->w - r.x;

		wassert(rpitch >= r.w * rbpp);
		memcpy(line_dest, line_src, size * rbpp);
	}

	return res;
}

surface blend_surface(surface const &surf, double amount, Uint32 colour)
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

		Uint8 red2, green2, blue2, alpha2;
		SDL_GetRGBA(colour,nsurf->format,&red2,&green2,&blue2,&alpha2);

		red2 = Uint8(red2*amount);
		green2 = Uint8(green2*amount);
		blue2 = Uint8(blue2*amount);

		amount = 1.0 - amount;

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			red = Uint8(red*amount) + red2;
			green = Uint8(green*amount) + green2;
			blue = Uint8(blue*amount) + blue2;

			*beg = SDL_MapRGBA(nsurf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface flip_surface(surface const &surf)
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

	return create_optimized_surface(nsurf);
}

surface flop_surface(surface const &surf)
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

	return create_optimized_surface(nsurf);
}


surface create_compatible_surface(surface const &surf, int width, int height)
{
	if(surf == NULL)
		return NULL;

	if(width == -1)
		width = surf->w;

	if(height == -1)
		height = surf->h;

	return SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,surf->format->BitsPerPixel,
		                        surf->format->Rmask,surf->format->Gmask,surf->format->Bmask,surf->format->Amask);
}

void fill_rect_alpha(SDL_Rect &rect, Uint32 colour, Uint8 alpha, surface const &target)
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

surface get_surface_portion(surface const &src, SDL_Rect &area)
{
	// Check if there is something in the portion
	if(area.x >= src->w || area.y >= src->h || area.x + area.w < 0 || area.y + area.h < 0) {
		//std::cerr << "illegal surface portion...\n";
		return NULL;
	}

	if(area.x + area.w > src->w) {
		area.w = src->w - area.x;
	}
	if(area.y + area.h > src->h) {
		area.h = src->h - area.y;
	}

	surface const dst = create_compatible_surface(src,area.w,area.h);
	if(dst == NULL) {
		std::cerr << "Could not create a new surface in get_surface_portion()\n";
		return NULL;
	}

	SDL_BlitSurface(src,&area,dst, NULL);

	return dst;
}

namespace {

struct not_alpha
{
	not_alpha(SDL_PixelFormat& format) : fmt_(format) {}

	bool operator()(Uint32 pixel) const {
		Uint8 r, g, b, a;
		SDL_GetRGBA(pixel,&fmt_,&r,&g,&b,&a);
		return a != 0x00;
	}

private:
	SDL_PixelFormat& fmt_;
};

}

SDL_Rect get_non_transparent_portion(surface const &surf)
{
	SDL_Rect res = {0,0,0,0};
	const surface nsurf(make_neutral_surface(surf));
	if(nsurf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return res;
	}

	const not_alpha calc(*(nsurf->format));

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

void pixel_data::read(const config& cfg) {
	const std::string& red = cfg["red"];
	const std::string& green = cfg["green"];
	const std::string& blue = cfg["blue"];

	if (red.empty())
		r = 0;
	else
		r = atoi(red.c_str());

	if (green.empty())
		g = 0;
	else
		g = atoi(green.c_str());

	if (blue.empty())
		b = 0;
	else
		b = atoi(blue.c_str());
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
