/*
	Copyright (C) 2024 by the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#ifdef __cpp_lib_span
#include <span>
#else
#include <boost/core/span.hpp>
#endif

namespace utils
{
#ifdef __cpp_lib_span
using span = std::span;
#else
using span = boost::span;
#endif
} // end namespace utils
