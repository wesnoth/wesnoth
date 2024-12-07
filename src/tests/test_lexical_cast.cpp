/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "log.hpp"

#include <boost/test/unit_test.hpp>

#include <tuple>

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

using test_bool_types = std::tuple<
	/* note Wesnoth's coding style doesn't allow w_char so ignore them. */
	bool>;

using test_integral_types = std::tuple<
	/*
	* We don't want chars to match since a string cast of a char is
	* ambiguous; does the user want it interpreted as a char or as a number?
	* But as long as that hasn't been fixed, leave the char.
	*/
	char, signed char, unsigned char,
	short, int, long, long long,
	unsigned short, unsigned int, unsigned long, unsigned long long
	>;

using test_floating_point_types = std::tuple<float, double, long double>;



using test_match_types = decltype(std::tuple_cat(test_bool_types{}, test_integral_types{}));
using test_nomatch_types = decltype(std::tuple_cat(test_floating_point_types{}));
using test_types = decltype(std::tuple_cat(test_nomatch_types{}, test_match_types{}));

using test_arethmetic_types = decltype(std::tuple_cat(test_integral_types{}, test_floating_point_types{}));

namespace {

	std::string result;

bool validate(const char* str)
{
	if(str != result) {
		PLAIN_LOG << "Received " << str << '\n'
				<< "Expected " << result << '\n';
		return false;
	} else {
		return true;
	}
}

template<typename Test, typename... Types>
constexpr bool contains_type(std::tuple<Types...>)
{
	return (std::is_same_v<Test, Types> || ...);
}

} // namespace

#define TEST_CASE(type_send)                           \
	{                                                               \
	type_send val = value;                              \
                                                                    \
	BOOST_CHECK_EXCEPTION(                                          \
			lexical_cast<std::string>(val), const char*, validate); \
	}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_lexical_cast_throw, T, test_types)
{
	T value = T();

	result = "specialized - To std::string - From arithmetic";


	TEST_CASE(T);
	TEST_CASE(const T);

	TEST_CASE(T&);
	TEST_CASE(const T&);
}

#undef TEST_CASE

BOOST_AUTO_TEST_CASE_TEMPLATE(
		test_lexical_arethmetic_signed, T, test_arethmetic_types)
{
	result = "specialized - To arithmetic - From string";

	const char* value = "test";
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			value), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			const_cast<char*>(value)), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			std::string(value)), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<T>(
			std::string_view(value)), const char*, validate);
}

BOOST_AUTO_TEST_CASE(test_lexical_cast_bool)
{
	result = "specialized - To bool - From string";

	const char* value = "test";
	BOOST_CHECK_EXCEPTION(lexical_cast<bool>(
			value), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<bool>(
			const_cast<char*>(value)), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<bool>(
			std::string(value)), const char*, validate);
	BOOST_CHECK_EXCEPTION(lexical_cast<bool>(
			std::string_view(value)), const char*, validate);
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

	// The unit [effect] code uses this a lot
	BOOST_CHECK_EQUAL(lexical_cast_default<int>("80%"), 80);

	BOOST_CHECK_EQUAL(lexical_cast<double>("0x11"), 0);

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
