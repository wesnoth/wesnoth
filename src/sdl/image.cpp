
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
#include "video.hpp"

#include <cassert>

#ifdef SDL_GPU
#include "rect.hpp"
namespace sdl
{
timage::timage(Uint16 w, Uint16 h)
	: image_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, clip_()
	, red_mod_(0)
	, green_mod_(0)
	, blue_mod_(0)
	, alpha_mod_(0)
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
{
	clip_ = create_gpu_rect(0, 0, w, h);
	image_ = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
	if (image_ == NULL) {
		throw tgpu_exception("Failed to construct timage object.", true);
	} else {
		image_->refcount = 1;
		static SDL_Color black = {0, 0, 0, 0};
		GPU_SetColor(image_, &black);
	}
}

timage::timage(const std::string &file)
	: image_(GPU_LoadImage(file.c_str()))
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, clip_()
	, red_mod_(0)
	, green_mod_(0)
	, blue_mod_(0)
	, alpha_mod_(0)
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
{
	if (image_ == NULL) {
		throw tgpu_exception("Failed to construct timage object.", true);
	} else {
		clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
		static SDL_Color black = {0, 0, 0, 0};
		GPU_SetColor(image_, &black);
	}
}

timage::timage(const surface &source)
	: image_(GPU_CopyImageFromSurface(source))
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, clip_()
	, red_mod_(0)
	, green_mod_(0)
	, blue_mod_(0)
	, alpha_mod_(0)
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
	, smooth_(false)
{
	if (image_ == NULL) {
		throw tgpu_exception("Failed to construct timage object.", true);
	} else {
		clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
		static SDL_Color black = {0, 0, 0, 0};
		GPU_SetColor(image_, &black);
	}
}

timage::timage(SDL_Surface *source)
	: image_(GPU_CopyImageFromSurface(source))
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, clip_()
	, red_mod_(0)
	, green_mod_(0)
	, blue_mod_(0)
	, alpha_mod_(0)
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
	, smooth_(false)
{
	if (image_ == NULL) {
		throw tgpu_exception("Failed to construct timage object.", true);
	} else {
		clip_ = create_gpu_rect(0, 0, image_->w, image_->h);
		image_->refcount = 1;
		static SDL_Color black = {0, 0, 0, 0};
		GPU_SetColor(image_, &black);
	}
}

timage::timage()
	: image_(NULL)
	, rotation_(0)
	, hscale_(1)
	, vscale_(1)
	, clip_()
	, red_mod_(0)
	, green_mod_(0)
	, blue_mod_(0)
	, alpha_mod_(0)
	, hwrap_(GPU_WRAP_NONE)
	, vwrap_(GPU_WRAP_NONE)
	, smooth_(false)
{
}

sdl::timage::~timage()
{
	if (image_ != NULL) {
		image_->refcount -= 1;
		if (image_->refcount == 0) {
			GPU_FreeImage(image_);
		}
	}
}

timage::timage(const timage &texture)
	: image_(texture.image_)
	, rotation_(texture.rotation_)
	, hscale_(texture.hscale_)
	, vscale_(texture.vscale_)
	, clip_(texture.clip_)
	, red_mod_(texture.red_mod_)
	, green_mod_(texture.green_mod_)
	, blue_mod_(texture.blue_mod_)
	, alpha_mod_(texture.alpha_mod_)
	, hwrap_(texture.hwrap_)
	, vwrap_(texture.vwrap_)
	, smooth_(texture.smooth_)
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

void timage::draw(CVideo &video, const int x, const int y)
{
	GPU_SetImageFilter(image_, smooth_ ? GPU_FILTER_LINEAR : GPU_FILTER_NEAREST);
	GPU_SetWrapMode(image_, hwrap_, vwrap_);
	video.set_texture_color_modulation(red_mod_, green_mod_, blue_mod_, alpha_mod_);
	GPU_BlitTransform(image_, &clip_, video.render_target(), x + width()/2, y + height()/2,
					  rotation_, hscale_, vscale_);
}

void timage::set_rotation(float rotation)
{
	rotation_ = rotation;
}

float timage::rotation() const
{
	return rotation_;
}

void timage::set_hscale(float factor)
{
	hscale_ = factor;
}

void timage::set_vscale(float factor)
{
	vscale_ = factor;
}

void timage::set_scale(float hfactor, float vfactor)
{
	hscale_ = hfactor;
	vscale_ = vfactor;
}

float timage::hscale() const
{
	return hscale_;
}

float timage::vscale() const
{
	return vscale_;
}

void timage::set_smooth_scaling(bool use_smooth)
{
	smooth_ = use_smooth;
}

bool timage::smooth_scaling() const
{
	return smooth_;
}

int timage::width() const
{
	return clip_.w * hscale_;
}

int timage::height() const
{
	return clip_.h * vscale_;
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
	clip_.x = rect.x;
	clip_.y = rect.y;
	clip_.w = rect.w;
	clip_.h = rect.h;
}

SDL_Rect timage::clip() const
{
	SDL_Rect result;
	result.x = clip_.x;
	result.y = clip_.y;
	result.w = clip_.w;
	result.h = clip_.h;

	return result;
}

void timage::set_alpha(int alpha)
{
	alpha_mod_ = alpha;
}

int timage::alpha() const
{
	return alpha_mod_;
}

void timage::set_color_mod(int r, int g, int b)
{
	red_mod_ = r;
	green_mod_ = g;
	blue_mod_ = b;
}

int timage::red_mod() const
{
	return red_mod_;
}

int timage::green_mod() const
{
	return green_mod_;
}

int timage::blue_mod() const
{
	return blue_mod_;
}

void timage::set_hwrap(GPU_WrapEnum mode)
{
	hwrap_ = mode;
}

void timage::set_vwrap(GPU_WrapEnum mode)
{
	vwrap_ = mode;
}

void timage::set_wrap(GPU_WrapEnum hmode, GPU_WrapEnum vmode)
{
	hwrap_ = hmode;
	vwrap_ = vmode;
}

GPU_WrapEnum timage::hwrap() const
{
	return hwrap_;
}

GPU_WrapEnum timage::vwrap() const
{
	return vwrap_;
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
