/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
// These aliases are part of the standard starting with C++14.
// Forward to their declarations if appropriate.
//
#ifdef HAVE_CXX14

using std::add_const_t;
using std::conditional_t;
using std::enable_if_t;
using std::remove_const_t;
using std::remove_reference_t;
using std::remove_pointer_t;

#else // We do not have C++14

// add_const
template<typename T>
using add_const_t = typename std::add_const<T>::type;

// conditional
template<bool B, typename T, typename F>
using conditional_t = typename std::conditional<B, T, F>::type;

// enable_if
template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

// remove_const
template<typename T>
using remove_const_t = typename std::remove_const<T>::type;

// remove_reference
template<typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

// remove_pointer
template<typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

#endif // HAVE_CXX14

//
// These aliases are part of the standard starting with C++17.
// However, they can also be implemented using variable templates in C++14. VS doesn't
// support variable templates until VS 2017, so fall back to the C++11 implementation
// if applicable.
//
#ifdef HAVE_CXX17

using std::is_base_of_v;
using std::is_same_v;

#elif defined(HAVE_CXX14) || defined(_MSC_VER) && _MSC_VER >= 1910 // We have C++14, and are using VC >= 2017

// is_base_of
template<typename Base, typename Derived>
static constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;

// is_same
template<typename T, typename U>
static constexpr bool is_same_v = std::is_same<T, U>::value;

#else // We do not have C++14 or C++17

// is_base_of
template<typename Base, typename Derived>
using is_base_of_v = typename std::is_base_of<Base, Derived>::value;

// is_same
template<typename T, typename U>
using is_same_v = typename std::is_same<T, U>::value;

#endif // HAVE_CXX17

} // end namespace utils
