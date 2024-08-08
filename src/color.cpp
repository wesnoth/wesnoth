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

#include "color.hpp"

#include "serialization/string_utils.hpp"
#include "utils/from_chars.hpp"

#include <iomanip>
#include <sstream>

color_t color_t::from_rgba_string(std::string_view c)
{
	if(c.empty()) {
		return null_color();
	}

	std::vector<std::string_view> fields = utils::split_view(c);

	// Allow either 3 (automatic opaque alpha) or 4 (explicit alpha) fields
	if(fields.size() != 3 && fields.size() != 4) {
		throw std::invalid_argument("Wrong number of components for RGBA color");
	}

	return {
		*utils::from_chars<uint8_t>(fields[0]),
		*utils::from_chars<uint8_t>(fields[1]),
		*utils::from_chars<uint8_t>(fields[2]),
		fields.size() == 4 ? *utils::from_chars<uint8_t>(fields[3]) : ALPHA_OPAQUE
	};
}

color_t color_t::from_rgb_string(std::string_view c)
{
	if(c.empty()) {
		return null_color();
	}

	std::vector<std::string_view> fields = utils::split_view(c);

	if(fields.size() != 3) {
		throw std::invalid_argument("Wrong number of components for RGB color");
	}

	return {
		*utils::from_chars<uint8_t>(fields[0]),
		*utils::from_chars<uint8_t>(fields[1]),
		*utils::from_chars<uint8_t>(fields[2]),
		ALPHA_OPAQUE
	};
}

color_t color_t::from_hex_string(std::string_view c)
{
	if(c[0] == '#') {
		c = c.substr(1);
	}

	if(c.length() != 6) {
		throw std::invalid_argument("Color hex string should be exactly 6 digits (leading '#' optional)");
	}

	if(std::any_of(c.begin(), c.end(), [](const char& ch) { return std::isxdigit(ch) == 0; })) {
		throw std::invalid_argument("Color hex string contains invalid characters");
	}

	auto temp_c = *utils::from_chars<uint32_t>(c, 16);

	return {
		static_cast<uint8_t>((0x00FFFFFF & temp_c) >> 16),
		static_cast<uint8_t>((0x00FFFFFF & temp_c) >> 8),
		static_cast<uint8_t>((0x00FFFFFF & temp_c)),
		ALPHA_OPAQUE
	};
}

std::string color_t::to_hex_string() const
{
	std::ostringstream h;

	h << "#"
	  << std::hex << std::setfill('0')
	  << std::setw(2) << static_cast<int>(r)
	  << std::setw(2) << static_cast<int>(g)
	  << std::setw(2) << static_cast<int>(b);

	if(a != ALPHA_OPAQUE) {
		h << std::setw(2) << static_cast<int>(a);
	}

	return h.str();
}

std::string color_t::to_rgba_string() const
{
	std::ostringstream color;

	color << static_cast<int>(r) << ','
	      << static_cast<int>(g) << ','
	      << static_cast<int>(b) << ','
	      << static_cast<int>(a);

	return color.str();
}

std::string color_t::to_rgb_string() const
{
	std::ostringstream color;

	color << static_cast<int>(r) << ','
	      << static_cast<int>(g) << ','
	      << static_cast<int>(b);

	return color.str();
}
