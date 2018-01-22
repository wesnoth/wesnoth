/*
   Copyright (C) 2003 - 2018 the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "sdl/utils.hpp"
#include "video.hpp"

void surface::free_surface()
{
	if(surface_) {
		/* Workaround for an SDL bug.
		* SDL 2.0.6 frees the blit map unconditionally in SDL_FreeSurface() without checking
		* if the reference count has fallen to zero. However, many SDL functions such as
		* SDL_ConvertSurface() assume that the blit map is present.
		* Thus, we only call SDL_FreeSurface() if this is the last reference to the surface.
		* Otherwise we just decrement the reference count ourselves.
		*
		* - Jyrki, 2017-09-23
		*/
		if(surface_->refcount > 1 && sdl_get_version() >= version_info(2, 0, 6)) {
			--surface_->refcount;
		} else {
			SDL_FreeSurface(surface_);
		}
	}
}

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

void surface_restorer::restore(const SDL_Rect& dst) const
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

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}
