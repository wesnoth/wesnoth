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

#undef SIGNATURE_2

/**
 * Signature for a function with two parameters.
 *
 * This version throws an exception with the typeid of the function used.
 *
 * @param res                     The result type.
 * @param name                    The name of the function.
 * @param p1                      Parameter 1, both type and variable name.
 * @param p2                      Parameter 2, both type and variable name.
 */
#define SIGNATURE_2(res, name, p1, p2) res name(p1, p2) {                      \
	static const std::type_info& type =                                        \
		typeid((res (tclass::*)(p1, p2)) &tclass::name);                       \
	throw(&type);

#else

#include <string>
#include <sstream>
#include <typeinfo>
#include <boost/type_traits.hpp>

#define SIGNATURE_2(res, name, p1, p2) res name(p1, p2) {

#endif

/**
 * @namespace implementation
 * Contains the implementation details for lexical_cast and shouldn't be used
 * directly.
 *//*
namespace implementation {

template<typename To, typename From>
struct tlexical_cast;

} // namespace implementation */

/** Thrown when a lexical_cast fails. */
struct bad_lexical_cast {};

namespace implementation {

/** Fallback if no specialized cast exists. */
template<typename To, typename From>
To lexical_cast_generic(From value)
{
	To result;
	std::stringstream sstr;

	if(!(sstr << value && sstr >> result)) {
		throw bad_lexical_cast();
	} else {
		return result;
	}
}

/**
 * Base class for the conversion.
 *
 * Since functions can't be partially specialized we use a class, which can be
 * partially specialized for the conversion.
 *
 * @tparam To                     The type to convert to.
 * @tparam From                   The type to convert from.
 */
template<typename To, typename From>
struct tlexical_cast
{
	/**
	 * The conversion operator.
	 *
	 * All (partially) specialized classes need to implement this function to
	 * do the conversion.
	 *
	 * @tparam To                 The type to convert to.
	 * @tparam From               The type to convert from.
	 * 
	 * @param value               The value to convert.
	 *
	 * @returns                   The converted value.
	 */
	To operator()(From value)
	{
		return lexical_cast_generic<To/*,
				typename boost::add_reference<
				typename boost::add_const<From>::type>::type*/>(value);
	}
};
/*
template<typename To, typename From>
struct tlexical_cast<To, From*>
{
	To operator()(From* value)
	{
		return lexical_cast_generic<To, const From*>(value);
	}
};
*/

/** Specialized class to return strings. */
template<typename From>
struct tlexical_cast<std::string, From>
{
	typedef tlexical_cast<std::string, From> tclass;

	template <class T>
	SIGNATURE_2(std::string, cast, T value, const boost::true_type&)
		std::stringstream sstr;
		sstr << value;
		return sstr.str();
	}

	template <class T>
	SIGNATURE_2(std::string, cast, T value, const boost::false_type&)

		return lexical_cast_generic<std::string, T>(value);
	}

	std::string operator()(From value)
	{
		return this->cast<From>(value, boost::is_integral<
				typename boost::remove_pointer<From>::type>());
	}
};

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

#endif

