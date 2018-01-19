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

#pragma once

#include "utils/const_clone.hpp"

#include <SDL.h>

class CVideo;

class surface
{
public:
	surface() : surface_(nullptr)
	{}

	surface(SDL_Surface* surf) : surface_(surf)
	{}

	surface(const surface& s) : surface_(s.get())
	{
		add_surface_ref(surface_);
	}

	surface(surface&& s) : surface_(s.get())
	{
		s.surface_ = nullptr;
	}

	~surface()
	{
		free_surface();
	}

	void assign(SDL_Surface* surf)
	{
		assign_surface_internal(surf);
	}

	void assign(const surface& s)
	{
		assign_surface_internal(s.get());
	}

	surface& operator=(const surface& s)
	{
		assign(s);
		return *this;
	}

	surface& operator=(surface&& s)
	{
		free_surface();
		surface_ = s.surface_;
		s.surface_ = nullptr;
		return *this;
	}

	// Intended to be used when SDL has already freed the surface
	void clear_without_free() { surface_ = nullptr; }

	operator SDL_Surface*() const { return surface_; }

	SDL_Surface* get() const { return surface_; }

	SDL_Surface* operator->() const { return surface_; }

	bool null() const { return surface_ == nullptr; }

private:
	static void add_surface_ref(SDL_Surface* surf)
	{
		if(surf) {
			++surf->refcount;
		}
	}

	void assign_surface_internal(SDL_Surface* surf)
	{
		add_surface_ref(surf); // Needs to be done before assignment to avoid corruption on "a = a;"
		free_surface();
		surface_ = surf;
	}

	void free_surface();

	SDL_Surface* surface_;
};

bool operator<(const surface& a, const surface& b);

struct surface_restorer
{
	surface_restorer();
	surface_restorer(class CVideo* target, const SDL_Rect& rect);
	~surface_restorer();

	void restore() const;
	void restore(SDL_Rect const &dst) const;
	void update();
	void cancel();

	const SDL_Rect& area() const { return rect_; }

private:
	class CVideo* target_;
	SDL_Rect rect_;
	surface surface_;
};

/**
 * Helper class for pinning SDL surfaces into memory.
 * @note This class should be used only with neutral surfaces, so that
 *       the pointer returned by #pixels is meaningful.
 */
template<typename T>
class surface_locker
{
private:
	using pixel_t = utils::const_clone_t<uint32_t, T>;

public:
	surface_locker(T& surf) : surface_(surf), locked_(false)
	{
		if(SDL_MUSTLOCK(surface_)) {
			locked_ = SDL_LockSurface(surface_) == 0;
		}
	}

	~surface_locker()
	{
		if(locked_) {
			SDL_UnlockSurface(surface_);
		}
	}

	pixel_t* pixels() const { return reinterpret_cast<pixel_t*>(surface_->pixels); }

private:
	T& surface_;
	bool locked_;
};

using surface_lock = surface_locker<surface>;
using const_surface_lock = surface_locker<const surface>;

struct clip_rect_setter
{
	// if r is nullptr, clip to the full size of the surface.
	clip_rect_setter(const surface &surf, const SDL_Rect* r, bool operate = true) : surface_(surf), rect_(), operate_(operate)
	{
		if(operate_){
			SDL_GetClipRect(surface_, &rect_);
			SDL_Rect final_rect;
			SDL_IntersectRect(&rect_, r, &final_rect);
			SDL_SetClipRect(surface_, &final_rect);
		}
	}

	~clip_rect_setter()
	{
		if(operate_) {
			SDL_SetClipRect(surface_, &rect_);
		}
	}

private:
	surface surface_;
	SDL_Rect rect_;
	const bool operate_;
};
