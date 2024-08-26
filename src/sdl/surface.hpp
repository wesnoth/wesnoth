/*
	Copyright (C) 2003 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "sdl/rect.hpp"
#include "utils/const_clone.hpp"

#include <SDL2/SDL.h>

#include <ostream>

class surface
{
public:
	surface() : surface_(nullptr)
	{}

	surface(SDL_Surface* surf);

	/** Allocates a new surface with the given dimensions. */
	surface(int w, int h);

	surface(const surface& s) : surface_(s.get())
	{
		add_surface_ref(surface_);
	}

	surface(surface&& s) noexcept : surface_(s.get())
	{
		s.surface_ = nullptr;
	}

	~surface()
	{
		free_surface();
	}

	surface& operator=(const surface& s)
	{
		assign_surface_internal(s.get());
		return *this;
	}

	surface& operator=(surface&& s) noexcept
	{
		free_surface();
		surface_ = s.surface_;
		s.surface_ = nullptr;
		return *this;
	}

	/**
	 * Check that the surface is neutral bpp 32.
	 *
	 * The surface may have an empty alpha channel.
	 *
	 * @returns                       The status @c true if neutral, @c false if not.
	 */
	bool is_neutral() const;

	/**
	 * Converts this surface to a neutral format if it is not already.
	 *
	 * @returns                       A reference to this object for chaining convenience.
	 */
	surface& make_neutral();

	/**
	 * Makes a copy of this surface. The copy will be in the 'neutral' pixel format.
	 *
	 * Note this is creates a new, duplicate surface in memory. Making a copy of this
	 * 'surface' object will *not* duplicate the surface itself since we only hold a
	 * pointer to the actual surface.
	 */
	surface clone() const;

	operator SDL_Surface*() const { return surface_; }

	SDL_Surface* get() const { return surface_; }

	SDL_Surface* operator->() const { return surface_; }

private:
	static void add_surface_ref(SDL_Surface* surf)
	{
		if(surf) {
			++surf->refcount;
		}
	}

	void assign_surface_internal(SDL_Surface* surf);

	void free_surface();

	SDL_Surface* surface_;

	static const SDL_PixelFormat neutral_pixel_format;
};

std::ostream& operator<<(std::ostream& stream, const surface& surf);

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
		if(operate_ && surface_.get()){
			SDL_GetClipRect(surface_, &rect_);
			SDL_Rect final_rect = { 0, 0, 0, 0 };

			if(r) {
				SDL_IntersectRect(&rect_, r, &final_rect);
			} else {
				final_rect.w = surface_->w;
				final_rect.h = surface_->h;
			}

			SDL_SetClipRect(surface_, &final_rect);
		}
	}

	~clip_rect_setter()
	{
		if(operate_ && surface_.get()) {
			SDL_SetClipRect(surface_, &rect_);
		}
	}

private:
	surface surface_;
	SDL_Rect rect_;
	const bool operate_;
};
