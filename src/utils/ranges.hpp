/*
	Copyright (C) 2021 - 2024
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

#ifdef __cpp_lib_ranges
#include <ranges>
#else
#include <boost/range/adaptor/reversed.hpp>
#endif

namespace utils
{
template<typename T>
inline auto reversed_view(T& container)
{
#ifdef __cpp_lib_ranges
	return std::ranges::reverse_view(container);
#else
	return boost::adaptors::reverse(container);
#endif
}

} // namespace utils
