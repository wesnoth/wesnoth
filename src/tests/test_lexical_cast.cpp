/*
   Copyright (C) 2009 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "lexical_cast.hpp"

#include <boost/test/unit_test.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/test/test_case_template.hpp>

#include <iostream>

namespace test_throw {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

#define LEXICAL_CAST_DEBUG
#include "lexical_cast.hpp"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

typedef boost::mpl::vector<
	/* note Wesnoth's coding style doesn't allow w_char so ignore them. */

	bool,

	/*
	* We don't want chars to match since a string cast of a char is
	* ambiguous; does the user want it interpreted as a char or as a number?
	* But as long as that hasn't been fixed, leave the char.
	*/
	char, signed char, unsigned char,
	short, int, long, long long,
	unsigned short, unsigned int, unsigned long, unsigned long long
	> test_match_types;

typedef boost::mpl::vector<
	float, double, long double
	> test_nomatch_types;

typedef boost::mpl::copy<
	test_nomatch_types,
	boost::mpl::back_inserter<test_match_types>
	>::type test_types;


namespace {

	std::string result;

bool validate(const char* str)
{
	if(str != result) {
		std::cerr << "Received " << str << '\n'
				<< "Expected " << result << '\n';
		return false;
	} else {
		return true;
	}
}

} // namespace

#define TEST_CASE(type_send, initializer)                           \
	{                                                               \
	type_send val = initializer value;                              \
                                                                    \
	BOOST_CHECK_EXCEPTION(                                          \
			lexical_cast<std::string>(val), const char*, validate); \
	}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_lexical_cast_throw, T, test_types)
{
	T value = T();

	typedef typename boost::mpl::contains<test_match_types, T>::type test;
	typedef typename boost::mpl::contains<test_match_types, int >::type match;

	result = typeid(test) == typeid(match)
			? "specialized - To std::string - From integral (pointer)"
			: "generic";

	TEST_CASE(T, );
	TEST_CASE(const T, );

	TEST_CASE(T&, );
	TEST_CASE(const T&, );

	TEST_CASE(T*, &);
	TEST_CASE(const T*, &);

	TEST_CASE(T* const, &);
	TEST_CASE(const T* const, &);
}

#undef TEST_CASE

typedef boost::mpl::vector<
	  signed char
	, short
	, int
	, long> test_lexical_cast_signed_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(
		test_lexical_cast_signed, T, test_lexical_cast_signed_types)
{
	result = "specialized - To signed - From (const) char*";

	const char* value = "test";
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			value), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			const_cast<char*>(value)), const char*, validate);

	result = "specialized - To signed - From std::string";

	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			std::string(value)), const char*, validate);
}

BOOST_AUTO_TEST_CASE(test_lexical_cast_long_long)
{
	result = "specialized - To long long - From (const) char*";

	const char* value = "test";
	BOOST_CHECK_EXCEPTION(lexical_cast<long long>(
			value), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<long long>(
			const_cast<char*>(value)), const char*, validate);

	result = "specialized - To long long - From std::string";

	BOOST_CHECK_EXCEPTION(lexical_cast<long long>(
			std::string(value)), const char*, validate);
}

typedef boost::mpl::vector<
	  bool
	, unsigned char
	, unsigned short
	, unsigned int
	, unsigned long> test_lexical_cast_unsigned_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(
		test_lexical_cast_unsigned, T, test_lexical_cast_unsigned_types)
{
	result = "specialized - To unsigned - From (const) char*";

	const char* value = "test";
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			value), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			const_cast<char*>(value)), const char*, validate);

	result = "specialized - To unsigned - From std::string";

	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			std::string(value)), const char*, validate);

}

BOOST_AUTO_TEST_CASE(test_lexical_cast_unsigned_long_long)
{
	result = "specialized - To unsigned long long - From (const) char*";

	const char* value = "test";
	BOOST_CHECK_EXCEPTION(lexical_cast<unsigned long long>(
			value), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<unsigned long long>(
			const_cast<char*>(value)), const char*, validate);

	result = "specialized - To unsigned long long - From std::string";

	BOOST_CHECK_EXCEPTION(lexical_cast<unsigned long long>(
			std::string(value)), const char*, validate);
}

typedef boost::mpl::vector<
	  float
	, double
	, long double> test_lexical_cast_floating_point_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(
		test_lexical_cast_floating_point, T, test_lexical_cast_floating_point_types)
{
	result = "specialized - To floating point - From (const) char*";

	const char* value = "test";
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			value), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			const_cast<char*>(value)), const char*, validate);

	result = "specialized - To floating point - From std::string";

	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
		std::string(value)), const char*, validate);
}

} //  namespace test_throw

BOOST_AUTO_TEST_CASE(test_lexical_cast_result)
{
	BOOST_CHECK_EQUAL(lexical_cast<std::string>(true), "1");
	BOOST_CHECK_EQUAL(lexical_cast<std::string>(false), "0");

	BOOST_CHECK_EQUAL(lexical_cast<std::string>(1), "1");
	BOOST_CHECK_EQUAL(lexical_cast<std::string>(1u), "1");

	BOOST_CHECK_EQUAL(lexical_cast<std::string>(1.2f), "1.2");
	BOOST_CHECK_EQUAL(lexical_cast<std::string>(1.2), "1.2");

	BOOST_CHECK_EQUAL(lexical_cast<int>("1"), 1);
	BOOST_CHECK_EQUAL(lexical_cast<int>("-1"), -1);
	BOOST_CHECK_EQUAL(lexical_cast<unsigned>("1"), 1);
	BOOST_CHECK_EQUAL(lexical_cast<double>("1.2"), 1.2);
	BOOST_CHECK_THROW(lexical_cast<double>("0x11"), bad_lexical_cast);

	std::string a = "01234567890123456789";
	BOOST_CHECK_EQUAL(lexical_cast<long long>(a), 1234567890123456789ll);
	BOOST_CHECK_THROW(lexical_cast<int>(a), bad_lexical_cast);
	BOOST_CHECK_EQUAL(lexical_cast<double>(a), 1.23456789012345678e18);
	BOOST_CHECK_EQUAL(lexical_cast_default<long long>(a, 0ll), 1234567890123456789ll);
	BOOST_CHECK_EQUAL(lexical_cast_default<int>(a, 0), 0);
	BOOST_CHECK_EQUAL(lexical_cast_default<double>(a, 0.0), 1.23456789012345678e18);

	std::string b = "99999999999999999999";
	BOOST_CHECK_THROW(lexical_cast<long long>(b), bad_lexical_cast);
	BOOST_CHECK_THROW(lexical_cast<int>(b), bad_lexical_cast);
	BOOST_CHECK_EQUAL(lexical_cast<double>(b), 1e20);
	BOOST_CHECK_EQUAL(lexical_cast_default<long long>(b, 0ll), 0ll);
	BOOST_CHECK_EQUAL(lexical_cast_default<int>(b, 0), 0);
	BOOST_CHECK_EQUAL(lexical_cast_default<double>(b, 0.0), 1e20);
}
