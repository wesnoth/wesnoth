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
 *  Support-routines for the SDL-graphics-library.
 */

#include "global.hpp"
#include "color_range.hpp"

#include "sdl/utils.hpp"
#include "sdl/alpha.hpp"
#include "sdl/rect.hpp"

#include "floating_point_emulation.hpp"
#include "neon.hpp"
#include "video.hpp"
#include "xBRZ/xbrz.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

#include <boost/math/constants/constants.hpp>

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

SDL_Color int_to_color(const Uint32 rgb)
{
	SDL_Color result;
	result.r = (0x00FF0000 & rgb )>> 16;
	result.g = (0x0000FF00 & rgb) >> 8;
	result.b = (0x000000FF & rgb);
#ifdef SDL_GPU
	result.unused = SDL_ALPHA_OPAQUE;
#else
	result.a = SDL_ALPHA_OPAQUE;
#endif
	return result;
}

SDL_Color string_to_color(const std::string& color_string)
{
	SDL_Color color;

	std::vector<Uint32> temp_rgb;
	if(string2rgb(color_string, temp_rgb) && !temp_rgb.empty()) {
		color = int_to_color(temp_rgb[0]);
	}

	return color;
}

SDL_Color create_color(const unsigned char red
		, unsigned char green
		, unsigned char blue
		, unsigned char alpha)
{
	SDL_Color result;
	result.r = red;
	result.g = green;
	result.b = blue;
	result.a = alpha;

	return result;
}

SDLKey sdl_keysym_from_name(std::string const &keyname)
{
	return SDL_GetKeyFromName(keyname.c_str());
}

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}

bool is_neutral(const surface& surf)
{
	return (surf->format->BytesPerPixel == 4 &&
		surf->format->Rmask == 0xFF0000u &&
		(surf->format->Amask | 0xFF000000u) == 0xFF000000u);
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
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		const Uint32* const src_pixels = src_lock.pixels();
		Uint32* dst_pixels = dst_lock.pixels();

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
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		const Uint32* const src_pixels = src_lock.pixels();
		Uint32* dst_pixels = dst_lock.pixels();

		for(unsigned y = 0; y < static_cast<unsigned>(h); ++y) {
		  for(unsigned x = 0; x < static_cast<unsigned>(src->w); ++x) {

				*dst_pixels++ = src_pixels[x];
			}
		}
	}

	return optimize ? create_optimized_surface(dst) : dst;
}

#ifdef PANDORA
static void
scale_surface_down(surface& dst, const surface& src, const int w_dst, const int h_dst)
{
	const_surface_lock src_lock(src);
	surface_lock dst_lock(dst);

	const Uint32* const src_pixels = src_lock.pixels();
	Uint32* const dst_pixels = dst_lock.pixels();

	int y_dst = 0;       // The current y in the destination surface

	int y_src = 0;       // The current y in the source surface
	int y_src_next = 0;  // The next y in the source surface
	int y_step = 0;      // The y stepper
	int h_src = src->h;  // The height of the source surface

	for( ; y_dst != h_dst; ++y_dst, y_src = y_src_next) {

		y_step += h_src;
		do {
			++y_src_next;
			y_step -= h_dst;
		} while(y_step >= h_dst);

		int x_dst = 0;       // The current x in the destination surface

		int x_src = 0;       // The current x in the source surface
		int x_src_next = 0;  // The next x in the source surface
		int x_step = 0;      // The x stepper
		int w_src = src->w;  // The width of the source surface

		for( ; x_dst != w_dst; ++x_dst, x_src = x_src_next) {

			x_step += w_src;
			do {
				++x_src_next;
				x_step -= w_dst;
			} while(x_step >= w_dst);

			int r_sum = 0, g_sum = 0, b_sum = 0, a_sum = 0;
			int samples = 0;

			// We now have a rectangle, (xsrc,ysrc,xratio,yratio)
			// which we want to derive the pixel from
			for(int x = x_src; x < x_src_next; ++x) {
				for(int y = y_src; y < y_src_next; ++y) {

					++samples;

					const Uint32 pixel = src_pixels[y_src * w_src + x_src];
					const Uint8 a = pixel >> 24;
					if(a) {
						a_sum += a;
						r_sum += a * static_cast<Uint8>(pixel >> 16);
						g_sum += a * static_cast<Uint8>(pixel >> 8);
						b_sum += a * static_cast<Uint8>(pixel);
					}
				}
			}

			if(a_sum) {

				const int adjustment = (a_sum | 1) >> 1;
				r_sum += adjustment;
				g_sum += adjustment;
				b_sum += adjustment;

				r_sum /= a_sum;
				g_sum /= a_sum;
				b_sum /= a_sum;

				assert(samples == (x_src_next - x_src) * (y_src_next - y_src));
				if(samples != 1) {
					a_sum += (samples | 1) >> 1;
					a_sum /= samples;
				}
			}

			dst_pixels[y_dst * w_dst + x_dst] =
					  static_cast<Uint8>(a_sum) << 24
					| static_cast<Uint8>(r_sum) << 16
					| static_cast<Uint8>(g_sum) << 8
					| static_cast<Uint8>(b_sum);
		}
	}
}

