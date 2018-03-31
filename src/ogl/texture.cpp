/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "ogl/texture.hpp"

#include <SDL.h>

#include <cassert>
#include <stdexcept>

namespace gl
{

std::pair<int, int> texture::get_size() const
{
	std::pair<int, int> size;
	glBindTexture(GL_TEXTURE_2D, name_);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size.first);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &size.second);
	glBindTexture(GL_TEXTURE_2D, 0u);
	return size;
}

void texture::set_size(const std::pair<int, int>& size)
{
	if(size.first > MAX_DIMENSION || size.second > MAX_DIMENSION) {
		throw std::invalid_argument("Too large texture size");
	}

	glBindTexture(GL_TEXTURE_2D, name_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.first, size.second, 0,
		GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0u);
}

void texture::set_pixels(surface pixels)
{
	assert(check_format(*pixels->format));
	surface_locker<surface> lock(pixels);

	glBindTexture(GL_TEXTURE_2D, name_);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixels->w, pixels->h,
		GL_BGRA, GL_UNSIGNED_BYTE, lock.pixels());
	glBindTexture(GL_TEXTURE_2D, 0u);
}

bool texture::check_format(const SDL_PixelFormat& format)
{
	return format.format == SDL_PIXELFORMAT_ARGB8888;
}

}