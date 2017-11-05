/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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

#include "sdl/utils.hpp"
#include "sdl/rect.hpp"
#include "color.hpp"

#include "serialization/string_utils.hpp"
#include "video.hpp"
#include "xBRZ/xbrz.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

#include <boost/circular_buffer.hpp>
#include <boost/math/constants/constants.hpp>

version_info sdl_get_version()
{
	SDL_version sdl_version;
	SDL_GetVersion(&sdl_version);
	return version_info(sdl_version.major, sdl_version.minor, sdl_version.patch);
}

bool is_neutral(const surface& surf)
{
	return (surf->format->BytesPerPixel == 4 &&
			surf->format->Rmask == SDL_RED_MASK &&
			(surf->format->Amask | SDL_ALPHA_MASK) == SDL_ALPHA_MASK);
}

static SDL_PixelFormat& get_neutral_pixel_format()
	{
		static bool first_time = true;
		static SDL_PixelFormat format;

		if(first_time) {
			first_time = false;
			surface surf(SDL_CreateRGBSurface(0,1,1,32,SDL_RED_MASK,SDL_GREEN_MASK,
											  SDL_BLUE_MASK,SDL_ALPHA_MASK));
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

	surface result = SDL_ConvertSurface(surf,&get_neutral_pixel_format(),0);

	return result;
}

surface create_neutral_surface(int w, int h)
{
	if (w < 0 || h < 0) {
		std::cerr << "error : neutral surface with negative dimensions\n";
		return nullptr;
	}

	SDL_PixelFormat format = get_neutral_pixel_format();
	surface result = SDL_CreateRGBSurface(0, w, h,
			format.BitsPerPixel,
			format.Rmask,
			format.Gmask,
			format.Bmask,
			format.Amask);

	return result;
}

surface stretch_surface_horizontal(
		const surface& surf, const unsigned w)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if(surf == nullptr)
		return nullptr;

	if(static_cast<int>(w) == surf->w) {
		return surf;
	}
	assert(w > 0);

	surface dst(create_neutral_surface(w, surf->h));

	surface src(make_neutral_surface(surf));
	// Now both surfaces are always in the "neutral" pixel format

	if(src == nullptr || dst == nullptr) {
		std::cerr << "Could not create surface to scale onto\n";
		return nullptr;
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

	return dst;
}

surface stretch_surface_vertical(
		const surface& surf, const unsigned h)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if(surf == nullptr)
		return nullptr;

	if(static_cast<int>(h) == surf->h) {
		return surf;
	}
	assert(h > 0);

	surface dst(create_neutral_surface(surf->w, h));

	surface src(make_neutral_surface(surf));
	// Now both surfaces are always in the "neutral" pixel format

	if(src == nullptr || dst == nullptr) {
		std::cerr << "Could not create surface to scale onto\n";
		return nullptr;
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

	return dst;
}

surface scale_surface_xbrz(const surface & surf, size_t z)
{
	if(surf == nullptr)
		return nullptr;

	if (z > 5) {
		std::cerr << "Cannot use xbrz scaling with zoom factor > 5." << std::endl;
		z = 1;
	}

	if (z == 1) {
		surface temp = surf; // TODO: no temp surface
		return temp;
	}

	surface dst(create_neutral_surface(surf->w *z, surf->h * z));

	if (z == 0) {
		std::cerr << "Create an empty image\n";
		return dst;
	}

	surface src(make_neutral_surface(surf));

	if(src == nullptr || dst == nullptr) {
		std::cerr << "Could not create surface to scale onto\n";
		return nullptr;
	}

	{
		const_surface_lock src_lock(src);
		surface_lock dst_lock(dst);

		xbrz::scale(z, src_lock.pixels(), dst_lock.pixels(), surf->w, surf->h);
	}

	return dst;
}

surface scale_surface_nn (const surface & surf, int w, int h)
{
	// Since SDL version 1.1.5 0 is transparent, before 255 was transparent.
	assert(SDL_ALPHA_TRANSPARENT==0);

	if (surf == nullptr)
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

		xbrz::nearestNeighborScale(src_lock.pixels(), surf->w, surf->h, dst_lock.pixels(), w, h);
	}

	return dst;
}

// NOTE: Don't pass this function 0 scaling arguments.
surface scale_surface(const surface &surf, int w, int h)
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

	return dst;
}

