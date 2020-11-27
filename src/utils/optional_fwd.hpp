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

#include "global.hpp"

#ifdef HAVE_CXX17
#include <optional>
#else
#include <boost/optional.hpp>
#endif

namespace utils
{
#ifdef HAVE_CXX17

using std::optional;
using std::nullopt;

#else

using boost::optional;

// Create a new nullopt object equivalent to boost::none to match the STL interface
static const boost::none_t nullopt{boost::none_t::init_tag{}};

#endif
} // end namespace utils
