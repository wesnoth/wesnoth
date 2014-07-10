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

#ifndef SDL_IMAGE_HPP_INCLUDED
#define SDL_IMAGE_HPP_INCLUDED

/**
 * @file
 * Contains a wrapper class for the @ref GPU_Image class.
 */

#ifdef SDL_GPU
#include "gpu.hpp"
#include <string>

struct surface;

namespace sdl
{

class timage
{
public:
	/**
	 * Creates a texture with the given width and height.
	 *
	 * @param w                   Width.
	 * @param h                   Height.
	 */
	timage(Uint16 w, Uint16 h);

	/**
	 * Loads a texture from an image file.
	 *
	 * @param file                Full path of the file.
	 */
	timage(const std::string &file);

	/**
	 * Creates a texture from an SDL surface.
	 *
	 * @param source              Pointer to the surface.
	 */
	timage( SDL_Surface *source);

	/**
	 * Creates a texture from an SDL surface.
	 *
	 * @param source              The surface.
	 */
	timage(const surface &source);

	timage();

	~timage();

	timage(const timage &texture);

	timage &operator=(const timage &texture);

	/**
	 * Render the texture on a specified target, with respect to the previously
	 * set rotation, coloring etc.
	 *
	 * @param target              The target to draw onto.
	 * @param x                   Where to draw (x coordinate).
	 * @param y                   Where to draw (y coordinate).
	 */
	void draw(GPU_Target &target, const int x, const int y);

	/**
	 * Rotates the texture by a given angle.
	 *
	 * @param rotation            The angle in degrees.
	 */
	void set_rotation(float rotation);

	/**
	 * Returns the angle by which the texture is rotated.
	 */
	float rotation() const;

	/**
	 * Scales the surface horizontally by a given factor.
	 *
	 * @param factor              Scaling ratio.
	 */
	void set_hscale(float factor);

	/**
	 * Scales the surface vertically by a given factor.
	 *
	 * @param factor              Scaling ratio.
	 */
	void set_vscale(float factor);

	/**
	 * Scales the surface by a given factor.
	 *
	 * @param hfactor             Horizontal scaling factor.
	 * @param vfactor             Vertical scaling factor.
	 */
	void set_scale(float hfactor, float vfactor);

	/**
	 * Returns the horizontal scaling factor.
	 */
	float hscale() const;

	/**
	 * Returns the vertical scaling factor.
	 */
	float vscale() const;

	/**
	 * Tell the renderer which scaling algorithm to use.
	 *
	 * @param use_smooth          True to use bilinear scaling.
	 */
	void set_smooth_scaling(bool use_smooth);

	/**
	 * Returns true if bilinear scaling is enabled.
	 */
	bool smooth_scaling() const;

	/**
	 * Returns the width of the texture after scaling applied.
	 */
	Uint16 width() const;

	/**
	 * Returns the height of the texture after scaling applied.
	 */
	Uint16 height() const;

	/**
	 * Returns the width of the texture before scaling.
	 */
	Uint16 base_width() const;

	/**
	 * Returns the height of the texture before scaling.
	 */
	Uint16 base_height() const;

	/**
	 * Set the area of the texture that should be displayed when drawing.
	 * Clipping is performed before drawing. The clip area can be larger than
	 * the texture itself, if this happens, extra pixels will be filled
	 * according the configured wrap mode.
	 *
	 * @param rect                The clip area.
	 */
	void set_clip(const SDL_Rect &rect);

	/**
	 * Returns the current clip area.
	 */
	SDL_Rect clip() const;

	/**
	 * Sets the alpha of the texture.
	 *
	 * @param alpha               Alpha value.
	 */
	void set_alpha(Uint8 alpha);

	/**
	 * Returns the alpha of the texture.
	 */
	Uint8 alpha() const;

	/**
	 * Set color modulation.
	 *
	 * @param r                   Red modulation.
	 * @param g                   Green modulation.
	 * @param b                   Blue modulation.
	 */
	void set_color_mod(Uint8 r, Uint8 g, Uint8 b);

	/**
	 * Returns red modulation.
	 */
	Uint8 red_mod() const;

	/**
	 * Returns green modulation.
	 */
	Uint8 green_mod() const;

	/**
	 * Returns blue modulation.
	 */
	Uint8 blue_mod() const;

	/**
	 * Set horizontal wrap mode.
	 *
	 * @param mode                Wrap mode.
	 */
	void set_hwrap(GPU_WrapEnum mode);

	/**
	 * Set vertical wrap mode.
	 *
	 * @param mode                Wrap mode.
	 */
	void set_vwrap(GPU_WrapEnum mode);

	/**
	 * Set wrap mode.
	 *
	 * @param hmode               Horizontal wrap mode.
	 * @param vmode               Vertical wrap mode.
	 */
	void set_wrap(GPU_WrapEnum hmode, GPU_WrapEnum vmode);

	/**
	 * Returns the horizontal wrap policy of the texture.
	 */
	GPU_WrapEnum hwrap() const;

	/**
	 * Returns the vertical wrap policy of the texture.
	 */
	GPU_WrapEnum vwrap( )const;

	/**
	 * Returns true if the managed texture is NULL.
	 */
	bool null() const;

private:
	/** The texture itself. */
	GPU_Image *image_;

	/** How much will it be rotated. */
	float rotation_;

	/** How much will it be scaled horizontally. */
	float hscale_;

	/** How much will it be scaled vertically. */
	float vscale_;

	/** Whether to use bilinear scaling. */
	bool smooth_scaling_;

	/** Which part of the texture should be displayed. */
	GPU_Rect clip_;

	/** Color modulation. */
	SDL_Color color_mod_;

	/** Horizontal wrap policy. */
	GPU_WrapEnum hwrap_;

	/** Vertical wrap policy. */
	GPU_WrapEnum vwrap_;
};
}
#endif

#endif
