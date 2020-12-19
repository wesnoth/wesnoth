/*
   Copyright (c) Marshall Clow 2012-2015.
   Copyright (c) Beman Dawes 2015
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/* This file is the Boost string_view implementation in a single header file.
We have an in-tree copy because not all versions of Boost we support have
that class. */

#pragma once

#include <cstdint>

#include <boost/utility/string_view.hpp>

namespace utils
{
using string_view = boost::string_view;
using byte_string_view = boost::basic_string_view<uint8_t, std::char_traits<uint8_t>>;
} // namespace utils
