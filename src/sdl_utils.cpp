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

#include <algorithm>
#include <cmath>
#include <iostream>

#include "game.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "show_dialog.hpp"
#include "util.hpp"
#include "video.hpp"

bool point_in_rect(int x, int y, const SDL_Rect& rect)
{
	return x >= rect.x && y >= rect.y && x < rect.x + rect.w && y < rect.y + rect.h;
}

bool rects_overlap(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	return point_in_rect(rect1.x,rect1.y,rect2) || point_in_rect(rect1.x+rect1.w,rect1.y,rect2) ||
	       point_in_rect(rect1.x,rect1.y+rect1.h,rect2) || point_in_rect(rect1.x+rect1.w,rect1.y+rect1.h,rect2);
}

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}

namespace {
	SDL_PixelFormat& get_neutral_pixel_format()
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

}

void free_sdl_surface::operator()(SDL_Surface* surf) const
{
	if(surf != NULL)
		 SDL_FreeSurface(surf); 
}

/*explicit*/ surface::surface(SDL_Surface* surf) : surface_(surf)
{
}


surface make_neutral_surface(surface surf)
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


surface create_optimized_surface(surface surf)
{
	if(surf == NULL)
		return NULL;

	surface const result = display_format_alpha(surf);
	if(result == surf) {
		std::cerr << "resulting surface is the same as the source!!!\n";
	}

	if(result != NULL) {
		SDL_SetAlpha(result,SDL_SRCALPHA|SDL_RLEACCEL,SDL_ALPHA_OPAQUE);
	}

	return result;
}

surface scale_surface(surface surf, int w, int h)
{
	if(surf == NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}

	surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
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
				const int xsrcint = static_cast<int>(xsrc);
				const int ysrcint = static_cast<int>(ysrc);

				dst_pixels[ydst*dst->w + xdst] = src_pixels[ysrcint*src->w + xsrcint];
			}
		}
	}

	return create_optimized_surface(dst);
}

