/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * New lexcical_cast header.
 *
 * For debugging you can include this header _in_ a namespace (to honor ODR)
 * and have a set of functions that throws exceptions instead of doing the
 * real job. This is done for the unit tests but should normally not be done.
 */

#ifdef LEXICAL_CAST_DEBUG
#undef LEXICAL_CAST_HPP_INCLUDED
#endif

#ifndef LEXICAL_CAST_HPP_INCLUDED
#define LEXICAL_CAST_HPP_INCLUDED

#ifdef LEXICAL_CAST_DEBUG

#undef DEBUG_THROW
/**
 * Throws an exception for debugging.
 *
 * @param id                      The unique name to identify the function.
 *                                @note this name is a user defined string and
 *                                should not be modified once used!
 */
#define DEBUG_THROW(id) throw id;
#else

#ifdef __FreeBSD__
#define __LONG_LONG_SUPPORTED
#endif

#include "global.hpp"

#include "utils/type_trait_aliases.hpp"

#include <cstdlib>
#include <limits>
#include <string>
#include <sstream>
#include <type_traits>

#include <boost/mpl/set.hpp>
#include <boost/optional.hpp>

#define DEBUG_THROW(id)
#endif

/**
 * @namespace implementation
 * Contains the implementation details for lexical_cast and shouldn't be used
 * directly.
 */
namespace implementation {

	template<
		  typename To
		, typename From
		, typename ToEnable = void
		, typename FromEnable = void
	>
	struct lexical_caster;

} // namespace implementation

/**
 * Lexical cast converts one type to another.
 *
 * @tparam To                     The type to convert to.
 * @tparam From                   The type to convert from.
 *
 * @param value                   The value to convert.
 *
 * @returns                       The converted value.
 *
 * @throw                         bad_lexical_cast if the cast was unsuccessful.
 */
template<typename To, typename From>
inline To lexical_cast(From value)
{
	return implementation::lexical_caster<To, From>().operator()(value, boost::none);
}

/**
 * Lexical cast converts one type to another with a fallback.
 *
 * @tparam To                     The type to convert to.
 * @tparam From                   The type to convert from.
 *
 * @param value                   The value to convert.
 * @param fallback                The fallback value to return if the cast fails.
 *
 * @returns                       The converted value.
 */
template<typename To, typename From>
inline To lexical_cast_default(From value, To fallback = To())
{
	return implementation::lexical_caster<To, From>().operator()(value, fallback);
}

/** Thrown when a lexical_cast fails. */
struct bad_lexical_cast : std::exception
{
	const char* what() const NOEXCEPT
	{
		return "bad_lexical_cast";
	}
};

