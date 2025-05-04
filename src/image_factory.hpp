/*
	Copyright (C) 2025 - 2025
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

#include "sdl/surface.hpp"
#include "sdl/texture.hpp"

#ifdef __cpp_concepts
#include <concepts>

// TODO: move this somewhere else once we support concepts generally
template<typename T>
concept Drawable = std::same_as<T, texture> || std::same_as<T, surface>;
#endif

namespace image
{
class locator;

/**
 * Loads image files and URIs to surfaces and textures.
 *
 * @note Results are not cached. This encapsulate the loading process only.
 * For a higher-level interface which handles caching and ToD coloring effects,
 * see image::get_surface and image::get_texture.
 */
#ifdef __cpp_concepts
template<Drawable T>
#else
template<typename T>
#endif
class factory
{
public:
	/** Loads from a URI or disk file depending on locator type. */
	static T load(const image::locator& loc);

private:
	/** Loads the given file from disk, accounting for localized variants and overlays. */
	static T from_disk(const image::locator& loc);

	/** Generates a new surface or texture from the contained base64 URI data. */
	static T from_data_uri(const image::locator& loc);
};

} // namespace image
