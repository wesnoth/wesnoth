/*
	Copyright (C) 2003 - 2025
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

#include <utility>

namespace
{
void add_refcount(surface& surf)
{
	if(surf) {
		++surf->refcount;
	}
}

void free_surface(surface& surf)
{
	if(surf) {
		SDL_FreeSurface(surf);
	}
}

void make_neutral(surface& surf)
{
	if(surf && surf->format->format != SDL_PIXELFORMAT_ARGB8888) {
		surf = surf.clone();
	}
}

} // namespace

surface::surface(SDL_Surface* surf)
	: surface_(surf)
{
	make_neutral(*this); // EXTREMELY IMPORTANT!
}

surface::surface(int w, int h)
	: surface_(nullptr)
{
	if (w < 0 || h < 0) {
		throw std::invalid_argument("Creating surface with negative dimensions");
	}

	surface_ = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
}

surface::surface(const surface& s)
	: surface_(s)
{
	add_refcount(*this);
}

surface::surface(surface&& s) noexcept
	: surface_(std::exchange(s.surface_, nullptr))
{
}

surface::~surface()
{
	free_surface(*this);
}

surface& surface::operator=(const surface& s)
{
	if(this != &s) {
		free_surface(*this);
		surface_ = s;
		add_refcount(*this);
	}

	return *this;
}

surface& surface::operator=(surface&& s) noexcept
{
	free_surface(*this);
	surface_ = std::exchange(s.surface_, nullptr);
	return *this;
}

surface surface::clone() const
{
	// Use SDL_ConvertSurfaceFormat to make a copy
	return surface(SDL_ConvertSurfaceFormat(surface_, SDL_PIXELFORMAT_ARGB8888, 0));
}

std::size_t surface::area() const
{
	return surface_ ? surface_->w * surface_->h : 0;
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
