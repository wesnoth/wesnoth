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

namespace {
	SDL_PixelFormat& get_neutral_pixel_format()
	{
		static bool first_time = true;
		static SDL_PixelFormat format;

		if(first_time) {
			first_time = false;
			scoped_sdl_surface surf(SDL_CreateRGBSurface(SDL_SWSURFACE,1,1,32,0xFF0000,0xFF00,0xFF,0xFF000000));
			format = *surf->format;
			format.palette = NULL;
		}

		return format;
	}

}

SDL_Surface* make_neutral_surface(SDL_Surface* surf)
{
	if(surf == NULL) {
		std::cerr << "null neutral surface...\n";
		return NULL;
	}

	SDL_Surface* const result = SDL_ConvertSurface(surf,&get_neutral_pixel_format(),SDL_SWSURFACE);
	invalidate_sdl_surface_cache(surf);
	if(result != NULL) {
		SDL_SetAlpha(result,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
	}

	return result;
}

int sdl_add_ref(SDL_Surface* surface)
{
	if(surface != NULL)
		return surface->refcount++;
	else
		return 0;
}

void draw_unit_ellipse(SDL_Surface* target, Uint16 colour, Uint8 alpha, const SDL_Rect& clip, int unitx, int unity, SDL_Surface* behind, bool image_reverse, ELLIPSE_HALF half)
{
	const int xloc = unitx + (behind->w*15)/100;
	const int yloc = unity + (behind->h*7)/10;
	const int width = (behind->w*70)/100;
	const int height = behind->h/6;

	const double centerx = xloc + double(width)*0.5;
	const double centery = yloc + double(height)*0.5;
	const double r = double(width)*0.5;

	const double yratio = double(height)/double(width);

	int last_y = 0;
	for(int xit = xloc; xit != xloc+width; ++xit) {
		//r^2 = x^2 + y^2
		//y^2 = r^2 - x^2
		const double x = double(xit) - centerx;
		const int y = int(sqrt(r*r - x*x)*yratio);

		const int direction = y > last_y ? 1 : -1;
		for(int i = last_y; i != y+direction; i += direction) {
			int yit = yloc+height/2-y;
			int xpos = xit - unitx;
			if(image_reverse)
				xpos = behind->w - xpos - 1;

			int ypos = yit - unity;
			if(half == ELLIPSE_TOP && xit >= clip.x && yit >= clip.y && xit < clip.x + clip.w && yit+1 < clip.y + clip.h &&
				xpos >= 0 && ypos >= 0 && xpos < behind->w && ypos+1 < behind->h) {
				SDL_Rect rect = {xit,yit,1,2};
				fill_rect_alpha(rect,colour,alpha,target);
			}

			yit = yloc+height/2+y;
			ypos = yit - unity;
			if(half == ELLIPSE_BOTTOM && xit >= clip.x && yit >= clip.y && xit < clip.x + clip.w && yit+1 < clip.y + clip.h &&
				xpos >= 0 && ypos >= 0 && xpos < behind->w && ypos+1 < behind->h) {
				SDL_Rect rect = {xit,yit,1,2};
				fill_rect_alpha(rect,colour,alpha,target);
			}
		}

		last_y = y;
	}
}

SDL_Surface* clone_surface(SDL_Surface* surface)
{
	if(surface == NULL)
		return NULL;

	SDL_Surface* const result = SDL_DisplayFormatAlpha(surface);
	invalidate_sdl_surface_cache(surface);
	if(result == surface) {
		std::cerr << "resulting surface is the same as the source!!!\n";
	}

	if(result != NULL) {
		SDL_SetAlpha(result,SDL_SRCALPHA|SDL_RLEACCEL,SDL_ALPHA_OPAQUE);
	}

	return result;
}

SDL_Surface* scale_surface(SDL_Surface* surface, int w, int h)
{
	if(surface == NULL)
		return NULL;

	if(w == surface->w && h == surface->h) {
		sdl_add_ref(surface);
		return surface;
	}

	scoped_sdl_surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
	scoped_sdl_surface src(make_neutral_surface(surface));

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const double xratio = static_cast<double>(surface->w)/
			              static_cast<double>(w);
	const double yratio = static_cast<double>(surface->h)/
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

	return clone_surface(dst);
}

SDL_Surface* scale_surface_blended(SDL_Surface* surface, int w, int h)
{
	if(surface == NULL)
		return NULL;

	if(w == surface->w && h == surface->h) {
		sdl_add_ref(surface);
		return surface;
	}

	scoped_sdl_surface dst(SDL_CreateRGBSurface(SDL_SWSURFACE,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
	scoped_sdl_surface src(make_neutral_surface(surface));

	if(src == NULL || dst == NULL) {
		std::cerr << "Could not create surface to scale onto\n";
		return NULL;
	}

	const double xratio = static_cast<double>(surface->w)/
			              static_cast<double>(w);
	const double yratio = static_cast<double>(surface->h)/
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

	return clone_surface(dst);
}

SDL_Surface* adjust_surface_colour(SDL_Surface* surface, int r, int g, int b)
{
	if(r == 0 && g == 0 && b == 0 || surface == NULL)
		return clone_surface(surface);

	scoped_sdl_surface surf(make_neutral_surface(surface));

	if(surf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return NULL;
	}

	{
		surface_lock lock(surf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + surf->w*surf->h;

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,surf->format,&red,&green,&blue,&alpha);

			red = maximum<int>(8,minimum<int>(255,int(red)+r));
			green = maximum<int>(0,minimum<int>(255,int(green)+g));
			blue  = maximum<int>(0,minimum<int>(255,int(blue)+b));

			*beg = SDL_MapRGBA(surf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return clone_surface(surf);
}

SDL_Surface* greyscale_image(SDL_Surface* surface)
{
	if(surface == NULL)
		return NULL;

	scoped_sdl_surface surf(make_neutral_surface(surface));
	if(surf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return NULL;
	}

	{
		surface_lock lock(surf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + surf->w*surf->h;

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,surf->format,&red,&green,&blue,&alpha);

			const Uint8 avg = (red+green+blue)/3;

			*beg = SDL_MapRGBA(surf->format,avg,avg,avg,alpha);

			++beg;
		}
	}

	return clone_surface(surf);
}

SDL_Surface* brighten_image(SDL_Surface* surface, double amount)
{
	if(surface == NULL) {
		return NULL;
	}

	scoped_sdl_surface surf(make_neutral_surface(surface));

	if(surf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(surf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + surf->w*surf->h;

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,surf->format,&red,&green,&blue,&alpha);

			red = Uint8(minimum<double>(maximum<double>(double(red) * amount,0.0),255.0));
			green = Uint8(minimum<double>(maximum<double>(double(green) * amount,0.0),255.0));
			blue = Uint8(minimum<double>(maximum<double>(double(blue) * amount,0.0),255.0));

			*beg = SDL_MapRGBA(surf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return clone_surface(surf);
}

SDL_Surface* adjust_surface_alpha(SDL_Surface* surface, double amount)
{
	if(surface == NULL) {
		return NULL;
	}

	scoped_sdl_surface surf(make_neutral_surface(surface));

	if(surf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(surf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + surf->w*surf->h;

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,surf->format,&red,&green,&blue,&alpha);

			alpha = Uint8(minimum<double>(maximum<double>(double(alpha) * amount,0.0),255.0));

			*beg = SDL_MapRGBA(surf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return clone_surface(surf);
}

SDL_Surface* blend_surface(SDL_Surface* surface, double amount, Uint32 colour)
{
	if(surface == NULL) {
		return NULL;
	}

	scoped_sdl_surface surf(make_neutral_surface(surface));

	if(surf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(surf);
		Uint32* beg = lock.pixels();
		Uint32* end = beg + surf->w*surf->h;

		Uint8 red2, green2, blue2, alpha2;
		SDL_GetRGBA(colour,surf->format,&red2,&green2,&blue2,&alpha2);

		red2 *= amount;
		green2 *= amount;
		blue2 *= amount;

		amount = 1.0 - amount;

		while(beg != end) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*beg,surf->format,&red,&green,&blue,&alpha);

			red = red*amount + red2;
			green = green*amount + green2;
			blue = blue*amount + blue2;

			*beg = SDL_MapRGBA(surf->format,red,green,blue,alpha);

			++beg;
		}
	}

	return clone_surface(surf);
}

SDL_Surface* flip_surface(SDL_Surface* surface)
{
	if(surface == NULL) {
		return NULL;
	}

	scoped_sdl_surface surf(make_neutral_surface(surface));

	if(surf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(surf);
		Uint32* const pixels = lock.pixels();

		for(size_t y = 0; y != surf->h; ++y) {
			for(size_t x = 0; x != surf->w/2; ++x) {
				const size_t index1 = y*surf->w + x;
				const size_t index2 = (y+1)*surf->w - x - 1;
				std::swap(pixels[index1],pixels[index2]);
			}
		}
	}

	return clone_surface(surf);
}

SDL_Surface* flop_surface(SDL_Surface* surface)
{
	if(surface == NULL) {
		return NULL;
	}

	scoped_sdl_surface surf(make_neutral_surface(surface));

	if(surf == NULL) {
		std::cerr << "could not make neutral surface...\n";
		return NULL;
	}

	{
		surface_lock lock(surf);
		Uint32* const pixels = lock.pixels();

		for(size_t x = 0; x != surf->w; ++x) {
			for(size_t y = 0; y != surf->h/2; ++y) {
				const size_t index1 = y*surf->w + x;
				const size_t index2 = (surf->h-y-1)*surf->w + x;
				std::swap(pixels[index1],pixels[index2]);
			}
		}		
	}

	return clone_surface(surf);
}

void fill_rect_alpha(SDL_Rect& rect, Uint32 colour, Uint8 alpha, SDL_Surface* target)
{
	if(alpha == SDL_ALPHA_OPAQUE) {
		SDL_FillRect(target,&rect,colour);
		return;
	} else if(alpha == SDL_ALPHA_TRANSPARENT) {
		return;
	}

	scoped_sdl_surface tmp(SDL_CreateRGBSurface(0,rect.w,rect.h,target->format->BitsPerPixel,
	                                            target->format->Rmask,target->format->Gmask,target->format->Bmask,0));
	if(tmp == NULL) {
		return;
	}

	SDL_Rect r = {0,0,rect.w,rect.h};
	SDL_FillRect(tmp,&r,colour);
	SDL_SetAlpha(tmp,SDL_SRCALPHA,alpha);
	SDL_BlitSurface(tmp,NULL,target,&rect);
}

SDL_Surface* get_surface_portion(SDL_Surface* src, SDL_Rect& area)
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

	const SDL_PixelFormat* const fmt = src->format;
	SDL_Surface* const dst = SDL_CreateRGBSurface(0,area.w,area.h,
	                                              fmt->BitsPerPixel,fmt->Rmask,
	                                              fmt->Gmask,fmt->Bmask,
	                                              fmt->Amask);
	if(dst == NULL) {
		std::cerr << "Could not create a new surface in get_surface_portion()\n";
		return NULL;
	}

	SDL_Rect dstarea = {0,0,0,0};

	sdl_safe_blit(src,&area,dst,&dstarea);

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

SDL_Rect get_non_transperant_portion(SDL_Surface* surface)
{
	SDL_Rect res = {0,0,0,0};
	const scoped_sdl_surface surf(make_neutral_surface(surface));
	if(surf == NULL) {
		std::cerr << "failed to make neutral surface\n";
		return res;
	}

	const not_alpha calc(*(surf->format));

	surface_lock lock(surf);
	const Uint32* const pixels = lock.pixels();

	size_t n;
	for(n = 0; n != surf->h; ++n) {
		const Uint32* const start_row = pixels + n*surf->w;
		const Uint32* const end_row = start_row + surf->w;

		if(std::find_if(start_row,end_row,calc) != end_row)
			break;
	}

	res.y = n;

	for(n = 0; n != surf->h-res.y; ++n) {
		const Uint32* const start_row = pixels + (surf->h-n-1)*surf->w;
		const Uint32* const end_row = start_row + surf->w;

		if(std::find_if(start_row,end_row,calc) != end_row)
			break;
	}

	//the height is the height of the surface, minus the distance from the top and the
	//distance from the bottom
	res.h = surf->h - res.y - n;

	for(n = 0; n != surf->w; ++n) {
		size_t y;
		for(y = 0; y != surf->h; ++y) {
			const Uint32 pixel = pixels[y*surf->w + n];
			if(calc(pixel))
				break;
		}

		if(y != surf->h)
			break;
	}

	res.x = n;

	for(n = 0; n != surf->w-res.x; ++n) {
		size_t y;
		for(y = 0; y != surf->h; ++y) {
			const Uint32 pixel = pixels[y*surf->w + surf->w - n - 1];
			if(calc(pixel))
				break;
		}

		if(y != surf->h)
			break;
	}

	res.w = surf->w - res.x - n;

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
{}

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
		sdl_safe_blit(surface_,NULL,target_->getSurface(),&rect_);
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

//dummy definition of this SDL-private data structure, so that we can clear
//the surface's cache
struct SDL_BlitMap {
	SDL_Surface* dst;
};

void invalidate_sdl_surface_cache(SDL_Surface* surf)
{
	if(surf->map->dst != SDL_GetVideoSurface()) {
		surf->map->dst = NULL;
	}
}

void sdl_safe_blit(SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	SDL_BlitSurface(src,srcrect,dst,dstrect);
	invalidate_sdl_surface_cache(src);
}
