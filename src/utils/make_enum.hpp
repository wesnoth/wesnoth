/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
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
 * Defines the MAKE_ENUM macro.
 *
 * Currently this comes in 1-argument and 2-argument versions.
 *
 * <b>Example usage:</b>
 *
 * @code
 * #include "utils/make_enum.hpp"
 *
 * MAKE_ENUM(enumname,
 *     (val1, "name1")
 *     (val2, "name2")
 *     (val3, "name3")
 * )
 * @endcode
 *
 * <b>What it does:</b>
 *
 * Generates a struct @a enumname which holds an enum and provides functions to
 * convert to and from string, as well as a constant @a enumname::count, which
 * is set to the number of possible enum values defined.
 *
 * @code
 * // Throws bad_enum_cast if the string value is not recognized.
 * enumname enumname::string_to_enum(const std::string&);
 *
 * // Returns the specified default instead of throwing if the string value is
 * // not recognized. Never throws.
 * enumname enumname::string_to_enum(const std::string&, enumname);
 *
 * // Never throws.
 * std::string enumname::enum_to_string(enumname);
 *
 * // Count of defined enum values.
 * const size_t enumname::count;
 * @endcode
 *
 * It also defines the following stream operations:
 *
 * @code
 * // Never throws. Fails an assertion check if the value passed is not defined
 * // by the enumeration.
 * std::ostream& operator<<(std::ostream&, enumname);
 *
 * // Never throws. Sets the stream's fail bit if the input is not recognized.
 * std::istream& operator>>(std::istream&, enumname&);
 * @endcode
 *
 * In case of a bad string -> enum conversion from istream input, the istream
 * will have its fail bit set. This means that lexical_casts will throw a
 * bad_lexical_cast in the similar scenario; however, note that
 * bad_lexical_cast does not provide any details about the error.
 *
 * It is recommended to use MAKE_ENUM types with the built-in versions of
 * lexical_cast or lexical_cast_default provided by Wesnoth (see lexical_cast.hpp).
 * However, if you do <b>not</b> want wml_exception to be thrown under any
 * circumstances, use the string_to_enumname functions instead.
 *
 * See src/tests/test_make_enum.cpp for example code.
 */

#pragma once

#include "global.hpp"

#include <cassert>
#include <exception>
#include <string>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor.hpp>

#include <istream>
#include <ostream>

class bad_enum_cast : public std::exception
{
public:
	bad_enum_cast(const std::string& enumname, const std::string& str)
		: message("Failed to convert string \"" + str + "\" to type " + enumname)
		, name(enumname)
		, bad_val(str)
	{}

	virtual ~bad_enum_cast() NOEXCEPT {}

	const char * what() const NOEXCEPT
	{
		return message.c_str();
	}

	const char * type() const NOEXCEPT
	{
		return name.c_str();
	}

	const char * value() const NOEXCEPT
	{
		return bad_val.c_str();
	}

private:
	std::string message, name, bad_val;
};

namespace make_enum_detail
{
	void debug_conversion_error(const std::string& tmp, const bad_enum_cast & e);
}


#define ADD_PAREN_1( A, B ) ((A, B)) ADD_PAREN_2
#define ADD_PAREN_2( A, B ) ((A, B)) ADD_PAREN_1
#define ADD_PAREN_1_END
#define ADD_PAREN_2_END
#define MAKEPAIRS( INPUT ) BOOST_PP_CAT(ADD_PAREN_1 INPUT,_END)
#define PP_SEQ_FOR_EACH_I_PAIR(macro, data, pairs) BOOST_PP_SEQ_FOR_EACH_I(macro, data, MAKEPAIRS(pairs))


#define CAT2( A, B ) A ## B
#define CAT3( A, B, C ) A ## B ## C


#define EXPAND_ENUMVALUE_NORMAL(r, data, i, record) \
    BOOST_PP_TUPLE_ELEM(2, 0, record) = i,


#define EXPAND_ENUMFUNC_NORMAL(r, data, i, record) \
    if(data == BOOST_PP_TUPLE_ELEM(2, 1, record)) return BOOST_PP_TUPLE_ELEM(2, 0, record);
#define EXPAND_ENUMPARSE_NORMAL(r, data, i, record) \
	if(data == BOOST_PP_TUPLE_ELEM(2, 1, record)) { *this = BOOST_PP_TUPLE_ELEM(2, 0, record); return true; }
#define EXPAND_ENUMFUNCREV_NORMAL(r, data, i, record) \
    if(data == BOOST_PP_TUPLE_ELEM(2, 0, record)) return BOOST_PP_TUPLE_ELEM(2, 1, record);

#define EXPAND_ENUMFUNCTIONCOUNT(r, data, i, record) \
	+ 1

