/*
	Copyright (C) 2020 by the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#ifndef __APPLE__
#include <optional>
#else
#include <boost/optional.hpp>
#endif

namespace utils
{
#ifndef __APPLE__

using std::optional;
using std::make_optional;
using std::nullopt;
using std::nullopt_t;
using std::bad_optional_access;

#else

using boost::optional;
using boost::make_optional;
using boost::bad_optional_access;
using nullopt_t = boost::none_t;

// Create a new nullopt object equivalent to boost::none to match the STL interface
static const boost::none_t nullopt{boost::none_t::init_tag{}};

#endif

} // end namespace utils
