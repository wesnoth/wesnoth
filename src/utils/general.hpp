/*
   Copyright (C) 2003 - 2018 the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include <algorithm>
#include <cctype>

inline bool chars_equal_insensitive(char a, char b) { return tolower(a) == tolower(b); }
inline bool chars_less_insensitive(char a, char b) { return tolower(a) < tolower(b); }

namespace utils {

#ifdef HAVE_CXX17
using std::clamp;
#else
// NOTE: remove once we have C++17 support and can use std::clamp
template<typename T>
CONSTEXPR const T& clamp(const T& value, const T& min, const T& max)
{
	return std::max<T>(std::min<T>(value, max), min);
}
#endif

namespace detail
{
/**
 * A struct that exists to implement a generic wrapper for std::find.
 * Container should "look like" an STL container of Values.
 */
template<typename Container, typename Value>
struct contains_impl
{
	static bool eval(const Container& container, const Value& value)
	{
		typename Container::const_iterator end = container.end();
		return std::find(container.begin(), end, value) != end;
	}
};

/**
 * A struct that exists to implement a generic wrapper for the find()
 * member of associative containers.
 * Container should "look like" an STL associative container.
 */
template<typename Container>
struct contains_impl<Container, typename Container::key_type>
{
	static bool eval(const Container& container, const typename Container::key_type& value)
	{
		return container.find(value) != container.end();
	}
};

} // namespace detail

/**
 * Returns true iff @a value is found in @a container.
 *
 * This should work whenever Container "looks like" an STL container of Values.
 * Normally this uses std::find(), but a simulated partial template specialization
 * exists when Value is Container::key_type. In this case, Container is assumed
 * an associative container, and the member function find() is used.
 */
template<typename Container, typename Value>
inline bool contains(const Container& container, const Value& value)
{
	return detail::contains_impl<Container, Value>::eval(container, value);
}

} // namespace utils
