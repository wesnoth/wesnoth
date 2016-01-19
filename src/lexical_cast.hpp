/*
   Copyright (C) 2009 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#ifdef _MSC_VER
#define strtoll _strtoi64
#define strtoull _strtoui64
#endif

#include "global.hpp"

#include <cstdlib>
#include <string>
#include <sstream>
#include <boost/mpl/set.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

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
	struct tlexical_cast;

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
 */
template<typename To, typename From>
inline To lexical_cast(From value)
{
	return implementation::tlexical_cast<To, From>().operator()(value);
}

/** Thrown when a lexical_cast fails. */
struct bad_lexical_cast : std::exception {};

namespace implementation {

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
struct tlexical_cast
{
	To operator()(From value)
	{
		DEBUG_THROW("generic");

		To result;
		std::stringstream sstr;

		if(!(sstr << value && sstr >> result)) {
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
struct tlexical_cast<
	  std::string
	, From
	, void
	, typename boost::enable_if<boost::is_integral<
			typename boost::remove_pointer<From>::type> >::type
>
{
	std::string operator()(From value)
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
struct tlexical_cast<
	  long long
	, From
	, void
	, typename boost::enable_if<boost::mpl::has_key<boost::mpl::set<
			char*, const char*> , From> >::type
	>
{
	long long operator()(From value)
	{
		DEBUG_THROW("specialized - To long long - From (const) char*");

		char* endptr;
		int res = strtoll(value, &endptr, 10);

		if (*value == '\0' || *endptr != '\0') {
			throw bad_lexical_cast();
		} else {
			return res;
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
struct tlexical_cast<
	  long long
	, std::string
	>
{
	long long operator()(const std::string& value)
	{
		DEBUG_THROW("specialized - To long long - From std::string");

		return lexical_cast<long long>(value.c_str());
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a signed type from a (const) char*.
 */
template <class To, class From>
struct tlexical_cast<
	  To
	, From
	, typename boost::enable_if<boost::is_signed<To> >::type
	, typename boost::enable_if<boost::mpl::has_key<boost::mpl::set<
			char*, const char*> , From> >::type
	>
{
	To operator()(From value)
	{
		DEBUG_THROW("specialized - To signed - From (const) char*");

		char* endptr;
		int res = strtol(value, &endptr, 10);

		if (*value == '\0' || *endptr != '\0') {
			throw bad_lexical_cast();
		} else {
			return res;
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a signed type from a std::string.
 */
template <class To>
struct tlexical_cast<
	  To
	, std::string
	, typename boost::enable_if<boost::is_signed<To> >::type
	>
{
	To operator()(const std::string& value)
	{
		DEBUG_THROW("specialized - To signed - From std::string");

		return lexical_cast<To>(value.c_str());
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
struct tlexical_cast<
	  unsigned long long
	, From
	, void
	, typename boost::enable_if<boost::mpl::has_key<boost::mpl::set<
			char*, const char*> , From> >::type
	>
{
	long long operator()(From value)
	{
		DEBUG_THROW(
				"specialized - To unsigned long long - From (const) char*");

		char* endptr;
		int res = strtoull(value, &endptr, 10);

		if (*value == '\0' || *endptr != '\0') {
			throw bad_lexical_cast();
		} else {
			return res;
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
struct tlexical_cast<
	  unsigned long long
	, std::string
	>
{
	long long operator()(const std::string& value)
	{
		DEBUG_THROW("specialized - To unsigned long long - From std::string");

		return lexical_cast<unsigned long long>(value.c_str());
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a unsigned type from a (const) char*.
 */
template <class To, class From>
struct tlexical_cast<
	  To
	, From
	, typename boost::enable_if<boost::is_unsigned<To> >::type
	, typename boost::enable_if<boost::mpl::has_key<boost::mpl::set<
			char*, const char*> , From> >::type
	>
{
	To operator()(From value)
	{
		DEBUG_THROW("specialized - To unsigned - From (const) char*");

		char* endptr;
		int res = strtoul(value, &endptr, 10);

		if (*value == '\0' || *endptr != '\0') {
			throw bad_lexical_cast();
		} else {
			return res;
		}
	}
};

/**
 * Specialized conversion class.
 *
 * Specialized for returning a unsigned type from a std::string.
 */
template <class To>
struct tlexical_cast<
	  To
	, std::string
	, typename boost::enable_if<boost::is_unsigned<To> >::type
	>
{
	To operator()(const std::string& value)
	{
		DEBUG_THROW("specialized - To unsigned - From std::string");

		return lexical_cast<To>(value.c_str());
	}
};

} // namespace implementation

#endif

