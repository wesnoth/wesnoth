/*
	Copyright (C) 2022 - 2024
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

/**
 * @file
 *
 * MacOS doesn't support std::any_cast when targing MacOS < 10.14 (currently we target 10.11).
 * This provides a wrapper around the STL variant API on all platforms except MacOS, which
 * instead utilizes boost::any.
 */

#ifdef __APPLE__
#define USING_BOOST_ANY
#endif

#ifndef USING_BOOST_ANY
#include <any>
#else
#include <boost/any.hpp>
#endif

namespace utils
{
#ifndef USING_BOOST_ANY

using std::any;
using std::any_cast;

#else

using boost::any;
using boost::any_cast;

#endif
} // namespace utils
