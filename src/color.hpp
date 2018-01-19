/*
   Copyright (C) 2003 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <algorithm> // for max
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

struct SDL_Color;

//
// TODO: constexpr
//

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

const uint8_t ALPHA_OPAQUE = 255;

struct color_t
{
	color_t()
		: r(255)
		, g(255)
		, b(255)
		, a(ALPHA_OPAQUE)
	{}

	color_t(uint8_t r_val, uint8_t g_val, uint8_t b_val, uint8_t a_val = ALPHA_OPAQUE)
		: r(r_val)
		, g(g_val)
		, b(b_val)
		, a(a_val)
	{}

	// Implemented in sdl/utils.cpp to avoid dependency nightmares
	explicit color_t(const SDL_Color& c);

	/**
	 * Creates a new color_t object from a string variable in "R,G,B,A" format.
	 * An empty string results in white. Otherwise, omitting components other than
	 * alpha is an error.
	 *
	 * @param c      A string variable, in "R,G,B,A" format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument if the string is not correctly formatted
	 */
	static color_t from_rgba_string(const std::string& c);

	/**
	 * Creates a new opaque color_t object from a string variable in "R,G,B" format.
	 * An empty string results in white. Otherwise, omitting components is an error.
	 *
	 * @param c      A string variable, in "R,G,B" format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument if the string is not correctly formatted
	 */
	static color_t from_rgb_string(const std::string& c);

	/**
	 * Creates a new color_t object from a string variable in hex format.
	 *
	 * @param c      A string variable, in rrggbb hex format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument if the string is not correctly formatted
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
	 * @return       The string in hex format. The preceeding '#' needed for pango markup
	 *               is prepended.
	 */
	std::string to_hex_string() const;

	/**
	 * Returns the stored color as a uint32_t, in RGBA format.
	 *
	 * @return       The new uint32_t object.
	 */
	uint32_t to_rgba_bytes() const
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
	uint32_t to_argb_bytes() const
	{
		return
			(static_cast<uint32_t>(r) << SDL_RED_BITSHIFT) |
			(static_cast<uint32_t>(g) << SDL_GREEN_BITSHIFT) |
			(static_cast<uint32_t>(b) << SDL_BLUE_BITSHIFT) |
			(static_cast<uint32_t>(a) << SDL_ALPHA_BITSHIFT);
	}

	/**
	 * Returns the stored color as an "R,G,B,A" string
	 *
	 * @return      The new color string.
	 */
	std::string to_rgba_string() const;

	/**
	 * Returns the stored color as an "R,G,B" string
	 *
	 * @return      The new color string.
	 */
	std::string to_rgb_string() const;

	/**
	 * Returns the stored color as an color_t object.
	 *
	 * @return       The new color_t object.
	 */
	// Implemented in sdl/utils.cpp to avoid dependency nightmares
	SDL_Color to_sdl() const;

	/** Red value */
	uint8_t r;

	/** Green value */
	uint8_t g;

	/** Blue value */
	uint8_t b;

	/** Alpha value */
	uint8_t a;

	bool null() const
	{
		return *this == null_color();
	}

	bool operator==(const color_t& c) const
	{
		return r == c.r && g == c.g && b == c.b && a == c.a;
	}

	bool operator!=(const color_t& c) const
	{
		return !(*this == c);
	}

	color_t blend_add(const color_t& c) const
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

	color_t blend_lighten(const color_t& c) const
	{
		return {
			std::max<uint8_t>(r, c.r),
			std::max<uint8_t>(g, c.g),
			std::max<uint8_t>(b, c.b),
			std::max<uint8_t>(a, c.a),
		};
	}

	color_t inverse() const {
		return {
			static_cast<uint8_t>(255 - r),
			static_cast<uint8_t>(255 - g),
			static_cast<uint8_t>(255 - b),
			a
		};
	}

	/** Definition of a 'null' color - fully transparent black. */
	static color_t null_color()
	{
		return {0,0,0,0};
	}
};

inline std::ostream& operator<<(std::ostream& s, const color_t& c)
{
	s << int(c.r) << " " << int(c.g) << " " << int(c.b) << " " << int(c.a) << std::endl;
	return s;
}

namespace std
{
	template<>
	struct hash<color_t>
	{
		size_t operator()(const color_t& c) const
		{
			return c.to_rgba_bytes();
		}
	};
}
