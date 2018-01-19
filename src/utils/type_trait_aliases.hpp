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
// These aliases are part of the standard starting with C++14.
// MSVC included them itself starting from VS2013 (our min supported version).
// However, they can't be used via alias templates in VS2013 due to lack of
// support for expression SFINAE.
// Forward to their declarations as appropriate.
//
#if defined(HAVE_CXX14) || _MSC_VER >= 1900

template<typename T>
using add_const_t = std::add_const_t<T>;
template<bool B, typename T, typename F>
using conditional_t = std::conditional_t<B, T, F>;
template<bool B, typename T = void>
using enable_if_t = std::enable_if_t<B, T>;
template<typename T>
using remove_const_t = std::remove_const_t<T>;
template<typename T>
using remove_reference_t = std::remove_reference_t<T>;
template<typename T>
using remove_pointer_t = std::remove_pointer_t<T>;

#else // We do not have C++14 or MSVC >= 2015

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

#endif // defined(HAVE_CXX14) || _MSC_VER >= 1900

// Since there's really no way to implement these without variable templates, I've commented
// this whole section out until we bump our min compiler support to VS 2015 and GCC 5.
#if 0

//
// These aliases are part of the standard starting with C++17.
// However, MSVC includes them as of VS 2015 Update 2, and they can also be implemented
// using variable templates in C++14.
//
#ifdef HAVE_CXX17 || defined(_MSC_VER) && _MSC_VER >= 1900

using std::is_base_of_v;
using std::is_same_v;

#elif defined(HAVE_CXX14)

// is_base_of
template<typename Base, typename Derived>
static constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;

// is_same
template<typename T, typename U>
static constexpr bool is_same_v = std::is_same<T, U>::value;

#endif // HAVE_CXX17

#endif

} // end namespace utils
