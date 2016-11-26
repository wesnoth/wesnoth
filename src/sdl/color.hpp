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
	/**
	 * Constructors
	 */
	color_t();

	color_t(const color_t& c);

	color_t(uint8_t r_val, uint8_t g_val, uint8_t b_val, uint8_t a_val);

	color_t(uint32_t c);

	color_t(const std::string& c);

	color_t(const SDL_Color& c);

	/**
	 * Conversion functions
	 */
	uint32_t to_uint32();

	std::string to_pango_markup();

	SDL_Color to_sdl();

	/**
	 * Color members
	 */
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

#endif
