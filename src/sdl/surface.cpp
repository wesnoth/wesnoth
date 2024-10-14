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

const SDL_PixelFormat neutral_format = SDL_PIXELFORMAT_ARGB8888;

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

	surface_ = SDL_CreateSurface(w, h, neutral_format);
}

bool surface::is_neutral() const
{
	if(surface_) {
		const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface_->format);

		return SDL_ISPIXELFORMAT_INDEXED(surface_->format) == false
			&&  details->bytes_per_pixel == 4
			&&  details->Rmask == SDL_RED_MASK
			&& (details->Amask | SDL_ALPHA_MASK) == SDL_ALPHA_MASK;
	}
	return false;
}

surface& surface::make_neutral()
{
	if(surface_ && !is_neutral()) {
		SDL_Surface* res = SDL_ConvertSurface(surface_, neutral_format);

		// Ensure we don't leak memory with the old surface.
		free_surface();

		surface_ = res;
	}

	return *this;
}

surface surface::clone() const
{
	// Use SDL_ConvertSurface to make a copy
	return surface(SDL_ConvertSurface(surface_, neutral_format));
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
		SDL_DestroySurface(surface_);
	}
}

std::ostream& operator<<(std::ostream& stream, const surface& surf)
{
	if(!surf.get()) {
		stream << "<null surface>";
	} else if(!surf->format) {
		stream << "<invalid surface>";
	} else {
		const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surf->format);

		stream << "{ " << surf->w << 'x' << surf->h << '@'
			   << unsigned(details->bits_per_pixel) << "bpp"
			   << " refcount=" << surf->refcount
			   << " }";
	}

	return stream;
}