class enum_tag
{
};
#define MAKE_ENUM(NAME, CONTENT) \
struct NAME : public enum_tag \
{ \
	enum type \
	{ \
		PP_SEQ_FOR_EACH_I_PAIR(EXPAND_ENUMVALUE_NORMAL, ,CONTENT) \
	}; \
	type v; \
	NAME(type v) : v(v) {} \
	/* We don't want a default constructor but we need one in order to make lexical_cast working */ \
	NAME() : v() {} \
	/* operator type() const { return v; } */\
	static NAME string_to_enum (const std::string& str, NAME def) \
	{ \
		PP_SEQ_FOR_EACH_I_PAIR(EXPAND_ENUMFUNC_NORMAL, str , CONTENT) \
		return def; \
	} \
	static NAME string_to_enum (const std::string& str) \
	{ \
		PP_SEQ_FOR_EACH_I_PAIR(EXPAND_ENUMFUNC_NORMAL, str , CONTENT) \
		throw bad_enum_cast( #NAME , str); \
	} \
	/** Sets *this to the value given in str, does nothing if str was not valid enum value. */ \
	/** @returns true iff @a str was a valid enum value. */ \
	/** @param TStr a std::string or a string_span (from the wesnothd code). */ \
	template<typename TStr> \
	bool parse (const TStr& str) \
	{ \
		PP_SEQ_FOR_EACH_I_PAIR(EXPAND_ENUMPARSE_NORMAL, str , CONTENT) \
		return false; \
	} \
	/* for const char* parameters we cannot use the template above because it would only compare the pointer. */ \
	bool parse (const char* str) \
	{ \
		return parse(std::string(str)); \
	} \
	static std::string name() \
	{ \
		return #NAME; \
	} \
	static std::string enum_to_string (NAME val) \
	{ \
		PP_SEQ_FOR_EACH_I_PAIR(EXPAND_ENUMFUNCREV_NORMAL, val.v , CONTENT) \
		assert(false && "Corrupted enum found with identifier NAME"); \
		throw "assertion ignored"; \
	} \
	static const char* enum_to_cstring (NAME val) \
	{ \
		PP_SEQ_FOR_EACH_I_PAIR(EXPAND_ENUMFUNCREV_NORMAL, val.v , CONTENT) \
		assert(false && "Corrupted enum found with identifier NAME"); \
		throw "assertion ignored"; \
	} \
	std::string to_string () const \
	{ \
		return enum_to_string(*this); \
	} \
	const char* to_cstring () const \
	{ \
		return enum_to_cstring(*this); \
	} \
	friend std::ostream& operator<< (std::ostream & os, NAME val) \
	{ \
		os << enum_to_string(val); \
		return os; \
	} \
	friend std::ostream& operator<< (std::ostream & os, NAME::type val) \
	{ \
		return (os << NAME(val)); \
	} \
	friend std::istream& operator>> (std::istream & is, NAME& val) \
	{ \
		std::istream::sentry s(is, true); \
		if(!s) return is; \
		std::string temp; \
		is >> temp; \
		try { \
			val = string_to_enum(temp); \
		} catch (const bad_enum_cast & /*e*/) {\
			is.setstate(std::ios::failbit); \
			/*make_enum_detail::debug_conversion_error(temp, e); */\
		} \
		return is; \
	} \
	friend std::istream& operator>> (std::istream & os, NAME::type& val) \
	{ \
		return (os >> reinterpret_cast< NAME &>(val)); \
	} \
	friend bool operator==(NAME v1, NAME v2) \
	{ \
		return v1.v == v2.v; \
	} \
	friend bool operator==(NAME::type v1, NAME v2) \
	{ \
		return v1 == v2.v; \
	} \
	friend bool operator==(NAME v1, NAME::type v2) \
	{ \
		return v1.v == v2; \
	} \
	friend bool operator!=(NAME v1, NAME v2) \
	{ \
		return v1.v != v2.v; \
	} \
	friend bool operator!=(NAME::type v1, NAME v2) \
	{ \
		return v1 != v2.v; \
	} \
	friend bool operator!=(NAME v1, NAME::type v2) \
	{ \
		return v1.v != v2; \
	} \
	/* operator< is used for by std::map and similar */ \
	friend bool operator<(NAME v1, NAME v2) \
	{ \
		return v1.v < v2.v; \
	} \
	template<typename T> \
	T cast() \
	{ \
		return static_cast<T>(v); \
	} \
	static NAME from_int(int i) \
	{ \
		return static_cast<type>(i); \
	} \
	static const size_t count = 0 PP_SEQ_FOR_EACH_I_PAIR(EXPAND_ENUMFUNCTIONCOUNT, , CONTENT);\
	bool valid() \
	{ \
		return cast<size_t>() < count; \
	} \
private: \
	/*prevent automatic conversion for any other built-in types such as bool, int, etc*/ \
	/*template<typename T> \
	operator T () const; */\
	/* For some unknown reason the following version doesn't compile: */ \
	/* template<typename T, typename = typename boost::enable_if<Cond>::type> \
	operator T(); */\
};
