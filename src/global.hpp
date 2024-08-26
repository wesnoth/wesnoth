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

// To allow using some optional C++20 features
#if __cplusplus >= 202002L
#define HAVE_CXX20
#endif

#if defined(__clang__)
#endif

#if defined(__GNUC__) && !defined(__clang__)
#endif

/*
 * GCC-13 and GCC-14 warn about functions that return a reference and whose arguments also include a
 * reference to a temporary, because they assume that the returned reference may point into the
 * argument. This causes false positives for functions that take a std::map-like object as a
 * separate argument (or as their "this" pointer), where the temporary being passed in is only used
 * as a key to find an object in the map, and the returned reference points to an object owned by
 * the map. Similarly, it's a false positive for data owned by a singleton.
 *
 * GCC-14 supports supressing the warnings with [[gnu::no_dangling]]. Clang complains about unknown
 * attributes in the gnu:: namespace, so has to have the #if, and the #if means we need the #ifdef.
 */
#ifdef __has_cpp_attribute

#if __has_cpp_attribute(gnu::no_dangling)
#define NOT_DANGLING [[gnu::no_dangling]]
#endif

#if __has_cpp_attribute(likely)
#define LIKELY [[likely]]
#endif

#if __has_cpp_attribute(unlikely)
#define UNLIKELY [[unlikely]]
#endif

#endif // __has_cpp_attribute

#ifndef NOT_DANGLING
#define NOT_DANGLING
#endif

#ifndef LIKELY
#define LIKELY
#endif

#ifndef UNLIKELY
#define UNLIKELY
#endif

#ifdef __cpp_aggregate_paren_init
#define AGGREGATE_EMPLACE(...) emplace_back(__VA_ARGS__)
#else
#define AGGREGATE_EMPLACE(...) push_back({ __VA_ARGS__ })
#endif
