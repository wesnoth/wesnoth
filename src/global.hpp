/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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

#ifdef _MSC_VER

// Enable C99 support for lua
#define STDC99

#if _MSVC_LANG > 201402	// fallthrough only supported when MSVC targets later than C++14
#define FALLTHROUGH [[fallthrough]]
#else
#define FALLTHROUGH
#endif

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

// To allow using some optional C++17 features
#if __cplusplus >= 201703L
#define HAVE_CXX17
#endif

#if defined(__clang__)

// All supported versions of clang have these
#define FALLTHROUGH [[clang::fallthrough]]

#endif

#if defined(__GNUC__) && !defined(__clang__)
// Fallthrough is supported from GCC 7 up
#if __GNUC__ >= 7
#define FALLTHROUGH [[fallthrough]]
#else
#define FALLTHROUGH
#endif
#endif
