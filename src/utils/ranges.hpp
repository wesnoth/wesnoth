/*
	Copyright (C) 2021 - 2025
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
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/transformed.hpp>
#endif

#ifndef __cpp_lib_ranges_stride
#include <boost/range/adaptor/strided.hpp>
#endif

namespace utils::views
{
#ifdef __cpp_lib_ranges

using std::views::filter;
using std::views::keys;
using std::views::reverse;
using std::views::transform;
using std::views::values;

#else

constexpr auto filter    = boost::adaptors::filtered;
constexpr auto keys      = boost::adaptors::map_keys;
constexpr auto reverse   = boost::adaptors::reversed;
constexpr auto transform = boost::adaptors::transformed;
constexpr auto values    = boost::adaptors::map_values;

#endif

//
// Ranges introduced in C++23
//

#ifdef __cpp_lib_ranges_stride
using std::views::stride;
#else
constexpr auto stride = boost::adaptors::strided;
#endif

} // namespace utils::views
