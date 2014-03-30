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
{
	if(!texture_) {
		throw texception("Failed to create a SDL_Texture object.", true);
	}
}

ttexture::ttexture(SDL_Renderer& renderer,
				   const std::string& file)
	: reference_count_(new unsigned(1))
	, texture_(NULL)
{
	SDL_Surface* img;
	img = IMG_Load(file.c_str());

	if (img == NULL) {
		throw texception("Failed to create SDL_Texture object.", true);
	} else {
		texture_ = SDL_CreateTextureFromSurface(&renderer, img);

		if (texture_ == NULL) {
			SDL_FreeSurface(img);
			throw texception("Failed to create SDL_Texture object.", true);
		}

		SDL_FreeSurface(img);
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

} // namespace sdl

#endif
