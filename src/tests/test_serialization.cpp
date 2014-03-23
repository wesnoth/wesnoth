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
	utf8_string apple_u8("apple");
	ucs4_string apple_u4 = utils::string_to_ucs4string(apple_u8);
	utf16_string apple_u16 = utils::ucs4string_to_utf16string(apple_u4);

	BOOST_CHECK( apple_u4.size() == 5 );
	BOOST_CHECK_EQUAL( apple_u8, utils::ucs4string_to_string(apple_u4) );
	BOOST_CHECK_EQUAL( apple_u8.size(), apple_u16.size() );

	ucs4_string water_u4;
	water_u4.push_back(0x6C34);
	utf8_string water_u8 = utils::ucs4string_to_string(water_u4);
	utf16_string water_u16 = utils::ucs4string_to_utf16string(water_u4);

	BOOST_CHECK_EQUAL(water_u4[0], water_u16[0]);
	BOOST_CHECK_EQUAL(water_u8, "\u6C34");

	utf8_string nonbmp_u8("\U00010000");
	ucs4_string nonbmp_u4 = utils::string_to_ucs4string(nonbmp_u8);
	utf16_string nonbmp_u16 = utils::ucs4string_to_utf16string(nonbmp_u4);

	BOOST_CHECK_EQUAL(nonbmp_u8.size(), 4);
	BOOST_CHECK_EQUAL(nonbmp_u4[0], 0x10000);
	BOOST_CHECK_EQUAL(nonbmp_u16[0], 0xD800);
	BOOST_CHECK_EQUAL(nonbmp_u16[1], 0xDC00);
	BOOST_CHECK_EQUAL(nonbmp_u8, utils::ucs4string_to_string(nonbmp_u4));
}