namespace implementation {

/* Suppress -Wmaybe-uninitialized warnings.
 * I (Jyrki) noticed that building with -Og on my system (GCC 7.2.0, Boost 1.61)
 * GCC may omit a warning that the value of `fallback` may be used uninitialized.
 * It's impossible, but GCC can't prove that to itself.
 */
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

/**
 * Base class for the conversion.
 *
 * Since functions can't be partially specialized we use a class, which can be
 * partially specialized for the conversion.
 *
 * @tparam To                     The type to convert to.
 * @tparam From                   The type to convert from.
 * @tparam ToEnable               Filter to enable the To type.
 * @tparam FromEnable             Filter to enable the From type.
 */
template<
	  typename To
	, typename From
	, typename ToEnable
	, typename FromEnable
>
struct lexical_caster
{
	To operator()(From value, boost::optional<To> fallback) const
	{
		DEBUG_THROW("generic");

		To result = To();
		std::stringstream sstr;

		if(!(sstr << value && sstr >> result)) {
			if(fallback) { return fallback.get(); }

			throw bad_lexical_cast();
		} else {
			return result;
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning strings from an integral type or a pointer to an
 * integral type.
 */
template <typename From>
struct lexical_caster<
	  std::string
	, From
	, void
	, utils::enable_if_t<std::is_integral<utils::remove_pointer_t<From>>::value>
>
{
	std::string operator()(From value, boost::optional<std::string>) const
	{
		DEBUG_THROW("specialized - To std::string - From integral (pointer)");

		std::stringstream sstr;
		sstr << value;
		return sstr.str();
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a long long from a (const) char*.
 * @note is separate from the other signed types since a long long has a
 * performance penalty at 32 bit systems.
 */
template <class From>
struct lexical_caster<
	  long long
	, From
	, void
	, utils::enable_if_t<boost::mpl::has_key<boost::mpl::set<char*, const char*> , From>::value>
	>
{
	long long operator()(From value, boost::optional<long long> fallback) const
	{
		DEBUG_THROW("specialized - To long long - From (const) char*");

		if(fallback) {
			return lexical_cast_default<long long>(std::string(value), fallback.get());
		} else {
			return lexical_cast<long long>(std::string(value));
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a long long from a std::string.
 * @note is separate from the other signed types since a long long has a
 * performance penalty at 32 bit systems.
 */
template <>
struct lexical_caster<
	  long long
	, std::string
	>
{
	long long operator()(const std::string& value, boost::optional<long long> fallback) const
	{
		DEBUG_THROW("specialized - To long long - From std::string");

		try {
			return std::stoll(value);
		} catch(std::invalid_argument&) {
		} catch(std::out_of_range&) {
		}

		if(fallback) {
			return fallback.get();
		} else {
			throw bad_lexical_cast();
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a signed type from a (const) char*.
 */
template <class To, class From>
struct lexical_caster<
	  To
	, From
	, utils::enable_if_t<std::is_integral<To>::value && std::is_signed<To>::value && !std::is_same<To, long long>::value>
	, utils::enable_if_t<boost::mpl::has_key<boost::mpl::set<char*, const char*> , From>::value>
	>
{
	To operator()(From value, boost::optional<To> fallback) const
	{
		DEBUG_THROW("specialized - To signed - From (const) char*");

		if(fallback) {
			return lexical_cast_default<To>(std::string(value), fallback.get());
		} else {
			return lexical_cast<To>(std::string(value));
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a signed type from a std::string.
 */
template <class To>
struct lexical_caster<
	  To
	, std::string
	, utils::enable_if_t<std::is_integral<To>::value && std::is_signed<To>::value && !std::is_same<To, long long>::value>
	>
{
	To operator()(const std::string& value, boost::optional<To> fallback) const
	{
		DEBUG_THROW("specialized - To signed - From std::string");

		try {
			long res = std::stol(value);
			if(std::numeric_limits<To>::lowest() <= res && std::numeric_limits<To>::max() >= res) {
				return static_cast<To>(res);
			}
		} catch(std::invalid_argument&) {
		} catch(std::out_of_range&) {
		}

		if(fallback) {
			return fallback.get();
		} else {
			throw bad_lexical_cast();
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a floating point type from a (const) char*.
 */
template <class To, class From>
struct lexical_caster<
	  To
	, From
	, utils::enable_if_t<std::is_floating_point<To>::value>
	, utils::enable_if_t<boost::mpl::has_key<boost::mpl::set<char*, const char*> , From>::value>
	>
{
	To operator()(From value, boost::optional<To> fallback) const
	{
		DEBUG_THROW("specialized - To floating point - From (const) char*");

		if(fallback) {
			return lexical_cast_default<To>(std::string(value), fallback.get());
		} else {
			return lexical_cast<To>(std::string(value));
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a floating point type from a std::string.
 */
template <class To>
struct lexical_caster<
	  To
	, std::string
	, utils::enable_if_t<std::is_floating_point<To>::value>
	>
{
	To operator()(const std::string& value, boost::optional<To> fallback) const
	{
		DEBUG_THROW("specialized - To floating point - From std::string");

		// Explicitly reject hexadecimal values. Unit tests of the config class require that.
		if(value.find_first_of("Xx") != std::string::npos) {
			if(fallback) {
				return fallback.get();
			} else {
				throw bad_lexical_cast();
			}
		}

		try {
			long double res = std::stold(value);
			if((static_cast<long double>(std::numeric_limits<To>::lowest()) <= res) && (static_cast<long double>(std::numeric_limits<To>::max()) >= res)) {
				return static_cast<To>(res);
			}
		} catch(std::invalid_argument&) {
		} catch(std::out_of_range&) {
		}

		if(fallback) {
			return fallback.get();
		} else {
			throw bad_lexical_cast();
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a unsigned long long from a (const) char*.
 * @note is separate from the other unsigned types since a unsigned long long
 * has a performance penalty at 32 bit systems.
 */
template <class From>
struct lexical_caster<
	  unsigned long long
	, From
	, void
	, utils::enable_if_t<boost::mpl::has_key<boost::mpl::set<char*, const char*> , From>::value>
	>
{
	unsigned long long operator()(From value, boost::optional<unsigned long long> fallback) const
	{
		DEBUG_THROW(
				"specialized - To unsigned long long - From (const) char*");

		if(fallback) {
			return lexical_cast_default<unsigned long long>(std::string(value), fallback.get());
		} else {
			return lexical_cast<unsigned long long>(std::string(value));
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a unsigned long long from a std::string.
 * @note is separate from the other unsigned types since a unsigned long long
 * has a performance penalty at 32 bit systems.
 */
template <>
struct lexical_caster<
	  unsigned long long
	, std::string
	>
{
	unsigned long long operator()(const std::string& value, boost::optional<unsigned long long> fallback) const
	{
		DEBUG_THROW("specialized - To unsigned long long - From std::string");

		try {
			return std::stoull(value);
		} catch(std::invalid_argument&) {
		} catch(std::out_of_range&) {
		}

		if(fallback) {
			return fallback.get();
		} else {
			throw bad_lexical_cast();
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a unsigned type from a (const) char*.
 */
template <class To, class From>
struct lexical_caster<
	  To
	, From
	, utils::enable_if_t<std::is_unsigned<To>::value && !std::is_same<To, unsigned long long>::value>
	, utils::enable_if_t<boost::mpl::has_key<boost::mpl::set<char*, const char*> , From>::value>
	>
{
	To operator()(From value, boost::optional<To> fallback) const
	{
		DEBUG_THROW("specialized - To unsigned - From (const) char*");

		if(fallback) {
			return lexical_cast_default<To>(std::string(value), fallback.get());
		} else {
			return lexical_cast<To>(std::string(value));
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a unsigned type from a std::string.
 */
template <class To>
struct lexical_caster<
	  To
	, std::string
	, utils::enable_if_t<std::is_unsigned<To>::value>
	>
{
	To operator()(const std::string& value, boost::optional<To> fallback) const
	{
		DEBUG_THROW("specialized - To unsigned - From std::string");

		try {
			unsigned long res = std::stoul(value);
			// No need to check the lower bound, it's zero for all unsigned types.
			if(std::numeric_limits<To>::max() >= res) {
				return static_cast<To>(res);
			}
		} catch(std::invalid_argument&) {
		} catch(std::out_of_range&) {
		}

		if(fallback) {
			return fallback.get();
		} else {
			throw bad_lexical_cast();
		}
	}
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

} // namespace implementation

#endif

