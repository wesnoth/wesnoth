/*
   Copyright (C) 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sdl/texture.hpp"

#include "log.hpp"
#include "sdl/surface.hpp"
#include "video.hpp"

static lg::log_domain log_sdl("SDL");
#define ERR_SDL LOG_STREAM(err, log_sdl)

// The default pixel format to create textures with.
static int default_texture_format = SDL_PIXELFORMAT_ARGB8888;

texture::texture()
	: texture_(nullptr)
{
}

texture::texture(SDL_Texture* txt)
	: texture_(txt)
{
	finalize();
}

texture::texture(const surface& surf)
	: texture_(nullptr)
{
	SDL_Renderer* renderer = CVideo::get_singleton().get_renderer();
	if(!renderer) {
		return;
	}

	texture_ = SDL_CreateTextureFromSurface(renderer, surf);
	if(!texture_) {
		ERR_SDL << "When creating texture from surface: " << SDL_GetError() << std::endl;
	}
}

texture::texture(int w, int h, SDL_TextureAccess access)
	: texture_(nullptr)
{
	reset(w, h, access);
}

texture::~texture()
{
	destroy_texture();
}

void texture::finalize()
{
	SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
}

void texture::reset(int w, int h, SDL_TextureAccess access)
{
	// No-op if texture is null.
	destroy_texture();

	SDL_Renderer* renderer = CVideo::get_singleton().get_renderer();
	if(!renderer) {
		return;
	}

	texture_ = SDL_CreateTexture(renderer, default_texture_format, access, w, h);
	if(!texture_) {
		ERR_SDL << "When creating texture: " << SDL_GetError() << std::endl;
	}

	finalize();
}

void texture::destroy_texture()
{
	if(texture_) {
		SDL_DestroyTexture(texture_);
	}
}

#if 0
texture& texture::operator=(texture&& t)
{
	destroy_texture();

	texture_ = t.texture_;
	t.texture_ = nullptr;

	return *this;
}
#endif

texture::info::info(const texture& t)
	: format(0)
	, access(0)
	, w(0)
	, h(0)
{
	SDL_QueryTexture(t, &format, &access, &w, &h);
}
