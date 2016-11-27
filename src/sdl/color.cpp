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

#include <iomanip>
#include <sstream>

color_t color_t::from_rgba_string(const std::string& c)
{
	std::vector<std::string> fields = utils::split(c);

	// Make sure we have 4 fields
	while(fields.size() < 4) {
		fields.push_back("0");
	}

	return {
		static_cast<uint8_t>(std::stoul(fields[0])),
		static_cast<uint8_t>(std::stoul(fields[0])),
		static_cast<uint8_t>(std::stoul(fields[0])),
		static_cast<uint8_t>(std::stoul(fields[0]))
	};
}

color_t color_t::from_hex_string(const std::string& c)
{
	unsigned long temp_c = std::strtol(c.c_str(), nullptr, 16);

	return {
		static_cast<uint8_t>((0x00FFFFFF & temp_c) >> 16),
		static_cast<uint8_t>((0x00FFFFFF & temp_c) >> 8),
		static_cast<uint8_t>((0x00FFFFFF & temp_c)),
		SDL_ALPHA_OPAQUE
	};
}

color_t color_t::from_rgba_bytes(uint32_t c)
{
	return {
		static_cast<uint8_t>((RGBA_RED_MASK   & c) >> RGBA_RED_BITSHIFT),
		static_cast<uint8_t>((RGBA_GREEN_MASK & c) >> RGBA_GREEN_BITSHIFT),
		static_cast<uint8_t>((RGBA_BLUE_MASK  & c) >> RGBA_BLUE_BITSHIFT),
		static_cast<uint8_t>((RGBA_ALPHA_MASK & c) >> RGBA_ALPHA_BITSHIFT),
	};
}

color_t color_t::from_argb_bytes(uint32_t c)
{
	return {
		static_cast<uint8_t>((SDL_RED_MASK   & c) >> SDL_RED_BITSHIFT),
		static_cast<uint8_t>((SDL_GREEN_MASK & c) >> SDL_GREEN_BITSHIFT),
		static_cast<uint8_t>((SDL_BLUE_MASK  & c) >> SDL_BLUE_BITSHIFT),
		static_cast<uint8_t>((SDL_ALPHA_MASK & c) >> SDL_ALPHA_BITSHIFT),
	};
}

std::string color_t::to_hex_string()
{
	std::ostringstream h;

	h << std::hex << std::setfill('0') << std::setw(2) << (r & 0xFF0000)
	  << std::hex << std::setfill('0') << std::setw(2) << (g & 0x00FF00)
	  << std::hex << std::setfill('0') << std::setw(2) << (b & 0x0000FF);

	return h.str();
}
