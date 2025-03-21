/*
	Copyright (C) 2024 - 2025
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
#include "utils/optional_fwd.hpp"
#include <string_view>

namespace utils
{
template<typename T>
utils::optional<T> from_chars(std::string_view str, int base = 10)
{
	static_assert(std::is_integral_v<T>, "Float support for charconv incomplete on current build requirements");
	T result {};
	const auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), result, base);
	return ec == std::errc{} ? utils::make_optional(result) : utils::nullopt;
}

} // namespace utils
