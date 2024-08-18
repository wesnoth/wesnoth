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

	SDL_Renderer* renderer = video::get_renderer();
	if(!renderer) {
		return;
	}

	// Filtering mode must be set before texture creation.
	const char* scale_quality = linear_interpolation ? "linear" : "nearest";
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scale_quality);

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surf), &cleanup_texture);
	if(!texture_) {
		ERR_SDL << "When creating texture from surface: " << SDL_GetError();
	}

	w_ = surf->w; h_ = surf->h;

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
	uint32_t ret;
	if(texture_) {
		SDL_QueryTexture(texture_.get(), &ret, nullptr, nullptr, nullptr);
	} else {
		ret = SDL_PIXELFORMAT_UNKNOWN;
	}
	return ret;
}

int texture::get_access() const
{
	int ret;
	if(texture_) {
		SDL_QueryTexture(texture_.get(), nullptr, &ret, nullptr, nullptr);
	} else {
		ret = -1;
	}
	return ret;
}

point texture::get_raw_size() const
{
	point ret;
	if(texture_) {
		SDL_QueryTexture(texture_.get(), nullptr, nullptr, &ret.x, &ret.y);
	} else {
		ret = {0, 0};
	}
	return ret;
}

void texture::set_draw_size(const point& p)
{
	w_ = p.x;
	h_ = p.y;
}

void texture::set_src(const rect& r)
{
	rect dsrc = r.intersect({0, 0, w_, h_});
	point rsize = get_raw_size();
	if (draw_size() == rsize) {
		src_ = dsrc;
	} else {
		src_.x = (dsrc.x * rsize.x) / w_;
		src_.y = (dsrc.y * rsize.y) / h_;
		src_.w = (dsrc.w * rsize.x) / w_;
		src_.h = (dsrc.h * rsize.y) / h_;
	}
	has_src_ = true;
}

void texture::set_src_raw(const rect& r)
{
	rect max = {{}, get_raw_size()};
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

void texture::set_color_mod(color_t c)
{
	if (texture_) {
		SDL_SetTextureColorMod(texture_.get(), c.r, c.g, c.b);
	}
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
	w_ = 0; h_ = 0;
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

	w_ = width; h_ = height;
	has_src_ = false;

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
	has_src_ = false;
}

texture::info::info(SDL_Texture* t)
	: format(SDL_PIXELFORMAT_UNKNOWN)
	, access(-1)
	, w(0)
	, h(0)
{
	if (t) {
		SDL_QueryTexture(t, &format, &access, &w, &h);
	}
}
