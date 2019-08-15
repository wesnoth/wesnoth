/*
   Copyright (C) 2003 - 2018 the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sdl/surface.hpp"

#include "sdl/rect.hpp"
#include "video.hpp"

const SDL_PixelFormat surface::neutral_pixel_format = []() {
#if SDL_VERSION_ATLEAST(2, 0, 6)
	return *SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_ARGB8888)->format;
#else
	return *SDL_CreateRGBSurface(0, 1, 1, 32, SDL_RED_MASK, SDL_GREEN_MASK, SDL_BLUE_MASK, SDL_ALPHA_MASK)->format;
#endif
}();

surface::surface(SDL_Surface* surf)
	: surface_(surf)
{
	make_neutral(); // EXTREMELY IMPORTANT!
}

surface::surface(int w, int h)
	: surface_(nullptr)
{
	if (w < 0 || h < 0) {
		throw std::invalid_argument("Creating surface with negative dimensions");
	}

#if SDL_VERSION_ATLEAST(2, 0, 6)
	surface_ = SDL_CreateRGBSurfaceWithFormat(0, w, h, neutral_pixel_format.BitsPerPixel, neutral_pixel_format.format);
#else
	surface_ = SDL_CreateRGBSurface(0, w, h,
		neutral_pixel_format.BitsPerPixel,
		neutral_pixel_format.Rmask,
		neutral_pixel_format.Gmask,
		neutral_pixel_format.Bmask,
		neutral_pixel_format.Amask);
#endif
}

bool surface::is_neutral() const
{
	return surface_
		&& SDL_ISPIXELFORMAT_INDEXED(surface_->format->format) == SDL_FALSE
		&&  surface_->format->BytesPerPixel == 4
		&&  surface_->format->Rmask == SDL_RED_MASK
		&& (surface_->format->Amask | SDL_ALPHA_MASK) == SDL_ALPHA_MASK;
}

surface& surface::make_neutral()
{
	if(surface_ && !is_neutral()) {
		SDL_Surface* res = SDL_ConvertSurface(surface_, &neutral_pixel_format, 0);

		// Ensure we don't leak memory with the old surface.
		free_surface();

		surface_ = res;
	}

	return *this;
}

surface surface::clone() const
{
	// Use SDL_ConvertSurface to make a copy
	return surface(SDL_ConvertSurface(surface_, &neutral_pixel_format, 0));
}

void surface::assign_surface_internal(SDL_Surface* surf)
{
	add_surface_ref(surf); // Needs to be done before assignment to avoid corruption on "a = a;"
	free_surface();
	surface_ = surf;
	make_neutral(); // EXTREMELY IMPORTANT!
}

void surface::free_surface()
{
	if(surface_) {
		/* Workaround for an SDL bug.
		* SDL 2.0.6 frees the blit map unconditionally in SDL_FreeSurface() without checking
		* if the reference count has fallen to zero. However, many SDL functions such as
		* SDL_ConvertSurface() assume that the blit map is present.
		* Thus, we only call SDL_FreeSurface() if this is the last reference to the surface.
		* Otherwise we just decrement the reference count ourselves.
		*
		* - Jyrki, 2017-09-23
		*/
		if(surface_->refcount > 1 && sdl_get_version() == version_info(2, 0, 6)) {
			--surface_->refcount;
		} else {
			SDL_FreeSurface(surface_);
		}
	}
}

surface_restorer::surface_restorer()
	: target_(nullptr)
	, rect_(sdl::empty_rect)
	, surface_(nullptr)
{
}

surface_restorer::surface_restorer(CVideo* target, const SDL_Rect& rect)
	: target_(target)
	, rect_(rect)
	, surface_(nullptr)
{
	update();
}

surface_restorer::~surface_restorer()
{
	restore();
}

void surface_restorer::restore(const SDL_Rect& dst) const
{
	if(!surface_) {
		return;
	}

	SDL_Rect dst2 = sdl::intersect_rects(dst, rect_);
	if(dst2.w == 0 || dst2.h == 0) {
		return;
	}

	SDL_Rect src = dst2;
	src.x -= rect_.x;
	src.y -= rect_.y;
	sdl_blit(surface_, &src, target_->getSurface(), &dst2);
}

void surface_restorer::restore() const
{
	if(!surface_) {
		return;
	}

	SDL_Rect dst = rect_;
	sdl_blit(surface_, nullptr, target_->getSurface(), &dst);
}

void surface_restorer::update()
{
	if(rect_.w <= 0 || rect_.h <= 0) {
		surface_ = nullptr;
	} else {
		surface_ = ::get_surface_portion(target_->getSurface(),rect_);
	}
}

void surface_restorer::cancel()
{
	surface_ = nullptr;
}

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}
