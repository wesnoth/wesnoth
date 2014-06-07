/*
   Copyright (C) 2009 - 2014 by Karol Nowak <grywacz@gmail.com>
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

#include <vector>
#include <string>
#include "serialization/string_utils.hpp"
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_SUITE ( test_serialization_utils_and_unicode )

BOOST_AUTO_TEST_CASE( utils_join_test )
{
	std::vector<std::string> fruit;

	BOOST_CHECK( utils::join(fruit) == "" );

	fruit.push_back("apples");

	BOOST_CHECK( utils::join(fruit) == "apples" );

	fruit.push_back("oranges");
	fruit.push_back("lemons");

	BOOST_CHECK( utils::join(fruit) == "apples,oranges,lemons" );
	
	utf8_string unicode = "\xC3\xBCnicod\xE2\x82\xAC check"; // "ünicod€ check" in UTF-8
	BOOST_CHECK( utils::u8size(unicode) == 13 );
	
	int euro = utils::u8index(unicode,6);
	BOOST_CHECK( unicode.substr(euro,utils::u8index(unicode,7)-euro) == "\xE2\x82\xAC" ); // € sign
	
	BOOST_CHECK( utils::u8truncate(unicode,3) == "\xC3\xBCni"); // "üni"
}

BOOST_AUTO_TEST_CASE( test_lowercase )
{
	BOOST_CHECK_EQUAL ( utils::lowercase("FOO") , "foo" );
	BOOST_CHECK_EQUAL ( utils::lowercase("foo") , "foo" );
	BOOST_CHECK_EQUAL ( utils::lowercase("FoO") , "foo" );
	BOOST_CHECK_EQUAL ( utils::lowercase("fO0") , "fo0" );
}

BOOST_AUTO_TEST_CASE( test_wildcard_string_match )
{
	const std::string str = "foo bar baz";

	BOOST_CHECK(utils::wildcard_string_match(str, "*bar*"));

	BOOST_CHECK(!utils::wildcard_string_match(str, "*BAR*"));
	BOOST_CHECK(!utils::wildcard_string_match(str, "bar"));

	BOOST_CHECK(utils::wildcard_string_match(str, "*ba? b*"));
	BOOST_CHECK(utils::wildcard_string_match(str, "*?a?*"));

	BOOST_CHECK(!utils::wildcard_string_match(str, "foo? "));
	BOOST_CHECK(!utils::wildcard_string_match(str, "?foo"));

	std::string superfluous_mask;

	superfluous_mask = std::string(str.length(), '?');
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask));
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask + '?'));

	superfluous_mask = std::string(str.length(), '*');
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask));
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask + '*'));

	BOOST_CHECK(utils::wildcard_string_match("", ""));
	BOOST_CHECK(!utils::wildcard_string_match(str, ""));

	BOOST_CHECK(utils::wildcard_string_match("", "*"));
	BOOST_CHECK(utils::wildcard_string_match("", "***?**"));
	BOOST_CHECK(!utils::wildcard_string_match("", "?"));
	BOOST_CHECK(!utils::wildcard_string_match("", "???"));
}

BOOST_AUTO_TEST_SUITE_END()
