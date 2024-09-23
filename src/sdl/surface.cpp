/*
	Copyright (C) 2003 - 2024
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

#include "color.hpp"
#include "sdl/rect.hpp"

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
