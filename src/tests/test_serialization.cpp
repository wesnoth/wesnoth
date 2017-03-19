/*
   Copyright (C) 2009 - 2017 by Karol Nowak <grywacz@gmail.com>
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
#include "serialization/unicode.hpp"
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
}

BOOST_AUTO_TEST_CASE( utils_unicode_test )
{
	utf8::string unicode = "ünicod€ check";
	BOOST_CHECK( utf8::size(unicode) == 13 );

	int euro = utf8::index(unicode,6);
	BOOST_CHECK( unicode.substr(euro,utf8::index(unicode,7)-euro) == "€" );

	BOOST_CHECK( utf8::truncate(unicode,3) == "üni");

	utf8::string apple_u8("apple");
	ucs4::string apple_u4 = unicode_cast<ucs4::string>(apple_u8);
	utf16::string apple_u16 = unicode_cast<utf16::string>(apple_u4);

	BOOST_CHECK( apple_u4.size() == 5 );
	BOOST_CHECK_EQUAL( apple_u8, unicode_cast<utf8::string>(apple_u4) );
	BOOST_CHECK_EQUAL( apple_u8, unicode_cast<utf8::string>(apple_u16) );
	BOOST_CHECK( apple_u4 == unicode_cast<ucs4::string>(apple_u16) );
	BOOST_CHECK( apple_u16 == unicode_cast<utf16::string>(apple_u4) );
	BOOST_CHECK_EQUAL( apple_u8.size(), apple_u16.size() );

	ucs4::string water_u4;
	water_u4.push_back(0x6C34);
	utf8::string water_u8 = unicode_cast<utf8::string>(water_u4);
	utf16::string water_u16 = unicode_cast<utf16::string>(water_u4);

	BOOST_CHECK_EQUAL(water_u4[0], water_u16[0]);
	BOOST_CHECK_EQUAL(water_u8, "\u6C34");

	utf8::string nonbmp_u8("\U00010000");
	ucs4::string nonbmp_u4 = unicode_cast<ucs4::string>(nonbmp_u8);
	utf16::string nonbmp_u16 = unicode_cast<utf16::string>(nonbmp_u4);

	BOOST_CHECK_EQUAL(nonbmp_u8.size(), 4);
	BOOST_CHECK_EQUAL(nonbmp_u4[0], 0x10000);
	BOOST_CHECK_EQUAL(nonbmp_u16[0], 0xD800);
	BOOST_CHECK_EQUAL(nonbmp_u16[1], 0xDC00);
	BOOST_CHECK_EQUAL(nonbmp_u8, unicode_cast<utf8::string>(nonbmp_u4));
	BOOST_CHECK_EQUAL(nonbmp_u8, unicode_cast<utf8::string>(nonbmp_u16));
	BOOST_CHECK(nonbmp_u16 == unicode_cast<utf16::string>(nonbmp_u4));
	BOOST_CHECK(nonbmp_u4 == unicode_cast<ucs4::string>(nonbmp_u16));
}

BOOST_AUTO_TEST_CASE( test_lowercase )
{
	BOOST_CHECK_EQUAL ( utf8::lowercase("FOO") , "foo" );
	BOOST_CHECK_EQUAL ( utf8::lowercase("foo") , "foo" );
	BOOST_CHECK_EQUAL ( utf8::lowercase("FoO") , "foo" );
	BOOST_CHECK_EQUAL ( utf8::lowercase("fO0") , "fo0" );
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
