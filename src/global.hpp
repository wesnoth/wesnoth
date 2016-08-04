/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GLOBAL_HPP_INCLUDED
#define GLOBAL_HPP_INCLUDED

#ifdef _MSC_VER

// Enable C99 support for VC14
#if _MSC_VER>=1900
#define STDC99
#else
#undef snprintf
#define snprintf _snprintf
#endif

// Disable warning about source encoding not in current code page.
#pragma warning(disable: 4819)

// Disable warning about deprecated functions.
#pragma warning(disable: 4996)

// Disable warning when using time_t in snprintf.
#pragma warning(disable: 4477)

// Disable some MSVC warnings which are useless according to mordante
#pragma warning(disable: 4244)
#pragma warning(disable: 4345)
#pragma warning(disable: 4250)
#pragma warning(disable: 4355)
#pragma warning(disable: 4351)

#endif //_MSC_VER

#ifdef NDEBUG
/*
 * Wesnoth uses asserts to avoid undefined behaviour. For example, to make sure
 * pointers are not nullptr before deferring them, or collections are not empty
 * before accessing their elements. Therefore Wesnoth should not be compiled
 * with assertions disabled.
 */
#error "Compilation with NDEBUG defined isn't supported, Wesnoth depends on asserts."
#endif

#define UNUSED(x)  ((void)(x))     /* to avoid warnings */

// To allow using some optional C++14 features
#if __cplusplus >= 201402L
#define HAVE_CXX14
#endif

// Some C++11 features are not available on all supported platforms
#if defined(_MSC_VER)
// MSVC supports these starting in MSVC 2015
#if _MSC_VER >= 1900
#define HAVE_REF_QUALIFIERS 1
#define HAVE_INHERITING_CTORS 1
#define CONSTEXPR constexpr
#define NOEXCEPT noexcept
#else
#define CONSTEXPR
#define NOEXCEPT throw()
#endif
#endif

#if defined(__clang__)
// Clang has convenient feature detection macros \o/
#define HAVE_REF_QUALIFIERS __has_feature(cxx_reference_qualified_functions)
#define HAVE_INHERITING_CTORS __has_feature(cxx_inheriting_constructors)

#if __has_feature(cxx_constexpr)
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif

#if __has_feature(cxx_noexcept)
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif
#endif

#if defined(__GNUX__) && !defined(__clang__)
// GCC supports two of these from 4.6 up and the others from 4.8 up
#define CONSTEXPR constexpr
#define NOEXCEPT noexcept
#if __GNUC__ > 4 || (__GNU_C__ == 4 && __GNUC_MINOR__ >= 8)
#define HAVE_REF_QUALIFIERS 1
#define HAVE_INHERITING_CTORS 1
#endif
#endif

#endif //GLOBAL_HPP_INCLUDED
