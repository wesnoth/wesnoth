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

#ifndef SDL_SURFACE_HEADER_INCLUDED
#define SDL_SURFACE_HEADER_INCLUDED

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

	void free_surface()
	{
		if(surface_) {
			SDL_FreeSurface(surface_);
		}
	}

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
struct surface_lock
{
	surface_lock(surface &surf);
	~surface_lock();

	Uint32* pixels() { return reinterpret_cast<Uint32*>(surface_->pixels); }

private:
	surface& surface_;
	bool locked_;
};

struct const_surface_lock
{
	const_surface_lock(const surface &surf);
	~const_surface_lock();

	const Uint32* pixels() const { return reinterpret_cast<const Uint32*>(surface_->pixels); }

private:
	const surface& surface_;
	bool locked_;
};

struct clip_rect_setter
{
	// if r is nullptr, clip to the full size of the surface.
	clip_rect_setter(const surface &surf, const SDL_Rect* r, bool operate = true) : surface_(surf), rect_(), operate_(operate)
	{
		if(operate_){
			SDL_GetClipRect(surface_, &rect_);
			SDL_SetClipRect(surface_, r);
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

#endif
