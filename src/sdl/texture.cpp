/*
	Copyright (C) 2017 - 2022
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "sdl/point.hpp"
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
		SDL_DestroyTexture(t);
		;
	}
}

} // namespace

texture::texture()
	: texture_(nullptr)
	, w_(0)
	, h_(0)
{
}

texture::texture(SDL_Texture* txt)
	: texture_(txt, &cleanup_texture)
{
	if (txt) {
		SDL_QueryTexture(txt, nullptr, nullptr, &w_, &h_);
		finalize();
	}
}

texture::texture(const surface& surf, bool linear_interpolation)
	: texture()
{
	if (!surf) {
		return;
	}

	if (surf->w == 0 && surf->h == 0) {
		return;
	}

	SDL_Renderer* renderer = CVideo::get_singleton().get_renderer();
	if(!renderer) {
		return;
	}

	// Filtering mode must be set before texture creation.
	const char* scale_quality = linear_interpolation ? "linear" : "nearest";
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scale_quality);

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surf), &cleanup_texture);
	if(!texture_) {
		ERR_SDL << "When creating texture from surface: " << SDL_GetError() << std::endl;
	}

	w_ = surf->w; h_ = surf->h;
}

texture::texture(int width, int height, SDL_TextureAccess access)
	: texture_()
{
	reset(width, height, access);
}

void texture::finalize()
{
	if (texture_) {
		set_texture_blend_mode(*this, SDL_BLENDMODE_BLEND);
	}
}

void texture::set_draw_size(const point& p)
{
	w_ = p.x;
	h_ = p.y;
}

void texture::set_alpha_mod(uint8_t alpha)
{
	if (texture_) {
		SDL_SetTextureAlphaMod(texture_.get(), alpha);
	}
}

void texture::reset()
{
	if(texture_) {
		texture_.reset();
	}
	w_ = 0; h_ = 0;
}

void texture::reset(int width, int height, SDL_TextureAccess access)
{
	// No-op if texture is null.
	reset();

	SDL_Renderer* renderer = CVideo::get_singleton().get_renderer();
	if(!renderer) {
		return;
	}

	texture_.reset(SDL_CreateTexture(renderer, default_texture_format, access, width, height), &cleanup_texture);
	if(!texture_) {
		ERR_SDL << "When creating texture: " << SDL_GetError() << std::endl;
		return;
	}

	w_ = width; h_ = height;

	finalize();
}

void texture::assign(SDL_Texture* t)
{
	texture_.reset(t, &cleanup_texture);
	if (t) {
		SDL_QueryTexture(t, nullptr, nullptr, &w_, &h_);
	} else {
		w_ = 0;
		h_ = 0;
	}
}

texture& texture::operator=(texture&& t)
{
	texture_ = std::move(t.texture_);
	w_ = t.w_;
	h_ = t.h_;
	return *this;
}

texture::info::info(SDL_Texture* t)
	: format(0)
	, access(0)
	, w(0)
	, h(0)
{
	if (t) {
		SDL_QueryTexture(t, &format, &access, &w, &h);
	}
}