surface scale_surface_blended(surface surf, int w, int h)
{
	if(surf== NULL)
		return NULL;

	if(w == surf->w && h == surf->h) {
		return surf;
	}

	surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
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

				//we now have a rectangle, (xsrc,ysrc,xratio,yratio) which we
				//want to derive the pixel from
				for(double xloc = xsrc; xloc < xsrc+xratio; xloc += 1.0) {
					const double xsize = minimum<double>(floor(xloc+1.0)-xloc,xsrc+xratio-xloc);
					for(double yloc = ysrc; yloc < ysrc+yratio; yloc += 1.0) {
						const int xsrcint = maximum<int>(0,minimum<int>(src->w-1,static_cast<int>(xsrc)));
						const int ysrcint = maximum<int>(0,minimum<int>(src->h-1,static_cast<int>(ysrc)));

						const double ysize = minimum<double>(floor(yloc+1.0)-yloc,ysrc+yratio-yloc);		

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

surface adjust_surface_colour(surface surf, int r, int g, int b)
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
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			red = maximum<int>(8,minimum<int>(255,int(red)+r));
			green = maximum<int>(0,minimum<int>(255,int(green)+g));
			blue  = maximum<int>(0,minimum<int>(255,int(blue)+b));

			*beg = SDL_MapRGBA(nsurf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface greyscale_image(surface surf)
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

			//use the correct formula for RGB to grayscale
			//conversion. ok, this is no big deal :)
			//the correct formula being:
			//gray=0.299red+0.587green+0.114blue
			const Uint8 avg = (Uint8)((77*(Uint16)red + 
						   150*(Uint16)green +
						   29*(Uint16)blue) / 256);


			*beg = SDL_MapRGBA(nsurf->format,avg,avg,avg,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface brighten_image(surface surf, double amount)
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

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			red = Uint8(minimum<double>(maximum<double>(double(red) * amount,0.0),255.0));
			green = Uint8(minimum<double>(maximum<double>(double(green) * amount,0.0),255.0));
			blue = Uint8(minimum<double>(maximum<double>(double(blue) * amount,0.0),255.0));

			*beg = SDL_MapRGBA(nsurf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface adjust_surface_alpha(surface surf, double amount)
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
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			alpha = Uint8(minimum<double>(maximum<double>(double(alpha) * amount,0.0),255.0));

			*beg = SDL_MapRGBA(nsurf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

surface adjust_surface_alpha_add(surface surf, int amount)
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
			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);

			alpha = Uint8(maximum<int>(0,minimum<int>(255,int(alpha) + amount)));

			*beg = SDL_MapRGBA(nsurf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return create_optimized_surface(nsurf);
}

// Applies a mask on a surface
surface mask_surface(surface surf, surface mask)
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

	{
		surface_lock lock(nsurf);
		surface_lock mlock(nmask);
		
		Uint32* beg = lock.pixels();
		Uint32* end = beg + nsurf->w*surf->h;
		Uint32* mbeg = mlock.pixels();
		Uint32* mend = mbeg + nmask->w*nmask->h;

		while(beg != end && mbeg != mend) {
			Uint8 red, green, blue, alpha;
			Uint8 mred, mgreen, mblue, malpha;

			SDL_GetRGBA(*beg,nsurf->format,&red,&green,&blue,&alpha);
			SDL_GetRGBA(*mbeg,nmask->format,&mred,&mgreen,&mblue,&malpha);

			alpha = Uint8(minimum<int>(malpha, alpha));

			*beg = SDL_MapRGBA(nsurf->format,red,green,blue,alpha);

			++beg;
			++mbeg;
		}
	}

	return nsurf;
	//return create_optimized_surface(nsurf);
}

// Cuts a rectangle from a surface.
surface cut_surface(surface surf, const SDL_Rect& r)
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
		
		assert(rpitch >= r.w * rbpp);
		memcpy(line_dest, line_src, size * rbpp);
	}

	return res;
}

surface blend_surface(surface surf, double amount, Uint32 colour)
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

surface flip_surface(surface surf)
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

		for(size_t y = 0; y != nsurf->h; ++y) {
			for(size_t x = 0; x != nsurf->w/2; ++x) {
				const size_t index1 = y*nsurf->w + x;
				const size_t index2 = (y+1)*nsurf->w - x - 1;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return create_optimized_surface(nsurf);
}

surface flop_surface(surface surf)
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

		for(size_t x = 0; x != nsurf->w; ++x) {
			for(size_t y = 0; y != nsurf->h/2; ++y) {
				const size_t index1 = y*nsurf->w + x;
				const size_t index2 = (nsurf->h-y-1)*surf->w + x;
				std::swap(pixels[index1],pixels[index2]);
			}
		}		
	}

	return create_optimized_surface(nsurf);
}

surface create_compatible_surface(surface surf, int width, int height)
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

void fill_rect_alpha(SDL_Rect& rect, Uint32 colour, Uint8 alpha, surface target)
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

surface get_surface_portion(surface src, SDL_Rect& area)
{
	if(area.x >= src->w || area.y >= src->h) {
		std::cerr << "illegal surface portion...\n";
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

	SDL_Rect dstarea = {0,0,0,0};

	SDL_BlitSurface(src,&area,dst,&dstarea);

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

SDL_Rect get_non_transperant_portion(surface surf)
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

	size_t n;
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

	//the height is the height of the surface, minus the distance from the top and the
	//distance from the bottom
	res.h = nsurf->h - res.y - n;

	for(n = 0; n != nsurf->w; ++n) {
		size_t y;
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
		size_t y;
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

namespace {
	const SDL_Rect empty_rect = {0,0,0,0};
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

void surface_restorer::restore()
{
	if(surface_ != NULL) {
		SDL_BlitSurface(surface_,NULL,target_->getSurface(),&rect_);
		update_rect(rect_);
	}
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
