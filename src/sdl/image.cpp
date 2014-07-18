
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

#include "sdl/image.hpp"

#include "SDL_image.h"
#include "sdl/exception.hpp"
#include "sdl/rect.hpp"
#include "sdl/utils.hpp"

#include <cassert>

#ifdef SDL_GPU
#include "rect.hpp"
namespace sdl
{
timage::timage(Uint16 w, Uint16 h)
	: image_(NULL)
	, rotation_(new float(0))
	, hscale_(new float(1))
	, vscale_(new float(1))
	, clip_(new GPU_Rect)
{
	*clip_ = create_gpu_rect(0, 0, w, h);
	image_ = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
	if (image_ == NULL) {
		//TODO: report errorr
	} else {
		image_->refcount = 1;
	}
}

timage::timage(const std::string &file)
	: image_(GPU_LoadImage(file.c_str()))
	, rotation_(new float(0))
	, hscale_(new float(1))
	, vscale_(new float(1))
	, clip_(new GPU_Rect)
{
	if (image_ == NULL) {
		//TODO: report error
	} else {
		*clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
	}
}

timage::timage(const surface &source)
	: image_(GPU_CopyImageFromSurface(source))
	, rotation_(new float(0))
	, hscale_(new float(1))
	, vscale_(new float(1))
	, clip_(new GPU_Rect)
{
	if (image_ == NULL) {
		//TODO: report error
	} else {
		*clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
	}
}

timage::timage(SDL_Surface *source)
	: image_(GPU_CopyImageFromSurface(source))
	, rotation_(new float(0))
	, hscale_(new float(1))
	, vscale_(new float(1))
	, clip_(new GPU_Rect)
{
	if (image_ == NULL) {
		//TODO: report error
	} else {
		*clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
	}
}

timage::timage()
	: image_(NULL)
	, rotation_(new float(0))
	, hscale_(new float(1))
	, vscale_(new float(1))
	, clip_(new GPU_Rect)
{
}

sdl::timage::~timage()
{
	if (image_ != NULL) {
		image_->refcount -= 1;
		if (image_->refcount == 0) {
			GPU_FreeImage(image_);
			delete clip_;
			delete hscale_;
			delete vscale_;
			delete rotation_;
		}
	}
}

timage::timage(const timage &texture)
	: image_(texture.image_)
	, rotation_(texture.rotation_)
	, hscale_(texture.hscale_)
	, vscale_(texture.vscale_)
	, clip_(texture.clip_)
{
	if (image_ != NULL) {
		image_->refcount += 1;
	}
}

timage &timage::operator =(const timage &texture)
{
	if (&texture != this) {
		this->~timage();
		new (this) timage(texture);
	}

	return *this;
}

void timage::draw(GPU_Target &target, const int x, const int y)
{
	GPU_BlitTransform(image_, clip_, &target, x + width()/2,
					  y + height()/2, *rotation_, *hscale_, *vscale_);
}

void timage::set_rotation(float rotation)
{
	*rotation_ = rotation;
}

float timage::rotation() const
{
	return *rotation_;
}

void timage::set_hscale(float factor)
{
	*hscale_ = factor;
}

void timage::set_vscale(float factor)
{
	*vscale_ = factor;
}

void timage::set_scale(float hfactor, float vfactor)
{
	*hscale_ = hfactor;
	*vscale_ = vfactor;
}

float timage::hscale() const
{
	return *hscale_;
}

float timage::vscale() const
{
	return *vscale_;
}

void timage::set_smooth_scaling(bool use_smooth)
{
	GPU_SetImageFilter(image_,
					   use_smooth ? GPU_FILTER_LINEAR : GPU_FILTER_NEAREST);
}

bool timage::smooth_scaling() const
{
	return image_->filter_mode == GPU_FILTER_LINEAR;
}

Uint16 timage::width() const
{
	return image_->w * *hscale_;
}

Uint16 timage::height() const
{
	return image_->h * *vscale_;
}

Uint16 timage::base_width() const
{
	return image_->w;
}

Uint16 timage::base_height() const
{
	return image_->h;
}

void timage::set_clip(const SDL_Rect &rect)
{
	clip_->x = rect.x;
	clip_->y = rect.y;
	clip_->w = rect.w;
	clip_->h = rect.h;
}

SDL_Rect timage::clip() const
{
	SDL_Rect result;
	result.x = clip_->x;
	result.y = clip_->y;
	result.w = clip_->w;
	result.h = clip_->h;

	return result;
}

void timage::set_alpha(Uint8 alpha)
{
	const SDL_Color prev = image_->color;
	SDL_Color next = create_color(prev.r, prev.g, prev.b, alpha);
	GPU_SetColor(image_, &next);
}

Uint8 timage::alpha() const
{
	return image_->color.unused;
}

void timage::set_color_mod(Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Color c = create_color(r, g, b, image_->color.unused);
	GPU_SetColor(image_, &c);
}

Uint8 timage::red_mod() const
{
	return image_->color.r;
}

Uint8 timage::green_mod() const
{
	return image_->color.g;
}

Uint8 timage::blue_mod() const
{
	return image_->color.b;
}

void timage::set_hwrap(GPU_WrapEnum mode)
{
	GPU_SetWrapMode(image_, mode, image_->wrap_mode_y);
}

void timage::set_vwrap(GPU_WrapEnum mode)
{
	GPU_SetWrapMode(image_, image_->wrap_mode_x, mode);
}

void timage::set_wrap(GPU_WrapEnum hmode, GPU_WrapEnum vmode)
{
	GPU_SetWrapMode(image_, hmode, vmode);
}

GPU_WrapEnum timage::hwrap() const
{
	return image_->wrap_mode_x;
}

GPU_WrapEnum timage::vwrap() const
{
	return image_->wrap_mode_y;
}

bool timage::null() const
{
	return image_ == NULL;
}

timage timage::clone() const
{
	timage res;
	res.image_ = GPU_CopyImage(image_);
	res.set_alpha(alpha());
	res.set_clip(clip());
	res.set_color_mod(red_mod(), green_mod(), blue_mod());
	res.set_wrap(hwrap(), vwrap());
	res.set_rotation(rotation());
	res.set_scale(hscale(), vscale());
	res.set_smooth_scaling(smooth_scaling());

	return res;
}

}

#endif
