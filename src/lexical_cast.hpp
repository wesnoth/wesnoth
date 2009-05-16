/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file lexical_cast.hpp
 * New lexcical_cast header.
 *
 * For debugging you can include this header _in_ a namespace (to honour ODR)
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

#include <string>
#include <sstream>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

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
struct bad_lexical_cast {};

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
 * intergral type.
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

} // namespace implementation

#endif