#endif

Uint32 blend_rgba(const surface& surf, unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned char drop)
{
	// We simply decrement each component.
	if(r < drop) r = 0; else r -= drop;
	if(g < drop) g = 0; else g -= drop;
	if(b < drop) b = 0; else b -= drop;

	return SDL_MapRGBA(surf->format, r, g, b, a);
}

surface scale_surface_xbrz(const surface & surf, size_t z)
{
	if(surf == NULL)
		return NULL;

	if (z > 5) {
		std::cerr << "Cannot use xbrz scaling with zoom factor > 5." << std::endl;
		z = 1;
	}


	if (z == 1) {
		return create_optimized_surface(surf);
	}

	surface dst(create_neutral_surface(surf->w *z, surf->h * z));

	if (z == 0) {
		std::cerr << "Create an empty image\n";
		return create_optimized_surface(dst);
	}

	surface src(make_neutral_surface(surf));

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	{
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		xbrz::scale(z, src_lock.pixels(), dst_lock.pixels(), surf->w, surf->h);
	}
	return create_optimized_surface(dst);
}

surface scale_surface_nn (const surface & surf, int w, int h)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if (surf == NULL)
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

	{
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		xbrz::nearestNeighborScale(src_lock.pixels(), surf->w, surf->h, dst_lock.pixels(), w, h);
	}
	return create_optimized_surface(dst);
}

// NOTE: Don't pass this function 0 scaling arguments.
surface scale_surface(const surface &surf, int w, int h) {
	return scale_surface(surf, w, h, true);
}

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

	return optimize ? create_optimized_surface(dst) : dst;
}

surface scale_surface_sharp(const surface& surf, int w, int h, bool optimize)
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

#ifdef PANDORA
	scale_surface_down(dst, src, w, h);
#else
	{
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		const Uint32* const src_pixels = src_lock.pixels();
		Uint32* const dst_pixels = dst_lock.pixels();

		tfloat xratio = tfloat(surf->w) / w;
		tfloat yratio = tfloat(surf->h) / h;

		tfloat ysrc;
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			tfloat xsrc;
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				tfloat red, green, blue, alpha;

				tfloat summation;

				// We now have a rectangle, (xsrc,ysrc,xratio,yratio)
				// which we want to derive the pixel from
				for(tfloat xloc = xsrc; xloc < xsrc+xratio; xloc += 1) {
					const tfloat xsize = std::min<tfloat>(floor(xloc + 1)-xloc,xsrc+xratio-xloc);

					for(tfloat yloc = ysrc; yloc < ysrc+yratio; yloc += 1) {
						const int xsrcint = std::max<int>(0,std::min<int>(src->w-1,xsrc.to_int()));
						const int ysrcint = std::max<int>(0,std::min<int>(src->h-1,ysrc.to_int()));
						const tfloat ysize = std::min<tfloat>(floor(yloc+1)-yloc,ysrc+yratio-yloc);

						Uint8 r,g,b,a;

						SDL_GetRGBA(src_pixels[ysrcint*src->w + xsrcint],src->format,&r,&g,&b,&a);
						tfloat value = xsize * ysize;
						summation += value;
						if (!a) continue;
						value *= a;
						alpha += value;
						red += r * value;
						green += g * value;
						blue += b * value;
					}
				}

				if (alpha != 0) {
					red = red / alpha + 0.5;
					green = green / alpha + 0.5;
					blue = blue / alpha + 0.5;
					alpha = alpha / summation + 0.5;
				}

				dst_pixels[ydst*dst->w + xdst] = SDL_MapRGBA(
				dst->format
				, red.to_int()
				, green.to_int()
				, blue.to_int()
				, alpha.to_int());
			}

		}
	}
