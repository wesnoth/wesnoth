/*
   Copyright (C) 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sdl/texture.hpp"

#if SDL_VERSION_ATLEAST(2, 0, 0)

#include "SDL_image.h"
#include "sdl/exception.hpp"

#include <cassert>

namespace sdl
{

ttexture::ttexture(SDL_Renderer& renderer,
				   const Uint32 format,
				   const int access,
				   const int w,
				   const int h)
	: reference_count_(new unsigned(1))
	, texture_(SDL_CreateTexture(&renderer, format, access, w, h))
	, source_surface_(NULL)
{
	if(!texture_) {
		throw texception("Failed to create a SDL_Texture object.", true);
	}
}

ttexture::ttexture(SDL_Renderer& renderer,
				   const std::string& file,
				   int access,
				   bool keep_surface)
	: reference_count_(new unsigned(1))
	, texture_(NULL)
	, source_surface_(NULL)
{
	source_surface_ = IMG_Load(file.c_str());

	if (source_surface_ == NULL) {
		throw texception("Failed to create SDL_Texture object.", true);
	}

	if (access == SDL_TEXTUREACCESS_STATIC) {
		texture_ = SDL_CreateTextureFromSurface(&renderer, source_surface_);

		if (!keep_surface) {
			SDL_FreeSurface(source_surface_);
			source_surface_ = NULL;
		}

		if (texture_ == NULL) {
			throw texception("Failed to create SDL_Texture object.", true);
		}
	} else if (access == SDL_TEXTUREACCESS_STREAMING) {
		texture_ = SDL_CreateTexture(&renderer,
									 source_surface_->format->format,
									 SDL_TEXTUREACCESS_STREAMING,
									 source_surface_->w, source_surface_->h);

		if (texture_ == NULL) {
			SDL_FreeSurface(source_surface_);
			source_surface_ = NULL;
			throw texception("Failed to create SDL_Texture object.", true);
		}

		SDL_UpdateTexture(texture_, NULL, source_surface_->pixels, source_surface_->pitch);

		if (!keep_surface) {
			SDL_FreeSurface(source_surface_);
			source_surface_ = NULL;
		}
	} else {
		throw texception("Unknown texture access mode.", false);
	}
}

ttexture::~ttexture()
{
	assert(reference_count_);

	--(*reference_count_);
	if(*reference_count_ == 0) {
		if(texture_) {
			SDL_DestroyTexture(texture_);
		}
		if (source_surface_) {
			SDL_FreeSurface(source_surface_);
		}
		delete reference_count_;
	}
}

ttexture::ttexture(const ttexture& texture)
	: reference_count_(texture.reference_count_), texture_(texture.texture_)
{
	assert(reference_count_);
	++(*reference_count_);

	/* In the unlikely case the reference count wraps, we die. */
	assert(*reference_count_ != 0);
}

ttexture& ttexture::operator=(const ttexture& texture)
{
	if(&texture != this) {
		this->~ttexture();
		new (this) ttexture(texture);
	}

	return *this;
}

surface ttexture::source_surface() const
{
	return source_surface_;
}

} // namespace sdl

#endif
