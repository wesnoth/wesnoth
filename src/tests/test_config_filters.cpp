/*
	Copyright (C) 2023 - 2024
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

#include <boost/test/unit_test.hpp>
// Work around a Boost<1.67 bug fixed in 1.67: Include test_case.hpp after
// unit_tests.hpp. https://svn.boost.org/trac10/ticket/13387
#include <boost/test/data/test_case.hpp> // for parametrized test

#include "utils/config_filters.hpp"

using namespace utils::config_filters;

BOOST_AUTO_TEST_SUITE(config_filters)

BOOST_AUTO_TEST_CASE(test_int_signed_filter)
{
	// These can be used both as the filter and as the input to be filtered.
	config minus_3 {"x", -3};
	config plus_3 {"x", 3};

	config any_negative {"x", "-infinity--1"};
	config any_positive {"x", "1-infinity"};
	config any_number {"x", "-infinity-infinity"};

	BOOST_ASSERT(int_matches_if_present(minus_3, minus_3, "x"));
	BOOST_ASSERT(int_matches_if_present(plus_3, plus_3, "x"));

	BOOST_ASSERT(!int_matches_if_present(minus_3, plus_3, "x"));
	BOOST_ASSERT(!int_matches_if_present(plus_3, minus_3, "x"));

	BOOST_ASSERT(int_matches_if_present(any_negative, minus_3, "x"));
	BOOST_ASSERT(!int_matches_if_present(any_positive, minus_3, "x"));
	BOOST_ASSERT(int_matches_if_present(any_number, minus_3, "x"));

	BOOST_ASSERT(!int_matches_if_present(any_negative, plus_3, "x"));
	BOOST_ASSERT(int_matches_if_present(any_positive, plus_3, "x"));
	BOOST_ASSERT(int_matches_if_present(any_number, plus_3, "x"));
}

BOOST_AUTO_TEST_CASE(test_int_add_sub_filter)
{
	config add_minus_3 {"add", -3};
	config sub_3 {"sub", 3};

	BOOST_ASSERT(int_matches_if_present(add_minus_3, add_minus_3, "add"));
	BOOST_ASSERT(int_matches_if_present_or_negative(add_minus_3, add_minus_3, "add", "sub"));
	BOOST_ASSERT(int_matches_if_present(add_minus_3, add_minus_3, "some_other_key"));

	// A range, and one that only matches positive numbers
	config add_2_to_4 {"add", "2-4"};
	config sub_2_to_4 {"sub", "2-4"};

	BOOST_ASSERT(!int_matches_if_present(add_2_to_4, add_minus_3, "add"));
	BOOST_ASSERT(int_matches_if_present(add_2_to_4, add_minus_3, "some_other_key"));
	BOOST_ASSERT(!int_matches_if_present(sub_2_to_4, add_minus_3, "sub"));
	BOOST_ASSERT(int_matches_if_present_or_negative(sub_2_to_4, add_minus_3, "sub", "add"));
}

BOOST_AUTO_TEST_CASE(test_without_attribute_filter)
{
	config add_3 {"add", 3};
	config value_3 {"value", 3};

	BOOST_ASSERT(!int_matches_if_present(value_3, add_3, "value"));
	BOOST_ASSERT(int_matches_if_present(value_3, add_3, "value", 3));
	BOOST_ASSERT(!int_matches_if_present_or_negative(add_3, value_3, "add", "sub"));
}

BOOST_AUTO_TEST_SUITE_END()
