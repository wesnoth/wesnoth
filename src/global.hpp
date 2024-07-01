/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
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

// To allow using some optional C++20 features
#if __cplusplus >= 202002L
#define HAVE_CXX20
#endif

#if defined(__clang__)
#endif

#if defined(__GNUC__) && !defined(__clang__)
#endif

/*
 * GCC-13 and GCC-14 warn about functions that take a reference and return a
 * reference, assuming the returned reference may point into the argument, they
 * have false positives for functions that take an id string and return a
 * string. GCC-14 supports supressing the warnings with [[gnu::no_dangling]].
 *
 * Clang complains about unknown attributes in the gnu:: namespace, so has to
 * have the #if, and the #if means we need the #ifdef.
 */
#ifdef __has_cpp_attribute
#if __has_cpp_attribute(gnu::no_dangling)
#define NOT_DANGLING [[gnu::no_dangling]]
#endif
#endif
#ifndef NOT_DANGLING
#define NOT_DANGLING
#endif