surface scale_surface_legacy(const surface &surf, int w, int h)
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
				Uint32 rr,gg,bb,aa;
				Uint16 avg_r, avg_g, avg_b;
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

				const fixed_t east = 0x000000FF & xsrc;
				const fixed_t south = 0x000000FF & ysrc;
				const fixed_t north = 0xFF - south;
				const fixed_t west = 0xFF - east;

				pix[0] = *src_word;              // northwest
				pix[1] = *(src_word + dx);       // northeast
				pix[2] = *(src_word + dy);       // southwest
				pix[3] = *(src_word + dx + dy);  // southeast

				bilin[0] = north*west;
				bilin[1] = north*east;
				bilin[2] = south*west;
				bilin[3] = south*east;

				// Scope out the neighboorhood, see
				// what the pixel values are like.

				int count = 0;
				avg_r = avg_g = avg_b = 0;
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
				    count++;
				  }
				}
				if (count>0) {
				  avg_r /= count;
				  avg_b /= count;
				  avg_g /= count;
				}

				// Perform modified bilinear interpolation.
				// Don't trust any color information from
				// an RGBA sample when the alpha channel
				// is set to fully transparent.
				//
				// Some of the input images are hex tiles,
				// created using a hexagon shaped alpha channel
				// that is either set to full-on or full-off.

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
				*dst_word = (a << 24) + (r << 16) + (g << 8) + b;
			}
		}
	}

	return dst;
}


surface scale_surface_sharp(const surface& surf, int w, int h)
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

		float xratio = static_cast<float>(surf->w) / w;
		float yratio = static_cast<float>(surf->h) / h;

		float ysrc = 0.0f;
		for(int ydst = 0; ydst != h; ++ydst, ysrc += yratio) {
			float xsrc = 0.0f;
			for(int xdst = 0; xdst != w; ++xdst, xsrc += xratio) {
				float red = 0.0f, green = 0.0f, blue = 0.0f, alpha = 0.0f;

				float summation = 0.0f;

				// We now have a rectangle, (xsrc,ysrc,xratio,yratio)
				// which we want to derive the pixel from
				for(float xloc = xsrc; xloc < xsrc+xratio; xloc += 1) {
					const float xsize = std::min<float>(std::floor(xloc+1)-xloc,xsrc+xratio-xloc);

					for(float yloc = ysrc; yloc < ysrc+yratio; yloc += 1) {
						const int xsrcint = std::max<int>(0,std::min<int>(src->w-1,static_cast<int>(xsrc)));
						const int ysrcint = std::max<int>(0,std::min<int>(src->h-1,static_cast<int>(ysrc)));
						const float ysize = std::min<float>(std::floor(yloc+1)-yloc,ysrc+yratio-yloc);

						Uint8 r,g,b,a;

						SDL_GetRGBA(src_pixels[ysrcint*src->w + xsrcint],src->format,&r,&g,&b,&a);
						float value = xsize * ysize;
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
					red = red / alpha + 0.5f;
					green = green / alpha + 0.5f;
					blue = blue / alpha + 0.5f;
					alpha = alpha / summation + 0.5f;
				}

				dst_pixels[ydst*dst->w + xdst] = SDL_MapRGBA(
				dst->format
				, static_cast<uint8_t>(red)
				, static_cast<uint8_t>(green)
				, static_cast<uint8_t>(blue)
				, static_cast<uint8_t>(alpha));
			}

		}
	}

	return dst;
}


