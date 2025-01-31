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

#include <SDL2/SDL_pixels.h>

#include <algorithm> // for max
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

constexpr uint32_t SDL_ALPHA_MASK = 0xFF000000;
constexpr uint32_t SDL_RED_MASK   = 0x00FF0000;
constexpr uint32_t SDL_GREEN_MASK = 0x0000FF00;
constexpr uint32_t SDL_BLUE_MASK  = 0x000000FF;

constexpr uint32_t SDL_ALPHA_BITSHIFT = 24;
constexpr uint32_t SDL_RED_BITSHIFT   = 16;
constexpr uint32_t SDL_GREEN_BITSHIFT = 8;
constexpr uint32_t SDL_BLUE_BITSHIFT  = 0;

constexpr uint32_t RGBA_ALPHA_MASK = 0x000000FF;
constexpr uint32_t RGBA_RED_MASK   = 0xFF000000;
constexpr uint32_t RGBA_GREEN_MASK = 0x00FF0000;
constexpr uint32_t RGBA_BLUE_MASK  = 0x0000FF00;

constexpr uint32_t RGBA_ALPHA_BITSHIFT = 0;
constexpr uint32_t RGBA_RED_BITSHIFT   = 24;
constexpr uint32_t RGBA_GREEN_BITSHIFT = 16;
constexpr uint32_t RGBA_BLUE_BITSHIFT  = 8;

constexpr uint8_t ALPHA_OPAQUE = SDL_ALPHA_OPAQUE; // This is always 255 in SDL2

// Functions for manipulating RGB values.
constexpr uint8_t float_to_color(double n);
constexpr uint8_t float_to_color(float n);
constexpr uint8_t color_multiply(uint8_t n1, uint8_t n2);
constexpr uint8_t color_blend(uint8_t n1, uint8_t n2, uint8_t p);

/**
 * The basic class for representing 8-bit RGB or RGBA colour values.
 *
 * This is a thin wrapper over SDL_Color, and can be used interchangeably.
 */
struct color_t : SDL_Color
{
	/** color_t initializes to fully opaque white by default. */
	constexpr color_t() : SDL_Color{255, 255, 255, ALPHA_OPAQUE} {}

	/** Basic RGB or RGBA constructor. */
	constexpr color_t(uint8_t r_val, uint8_t g_val, uint8_t b_val, uint8_t a_val = ALPHA_OPAQUE)
		: SDL_Color{r_val, g_val, b_val, a_val}
	{}

	/** This is a thin wrapper. There is nothing extra to do here. */
	constexpr color_t(const SDL_Color& c) : SDL_Color{c} {}

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
	static color_t from_rgba_string(std::string_view c);

	/**
	 * Creates a new opaque color_t object from a string variable in "R,G,B" format.
	 * An empty string results in white. Otherwise, omitting components is an error.
	 *
	 * @param c      A string variable, in "R,G,B" format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument if the string is not correctly formatted
	 */
	static color_t from_rgb_string(std::string_view c);

	/**
	 * Creates a new color_t object from a string variable in hex format.
	 *
	 * @param c      A string variable, in rrggbb hex format.
	 * @return       A new color_t object.
	 *
	 * @throw        std::invalid_argument if the string is not correctly formatted
	 */
	static color_t from_hex_string(std::string_view c);

	/**
	 * Creates a new color_t object from a uint32_t variable.
	 *
	 * @param c      A uint32_t variable, in RGBA format.
	 * @return       A new color_t object.
	 */
	static constexpr color_t from_rgba_bytes(uint32_t c)
	{
		return {
			static_cast<uint8_t>((RGBA_RED_MASK   & c) >> RGBA_RED_BITSHIFT),
			static_cast<uint8_t>((RGBA_GREEN_MASK & c) >> RGBA_GREEN_BITSHIFT),
			static_cast<uint8_t>((RGBA_BLUE_MASK  & c) >> RGBA_BLUE_BITSHIFT),
			static_cast<uint8_t>((RGBA_ALPHA_MASK & c) >> RGBA_ALPHA_BITSHIFT),
		};
	}

