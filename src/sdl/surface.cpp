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

bool operator<(const surface& a, const surface& b)
{
	return a.get() < b.get();
}
