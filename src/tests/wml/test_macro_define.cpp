/*
	Copyright (C) 2020 - 2025
	by CrawlCycle <73139676+CrawlCycle@users.noreply.github.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Test if the WML preprocessor and parser together can correctly process
 * \#define and \#undef macros.
 */

#define GETTEXT_DOMAIN "wesnoth-test"

#include "tests/utils/wml_equivalence.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(feature_test_WML_macro_define)

BOOST_AUTO_TEST_CASE(macro_define_noArgument_ParseAsExpected)
{
	// Test WML macro: #define
	std::string actual = "#define Macro\n"
						 "[A][/A]\n"
						 "#enddef\n"
						 "{Macro}\n"
						 "{Macro}{Macro}\n"
						 "{Macro}{Macro}{Macro}\n";
	std::string expected = "";
	for(int i = 0; i < 6; ++i) {
		expected += "[A][/A]";
	}
	check_wml_equivalence(actual, expected);
}

BOOST_AUTO_TEST_CASE(macro_define_1Argument_ParseAsExpected)
{
	// Test WML macro: #define with 1 argument
	std::string actual = "#define MACRO\n"
						 "abc#enddef\n"
						 "a=\"{MACRO}\"\n";
	std::string expected = "a=\"abc\"";
	check_wml_equivalence(actual, expected);
}

BOOST_AUTO_TEST_SUITE_END()
