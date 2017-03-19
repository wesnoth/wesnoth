/*
   Copyright (C) 2003 - 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sdl/surface.hpp"

#include "sdl/rect.hpp"
#include "video.hpp"

surface_restorer::surface_restorer()
	: target_(nullptr)
	, rect_(sdl::empty_rect)
	, surface_(nullptr)
{
}

surface_restorer::surface_restorer(CVideo* target, const SDL_Rect& rect)
	: target_(target)
	, rect_(rect)
	, surface_(nullptr)
{
	update();
}

surface_restorer::~surface_restorer()
{
	restore();
}

void surface_restorer::restore(SDL_Rect const &dst) const
{
	if(surface_.null()) {
		return;
	}

	SDL_Rect dst2 = sdl::intersect_rects(dst, rect_);
	if(dst2.w == 0 || dst2.h == 0) {
		return;
	}

	SDL_Rect src = dst2;
	src.x -= rect_.x;
	src.y -= rect_.y;
	sdl_blit(surface_, &src, target_->getSurface(), &dst2);
}

void surface_restorer::restore() const
{
	if(surface_.null()) {
		return;
	}

	SDL_Rect dst = rect_;
	sdl_blit(surface_, nullptr, target_->getSurface(), &dst);
}

void surface_restorer::update()
{
	if(rect_.w <= 0 || rect_.h <= 0) {
		surface_.assign(nullptr);
	} else {
		surface_.assign(::get_surface_portion(target_->getSurface(),rect_));
	}
}

void surface_restorer::cancel()
{
	surface_.assign(nullptr);
}

surface_lock::surface_lock(surface &surf) : surface_(surf), locked_(false)
{
	if(SDL_MUSTLOCK(surface_)) {
		locked_ = SDL_LockSurface(surface_) == 0;
	}
}

surface_lock::~surface_lock()
{
	if(locked_) {
		SDL_UnlockSurface(surface_);
	}
}

const_surface_lock::const_surface_lock(const surface &surf) : surface_(surf), locked_(false)
{
	if(SDL_MUSTLOCK(surface_)) {
		locked_ = SDL_LockSurface(surface_) == 0;
	}
}

const_surface_lock::~const_surface_lock()
{
	if(locked_) {
		SDL_UnlockSurface(surface_);
	}
}

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}
