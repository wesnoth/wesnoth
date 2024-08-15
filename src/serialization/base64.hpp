/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace utils
{
using byte_string_view = std::basic_string_view<uint8_t>;
}

// Official Base64 encoding (RFC4648)
namespace base64 {
	std::vector<uint8_t> decode(std::string_view encoded);
	std::string encode(utils::byte_string_view bytes);
}
// crypt()-compatible radix-64 encoding
namespace crypt64 {
	std::vector<uint8_t> decode(std::string_view encoded);
	std::string encode(utils::byte_string_view bytes);
	// Single character functions. For special use only
	int decode(char encoded_char);
	char encode(int value);
}
