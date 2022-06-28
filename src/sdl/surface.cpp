/*
	Copyright (C) 2003 - 2022
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "sdl/surface.hpp"

#include "draw.hpp" // for surface_restorer. Remove that then remove this.
#include "sdl/rect.hpp"
#include "video.hpp" // for surface_restorer. Remove that then remove this.

const SDL_PixelFormat surface::neutral_pixel_format = []() {
	return *SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_ARGB8888)->format;
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

	surface_ = SDL_CreateRGBSurfaceWithFormat(0, w, h, neutral_pixel_format.BitsPerPixel, neutral_pixel_format.format);
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
		SDL_FreeSurface(surface_);
	}
}

surface_restorer::surface_restorer()
	: target_(nullptr)
	, rect_(sdl::empty_rect)
	, surface_()
{
}

surface_restorer::surface_restorer(CVideo* target, const SDL_Rect& rect)
	: target_(target)
	, rect_(rect)
	, surface_()
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
	draw::blit(surface_, dst2, src);
	//target_->blit_surface(dst2.x, dst2.y, surface_, &src, nullptr);
}

void surface_restorer::restore() const
{
	if(!surface_) {
		return;
	}

	draw::blit(surface_, rect_);
}

void surface_restorer::update()
{
	if(rect_.w <= 0 || rect_.h <= 0) {
		surface_.reset();
	} else {
		surface_ = texture(target_->read_pixels_low_res(&rect_));
	}
}

void surface_restorer::cancel()
{
	surface_.reset();
}

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}

std::ostream& operator<<(std::ostream& stream, const surface& surf)
{
	if(!surf.get()) {
		stream << "<null surface>";
	} else if(!surf->format) {
		stream << "<invalid surface>";
	} else {
		stream << "{ " << surf->w << 'x' << surf->h << '@'
			   << unsigned(surf->format->BitsPerPixel) << "bpp"
			   << (surf->format->palette ? " indexed" : "")
			   << " clip_rect=[" << surf->clip_rect
			   << "] refcount=" << surf->refcount
			   << " }";
	}

	return stream;
}