surface tile_surface(const surface& surf, int w, int h, bool centered)
{
	if (surf->w == w && surf->h == h) {
		return surf;
	}

	surface dest(create_neutral_surface(w, h));
	surface src(make_neutral_surface(surf));

	if (src == nullptr || dest == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
	}

	{
		const_surface_lock srclock(src);
		surface_lock destlock(dest);

		const Uint32* srcpixels = srclock.pixels();
		Uint32* destpixels = destlock.pixels();

		const int& sw = src->w;
		const int& sh = src->h;

		const int xoff = centered ? (w - sw) / 2 : 0;
		const int yoff = centered ? (h - sh) / 2 : 0;

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

	return dest;
}

surface adjust_surface_color(const surface &surf, int red, int green, int blue)
{
	if(surf == nullptr)
		return nullptr;

	if((red == 0 && green == 0 && blue == 0)) {
		surface temp = surf; // TODO: remove temp surface
		return temp;
	}

	surface nsurf(make_neutral_surface(surf));

	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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

	return nsurf;
}

surface greyscale_image(const surface &surf)
{
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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

	return nsurf;
}

surface monochrome_image(const surface &surf, const int threshold)
{
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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

	return nsurf;
}

surface sepia_image(const surface &surf)
{
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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

	return nsurf;
}

surface negative_image(const surface &surf, const int thresholdR, const int thresholdG, const int thresholdB)
{
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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

	return nsurf;
}

surface alpha_to_greyscale(const surface &surf)
{
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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

	return nsurf;
}

surface wipe_alpha(const surface &surf)
{
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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

	return nsurf;
}


surface shadow_image(const surface &surf)
{
	if(surf == nullptr)
		return nullptr;

	// we blur it, and reuse the neutral surface created by the blur function
	surface nsurf (blur_alpha_surface(surf, 2));

	if(nsurf == nullptr) {
		std::cerr << "failed to blur the shadow surface\n";
		return nullptr;
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

	return nsurf;
}

surface swap_channels_image(const surface& surf, channel r, channel g, channel b, channel a) {
	if(surf == nullptr)
		return nullptr;

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface\n";
		return nullptr;
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
						return nullptr;
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
						return nullptr;
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
						return nullptr;
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
						return nullptr;
				}

				*beg = (newAlpha << 24) | (newRed << 16) | (newGreen << 8) | newBlue;
			}

			++beg;
		}
	}

	return nsurf;
}

surface recolor_image(surface surf, const color_range_map& map_rgb)
{
	if(surf == nullptr)
		return nullptr;

	if(map_rgb.empty()) {
		return surf;
	}

	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
		std::cerr << "failed to make neutral surface" << std::endl;
		return nullptr;
	}

	surface_lock lock(nsurf);
	Uint32* beg = lock.pixels();
	Uint32* end = beg + nsurf->w*surf->h;

	while(beg != end) {
		Uint8 alpha = (*beg) >> 24;

		// Don't recolor invisible pixels.
		if(alpha) {
			// Palette use only RGB channels, so remove alpha
			Uint32 oldrgb = (*beg) | 0xFF000000;

			auto i = map_rgb.find(color_t::from_argb_bytes(oldrgb));
			if(i != map_rgb.end()) {
				*beg = (alpha << 24) | (i->second.to_argb_bytes() & 0x00FFFFFF);
			}
		}

		++beg;
	}

	return nsurf;
}

surface brighten_image(const surface &surf, fixed_t amount)
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

	return nsurf;
}

void adjust_surface_alpha(surface& surf, fixed_t amount)
{
	if(surf == nullptr) {
		return;
	}

	SDL_SetSurfaceAlphaMod(surf, Uint8(amount));
}

surface adjust_surface_alpha_add(const surface &surf, int amount)
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

	return nsurf;
}

