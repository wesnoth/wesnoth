/*
   Copyright (C) 2003 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef COLOR_T_HPP_INCLUDED
#define COLOR_T_HPP_INCLUDED

#include <cstdint>
#include <string>

#include <SDL.h>

struct color_t
{
	color_t()
		: r(255)
		, g(255)
		, b(255)
		, a(SDL_ALPHA_OPAQUE)
	{}

	explicit color_t(const color_t& c)
		: r(c.r)
		, g(c.g)
		, b(c.b)
		, a(c.a)
	{}

	color_t(uint8_t r_val, uint8_t g_val, uint8_t b_val, uint8_t a_val = SDL_ALPHA_OPAQUE)
		: r(r_val)
		, g(g_val)
		, b(b_val)
		, a(a_val)
	{}

	explicit color_t(const SDL_Color& c)
		: r(c.r)
		, g(c.g)
		, b(c.b)
		, a(c.a)
	{}

	/**
	 * Creates a new color_t object from a string variable.
	 *
	 * @param        A string variable, in "R,G,B,A" format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument
	 */
	static color_t from_rgba_string(const std::string& c);

	/**
	 * Creates a new color_t object from a uint32_t variable.
	 *
	 * @param        A uint32_t variable, in RGBA format.
	 * @return       A new color_t object.
	 */
	static color_t from_rgba_uint32(uint32_t c);

	/**
	 * Returns the stored color in rrggbb hex pango markup.
	 *
	 * @return       The string in pango markup. Preceeding '#' is included.
	 */
	std::string to_pango_markup();

	/**
	 * Returns the stored color as a uint32_t.
	 *
	 * @return       The new uint32_t object.
	 */
	uint32_t to_rgba_uint32()
	{
		return (r << 24) + (g << 16) + (b << 8) + a;
	}

	/**
	 * Returns the stored color as an SDL_Color object.
	 *
	 * @return       The new SDL_Color object.
	 */
	SDL_Color to_sdl()
	{
		return {r, g, b, a};
	}

	/** Red value */
	uint8_t r;

	/** Green value */
	uint8_t g;

	/** Blue value */
	uint8_t b;

	/** Alpha value */
	uint8_t a;
};

#endif
