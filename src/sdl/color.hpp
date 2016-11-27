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

const uint32_t SDL_ALPHA_MASK = 0xFF000000;
const uint32_t SDL_RED_MASK   = 0x00FF0000;
const uint32_t SDL_GREEN_MASK = 0x0000FF00;
const uint32_t SDL_BLUE_MASK  = 0x000000FF;

const uint32_t SDL_ALPHA_BITSHIFT = 24;
const uint32_t SDL_RED_BITSHIFT   = 16;
const uint32_t SDL_GREEN_BITSHIFT = 8;
const uint32_t SDL_BLUE_BITSHIFT  = 0;

const uint32_t RGBA_ALPHA_MASK = 0x000000FF;
const uint32_t RGBA_RED_MASK   = 0xFF000000;
const uint32_t RGBA_GREEN_MASK = 0x00FF0000;
const uint32_t RGBA_BLUE_MASK  = 0x0000FF00;

const uint32_t RGBA_ALPHA_BITSHIFT = 0;
const uint32_t RGBA_RED_BITSHIFT   = 24;
const uint32_t RGBA_GREEN_BITSHIFT = 16;
const uint32_t RGBA_BLUE_BITSHIFT  = 8;

struct color_t
{
	color_t()
		: r(255)
		, g(255)
		, b(255)
		, a(SDL_ALPHA_OPAQUE)
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
	 * Creates a new color_t object from a string variable in "R,G,B,A" format.
	 *
	 * @param c      A string variable, in "R,G,B,A" format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument if the string is not correctly formatted
	 */
	static color_t from_rgba_string(const std::string& c);

	/**
	 * Creates a new color_t object from a string variable in hex format.
	 *
	 * @param c      A string variable, in rrggbb hex format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument
	 */
	static color_t from_hex_string(const std::string& c);

	/**
	 * Creates a new color_t object from a uint32_t variable.
	 *
	 * @param c      A uint32_t variable, in RGBA format.
	 * @return       A new color_t object.
	 */
	static color_t from_rgba_bytes(uint32_t c);

	/**
	 * Creates a new color_t object from a uint32_t variable.
	 *
	 * @param c      A uint32_t variable, in ARGB format.
	 * @return       A new color_t object.
	 */
	static color_t from_argb_bytes(uint32_t c);

	/**
	 * Returns the stored color in rrggbb hex format.
	 *
	 * @return       The string in hex format. Note the preceeding '#' needed for pango markup
	 *               is not prepended.
	 */
	std::string to_hex_string();

	/**
	 * Returns the stored color as a uint32_t, in RGBA format.
	 *
	 * @return       The new uint32_t object.
	 */
	uint32_t to_rgba_bytes()
	{
		return
			(static_cast<uint32_t>(r) << RGBA_RED_BITSHIFT) |
			(static_cast<uint32_t>(g) << RGBA_GREEN_BITSHIFT) |
			(static_cast<uint32_t>(b) << RGBA_BLUE_BITSHIFT) |
			(static_cast<uint32_t>(a) << RGBA_ALPHA_BITSHIFT);
	}

	/**
	 * Returns the stored color as a uint32_t, an ARGB format.
	 *
	 * @return       The new uint32_t object.
	 */
	uint32_t to_argb_bytes()
	{
		return
			(static_cast<uint32_t>(r) << SDL_RED_BITSHIFT) |
			(static_cast<uint32_t>(g) << SDL_GREEN_BITSHIFT) |
			(static_cast<uint32_t>(b) << SDL_BLUE_BITSHIFT) |
			(static_cast<uint32_t>(a) << SDL_ALPHA_BITSHIFT);
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

	bool operator==(const color_t& c)
	{
		return r == c.r && g == c.g && b == c.b && a == c.a;
	}

	bool operator!=(const color_t& c)
	{
		return !(*this == c);
	}

	color_t plus_clipped(const color_t& c)
	{
		// Do some magic to detect integer overflow
		// We want overflows to max out the component instead of wrapping.
		// The static_cast is to silence narrowing conversion warnings etc
		return {
			static_cast<uint8_t>(r > 255 - c.r ? 255 : r + c.r),
			static_cast<uint8_t>(g > 255 - c.g ? 255 : g + c.g),
			static_cast<uint8_t>(b > 255 - c.b ? 255 : b + c.b),
			static_cast<uint8_t>(a > 255 - c.a ? 255 : a + c.a),
		};
	}
};

std::ostream& operator<<(std::ostream& s, const color_t& c)
{
	s << int(c.r) << " " << int(c.g) << " " << int(c.b) << " " << int(c.a) << std::endl;
	return s;
}

#endif
