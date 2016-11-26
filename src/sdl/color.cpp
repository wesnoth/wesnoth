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

#include "serialization/string_utils.hpp"
#include "sdl/color.hpp"
#include "sdl/utils.hpp"

#include <iomanip>
#include <sstream>

color_t::color_t()
	: r(255)
	, g(255)
	, b(255)
	, a(255)
{
}

color_t::color_t(const color_t& c)
	: r(c.r)
	, g(c.g)
	, b(c.b)
	, a(c.a)
{
}

color_t::color_t(uint8_t r_val, uint8_t g_val , uint8_t b_val, uint8_t a_val)
	: r(r_val)
	, g(g_val)
	, b(b_val)
	, a(a_val)
{
}

color_t::color_t(uint32_t c)
	: r(static_cast<uint8_t>((SDL_RED_MASK   & c) >> 24))
	, g(static_cast<uint8_t>((SDL_GREEN_MASK & c) >> 16))
	, b(static_cast<uint8_t>((SDL_BLUE_MASK  & c) >> 8))
	, a(static_cast<uint8_t>( SDL_ALPHA_MASK & c))
{
}

color_t::color_t(const std::string& c)
	: r()
	, g()
	, b()
	, a()
{
	std::vector<std::string> fields = utils::split(c);

	// Make sure we have 4 fields
	while(fields.size() < 4) {
		fields.push_back("0");
	}

	r = static_cast<uint8_t>(std::stoul(fields[0]));
	g = static_cast<uint8_t>(std::stoul(fields[0]));
	b = static_cast<uint8_t>(std::stoul(fields[0]));
	a = static_cast<uint8_t>(std::stoul(fields[0]));
}

color_t::color_t(const SDL_Color& c)
	: r(c.r)
	, g(c.g)
	, b(c.b)
	, a(c.a)
{
}

uint32_t color_t::to_uint32()
{
	return (r << 24) + (g << 16) + (b << 8) + a;
}

std::string color_t::to_pango_markup()
{
	std::ostringstream h;

	// Must match what pango expects
	h << "#"
	  << std::hex << std::setfill('0') << std::setw(2) << (r & 0xFF0000)
	  << std::hex << std::setfill('0') << std::setw(2) << (g & 0x00FF00)
	  << std::hex << std::setfill('0') << std::setw(2) << (b & 0x0000FF);

	return h.str();
}

SDL_Color color_t::to_sdl()
{
	return {r, g, b, a};
}