	/**
	 * Creates a new color_t object from a uint32_t variable.
	 *
	 * @param c      A uint32_t variable, in ARGB format.
	 * @return       A new color_t object.
	 */
	static constexpr color_t from_argb_bytes(uint32_t c)
	{
		return {
			static_cast<uint8_t>((SDL_RED_MASK   & c) >> SDL_RED_BITSHIFT),
			static_cast<uint8_t>((SDL_GREEN_MASK & c) >> SDL_GREEN_BITSHIFT),
			static_cast<uint8_t>((SDL_BLUE_MASK  & c) >> SDL_BLUE_BITSHIFT),
			static_cast<uint8_t>((SDL_ALPHA_MASK & c) >> SDL_ALPHA_BITSHIFT),
		};
	}

	/**
	 * Returns the stored color in rrggbb hex format.
	 *
	 * @return       The string in hex format. The preceding '#' needed for pango markup
	 *               is prepended.
	 */
	std::string to_hex_string() const;

	/**
	 * Returns the stored color as a uint32_t, in RGBA format.
	 *
	 * @return       The new uint32_t object.
	 */
	constexpr uint32_t to_rgba_bytes() const
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
	constexpr uint32_t to_argb_bytes() const
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

	constexpr bool null() const
	{
		return *this == null_color();
	}

	constexpr bool operator==(const color_t& c) const
	{
		return r == c.r && g == c.g && b == c.b && a == c.a;
	}

	constexpr bool operator!=(const color_t& c) const
	{
		return !(*this == c);
	}

	constexpr color_t blend_add(const color_t& c) const
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

	constexpr color_t blend_lighten(const color_t& c) const
	{
		return {
			std::max<uint8_t>(r, c.r),
			std::max<uint8_t>(g, c.g),
			std::max<uint8_t>(b, c.b),
			std::max<uint8_t>(a, c.a),
		};
	}

	constexpr color_t inverse() const {
		return {
			static_cast<uint8_t>(255 - r),
			static_cast<uint8_t>(255 - g),
			static_cast<uint8_t>(255 - b),
			a
		};
	}

	/**
	 * Blend smoothly with another color_t.
	 *
	 * @param c     The color to blend with.
	 * @param p     The proportion of the other color to blend. 0 will
	 *              return the original color, 255 the blending color.
	 */
	constexpr color_t smooth_blend(const color_t& c, uint8_t p) const
	{
		return {
			color_blend(r, c.r, p),
			color_blend(g, c.g, p),
			color_blend(b, c.b, p),
			color_blend(a, c.a, p)
		};
	}

	/** Definition of a 'null' color - fully transparent black. */
	static constexpr color_t null_color()
	{
		return {0,0,0,0};
	}
};

inline std::ostream& operator<<(std::ostream& s, const color_t& c)
{
	s << c.to_hex_string();
	return s;
}

namespace std
{
	template<>
	struct hash<color_t>
	{
		std::size_t operator()(const color_t& c) const noexcept
		{
			return c.to_rgba_bytes();
		}
	};
}

/********************************************/
/* Functions for manipulating colour values */
/********************************************/

/** Convert a double in the range [0.0,1.0] to an 8-bit colour value. */
constexpr uint8_t float_to_color(double n)
{
	if(n <= 0.0) return 0;
	else if(n >= 1.0) return 255;
	else return uint8_t(n * 256.0);
}

/** Convert a float in the range [0.0,1.0] to an 8-bit colour value. */
constexpr uint8_t float_to_color(float n)
{
	if(n <= 0.0f) return 0;
	else if(n >= 1.0f) return 255;
	else return uint8_t(n * 256.0f);
}

/** Multiply two 8-bit colour values as if in the range [0.0,1.0]. */
constexpr uint8_t color_multiply(uint8_t n1, uint8_t n2)
{
	return uint8_t((uint16_t(n1) * uint16_t(n2))/255);
}

/**
 * Blend 8-bit colour value with another in the given proportion.
 *
 * A proportion of 0 returns the first value, 255 the second.
 */
constexpr uint8_t color_blend(uint8_t n1, uint8_t n2, uint8_t p)
{
	return color_multiply(n1, ~p) + color_multiply(n2, p);
}
