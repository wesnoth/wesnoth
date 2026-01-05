/*
	Copyright (C) 2003 - 2025
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

#include "utils/const_clone.hpp"
#include "utils/span.hpp"

#include <SDL2/SDL_surface.h>

#include <ostream>

struct point;

class surface
{
public:
	surface() = default;
	surface(SDL_Surface* surf);

	/** Allocates a new surface with the given dimensions. */
	surface(int w, int h);

	surface(const surface& s);
	surface(surface&& s) noexcept;

	~surface();

	surface& operator=(const surface& s);
	surface& operator=(surface&& s) noexcept;

	/**
	 * Creates a new, duplicate surface in memory using the 'neutral' pixel format.
	 *
	 * @note Making a copy of a surface object does *not* duplicate its pixel data,
	 * since we only hold a pointer to the actual buffer. For a true deep copy, use
	 * this method.
	 */
	surface clone() const;

	/** Dimensions of the surface. */
	point size() const;

	/** Total area of the surface in square pixels. */
	std::size_t area() const;

	operator SDL_Surface*() const { return surface_; }

	SDL_Surface* get() const { return surface_; }
	SDL_Surface* operator->() const { return surface_; }

private:
	SDL_Surface* surface_{};
};

std::ostream& operator<<(std::ostream& stream, const surface& surf);

namespace surface_helper
{
/**
 * Returns a read-only view over to @a surf's underlying pixel array.
 */
inline auto pixel_span(const surface& surf)
{
	auto* pixels = reinterpret_cast<const uint32_t*>(surf->pixels);
	return utils::span{pixels, surf.area()};
}

/**
 * Returns a mutable per-pixel view over @a surf's underlying pixel array.
 */
inline auto pixel_span(surface& surf)
{
	auto* pixels = reinterpret_cast<uint32_t*>(surf->pixels);
	return utils::span{pixels, surf.area()};
}

} // namespace surface_helper

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

	pixel_t* pixels() const
	{
		return reinterpret_cast<pixel_t*>(surface_->pixels);
	}

	utils::span<pixel_t> pixel_span() const
	{
		return surface_helper::pixel_span(surface_);
	}

private:
	T& surface_;
	bool locked_;
};

using surface_lock = surface_locker<surface>;
using const_surface_lock = surface_locker<const surface>;
