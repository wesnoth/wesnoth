/*
	Copyright (C) 2003 - 2024
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

#include <algorithm>
#include <cctype>
#include <string>

namespace utils
{
/**
 * Equivalent to as @c std::is_same_v except both types are passed through std::decay first.
 *
 * @tparam T1    The first type to compare.
 * @tparam T2    The second type to compare.
 */
template<typename T1, typename T2>
inline constexpr bool decayed_is_same = std::is_same_v<std::decay_t<T1>, std::decay_t<T2>>;

/**
 * Workaround for the fact that static_assert(false) is invalid.
 * See https://devblogs.microsoft.com/oldnewthing/20200311-00/?p=103553
 */
template<typename>
inline constexpr bool dependent_false_v = false;

template<typename Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept
{
	return static_cast<std::underlying_type_t<Enum>>(e);
}

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

/**
 * Utility function for finding the type of thing caught with `catch(...)`.
 * Not implemented for other compilers at this time.
 *
 * @return For the GCC/clang compilers, the unmangled name of an unknown exception that was caught.
 */
std::string get_unknown_exception_type();

/**
 * Convenience wrapper for using std::remove_if on a container.
 *
 * todoc++20 use C++20's std::erase_if instead. The C++20 function returns the number of elements
 * removed; this one could do that but it seems unnecessary to add it unless something is using it.
 */
template<typename Container, typename Predicate>
void erase_if(Container& container, const Predicate& predicate)
{
	container.erase(std::remove_if(container.begin(), container.end(), predicate), container.end());
}

/**
 * Convenience wrapper for using std::remove on a container.
 *
 * @todo C++20: use std::erase
 */
template<typename Container, typename Value>
std::size_t erase(Container& container, const Value& value)
{
	auto iter = std::remove(container.begin(), container.end(), value);
	auto num_removed = container.end() - iter;
	container.erase(iter, container.end());
	return num_removed;
}

/**
 * Convenience wrapper for using std::sort on a container.
 *
 */
template<typename Container, typename Predicate>
void sort_if(Container& container, const Predicate& predicate)
{
	std::sort(container.begin(), container.end(), predicate);
}

/**
 * Convenience wrapper for using find on a container without needing to comare to end()
 *
 */
template<typename Container, typename Value>
auto* find(Container& container, const Value& value)
{
	auto res = container.find(value);
	return (res == container.end()) ? nullptr : &*res;
}

} // namespace utils
