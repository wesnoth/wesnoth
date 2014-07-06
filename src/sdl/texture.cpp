
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

#include "SDL_image.h"
#include "sdl/exception.hpp"
#include "sdl/rect.hpp"
#include "sdl/utils.hpp"

#include <cassert>

#ifdef SDL_GPU
#include "rect.hpp"
namespace sdl
{
ttexture::ttexture(Uint16 w, Uint16 h)
	: image_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, clip_(create_gpu_rect(0, 0, w, h))
	, color_mod_(create_color(0, 0, 0, 255))
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
{
	image_ = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
	if (image_ == NULL) {
		//TODO: report errorr
	} else {
		image_->refcount = 1;
	}
}

ttexture::ttexture(const std::string &file)
	: image_(GPU_LoadImage(file.c_str()))
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, clip_()
	, color_mod_(create_color(0, 0, 0, 255))
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
{
	if (image_ == NULL) {
		//TODO: report error
	} else {
		clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
	}
}

ttexture::ttexture(const surface &source)
	: image_(GPU_CopyImageFromSurface(source))
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, clip_()
	, color_mod_(create_color(0, 0, 0, 255))
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
{
	if (image_ == NULL) {
		//TODO: report error
	} else {
		clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
	}
}

ttexture::ttexture(SDL_Surface *source)
	: image_(GPU_CopyImageFromSurface(source))
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, clip_()
	, color_mod_(create_color(0, 0, 0, 255))
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
{
	if (image_ == NULL) {
		//TODO: report error
	} else {
		clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
	}
}

ttexture::ttexture()
	: image_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, smooth_scaling_(false)
	, clip_(create_gpu_rect(0, 0, 0, 0))
	, color_mod_(create_color(0, 0, 0, 255))
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
{
}

sdl::ttexture::~ttexture()
{
	if (image_ != NULL) {
		image_->refcount -= 1;
		if (image_->refcount == 0) {
			GPU_FreeImage(image_);
		}
	}
}

ttexture::ttexture(const ttexture &texture)
	: image_(texture.image_)
	, rotation_(texture.rotation_)
	, hscale_(texture.hscale_)
	, vscale_(texture.vscale_)
	, smooth_scaling_(texture.smooth_scaling_)
	, clip_(texture.clip_)
	, color_mod_(texture.color_mod_)
	, hwrap_(texture.hwrap_)
	, vwrap_(texture.vwrap_)
{
	if (image_ != NULL) {
		image_->refcount += 1;
	}
}

ttexture &ttexture::operator =(const ttexture &texture)
{
	if (&texture != this) {
		this->~ttexture();
		new (this) ttexture(texture);
	}

	return *this;
}

void ttexture::draw(GPU_Target &target, const int x, const int y)
{
	GPU_SetImageFilter(image_,
					   smooth_scaling_
					   ? GPU_FILTER_LINEAR
					   : GPU_FILTER_NEAREST);
	//GPU_SetColor(image_, &color_mod_);
	GPU_SetWrapMode(image_, hwrap_, vwrap_);
	GPU_BlitTransform(image_, &clip_, &target, x + width()/2,
					  y + height()/2, rotation_, hscale_, vscale_);
}

void ttexture::set_rotation(float rotation)
{
	rotation_ = rotation;
}

float ttexture::rotation() const
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

Uint16 ttexture::width() const
{
	return image_->w * hscale_;
}

Uint16 ttexture::height() const
{
	return image_->h * vscale_;
}

void ttexture::set_clip(const SDL_Rect &rect)
{
	clip_.x = rect.x;
	clip_.y = rect.y;
	clip_.w = rect.w;
	clip_.h = rect.h;
}

SDL_Rect ttexture::clip() const
{
	SDL_Rect result;
	result.x = clip_.x;
	result.y = clip_.y;
	result.w = clip_.w;
	result.h = clip_.h;

	return result;
}

void ttexture::set_alpha(Uint8 alpha)
{
	color_mod_.unused = alpha;
}

Uint8 ttexture::alpha() const
{
	return color_mod_.unused;
}

void ttexture::set_color_mod(Uint8 r, Uint8 g, Uint8 b)
{
	color_mod_.r = r;
	color_mod_.g = g;
	color_mod_.b = b;
}

Uint8 ttexture::red_mod() const
{
	return color_mod_.r;
}

Uint8 ttexture::green_mod() const
{
	return color_mod_.g;
}

Uint8 ttexture::blue_mod() const
{
	return color_mod_.b;
}

void ttexture::set_hwrap(GPU_WrapEnum mode)
{
	hwrap_ = mode;
}

void ttexture::set_vwrap(GPU_WrapEnum mode)
{
	vwrap_ = mode;
}

void ttexture::set_wrap(GPU_WrapEnum hmode, GPU_WrapEnum vmode)
{
	hwrap_ = hmode;
	vwrap_ = vmode;
}

GPU_WrapEnum ttexture::hwrap() const
{
	return hwrap_;
}

GPU_WrapEnum ttexture::vwrap() const
{
	return vwrap_;
}

bool ttexture::null() const
{
	return image_ == NULL;
}

}

#endif
