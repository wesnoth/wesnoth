/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

// Defines the MAKE_ENUM macro,
// and companion macro MAKE_ENUM_STREAM_OPS, which also enables lexical_cast
// Currently this has 1 argument and 2 argument variety.

/**
 *
 * Example Usage:
 *
 */

/*

#include "make_enum.hpp"

namespace foo {

	MAKE_ENUM(enumname,
	        (val1, "name1")
	        (val2, "name2")
	        (val3, "name3")
	)

	MAKE_ENUM_STREAM_OPS1(enumname)
}





class bar {
	public:

	MAKE_ENUM(another,
	        (val1, "name1")
	        (val2, "name2")
	        (val3, "name3")
	)

};

MAKE_ENUM_STREAM_OPS2(bar , another)

*/

/**
 *
 * What it does:
 *
 * generates an enum foo::enumname, with functions to convert to and from string
 * const size_t foo::enumname_COUNT, which counts the number of possible values,
 *
 *
 * foo::string_to_enumname(std::string);                        //throws bad_enum_cast
 * foo::string_to_enumname(std::string, foo::enumname default); //no throw
 * foo::enumname_to_string(foo::enumname);                      //no throw
 *
 * The stream ops define
 * std::ostream & operator<< (std::ostream &, foo::enumname)    //no throw. asserts false if enum has an illegal value.
 * std::istream & operator>> (std::istream &, foo::enumname &)  //throws twml_exception including line number and arguments, IF game_config::debug is true.
 * 								//doesn't throw except in that case, and correctly sets istream state to failing always.
 *								//this is generally a recoverable exception that only shows a temporary dialog box,
 *								//and is caught at many places in the gui code. you may safely catch it and ignore it,
 *								//and then proceeding, or use a wrapper like lexical_cast_default which will assign the
 *								//default value and proceed after the dialog passes.
 *
 * In case of a bad string -> enum conversion from istream output,
 * the istream will have its fail bit set.
 * This means that lexical_casts will throw a bad_lexical_cast,
 * in the similar scenario. (but, that exception won't have any
 * details about the error.)
 *
 * It is recommended to use this type either the built-in wesnoth
 * lexical_cast or lexical_cast default.
 *
 * HOWEVER, if you DON'T want twml_exceptions to be thrown in any
 * circumstance, then use the string_to_enumname functions instead.
 *
 * To get lexical_cast, you must separately include util.hpp
 *
 *
 *
 * For example code, see src/tests/test_make_enum.cpp
 *
 **/

#ifndef MAKE_ENUM_HPP
#define MAKE_ENUM_HPP

#include "game_config.hpp"
#include "global.hpp"
#include "wml_exception.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <cassert>
#include <cstddef>
#include <exception>
#include <iostream>
#include <string>

class bad_enum_cast : public std::exception
{
public:
        bad_enum_cast(const std::string& enumname, const std::string& str)
                : message("Failed to convert String \"" + str + "\" to type " + enumname)
        {}

	virtual ~bad_enum_cast() throw() {}

        const char * what() const throw()
        {
                return message.c_str();
        }

private:
        const std::string message;
};

// Add compiler directive suppressing unused variable warning
#if defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__)
#define ATTR_UNUSED __attribute__((unused))
#else
#define ATTR_UNUSED
#endif


#define CAT2( A, B ) A ## B
#define CAT3( A, B, C ) A ## B ## C

//Clang apparently doesn't support __VA_ARGS__ with --C98 (???)
//Can try this again when we upgrade
/*
#define GET_COUNT(_1, _2, _3, _4, _5, COUNT, ...) COUNT
#define VA_SIZE(...) GET_COUNT(__VA_ARGS__, 5, 4, 3, 2, 1)

#define VA_SELECT(MNAME, ...) CAT2(MNAME, VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)
*/


#define EXPANDENUMVALUE( a, b ) a ,
#define EXPANDENUMTYPE( r, data, elem ) EXPANDENUMVALUE elem

#define EXPANDENUMFUNCTIONRETURN( a, b ) if ( str == b ) return a;
#define EXPANDENUMFUNCTION( r, data, elem ) EXPANDENUMFUNCTIONRETURN elem

#define EXPANDENUMFUNCTIONREVRETURN( a, b ) if ( val == a ) return b;
#define EXPANDENUMFUNCTIONREV( r, data, elem ) EXPANDENUMFUNCTIONREVRETURN elem

