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

#ifndef SDL_TEXTURE_HPP_INCLUDED
#define SDL_TEXTURE_HPP_INCLUDED

/**
 * @file
 * Contains a wrapper class for the @ref SDL_Texture class.
 */

#include <SDL_version.h>

#if SDL_VERSION_ATLEAST(2, 0, 0)

#include <SDL_render.h>
#include <string>
#include <sdl_utils.hpp>

namespace sdl
{

/**
 * The reference counted wrapper class for the @ref SDL_Texture class.
 */
class ttexture
{
public:

	/***** ***** ***** Constructor and destructor. ***** ***** *****/

	/**
	 * Constructor.
	 *
	 * The function calls @ref SDL_CreateTexture.
	 *
	 * @param renderer            Used as renderer for @ref SDL_CreateTexture.
	 * @param format              Used as format for @ref SDL_CreateTexture.
	 * @param access              Used as access for @ref SDL_CreateTexture.
	 * @param w                   Used as w for @ref SDL_CreateTexture.
	 * @param h                   Used as x for @ref SDL_CreateTexture.
	 */
	ttexture(SDL_Renderer& renderer,
			 const Uint32 format,
			 const int access,
			 const int w,
			 const int h);

	/**
	 * Constructor.
	 *
	 * Loads image data from @a file and converts it to a texture.
	 *
	 * @param renderer            The renderer the texture is associated with.
	 * @param file                Path of the file to load the pixels from.
	 * @param access              Access mode of the texture.
	 * @param keep_surface        Whether a copy of the image should be kept in
	 *                            an SDL_Surface.
	 */
	ttexture(SDL_Renderer& renderer,
			 const std::string& file,
			 int access,
			 bool keep_surface = false);

	~ttexture();

	ttexture(const ttexture& texture);

	ttexture& operator=(const ttexture& texture);


	/***** ***** ***** Members. ***** ***** *****/

	/**
	 * Returns a pointer to the surface the texture was created from, if it was
	 * saved.
	 *
	 * @return                    A pointer to the source surface, or NULL if
	 *                            it's unavailable.
	 */
	surface source_surface() const;

private:

	/**
	 * The reference count of the texture.
	 *
	 * Since allocating the reference counter can throw an exception it is the
	 * first member of the class.
	 */
	unsigned* reference_count_;

	/** The SDL_Texture we manage. */
	SDL_Texture* texture_;

	/** The SDL_Surface source of the texture. Probably NULL. */
	surface source_surface_;
};

} // namespace sdl

#endif

#endif
