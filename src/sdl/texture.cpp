
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
#include "sdl/rect.hpp"
#include "sdl/utils.hpp"

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
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, flip_(SDL_FLIP_NONE)
	, clip_(create_rect(0, 0, w, h))
	, mod_r_(255)
	, mod_g_(255)
	, mod_b_(255)
	, alpha_(255)
	, source_surface_(NULL)
{
	SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
	if(!texture_) {
		throw texception("Failed to create a SDL_Texture object.", true);
	}
}

ttexture::ttexture(SDL_Renderer& renderer,
				   const int access,
				   const std::string& file)
	: reference_count_(new unsigned(1))
	, texture_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, flip_(SDL_FLIP_NONE)
	, clip_()
	, mod_r_(255)
	, mod_g_(255)
	, mod_b_(255)
	, alpha_(255)
	, source_surface_(IMG_Load(file.c_str()))
{
	if(source_surface_ == NULL) {
		throw texception("Failed to create SDL_Texture object.", true);
	}

	initialise_from_surface(renderer, access);
}


ttexture::ttexture()
	: reference_count_(new unsigned(1))
	, texture_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, flip_(SDL_FLIP_NONE)
	, clip_()
	, mod_r_(255)
	, mod_g_(255)
	, mod_b_(255)
	, alpha_(255)
	, source_surface_(NULL)
{}

ttexture::~ttexture()
{
	assert(reference_count_);

	--(*reference_count_);
	if(*reference_count_ == 0) {
		if(source_surface_) {
			SDL_FreeSurface(source_surface_);
		}
		if(texture_) {
			SDL_DestroyTexture(texture_);
		}
		delete reference_count_;
	}
}

ttexture::ttexture(const ttexture& texture)
	: reference_count_(texture.reference_count_)
	, texture_(texture.texture_)
	, rotation_(texture.rotation_)
	, hscale_(texture.hscale_)
	, vscale_(texture.vscale_)
	, smooth_scaling_(texture.smooth_scaling_)
	, flip_(texture.flip_)
	, clip_(texture.clip_)
	, mod_r_(texture.mod_r_)
	, mod_g_(texture.mod_g_)
	, mod_b_(texture.mod_b_)
	, alpha_(texture.alpha_)
	, source_surface_(texture.source_surface_)
{
	assert(reference_count_);
	++(*reference_count_);

	/* In the unlikely case the reference count wraps, we die. */
	assert(*reference_count_ != 0);
}

ttexture::ttexture(SDL_Renderer& renderer,
				   const int access,
				   SDL_Surface* source_surface__)
	: reference_count_(new unsigned(1))
	, texture_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, flip_(SDL_FLIP_NONE)
	, clip_()
	, mod_r_(255)
	, mod_g_(255)
	, mod_b_(255)
	, alpha_(255)
	, source_surface_(source_surface__)
{
	if(source_surface_ == NULL) {
		throw texception("Invalid source_surface__ argument passed, failed to "
						 "create SDL_Texture object.",
						 false);
	}

	initialise_from_surface(renderer, access);
}

ttexture::ttexture(SDL_Renderer& renderer,
				   const int access,
				   const surface& surface)
	: reference_count_(new unsigned(1))
	, texture_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, flip_(SDL_FLIP_NONE)
	, clip_()
	, mod_r_(255)
	, mod_g_(255)
	, mod_b_(255)
	, alpha_(255)
	, source_surface_(
			SDL_ConvertSurface(surface, surface->format, surface->flags))
{
	if(source_surface_ == NULL) {
		throw texception("Invalid source_surface__ argument passed, failed to "
						 "create SDL_Texture object.",
						 false);
	}

	initialise_from_surface(renderer, access);
}

ttexture& ttexture::operator=(const ttexture& texture)
{
	if(&texture != this) {
		this->~ttexture();
		new (this) ttexture(texture);
	}

	return *this;
}

void ttexture::draw(SDL_Renderer& renderer, const int x, const int y)
{
	SDL_Rect dstrect = create_rect(x, y, clip_.w * hscale_, clip_.h * vscale_);

	SDL_SetTextureAlphaMod(texture_, alpha_);
	SDL_SetTextureColorMod(texture_, mod_r_, mod_g_, mod_b_);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, smooth_scaling_ ? "linear" : "nearest");
	SDL_RenderCopyEx(&renderer, texture_, &clip_, &dstrect,
					 rotation_, NULL, flip_);
}

const SDL_Surface* ttexture::source_surface() const
{
	return source_surface_;
}

void ttexture::set_rotation(double rotation)
{
	rotation_ = rotation;
}

double ttexture::rotation() const
{
	return rotation_;
}

void ttexture::set_hscale(float factor)
{
	hscale_ = factor;
}