#define EXPANDENUMFUNCTIONCOUNTRETURN( a, b ) 1+
#define EXPANDENUMFUNCTIONCOUNT( r, data, elem ) EXPANDENUMFUNCTIONCOUNTRETURN elem

#define ADD_PAREN_1( A, B ) ((A, B)) ADD_PAREN_2
#define ADD_PAREN_2( A, B ) ((A, B)) ADD_PAREN_1
#define ADD_PAREN_1_END
#define ADD_PAREN_2_END
#define MAKEPAIRS( INPUT ) BOOST_PP_CAT(ADD_PAREN_1 INPUT,_END)

#define MAKEENUMTYPE( NAME, CONTENT ) \
enum NAME { \
BOOST_PP_SEQ_FOR_EACH(EXPANDENUMTYPE,  , MAKEPAIRS(CONTENT)) \
};


#define MAKEENUMCAST( NAME, PREFIX, CONTENT, COUNT_VAR ) \
PREFIX NAME ATTR_UNUSED CAT3(string_to_, NAME, _default) (const std::string& str, NAME def) \
{ \
        BOOST_PP_SEQ_FOR_EACH(EXPANDENUMFUNCTION,  , MAKEPAIRS(CONTENT)) \
        return def; \
} \
PREFIX NAME ATTR_UNUSED CAT2(string_to_,NAME) (const std::string& str) \
{ \
        BOOST_PP_SEQ_FOR_EACH(EXPANDENUMFUNCTION,  , MAKEPAIRS(CONTENT)) \
        throw bad_enum_cast( #NAME , str); \
} \
PREFIX std::string ATTR_UNUSED CAT2(NAME,_to_string) (NAME val) \
{ \
        BOOST_PP_SEQ_FOR_EACH(EXPANDENUMFUNCTIONREV,  , MAKEPAIRS(CONTENT)) \
        assert(false && "Corrupted enum found with identifier NAME"); \
        throw 42; \
} \
\
PREFIX const size_t ATTR_UNUSED COUNT_VAR  = \
BOOST_PP_SEQ_FOR_EACH(EXPANDENUMFUNCTIONCOUNT, , MAKEPAIRS(CONTENT)) \
0;


#define MAKE_ENUM( NAME, CONTENT ) \
MAKEENUMTYPE( NAME, CONTENT ) \
MAKEENUMCAST( NAME, static , CONTENT, CAT2(NAME,_COUNT) )

#define MAKE_ENUM_STREAM_OPS1( NAME ) \
inline std::ostream& operator<< (std::ostream & os, NAME val) \
{ \
	os << CAT2(NAME,_to_string) ( val ); \
	return os; \
} \
inline std::istream& operator>> (std::istream & is, NAME & val) \
{ \
	std::istream::sentry s(is, true); \
	if(!s) return is; \
	std::string temp; \
	is >> temp; \
	try { \
		val = CAT2(string_to_, NAME) ( temp ); \
	} catch (bad_enum_cast & e) { \
		is.setstate(std::ios::failbit); \
		if (!temp.empty() && game_config::debug) { \
			FAIL( e.what() ); \
		} \
	} \
	return is; \
} \


#define MAKE_ENUM_STREAM_OPS2( NAMESPACE, NAME ) \
inline std::ostream& operator<< (std::ostream & os, NAMESPACE::NAME val) \
{ \
	os << CAT2(NAMESPACE::NAME,_to_string) ( val ); \
	return os; \
} \
inline std::istream& operator>> (std::istream & is, NAMESPACE::NAME & val) \
{ \
	std::istream::sentry s(is, true); \
	if(!s) return is; \
	std::string temp; \
	is >> temp; \
	try { \
		val = CAT2(NAMESPACE::string_to_,NAME) ( temp ); \
	} catch (bad_enum_cast & e) {\
		is.setstate(std::ios::failbit); \
		if (!temp.empty() && game_config::debug) { \
			FAIL( e.what() ); \
		} \
	} \
	return is; \
} \

//Clang apparently doesn't support __VA_ARGS__ with --C98 (???)
//Can try this again when we upgrade
//#define MAKE_ENUM_STREAM_OPS VA_SELECT(MAKE_ENUM_STREAM_OPS, __VA_ARGS__)


#endif