#endif

	return optimize ? create_optimized_surface(dst) : dst;
}


surface tile_surface(const surface& surf, int w, int h, bool optimize)
{
	if (surf->w == w && surf->h == h) {
		return surf;
	}

	surface dest(create_neutral_surface(w, h));
	surface src(make_neutral_surface(surf));

	if (src == NULL || dest == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return NULL;
	}

	{
		const_surface_lock srclock(src);
		surface_lock destlock(dest);

		const Uint32* srcpixels = srclock.pixels();
		Uint32* destpixels = destlock.pixels();

		const int& sw = src->w;
		const int& sh = src->h;

		const int xoff = (w - sw) / 2;
		const int yoff = (h - sh) / 2;

		for (int i = 0; i<w*h; ++i) {
			int x = ((i % w) - xoff);
			int y = ((i / w) - yoff);

			while ((x += sw) < 0) { /* DO NOTHING */ }
			while ((y += sh) < 0) { /* DO NOTHING */ }

			const int sx = x % sw;
			const int sy = y % sh;

			destpixels[i] = srcpixels[sy*sw + sx];
		}
	}

	return optimize ? create_optimized_surface(dest) : dest;
}

surface adjust_surface_color(const surface &surf, int red, int green, int blue, bool optimize)
{
	if(surf == NULL)
		return NULL;

	if((red == 0 && green == 0 && blue == 0))
		return optimize ? create_optimized_surface(surf) : surf;

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

surface monochrome_image(const surface &surf, const int threshold, bool optimize)
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
				Uint8 r, g, b, result;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				// first convert the pixel to grayscale
				// if the resulting value is above the threshold make it black
				// else make it white
				result = static_cast<Uint8>(0.299 * r + 0.587 * g + 0.114 * b) > threshold ? 255 : 0;

				*beg = (alpha << 24) | (result << 16) | (result << 8) | result;
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface sepia_image(const surface &surf, bool optimize)
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

				// this is the formula for applying a sepia effect
				// that can be found on various web sites
				// for example here: https://software.intel.com/sites/default/files/article/346220/sepiafilter-intelcilkplus.pdf
				Uint8 outRed = std::min(255, static_cast<int>((r * 0.393) + (g * 0.769) + (b * 0.189)));
				Uint8 outGreen = std::min(255, static_cast<int>((r * 0.349) + (g * 0.686) + (b * 0.168)));
				Uint8 outBlue = std::min(255, static_cast<int>((r * 0.272) + (g * 0.534) + (b * 0.131)));

				*beg = (alpha << 24) | (outRed << 16) | (outGreen << 8) | (outBlue);
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface negative_image(const surface &surf, const int thresholdR, const int thresholdG, const int thresholdB, bool optimize)
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
				Uint8 r, g, b, newR, newG, newB;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				// invert he channel only if its value is greater than the supplied threshold
				// this can be used for solarization effects
				// for a full negative effect, use a value of -1
				// 255 is a no-op value (doesn't do anything, since a Uint8 cannot contain a greater value than that)
				newR = r > thresholdR ? 255 - r : r;
				newG = g > thresholdG ? 255 - g : g;
				newB = b > thresholdB ? 255 - b : b;

				*beg = (alpha << 24) | (newR << 16) | (newG << 8) | (newB);
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface alpha_to_greyscale(const surface &surf, bool optimize)
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

			*beg = (0xff << 24) | (alpha << 16) | (alpha << 8) | alpha;

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface wipe_alpha(const surface &surf, bool optimize)
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

			*beg = 0xff000000 | *beg;

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

surface swap_channels_image(const surface& surf, channel r, channel g, channel b, channel a, bool optimize) {
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
				Uint8 red, green, blue, newRed, newGreen, newBlue, newAlpha;
				red = (*beg) >> 16;
				green = (*beg) >> 8;
				blue = (*beg);

				switch (r) {
					case RED:
						newRed = red;
						break;
					case GREEN:
						newRed = green;
						break;
					case BLUE:
						newRed = blue;
						break;
					case ALPHA:
						newRed = alpha;
						break;
					default:
						return NULL;
				}

				switch (g) {
					case RED:
						newGreen = red;
						break;
					case GREEN:
						newGreen = green;
						break;
					case BLUE:
						newGreen = blue;
						break;
					case ALPHA:
						newGreen = alpha;
						break;
					default:
						return NULL;
				}

				switch (b) {
					case RED:
						newBlue = red;
						break;
					case GREEN:
						newBlue = green;
						break;
					case BLUE:
						newBlue = blue;
						break;
					case ALPHA:
						newBlue = alpha;
						break;
					default:
						return NULL;
				}

				switch (a) {
					case RED:
						newAlpha = red;
						break;
					case GREEN:
						newAlpha = green;
						break;
					case BLUE:
						newAlpha = blue;
						break;
					case ALPHA:
						newAlpha = alpha;
						break;
					default:
						return NULL;
				}

				*beg = (newAlpha << 24) | (newRed << 16) | (newGreen << 8) | newBlue;
			}

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

surface recolor_image(surface surf, const std::map<Uint32, Uint32>& map_rgb, bool optimize){
	if(surf == NULL)
		return NULL;

	if(!map_rgb.empty()){
	     surface nsurf(make_neutral_surface(surf));
	     if(nsurf == NULL) {
			std::cerr << "failed to make neutral surface\n";
			return NULL;
	     }

		surface_lock lock(nsurf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;

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

surface mask_surface(const surface &surf, const surface &mask, bool* empty_result, const std::string& filename)
{
	if(surf == NULL) {
		return NULL;
	}
	if(mask == NULL) {
		return surf;
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
		std::stringstream ss;
		ss << "Detected an image with bad dimensions: ";
		if(!filename.empty()) ss << filename << ": ";
		ss << nsurf->w << "x" << nsurf->h << "\n";
		std::cerr << ss.str();
		std::cerr << "It will not be masked, please use: "<< nmask->w << "x" << nmask->h << "\n";
		return nsurf;
	}

	bool empty = true;
	{
		surface_lock lock(nsurf);
		const_surface_lock mlock(nmask);

		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;
		const Uint32* mbeg = mlock.pixels();
		const Uint32* mend = mbeg + nmask->w*nmask->h;

		while(beg != end && mbeg != mend) {
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				Uint8 malpha = (*mbeg) >> 24;
				if (alpha > malpha) {
					alpha = malpha;
				}
				if(alpha)
					empty = false;

				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
			++mbeg;
		}
	}
	if(empty_result)
		*empty_result = empty;

	return nsurf;
	//return create_optimized_surface(nsurf);
}

bool in_mask_surface(const surface &surf, const surface &mask)
{
	if(surf == NULL) {
		return false;
	}
	if(mask == NULL){
		return true;
	}

	if (surf->w != mask->w || surf->h != mask->h ) {
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
		const_surface_lock mlock(nmask);

		const Uint32* mbeg = mlock.pixels();
		const Uint32* mend = mbeg + nmask->w*nmask->h;
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

surface submerge_alpha(const surface &surf, int depth, float alpha_base, float alpha_delta,  bool optimize)
{
	if(surf== NULL) {
		return NULL;
	}

	surface nsurf(make_neutral_surface(surf));

	{
		surface_lock lock(nsurf);

		Uint32* beg = lock.pixels();
		Uint32* limit = beg + (nsurf->h-depth) * nsurf->w ;
		Uint32* end = beg + nsurf->w * nsurf->h;
		beg = limit; // directlt jump to the bottom part

		while(beg != end){
			Uint8 alpha = (*beg) >> 24;

			if(alpha) {
				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);
				int d = (beg-limit)/nsurf->w;  // current depth in pixels
				float a = alpha_base - d * alpha_delta;
				fixed_t amount = ftofxp(a<0?0:a);
				alpha = std::min<unsigned>(unsigned(fxpmult(alpha,amount)),255);
				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}

			++beg;
		}

/*
		for(int y = submerge_height; y < nsurf->h; ++y) {
			Uint32* cur = beg + y * nsurf->w;
			Uint32* row_end = beg + (y+1) * nsurf->w;
			float d = y * 1.0 / depth;
			double a = 0.2;//std::max<double>(0, (1-d)*0.3);
			fixed_t amount = ftofxp(a);
			while(cur != row_end) {
				Uint8 alpha = (*cur) >> 24;

				if(alpha) {
					Uint8 r, g, b;
					r = (*cur) >> 16;
					g = (*cur) >> 8;
					b = (*cur);
					alpha = std::min<unsigned>(unsigned(fxpmult(alpha,amount)),255);
					*cur = (alpha << 24) + (r << 16) + (g << 8) + b;
				}

				++cur;
			}
		}*/

	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;

}

surface light_surface(const surface &surf, const surface &lightmap, bool optimize)
{
	if(surf == NULL) {
		return NULL;
	}
	if(lightmap == NULL) {
		return surf;
	}

	surface nsurf = make_neutral_surface(surf);

	if(nsurf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}
	if (nsurf->w != lightmap->w) {
		// we don't support efficiently different width.
		// (different height is not a real problem)
		// This function is used on all hexes and usually only for that
		// so better keep it simple and efficient for the normal case
		std::cerr << "Detected an image with bad dimensions: " << nsurf->w << "x" << nsurf->h << "\n";
		std::cerr << "It will not be lighted, please use: "<< lightmap->w << "x" << lightmap->h << "\n";
		return nsurf;
	}
	{
		surface_lock lock(nsurf);
		const_surface_lock llock(lightmap);

		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w * nsurf->h;
		const Uint32* lbeg = llock.pixels();
		const Uint32* lend = lbeg + lightmap->w * lightmap->h;

		while(beg != end && lbeg != lend) {
			Uint8 alpha = (*beg) >> 24;
 			if(alpha) {
				Uint8 lr, lg, lb;

				lr = (*lbeg) >> 16;
				lg = (*lbeg) >> 8;
				lb = (*lbeg);

				Uint8 r, g, b;
				r = (*beg) >> 16;
				g = (*beg) >> 8;
				b = (*beg);

				int dr = (static_cast<int>(lr) - 128) * 2;
				int dg = (static_cast<int>(lg) - 128) * 2;
				int db = (static_cast<int>(lb) - 128) * 2;
				//note that r + dr will promote r to int (needed to avoid Uint8 math)
				r = std::max<int>(0,std::min<int>(255, r + dr));
				g = std::max<int>(0,std::min<int>(255, g + dg));
				b = std::max<int>(0,std::min<int>(255, b + db));

				*beg = (alpha << 24) + (r << 16) + (g << 8) + b;
			}
			++beg;
			++lbeg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
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

	SDL_Rect rect = sdl::create_rect(0, 0, surf->w, surf->h);
	blur_surface(res, rect, depth);

	return optimize ? create_optimized_surface(res) : res;
}

void blur_surface(surface& surf, SDL_Rect rect, int depth)
{
	if(surf == NULL) {
		return;
	}

	const int max_blur = 256;
	if(depth > max_blur) {
		depth = max_blur;
	}

	Uint32 queue[max_blur];
	const Uint32* end_queue = queue + max_blur;

	const Uint32 ff = 0xff;

	const unsigned pixel_offset = rect.y * surf->w + rect.x;

	surface_lock lock(surf);
	for(int y = 0; y < rect.h; ++y) {
		const Uint32* front = &queue[0];
		Uint32* back = &queue[0];
		Uint32 red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + pixel_offset + y * surf->w;
		for(int x = 0; x <= depth && x < rect.w; ++x, ++p) {
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
		for(int x = 0; x < rect.w; ++x, ++p) {
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

	for(int x = 0; x < rect.w; ++x) {
		const Uint32* front = &queue[0];
		Uint32* back = &queue[0];
		Uint32 red = 0, green = 0, blue = 0, avg = 0;
		Uint32* p = lock.pixels() + pixel_offset + x;
		for(int y = 0; y <= depth && y < rect.h; ++y, p += surf->w) {
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
		for(int y = 0; y < rect.h; ++y, p += surf->w) {
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
			assert(avg);
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
	if(surf == NULL)
		return NULL;

	surface res = create_compatible_surface(surf, r.w, r.h);

	if(res == NULL) {
		std::cerr << "Could not create a new surface in cut_surface()\n";
		return NULL;
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

		Uint16 ratio = amount * 256;
		const Uint16 red   = ratio * static_cast<Uint8>(color >> 16);
		const Uint16 green = ratio * static_cast<Uint8>(color >> 8);
		const Uint16 blue  = ratio * static_cast<Uint8>(color);
		ratio = 256 - ratio;

#ifdef PANDORA
		/*
		 * Use an optimised version of the generic algorithm. The optimised
		 * version processes 8 pixels a time. If the number of pixels is not an
		 * exact multiple of 8 it falls back to the generic algorithm to handle
		 * the last pixels.
		 */
		uint16x8_t vred = vdupq_n_u16(red);
		uint16x8_t vgreen = vdupq_n_u16(green);
		uint16x8_t vblue = vdupq_n_u16(blue);

		uint8x8_t vratio = vdup_n_u8(ratio);

		const int div = (nsurf->w * surf->h) / 8;
		for(int i = 0; i < div; ++i, beg += 8) {
			uint8x8x4_t rgba = vld4_u8(reinterpret_cast<Uint8*>(beg));

			uint16x8_t b = vmull_u8(rgba.val[0], vratio);
			uint16x8_t g = vmull_u8(rgba.val[1], vratio);
			uint16x8_t r = vmull_u8(rgba.val[2], vratio);

			b = vaddq_u16(b, vblue);
			g = vaddq_u16(g, vgreen);
			r = vaddq_u16(r, vred);

			rgba.val[0] = vshrn_n_u16(b, 8);
			rgba.val[1] = vshrn_n_u16(g, 8);
			rgba.val[2] = vshrn_n_u16(r, 8);

			vst4_u8(reinterpret_cast<Uint8*>(beg), rgba);
		}
#endif
		while(beg != end) {
			Uint8 a = static_cast<Uint8>(*beg >> 24);
			Uint8 r = (ratio * static_cast<Uint8>(*beg >> 16) + red)   >> 8;
			Uint8 g = (ratio * static_cast<Uint8>(*beg >> 8)  + green) >> 8;
			Uint8 b = (ratio * static_cast<Uint8>(*beg)       + blue)  >> 8;

			*beg = (a << 24) | (r << 16) | (g << 8) | b;

			++beg;
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}

/* Simplified RotSprite algorithm.
 * http://en.wikipedia.org/wiki/Image_scaling#RotSprite
 * Lifted from: http://github.com/salmonmoose/SpriteRotator
 * 1) Zoom the source image by a certain factor.
 * 2) Scan the zoomed source image at every step=offset and put it in the result. */
surface rotate_any_surface(const surface& surf, float angle, int zoom, int offset, bool optimize)
{
	int src_w, src_h, dst_w, dst_h;
	float min_x, max_x, min_y, max_y, sine, cosine;
	{
		// convert angle to radiant (angle * 2 * PI) / 360
		const float radians = angle * boost::math::constants::pi<float>() / 180;
		cosine = static_cast<float>(cos(radians));
		sine   = static_cast<float>(sin(radians));
		// calculate the size of the dst image
		src_w = surf->w * zoom;
		src_h = surf->h * zoom;
		/* See http://en.wikipedia.org/wiki/Rotation_(mathematics) */
		const float point_1x = src_h * -sine;
		const float point_1y = src_h * cosine;
		const float point_2x = src_w * cosine - src_h * sine;
		const float point_2y = src_h * cosine + src_w * sine;
		const float point_3x = src_w * cosine;
		const float point_3y = src_w * sine;
		/* After the rotation, the new image has different dimensions.
		 * E.g.: The maximum height equals the former diagonal in case the angle is 45, 135, 225 or 315 degree.
		 * See http://en.wikipedia.org/wiki/File:Rotation_illustration2.svg to get the idea. */
		min_x = std::min(0.0F, std::min(point_1x, std::min(point_2x, point_3x)));
		min_y = std::min(0.0F, std::min(point_1y, std::min(point_2y, point_3y)));
		max_x = (angle >  90 && angle < 180) ? 0 : std::max(point_1x, std::max(point_2x, point_3x));
		max_y = (angle > 180 && angle < 270) ? 0 : std::max(point_1y, std::max(point_2y, point_3y));
		dst_w = static_cast<int>(ceil(std::abs(max_x) - min_x)) / zoom;
		dst_h = static_cast<int>(ceil(std::abs(max_y) - min_y)) / zoom;
	}
	surface dst(create_neutral_surface(dst_w, dst_h));
	{
		surface_lock dst_lock(dst);
		const surface src = scale_surface(surf, src_w, src_h, false);
		const_surface_lock src_lock(src);
		const float scale =   1.f / zoom;
		const int   max_x = dst_w * zoom;
		const int   max_y = dst_h * zoom;
		/* Loop through the zoomed src image,
		 * take every pixel in steps with offset distance and place it in the dst image. */
		for (int x = 0; x < max_x; x += offset)
			for (int y = 0; y < max_y; y += offset) {
				// calculate the src pixel that fits in the dst
				const float source_x = (x + min_x)*cosine + (y + min_y)*sine;
				const float source_y = (y + min_y)*cosine - (x + min_x)*sine;
				// if the pixel exists on the src surface
				if (source_x >= 0 && source_x < src_w
						&& source_y >= 0 && source_y < src_h)
					// get it from the src surface and place it on the dst surface
					put_pixel(dst, dst_lock, x*scale , y*scale, // multiply with scale
							get_pixel(src, src_lock, source_x, source_y));
			}
	}
	return optimize ? create_optimized_surface(dst) : dst;
}

void put_pixel(const surface& surf, surface_lock& surf_lock, int x, int y, Uint32 pixel)
{
	const int bpp = surf->format->BytesPerPixel;
	/* dst is the address to the pixel we want to set */
	Uint8* const dst = reinterpret_cast<Uint8*>(surf_lock.pixels()) + y * surf->pitch + x * bpp;
	switch (bpp) {
	case 1:
		*dst = pixel;
		break;
	case 2:
		*reinterpret_cast<Uint16*>(dst) = pixel;
		break;
	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			dst[0] = (pixel >> 16) & 0xff;
			dst[1] = (pixel >> 8) & 0xff;
			dst[2] = pixel & 0xff;
		} else {
			dst[0] = pixel & 0xff;
			dst[1] = (pixel >> 8) & 0xff;
			dst[2] = (pixel >> 16) & 0xff;
		}
		break;
	case 4:
		*reinterpret_cast<Uint32*>(dst) = pixel;
		break;
	default:
		break;
	}
}

Uint32 get_pixel(const surface& surf, const const_surface_lock& surf_lock, int x, int y)
{
	const int bpp = surf->format->BytesPerPixel;
	/* p is the address to the pixel we want to retrieve */
	const Uint8* const src = reinterpret_cast<const Uint8*>(surf_lock.pixels()) + y * surf->pitch + x * bpp;
	switch (bpp) {
	case 1:
		return *src;
	case 2:
		return *reinterpret_cast<const Uint16*>(src);
	case 3:
		if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return src[0] << 16 | src[1] << 8 | src[2];
		else
			return src[0] | src[1] << 8 | src[2] << 16;
		break;
	case 4:
		return *reinterpret_cast<const Uint32*>(src);
	}
	return 0;
}

// Rotates a surface 180 degrees.
surface rotate_180_surface(const surface &surf, bool optimize)
{
	if ( surf == NULL )
		return NULL;

	// Work with a "neutral" (unoptimized) surface.
	surface nsurf(make_neutral_surface(surf));

	if ( nsurf == NULL ) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{// Code block to limit the scope of the surface lock.
		surface_lock lock(nsurf);
		Uint32* const pixels = lock.pixels();

		// Swap pixels in the upper half of the image with
		// those in the lower half.
		for (int y=0; y != nsurf->h/2; ++y) {
			for(int x=0; x != nsurf->w; ++x) {
				const int index1 = y*nsurf->w + x;
				const int index2 = (nsurf->h-y)*nsurf->w - x - 1;
				std::swap(pixels[index1],pixels[index2]);
			}
		}

		if ( is_odd(nsurf->h) ) {
			// The middle row still needs to be processed.
			for (int x=0; x != nsurf->w/2; ++x) {
				const int index1 = (nsurf->h/2)*nsurf->w + x;
				const int index2 = (nsurf->h/2)*nsurf->w + (nsurf->w - x - 1);
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return optimize ? create_optimized_surface(nsurf) : nsurf;
}


// Rotates a surface 90 degrees, either clockwise or counter-clockwise.
surface rotate_90_surface(const surface &surf, bool clockwise, bool optimize)
{
	if ( surf == NULL )
		return NULL;

	// Work with "neutral" (unoptimized) surfaces.
	surface dst(create_neutral_surface(surf->h, surf->w)); // Flipped dimensions.
	surface src(make_neutral_surface(surf));

	if ( src == NULL  ||  dst == NULL ) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{// Code block to limit the scope of the surface locks.
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		const Uint32* const src_pixels = src_lock.pixels();
		Uint32* const dst_pixels = dst_lock.pixels();

		// Copy the pixels.
		for ( int y = 0; y != src->h; ++y ) {
			for ( int x = 0; x != src->w; ++x ) {
				const int src_index = y*src->w + x;
				const int dst_index = clockwise ?
				                          x*dst->w + (dst->w-1-y) :
				                          (dst->h-1-x)*dst->w + y;
				dst_pixels[dst_index] = src_pixels[src_index];
			}
		}
	}

	return optimize ? create_optimized_surface(dst) : dst;
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
		SDL_SetPaletteColors(s->format->palette, surf->format->palette->colors, 0, surf->format->palette->ncolors);
	}
	return s;
}

void blit_surface(const surface& surf,
	const SDL_Rect* srcrect, surface& dst, const SDL_Rect* dstrect)
{
	assert(surf);
	assert(dst);
	assert(is_neutral(dst));

	const surface& src = is_neutral(surf) ? surf : make_neutral_surface(surf);

	// Get the areas to blit
	SDL_Rect dst_rect = sdl::create_rect(0, 0, dst->w, dst->h);
	if(dstrect) {
		dst_rect.x = dstrect->x;
		dst_rect.w -= dstrect->x;

		dst_rect.y = dstrect->y;
		dst_rect.h -= dstrect->y;

	}

	SDL_Rect src_rect = sdl::create_rect(0, 0, src->w, src->h);
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

	{
		// Extra scoping used for the surface_lock.
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		const Uint32* const src_pixels = src_lock.pixels();
		Uint32* dst_pixels = dst_lock.pixels();

		for(unsigned y = 0; y < height; ++y) {
			for(unsigned x = 0; x < width; ++x) {

				// We need to do the blitting using some optimizations
				// if the src is fully transparent we can ignore this pixel
				// if the src is fully opaque we can overwrite the destination with this pixel
				// if the destination is fully transparent we replace us with the source
				//
				// We do these optimizations between the extraction of the variables
				// to avoid creating variables not used (it might save us some cycles).

				const int src_offset = (y + src_rect.y) * src->w + (x + src_rect.x);
				assert(src_offset < src->w * src->h);
				const Uint32 src_pixel = src_pixels[src_offset];
				const Uint8 src_a = (src_pixel & 0xFF000000) >> 24;

				if(!src_a) {
					// Fully transparent source, ignore
					continue;
				}

				const ptrdiff_t dst_offset = (y + dst_rect.y) * dst->w + (x + dst_rect.x);
				assert(dst_offset < dst->w * dst->h);
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

					// acquired the data now do the blitting
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

	sdl_copy_portion(src, &area, dst, NULL);

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
	surface nsurf(make_neutral_surface(surf));
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

bool operator==(const SDL_Color& a, const SDL_Color& b) {
	return a.r == b.r && a.g == b.g && a.b == b.b;
}

bool operator!=(const SDL_Color& a, const SDL_Color& b) {
	return !operator==(a,b);
}

SDL_Color inverse(const SDL_Color& color) {
	SDL_Color inverse;
	inverse.r = 255 - color.r;
	inverse.g = 255 - color.g;
	inverse.b = 255 - color.b;
	inverse.a = 0;
	return inverse;
}

surface_restorer::surface_restorer() : target_(NULL), rect_(sdl::empty_rect), surface_(NULL)
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
	SDL_Rect dst2 = sdl::intersect_rects(dst, rect_);
	if (dst2.w == 0 || dst2.h == 0)
		return;
	SDL_Rect src = dst2;
	src.x -= rect_.x;
	src.y -= rect_.y;
	sdl_blit(surface_, &src, target_->getSurface(), &dst2);
	update_rect(dst2);
}

void surface_restorer::restore() const
{
	if (surface_.null())
		return;
	SDL_Rect dst = rect_;
	sdl_blit(surface_, NULL, target_->getSurface(), &dst);
	update_rect(rect_);
}

void surface_restorer::update()
{
	if(rect_.w <= 0 || rect_.h <= 0)
		surface_.assign(NULL);
	else
		surface_.assign(::get_surface_portion(target_->getSurface(),rect_));
}

void surface_restorer::cancel()
{
	surface_.assign(NULL);
}

void draw_centered_on_background(surface surf, const SDL_Rect& rect, const SDL_Color& color, surface target)
{
	clip_rect_setter clip_setter(target, &rect);

	Uint32 col = SDL_MapRGBA(target->format, color.r, color.g, color.b, color.a);
	//TODO: only draw background outside the image
	SDL_Rect r = rect;
	sdl::fill_rect(target, &r, col);

	if (surf != NULL) {
		r.x = rect.x + (rect.w-surf->w)/2;
		r.y = rect.y + (rect.h-surf->h)/2;
		sdl_blit(surf, NULL, target, &r);
	}
	update_rect(rect);
}

std::ostream& operator<<(std::ostream& s, const SDL_Rect& rect)
{
	s << rect.x << ',' << rect.y << " x "  << rect.w << ',' << rect.h;
	return s;
}
