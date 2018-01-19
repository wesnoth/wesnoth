/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#ifdef _MSC_VER

// Enable C99 support for VC14
#if _MSC_VER>=1900
#define STDC99
#endif

// Disable warning about source encoding not in current code page.
#pragma warning(disable: 4819)

// Disable warning about deprecated functions.
#pragma warning(disable: 4996)

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
 * pointers are not nullptr before dereferencing them, or collections are not empty
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
#define HAVE_PUT_TIME 1
// MSVC supports these starting in MSVC 2015
#if _MSC_VER >= 1900
#define HAVE_REF_QUALIFIERS 1
#define HAVE_INHERITING_CTORS 1
#define CONSTEXPR constexpr
#define NOEXCEPT noexcept
#define NORETURN [[noreturn]]
#else
#define CONSTEXPR
#define NOEXCEPT throw()
#define NORETURN __declspec(noreturn)
#endif
// MSVC supports these starting in 2017?
// Some sources claim MSVC 2015 supports them, but let's be safe...
#if _MSC_VER >= 1910
#define DEPRECATED(reason) [[deprecated(reason)]]
#if _MSVC_LANG > 201402	// fallthrough only supported when MSVC targets later than C++14
#define FALLTHROUGH [[fallthrough]]
#else
#define FALLTHROUGH
#endif
#else
#define DEPRECATED(reason) __declspec(deprecated)
#define FALLTHROUGH
#endif
#endif

#if defined(__clang__)
#include <ciso646> // To ensure standard library version macros are defined
// If it's libc++, no problem. Otherwise, attempt to detect libstdc++ version (needs GCC 5.1 or higher)
// by testing for the existence of a header added in that version.
#if defined(_LIBCPP_VERSION) || __has_include(<experimental/any>) || __has_include(<any>)
#define HAVE_PUT_TIME 1
#else
#define HAVE_PUT_TIME 0
#endif

// Clang has convenient feature detection macros \o/
#define HAVE_REF_QUALIFIERS __has_feature(cxx_reference_qualified_functions)
#define HAVE_INHERITING_CTORS __has_feature(cxx_inheriting_constructors)
// All supported versions of clang have these
#define NORETURN [[noreturn]]
#define FALLTHROUGH [[clang::fallthrough]]
// Use GCC-style attribute because the __has_cpp_attribute feature-checking macro doesn't exist in clang 3.5
#define DEPRECATED(reason) __attribute__((deprecated(reason)))

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

#if defined(__GNUC__) && !defined(__clang__)
// GCC 5 required for this
#define HAVE_PUT_TIME (__GNUC__ >= 5)
// GCC supports these from 4.8 up
#define CONSTEXPR constexpr
#define NOEXCEPT noexcept
#define NORETURN [[noreturn]]
#define HAVE_REF_QUALIFIERS 1
#define HAVE_INHERITING_CTORS 1

// Deprecated is supported from 4.9 up
#if __GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define DEPRECATED(reason) [[deprecated(reason)]]
#else
#define DEPRECATED(reason) __attribute__((deprecated(reason)))
#endif

// Fallthrough is supported from GCC 7 up
#if __GNUC__ >= 7
#define FALLTHROUGH [[fallthrough]]
#else
#define FALLTHROUGH
#endif
#endif
