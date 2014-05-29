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

struct surface;

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
	 * @param access              Access mode of the texture.
	 * @param file                Path of the file to load the pixels from.
	 */
	ttexture(SDL_Renderer& renderer, const int access, const std::string& file);

	/**
	 * Constructor.
	 *
	 * Loads data from and takes ownership of a SDL_surface.
	 *
	 * @pre                       @p source_surface__ points to a valid surface.
	 * @pre                       @p access == SDL_TEXTUREACCESS_STATIC
	 *                            || @p access == SDL_TEXTUREACCESS_STREAMING.
	 *
	 * @param renderer            The renderer the texture is associated with.
	 * @param access              Access mode of the texture.
	 * @param source_surface__    Forwarded to @ref source_surface_.
	 */
	ttexture(SDL_Renderer& renderer,
			 const int access,
			 SDL_Surface* source_surface__);

	/**
	 * Constructor.
	 *
	 * Loads data from a surface.
	 *
	 * @pre                       @p access == SDL_TEXTUREACCESS_STATIC
	 *                            || @p access == SDL_TEXTUREACCESS_STREAMING.
	 *
	 * @param renderer            The renderer the texture is associated with.
	 * @param access              Access mode of the texture.
	 * @param surface             Contains the surface data to copy.
	 */
	ttexture(SDL_Renderer& renderer, const int access, const surface& surface);

	~ttexture();

	ttexture(const ttexture& texture);

	ttexture& operator=(const ttexture& texture);


	/***** ***** ***** Draw function overloads. ***** ***** *****/

	/**
	 * Draw a texture on a renderer.
	 *
	 * The function calls @ref SDL_RenderCopy.
	 *
	 * The function draws the image unscaled at coordinates @p x, @p y.
	 *
	 * @param renderer            Used as renderer for @ref SDL_RenderCopy.
	 * @param x                   The x-coordinate where to draw the texture.
	 * @param y                   The y-coordinate where to draw the texture.
	 */
	void draw(SDL_Renderer& renderer, const int x, const int y);


	/***** ***** ***** Setters and getters. ***** ***** *****/

	/**
	 * Returns a pointer to the surface the texture was created from, if it was
	 * saved.
	 *
	 * @return                    A pointer to the source surface, or NULL if
	 *                            it's unavailable.
	 */
	const SDL_Surface* source_surface() const;

	/**
	 * Sets the angle of the texture.
	 *
	 * @param rotation            The angle the texture should be rotated by.
	 */
	void set_rotation(double rotation);

	/**
	 * Returns the current angle.
	 */
	double rotation() const;

	/**
	 * Sets the horizontal scaling factor.
	 *
	 * @param factor              The scaling factor.
	 */
	void set_hscale(float factor);

	/**
	 * Sets the vertical scaling factor.
	 *
	 * @param factor              The scaling factor.
	 */
	void set_vscale(float factor);

	/**
	 * Sets both scaling factors.
	 *
	 * @param hfactor             Horizontal scaling factor.
	 * @param vfactor             Vertical scaling factor.
	 */
	void set_scale(float hfactor, float vfactor);

	/**
	 * Returns the current horizontal scaling factor.
	 */
	float hscale() const;

	/**
	 * Returns the current vertical scaling factor.
	 */
	float vscale() const;

	/**
	 * Sets whether a smooth algorithm should be used when scaling the texture.
	 *
	 * @param use_smooth          true if smooth scaling should be used.
	 */
	void set_smooth_scaling(bool use_smooth);

	/**
	 * Tells whether smooth scaling is enabled for the texture.
	 */
	bool smooth_scaling() const;

	/**
	 * Sets whether the texture should be flipped horizontally.
	 *
	 * @param flip                true if yes.
	 */
	void set_flip(bool flip);

	/**
	 * Sets whether the texture should be flipped vertically.
	 *
	 * @param flop                true if yes.
	 */
	void set_flop(bool flop);

	/**
	 * Tells whether the texture is flipped horizontally.
	 */
	bool flipped() const;

	/**
	 * Tells whether the texture is flipped vertically.
	 */
	bool flopped() const;

	/**
	 * Returns the width of the texture.
	 */
	unsigned width() const;

	/**
	 * Returns the height of the texture.
	 */
	unsigned height() const;

	/**
	 * Returns the frame of the texture.
	 */
	SDL_Rect dimensions() const;

	/**
	 * Only display the specified area of the texture when rendering.
	 *
	 * @param rect                The rectangle which should be displayed.
	 */
	void set_clip(const SDL_Rect& rect);

	/**
	 * Returns the currently displayed area of the texture.
	 */
	const SDL_Rect& clip() const;

	/**
	 * Returns the format of the texture.
	 */
	Uint32 format() const;

	/**
	 * Sets the alpha for the texture.
	 *
	 * @param alpha               The alpha modifier.
	 */
	void set_alpha(Uint8 alpha);

	/**
	 * Returns the alpha of the texture.
	 */
	Uint8 alpha() const;

	/**
	 * Sets the blend mode of the texture.
	 *
	 * @param mode                One of the values enumerated in SDL_BlendMode.
	 */
	 void set_blend_mode(SDL_BlendMode mode);

	 /**
	  * Returns the blend mode of the texture.
	  */
	 SDL_BlendMode blend_mode() const;

	/***** ***** ***** Other. ***** ***** *****/

	/**
	 * Updates the pixels of the texture.
	 *
	 * @param surf                The surface the current data should be
	 *                            replaced with.
	 */
	 void update_pixels(SDL_Surface* surf);

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

	/** The angle the texture should be rotated by when rendering. */
	double rotation_;

	/** Horizontal scaling factor. */
	float hscale_;

	/** Vertical scaling factor. */
	float vscale_;

	/** Whether a smooth scaling algorithm should be used. */
	bool smooth_scaling_;

	/** Flip/flop. */
	SDL_RendererFlip flip_;

	/** What should actually be displayed of the texture. */
	SDL_Rect clip_;

	/**
	 * The SDL_Surface source of the @ref texture_.
	 *
	 * The value of the field dependings on the constructor used:
	 * - Image loading:
	 *   Depends on the @p access_mode argument:
	 *   * SDL_TEXTUREACCESS_STATIC @c NULL.
	 *   * SDL_TEXTUREACCESS_STREAMING the surface of the loaded image.
	 * - Other:
	 *   Always @c NULL.
	 */
	SDL_Surface* source_surface_;

	/**
	 * Creates the @ref texture_ from the @ref source_surface_.
	 *
	 * This is used in the constructor to create the texture when only a
	 * surface is available.
	 *
	 * @param renderer            The renderer argument of the constructor.
	 * @param access              The access argument of the constructor.
	 */
	void initialise_from_surface(SDL_Renderer& renderer, const int access);
};

} // namespace sdl

#endif

#endif
