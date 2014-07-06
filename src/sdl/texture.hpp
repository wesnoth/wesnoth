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
 * Contains a wrapper class for the @ref GPU_Image class.
 */

#ifdef SDL_GPU
#include "gpu.hpp"
#include <string>

struct surface;

namespace sdl
{

class ttexture
{
public:
	ttexture(Uint16 w, Uint16 h);

	ttexture(const std::string &file);

	ttexture( SDL_Surface *source);

	ttexture(const surface &source);

	ttexture();

	~ttexture();

	ttexture(const ttexture &texture);

	ttexture &operator=(const ttexture &texture);

	void draw(GPU_Target &target, const int x, const int y);

	void set_rotation(float rotation);

	float rotation() const;

	void set_hscale(float factor);

	void set_vscale(float factor);

	void set_scale(float hfactor, float vfactor);

	float hscale() const;

	float vscale() const;

	void set_smooth_scaling(bool use_smooth);

	bool smooth_scaling() const;

	Uint16 width() const;

	Uint16 height() const;

	void set_clip(const SDL_Rect &rect);

	SDL_Rect clip() const;

	void set_alpha(Uint8 alpha);

	Uint8 alpha() const;

	void set_color_mod(Uint8 r, Uint8 g, Uint8 b);

	Uint8 red_mod() const;

	Uint8 green_mod() const;

	Uint8 blue_mod() const;

	void set_hwrap(GPU_WrapEnum mode);

	void set_vwrap(GPU_WrapEnum mode);

	void set_wrap(GPU_WrapEnum hmode, GPU_WrapEnum vmode);

	GPU_WrapEnum hwrap() const;

	GPU_WrapEnum vwrap( )const;

	bool null() const;

private:
	GPU_Image *image_;

	float rotation_;

	float hscale_;

	float vscale_;

	bool smooth_scaling_;

	GPU_Rect clip_;

	SDL_Color color_mod_;

	GPU_WrapEnum hwrap_;

	GPU_WrapEnum vwrap_;
};
}
#endif

#endif