surface mask_surface(const surface &surf, const surface &mask, bool* empty_result, const std::string& filename)
{
	if(surf == nullptr) {
		return nullptr;
	}
	if(mask == nullptr) {
		return surf;
	}

	surface nsurf = make_neutral_surface(surf);
	surface nmask(make_neutral_surface(mask));

	if(nsurf == nullptr || nmask == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
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
}

bool in_mask_surface(const surface &surf, const surface &mask)
{
	if(surf == nullptr) {
		return false;
	}
	if(mask == nullptr){
		return true;
	}

	if (surf->w != mask->w || surf->h != mask->h ) {
		// not same size, consider it doesn't fit
		return false;
	}

	surface nsurf = make_neutral_surface(surf);
	surface nmask(make_neutral_surface(mask));

	if(nsurf == nullptr || nmask == nullptr) {
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

surface submerge_alpha(const surface &surf, int depth, float alpha_base, float alpha_delta)
{
	if(surf== nullptr) {
		return nullptr;
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

	return nsurf;
}

surface light_surface(const surface &surf, const surface &lightmap)
{
	if(surf == nullptr) {
		return nullptr;
	}
	if(lightmap == nullptr) {
		return surf;
	}

	surface nsurf = make_neutral_surface(surf);

	if(nsurf == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
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

	return nsurf;
}


surface blur_surface(const surface &surf, int depth)
{
	if(surf == nullptr) {
		return nullptr;
	}

	surface res = make_neutral_surface(surf);

	if(res == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
	}

	SDL_Rect rect {0, 0, surf->w, surf->h};
	blur_surface(res, rect, depth);

	return res;
}

void blur_surface(surface& surf, SDL_Rect rect, int depth)
{
	if(surf == nullptr) {
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

surface blur_alpha_surface(const surface &surf, int depth)
{
	if(surf == nullptr) {
		return nullptr;
	}

	surface res = make_neutral_surface(surf);

	if(res == nullptr) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
	}

	const int max_blur = 256;
	if(depth > max_blur) {
		depth = max_blur;
	}

	boost::circular_buffer<Uint32> queue(depth*2+1);

	const Uint32 ff = 0xff;

	surface_lock lock(res);
	int x, y;
	// Iterate over rows, blurring each row horizontally
	for(y = 0; y < res->h; ++y) {
		// Sum of pixel values stored here
		Uint32 alpha=0, red = 0, green = 0, blue = 0;

		// Preload the first depth+1 pixels
		Uint32* p = lock.pixels() + y*res->w;
		for(x = 0; x <= depth && x < res->w; ++x, ++p) {
			alpha += ((*p) >> 24)&0xFF;
			red += ((*p) >> 16)&0xFF;
			green += ((*p) >> 8)&0xFF;
			blue += (*p)&0xFF;
			assert(!queue.full());
			queue.push_back(*p);
		}

		// This is the actual inner loop
		p = lock.pixels() + y*res->w;
		for(x = 0; x < res->w; ++x, ++p) {
			// Write the current average
			const Uint32 num = queue.size();
			*p = (std::min(alpha/num,ff) << 24) | (std::min(red/num,ff) << 16) | (std::min(green/num,ff) << 8) | std::min(blue/num,ff);

			// Unload earlier pixels that are now too far away
			if(x >= depth) {
				{
					const auto &front = queue.front();
					alpha -= (front >> 24)&0xFF;
					red -= (front >> 16)&0xFF;
					green -= (front >> 8)&0xFF;
					blue -= front&0xFF;
				}
				assert(!queue.empty());
				queue.pop_front();
			}

			// Add new pixels
			if(x + depth+1 < res->w) {
				Uint32* q = p + depth+1;
				alpha += ((*q) >> 24)&0xFF;
				red += ((*q) >> 16)&0xFF;
				green += ((*q) >> 8)&0xFF;
				blue += (*q)&0xFF;
				assert(!queue.full());
				queue.push_back(*q);
			}
		}
		assert(static_cast<int>(queue.size()) == std::min(depth, res->w));
		queue.clear();
	}

	// Iterate over columns, blurring each column vertically
	for(x = 0; x < res->w; ++x) {
		// Sum of pixel values stored here
		Uint32 alpha=0, red = 0, green = 0, blue = 0;

		// Preload the first depth+1 pixels
		Uint32* p = lock.pixels() + x;
		for(y = 0; y <= depth && y < res->h; ++y, p += res->w) {
			alpha += ((*p) >> 24)&0xFF;
			red += ((*p) >> 16)&0xFF;
			green += ((*p) >> 8)&0xFF;
			blue += *p&0xFF;
			assert(!queue.full());
			queue.push_back(*p);
		}

		// This is the actual inner loop
		p = lock.pixels() + x;
		for(y = 0; y < res->h; ++y, p += res->w) {
			// Write the current average
			const Uint32 num = queue.size();
			*p = (std::min(alpha/num,ff) << 24) | (std::min(red/num,ff) << 16) | (std::min(green/num,ff) << 8) | std::min(blue/num,ff);

			// Unload earlier pixels that are now too far away
			if(y >= depth) {
				{
					const auto &front = queue.front();
					alpha -= (front >> 24)&0xFF;
					red -= (front >> 16)&0xFF;
					green -= (front >> 8)&0xFF;
					blue -= front&0xFF;
				}
				assert(!queue.empty());
				queue.pop_front();
			}

			// Add new pixels
			if(y + depth+1 < res->h) {
				Uint32* q = p + (depth+1)*res->w;
				alpha += ((*q) >> 24)&0xFF;
				red += ((*q) >> 16)&0xFF;
				green += ((*q) >> 8)&0xFF;
				blue += (*q)&0xFF;
				assert(!queue.full());
				queue.push_back(*q);
			}
		}
		assert(static_cast<int>(queue.size()) == std::min(depth, res->h));
		queue.clear();
	}

	return res;
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
	SDL_Rect dst_rect { 0, 0, r.w, r.h };

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
		, const color_t color)
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
		const Uint16 red   = ratio * color.r;
		const Uint16 green = ratio * color.g;
		const Uint16 blue  = ratio * color.b;
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

	return nsurf;
}

/* Simplified RotSprite algorithm.
 * http://en.wikipedia.org/wiki/Image_scaling#RotSprite
 * Lifted from: http://github.com/salmonmoose/SpriteRotator
 * 1) Zoom the source image by a certain factor.
 * 2) Scan the zoomed source image at every step=offset and put it in the result. */
surface rotate_any_surface(const surface& surf, float angle, int zoom, int offset)
{
	int src_w, src_h, dst_w, dst_h;
	float min_x, min_y, sine, cosine;
	{
		float max_x, max_y;
		// convert angle to radiant (angle * 2 * PI) / 360
		const float radians = angle * boost::math::constants::pi<float>() / 180;
		cosine = cos(radians);
		sine   = sin(radians);
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
		const surface src = scale_surface(surf, src_w, src_h);
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

	return dst;
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
surface rotate_180_surface(const surface &surf)
{
	if ( surf == nullptr )
		return nullptr;

	// Work with a "neutral" surface.
	surface nsurf(make_neutral_surface(surf));

	if ( nsurf == nullptr ) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
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

	return nsurf;
}


// Rotates a surface 90 degrees, either clockwise or counter-clockwise.
surface rotate_90_surface(const surface &surf, bool clockwise)
{
	if ( surf == nullptr )
		return nullptr;

	// Work with "neutral" surfaces.
	surface dst(create_neutral_surface(surf->h, surf->w)); // Flipped dimensions.
	surface src(make_neutral_surface(surf));

	if ( src == nullptr  ||  dst == nullptr ) {
		std::cerr << "could not make neutral surface...\n";
		return nullptr;
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

	return dst;
}


surface flip_surface(const surface &surf)
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
		Uint32* const pixels = lock.pixels();

		for(int y = 0; y != nsurf->h; ++y) {
			for(int x = 0; x != nsurf->w/2; ++x) {
				const int index1 = y*nsurf->w + x;
				const int index2 = (y+1)*nsurf->w - x - 1;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return nsurf;
}

surface flop_surface(const surface &surf)
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
		Uint32* const pixels = lock.pixels();

		for(int x = 0; x != nsurf->w; ++x) {
			for(int y = 0; y != nsurf->h/2; ++y) {
				const int index1 = y*nsurf->w + x;
				const int index2 = (nsurf->h-y-1)*surf->w + x;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
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

	surface s = SDL_CreateRGBSurface(0, width, height, surf->format->BitsPerPixel,
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
	SDL_Rect dst_rect {0, 0, dst->w, dst->h};
	if(dstrect) {
		dst_rect.x = dstrect->x;
		dst_rect.w -= dstrect->x;

		dst_rect.y = dstrect->y;
		dst_rect.h -= dstrect->y;

	}

	SDL_Rect src_rect {0, 0, src->w, src->h};
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

surface get_surface_portion(const surface &src, SDL_Rect &area)
{
	if (src == nullptr) {
		return nullptr;
	}

	// Check if there is something in the portion
	if(area.x >= src->w || area.y >= src->h || area.x + area.w < 0 || area.y + area.h < 0) {
		return nullptr;
	}

	if(area.x + area.w > src->w) {
		area.w = src->w - area.x;
	}
	if(area.y + area.h > src->h) {
		area.h = src->h - area.y;
	}

	// use same format as the source (almost always the screen)
	surface dst = create_compatible_surface(src, area.w, area.h);

	if(dst == nullptr) {
		std::cerr << "Could not create a new surface in get_surface_portion()\n";
		return nullptr;
	}

	sdl_copy_portion(src, &area, dst, nullptr);

	return dst;
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
	SDL_Rect res {0,0,0,0};
	surface nsurf(make_neutral_surface(surf));
	if(nsurf == nullptr) {
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

void draw_centered_on_background(surface surf, const SDL_Rect& rect, const color_t& color, surface target)
{
	clip_rect_setter clip_setter(target, &rect);

	Uint32 col = SDL_MapRGBA(target->format, color.r, color.g, color.b, color.a);
	//TODO: only draw background outside the image
	SDL_Rect r = rect;
	sdl::fill_surface_rect(target, &r, col);

	if (surf != nullptr) {
		r.x = rect.x + (rect.w-surf->w)/2;
		r.y = rect.y + (rect.h-surf->h)/2;
		sdl_blit(surf, nullptr, target, &r);
	}
}

SDL_Color color_t::to_sdl() const {
	return {r, g, b, a};
}

color_t::color_t(const SDL_Color& c)
	: r(c.r)
	, g(c.g)
	, b(c.b)
	, a(c.a)
{}