void ttexture::set_vscale(float factor)
{
	vscale_ = factor;
}

void ttexture::set_scale(float hfactor, float vfactor)
{
	hscale_ = hfactor;
	vscale_ = vfactor;
}

float ttexture::hscale() const
{
	return hscale_;
}

float ttexture::vscale() const
{
	return vscale_;
}

void ttexture::set_smooth_scaling(bool use_smooth)
{
	smooth_scaling_ = use_smooth;
}

bool ttexture::smooth_scaling() const
{
	return smooth_scaling_;
}

void ttexture::set_flip(bool flip)
{
	if (flip) {
		flip_ = SDL_RendererFlip(flip_ | SDL_FLIP_HORIZONTAL);
	} else {
		flip_ = SDL_RendererFlip(flip_ & ~SDL_FLIP_HORIZONTAL);
	}
}

void ttexture::set_flop(bool flop)
{
	if (flop) {
		flip_ = SDL_RendererFlip(flip_ | SDL_FLIP_VERTICAL);
	} else {
		flip_ = SDL_RendererFlip(flip_ & ~SDL_FLIP_VERTICAL);
	}
}

bool ttexture::flipped() const
{
	return flip_ & SDL_FLIP_HORIZONTAL;
}

bool ttexture::flopped() const
{
	return flip_ & SDL_FLIP_VERTICAL;
}

int ttexture::width() const
{
	int w;
	SDL_QueryTexture(texture_,NULL, NULL, &w, NULL);

	return w*hscale_;
}

int ttexture::height() const
{
	int h;
	SDL_QueryTexture(texture_,NULL, NULL, NULL, &h);

	return h*vscale_;
}

SDL_Rect ttexture::dimensions() const
{
	SDL_Rect dim;
	dim.x = 0;
	dim.y = 0;
	SDL_QueryTexture(texture_, NULL, NULL, &dim.w, &dim.h);

	dim.w *= hscale_;
	dim.h *= vscale_;
	return dim;
}

void ttexture::set_clip(const SDL_Rect &rect)
{
	clip_ = rect;
}

const SDL_Rect &ttexture::clip() const
{
	return clip_;
}

Uint32 ttexture::format() const
{
	Uint32 f;
	SDL_QueryTexture(texture_, &f, NULL, NULL, NULL);

	return f;
}

void ttexture::set_alpha(Uint8 alpha)
{
	alpha_ = alpha;
}

Uint8 ttexture::alpha() const
{
	return alpha_;
}

void ttexture::set_color_mod(Uint8 r, Uint8 g, Uint8 b)
{
	mod_r_ = r;
	mod_g_ = g;
	mod_b_ = b;
}

Uint8 ttexture::red_mod() const
{
	return mod_r_;
}

Uint8 ttexture::green_mod() const
{
	return mod_g_;
}

Uint8 ttexture::blue_mod() const
{
	return mod_b_;
}

void ttexture::update_pixels(SDL_Surface *surf)
{
	const int retcode = SDL_UpdateTexture(texture_,
										  NULL,
										  surf->pixels,
										  surf->pitch);

	if (retcode != 0) {
		throw texception("Failed to update SDL_Texture object.", true);
	}

	if (source_surface_ != NULL) {
		SDL_FreeSurface(source_surface_);
		source_surface_ = surf;

	}
}

bool ttexture::null() const
{
	return texture_ == NULL;
}

void ttexture::initialise_from_surface(SDL_Renderer& renderer, const int access)
{
	if(access == SDL_TEXTUREACCESS_STATIC) {
		texture_ = SDL_CreateTextureFromSurface(&renderer, source_surface_);

		if(texture_ == NULL) {
			throw texception("Failed to create SDL_Texture object.", true);
		}

		clip_ = create_rect(0, 0, source_surface_->w, source_surface_->h);
		SDL_FreeSurface(source_surface_);
		source_surface_ = NULL;
	} else if(access == SDL_TEXTUREACCESS_STREAMING) {
		texture_ = SDL_CreateTexture(&renderer,
									 source_surface_->format->format,
									 SDL_TEXTUREACCESS_STREAMING,
									 source_surface_->w,
									 source_surface_->h);

		if(texture_ == NULL) {
			throw texception("Failed to create SDL_Texture object.", true);
		}

		clip_ = create_rect(0, 0, source_surface_->w, source_surface_->h);
		const int update = SDL_UpdateTexture(texture_,
											 NULL,
											 source_surface_->pixels,
											 source_surface_->pitch);
		if(update != 0) {
			throw texception("Failed to update the SDL_Texture object during "
							 "its construction.",
							 true);
		}
	} else {
		throw texception("Unknown texture access mode.", false);
	}
	SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
}

} // namespace sdl

#endif
