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
#include "sdl/render_utils.hpp"
#include "sdl/surface.hpp"
#include "video.hpp"

static lg::log_domain log_sdl("SDL");
#define ERR_SDL LOG_STREAM(err, log_sdl)

namespace
{
// The default pixel format to create textures with.
const int default_texture_format = SDL_PIXELFORMAT_ARGB8888;

void cleanup_texture(SDL_Texture* t)
{
	if(t != nullptr) {
		SDL_DestroyTexture(t);;
	}
}

} // end anon namespace

texture::texture()
	: texture_(nullptr)
{
}

texture::texture(SDL_Texture* txt)
	: texture_(txt, &cleanup_texture)
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

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surf), &cleanup_texture);
	if(!texture_) {
		ERR_SDL << "When creating texture from surface: " << SDL_GetError() << std::endl;
	}
}

texture::texture(int w, int h, SDL_TextureAccess access)
	: texture_(nullptr)
{
	reset(w, h, access);
}

void texture::finalize()
{
	set_texture_blend_mode(*this, SDL_BLENDMODE_BLEND);
}

void texture::reset()
{
	if(texture_) {
		texture_.reset();
	}
}

void texture::reset(int w, int h, SDL_TextureAccess access)
{
	// No-op if texture is null.
	reset();

	SDL_Renderer* renderer = CVideo::get_singleton().get_renderer();
	if(!renderer) {
		return;
	}

	texture_.reset(SDL_CreateTexture(renderer, default_texture_format, access, w, h), &cleanup_texture);
	if(!texture_) {
		ERR_SDL << "When creating texture: " << SDL_GetError() << std::endl;
	}

	finalize();
}

void texture::assign(SDL_Texture* t)
{
	texture_.reset(t, &cleanup_texture);
}

texture::info::info(SDL_Texture* t)
	: format(0)
	, access(0)
	, w(0)
	, h(0)
{
	SDL_QueryTexture(t, &format, &access, &w, &h);
}
