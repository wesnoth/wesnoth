/*
 * Copyright (C) 2011 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY.
 *
 * See the COPYING file for more details.
 */

/**
 * @file
 * This file stream operators for the enum types.
 *
 * This code helps with input or outputting enumerates. For an example of its
 * usage look at @ref lib::texception::ttype.
 *
 * The file contains the declarations needed in the header and the
 * definitions needed in the implementation.
 *
 * In the header the @ref ENUM_DECLARE_STREAM_OPERATORS must be 'called'
 * after the @c enum is defined. (Not tested whether declaring the @c enum
 * also works.)
 *
 * In the implementation the
 * @ref ENUM_ENABLE_STREAM_OPERATORS_IMPLEMENTATION must be defined. Before
 * this file is included several macros must be defined:
 * - @c ENUM_TYPE must contain the type of @c enum to use.
 * - @c ENUM_LIST should be a list of @c ENUM macros, where every macro has
 *   two parameters:
 *   -# The @c enum value.
 *   -# The string representation of the @c enum value.
 *   All values of the @c enum should be in this list.
 */

#ifndef LIB_STRING_ENUMERATE_TPP_INCLUDED
#define LIB_STRING_ENUMERATE_TPP_INCLUDED

#include <iosfwd>
#include <string>

/**
 * Declares the stream operators for an enumerate type.
 *
 * The operators declared are:
 * - std::ostream& operator<<(std::ostream, T);
 * - void operator<<(std::string, T);
 * - std::istream& operator>>(std::istream, T&);
 * - void operator>>(std::string, T&);
 *
 * @pre                           std::is_enum<T>::value == true
 *
 * @param T                       The type of the enumerate.
 */
#define ENUM_DECLARE_STREAM_OPERATORS(T)                                     \
	std::ostream&                                                            \
	operator<<(std::ostream& lhs, const T rhs);                              \
                                                                             \
	void                                                                     \
	operator<<(std::string& lhs, const T rhs);                               \
                                                                             \
	std::istream&                                                            \
	operator>>(std::istream& lhs, T& rhs);                                   \
                                                                             \
	void                                                                     \
	operator>>(const std::string& lhs, T& rhs);                              \


/** This macro must be set when the operators need to be defined. */
#ifdef ENUM_ENABLE_STREAM_OPERATORS_IMPLEMENTATION

#include "gettext.hpp"
#include "wml_exception.hpp"

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_enum.hpp>

#include <cassert>
#include <iostream>

/**
 * Defines the stream operators for an enumerate type.
 *
 * This defines the operators declared with @ref ENUM_DECLARE_STREAM_OPERATORS.
 * For every 'call' of @ref ENUM_DECLARE_STREAM_OPERATORS there needs to be an
 * corrosponding 'call' to this macro.
 *
 * @param T                      The type of the enumerate.
 */
#define ENUM_DEFINE_STREAM_OPERATORS(T)                                      \
	std::ostream&                                                            \
	operator<<(std::ostream& lhs, const T rhs)                               \
	{                                                                        \
		return detail::operator<<(lhs, rhs);                                 \
	}	                                                                     \
                                                                             \
	void                                                                     \
	operator<<(std::string& lhs, const T rhs)                                \
	{                                                                        \
		detail::operator<<(lhs, rhs);                                        \
	}	                                                                     \
                                                                             \
	std::istream&                                                            \
	operator>>(std::istream& lhs, T& rhs)                                    \
	{                                                                        \
		return detail::operator>>(lhs, rhs);                                 \
	}	                                                                     \
                                                                             \
	void                                                                     \
	operator>>(const std::string& lhs, T& rhs)                               \
	{                                                                        \
		detail::operator>>(lhs, rhs);                                        \
	}	                                                                     \

namespace detail {

/**
 * Stream operator for an enumerate value.
 *
 * Contains the implementation details for an operator declared in
 * @ref ENUM_DECLARE_STREAM_OPERATORS. This version implements:
 * - std::ostream& operator<<(std::ostream, T);
 *
 * @pre                           std::is_enum<T>::value == true
 *
 * @param lhs                     The stream to write to.
 * @param rhs                     The enumerate to write.
 *
 * @returns                       The @p lhs parameter.
 */
template<class T>
typename boost::enable_if_c<boost::is_enum<T>::value, std::ostream&>::type
operator<<(std::ostream& lhs, const T rhs)
{
	std::string str;
	str << rhs;
	lhs << str;
	return lhs;
}

/**
 * Stream operator for an enumerate value.
 *
 * Contains the implementation details for an operator declared in
 * @ref ENUM_DECLARE_STREAM_OPERATORS. This version implements:
 * - void operator<<(std::string, T);
 *
 * @pre                           std::is_enum<T>::value == true
 *
 * @param lhs                     The string to write to.
 * @param rhs                     The enumerate to write.
 */
template<class T>
typename boost::enable_if_c<boost::is_enum<T>::value>::type
operator<<(std::string& lhs, const T rhs)
{
#define ENUM(ENUMERATE, STRING)    \
	case ENUM_TYPE::ENUMERATE: lhs = STRING; \
	return

	switch(rhs) {
		ENUM_LIST
	}

#undef ENUM

	assert(false);
}

/**
 * Stream operator for an enumerate value.
 *
 * Contains the implementation details for an operator declared in
 * @ref ENUM_DECLARE_STREAM_OPERATORS. This version implements:
 * - std::istream& operator>>(std::istream, T&);
 *
 * @pre                           std::is_enum<T>::value == true
 *
 * @param lhs                     The stream to read from.
 * @param rhs                     The enumerate to read.
 *
 * @returns                       The @p lhs parameter.
 */
template<class T>
typename boost::enable_if_c<boost::is_enum<T>::value, std::istream&>::type
operator>>(std::istream& lhs, T& rhs)
{
	std::string str;
	std::getline(lhs, str, '\0');
	str >> rhs;
	return lhs;
}

/**
 * Stream operator for an enumerate value.
 *
 * Contains the implementation details for an operator declared in
 * @ref ENUM_DECLARE_STREAM_OPERATORS. This version implements:
 * - void operator>>(std::string, T&);
 *
 * @pre                           std::is_enum<T>::value == true
 *
 * @param lhs                     The string to read from.
 * @param rhs                     The enumerate to read.
 */
template<class T>
typename boost::enable_if_c<boost::is_enum<T>::value>::type
operator>>(const std::string& lhs, T& rhs)
{

#define ENUM(enumerate, string)    \
	do { \
		if(lhs == string) { \
			rhs = ENUM_TYPE::enumerate; \
			return; \
		} \
	} while(0)

	ENUM_LIST

#undef ENUM

	FAIL_WITH_DEV_MESSAGE(_("Enumerate out of range during reading."), lhs);
}

} // namespace detail

#endif
#endif
