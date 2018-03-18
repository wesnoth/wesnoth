/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include <type_traits>

namespace utils
{
//
// These aliases are part of the standard starting with C++17.
// However, MSVC includes them as of VS 2015 Update 2, and they can also be implemented
// using variable templates in C++14.
//
#ifdef HAVE_CXX17 || defined(_MSC_VER) && _MSC_VER >= 1900

using std::is_base_of_v;
using std::is_same_v;

#else

// is_base_of
template<typename Base, typename Derived>
static constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;

// is_same
template<typename T, typename U>
static constexpr bool is_same_v = std::is_same<T, U>::value;

#endif // HAVE_CXX17

} // end namespace utils
