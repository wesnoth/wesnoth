/*
	Copyright (C) 2017 - 2024
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

#include "color.hpp"
#include "log.hpp"
#include "sdl/point.hpp"
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
	}
}

} // namespace

texture::texture(SDL_Texture* txt)
	: texture_(txt, &cleanup_texture)
{
	if (txt) {
		SDL_QueryTexture(txt, nullptr, nullptr, &size_.x, &size_.y);
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

	SDL_Renderer* renderer = video::get_renderer();
	if(!renderer) {
		return;
	}

	// Filtering mode must be set before texture creation.
	set_texture_scale_quality(linear_interpolation ? "linear" : "nearest");

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surf), &cleanup_texture);
	if(!texture_) {
		ERR_SDL << "When creating texture from surface: " << SDL_GetError();
	}

	size_ = {surf->w, surf->h};

	finalize();
}

texture::texture(int width, int height, SDL_TextureAccess access)
	: texture_()
{
	reset(width, height, access);
}

void texture::finalize()
{
	set_blend_mode(SDL_BLENDMODE_BLEND);
}

uint32_t texture::get_format() const
{
	return get_info().format;
}

int texture::get_access() const
{
	return get_info().access;
}

point texture::get_raw_size() const
{
	return get_info().size;
}

void texture::set_src(const rect& r)
{
	rect dsrc = r.intersect(rect{{0, 0}, size_});
	point rsize = get_raw_size();
	if (draw_size() == rsize) {
		src_ = dsrc;
	} else {
		src_ = rect {
			(dsrc.x * rsize.x) / size_.x,
			(dsrc.y * rsize.y) / size_.y,
			(dsrc.w * rsize.x) / size_.x,
			(dsrc.h * rsize.y) / size_.y
		};
	}
	has_src_ = true;
}

void texture::set_src_raw(const rect& r)
{
	rect max = {{0, 0}, get_raw_size()};
	src_ = r.intersect(max);
	has_src_ = true;
}

void texture::set_alpha_mod(uint8_t alpha)
{
	if (texture_) {
		SDL_SetTextureAlphaMod(texture_.get(), alpha);
	}
}

uint8_t texture::get_alpha_mod() const
{
	if (!texture_) {
		return 0;
	}
	uint8_t a;
	SDL_GetTextureAlphaMod(texture_.get(), &a);
	return a;
}

void texture::set_color_mod(const color_t& c)
{
	set_color_mod(c.r, c.g, c.b);
}

void texture::set_color_mod(uint8_t r, uint8_t g, uint8_t b)
{
	if (texture_) {
		SDL_SetTextureColorMod(texture_.get(), r, g, b);
	}
}

color_t texture::get_color_mod() const
{
	if (!texture_) {
		return {0,0,0};
	}
	color_t c;
	SDL_GetTextureColorMod(texture_.get(), &c.r, &c.g, &c.b);
	return c;
}

void texture::set_blend_mode(SDL_BlendMode b)
{
	if (texture_) {
		SDL_SetTextureBlendMode(texture_.get(), b);
	}
}

SDL_BlendMode texture::get_blend_mode() const
{
	if (!texture_) {
		return SDL_BLENDMODE_NONE;
	}
	SDL_BlendMode b;
	SDL_GetTextureBlendMode(texture_.get(), &b);
	return b;
}

void texture::reset()
{
	if(texture_) {
		texture_.reset();
	}
	size_ = {0, 0};
	has_src_ = false;
}

void texture::reset(int width, int height, SDL_TextureAccess access)
{
	// No-op if texture is null.
	reset();

	SDL_Renderer* renderer = video::get_renderer();
	if(!renderer) {
		return;
	}

	texture_.reset(SDL_CreateTexture(renderer, default_texture_format, access, width, height), &cleanup_texture);
	if(!texture_) {
		ERR_SDL << "When creating texture: " << SDL_GetError();
		return;
	}

	size_ = {width, height};
	has_src_ = false;

	finalize();
}

void texture::assign(SDL_Texture* t)
{
	texture_.reset(t, &cleanup_texture);
	size_ = get_raw_size();
	has_src_ = false;
}

texture::info::info(SDL_Texture* t)
	: format(SDL_PIXELFORMAT_UNKNOWN)
	, access(-1)
	, size(0, 0)
{
	if (t) {
		SDL_QueryTexture(t, &format, &access, &size.x, &size.y);
	}
}
