/*
	Copyright (C) 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <charconv>
#include <optional>
#include <string_view>

namespace utils
{
template<typename T>
std::optional<T> from_chars(std::string_view str)
{
	T result {};
	const auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), result);

	if(ec == std::errc{}) {
		return result;
	} else {
		return std::nullopt;
	}
}
} // namespace utils
