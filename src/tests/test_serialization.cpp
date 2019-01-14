/*
   Copyright (C) 2009 - 2018 by Karol Nowak <grywacz@gmail.com>
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

#include <vector>
#include <string>
#include "serialization/base64.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include <boost/test/auto_unit_test.hpp>

BOOST_AUTO_TEST_SUITE ( test_serialization_utils_and_unicode )

BOOST_AUTO_TEST_CASE( utils_join_test )
{
	std::vector<std::string> fruit;

	BOOST_CHECK( utils::join(fruit).empty() );

	fruit.push_back("apples");

	BOOST_CHECK( utils::join(fruit) == "apples" );

	fruit.push_back("oranges");
	fruit.push_back("lemons");

	BOOST_CHECK( utils::join(fruit) == "apples,oranges,lemons" );
}

BOOST_AUTO_TEST_CASE( utils_unicode_test )
{
	std::string unicode = "ünicod€ check";
	BOOST_CHECK( utf8::size(unicode) == 13 );

	int euro = utf8::index(unicode,6);
	BOOST_CHECK( unicode.substr(euro,utf8::index(unicode,7)-euro) == "€" );

	BOOST_CHECK( utf8::truncate(unicode,3) == "üni");

	std::string apple_u8("apple");
	std::u32string apple_u4 = unicode_cast<std::u32string>(apple_u8);
	std::u16string apple_u16 = unicode_cast<std::u16string>(apple_u4);

	BOOST_CHECK( apple_u4.size() == 5 );
	BOOST_CHECK_EQUAL( apple_u8, unicode_cast<std::string>(apple_u4) );
	BOOST_CHECK_EQUAL( apple_u8, unicode_cast<std::string>(apple_u16) );
	BOOST_CHECK( apple_u4 == unicode_cast<std::u32string>(apple_u16) );
	BOOST_CHECK( apple_u16 == unicode_cast<std::u16string>(apple_u4) );
	BOOST_CHECK_EQUAL( apple_u8.size(), apple_u16.size() );

	std::u32string water_u4;
	water_u4.push_back(0x6C34);
	std::string water_u8 = unicode_cast<std::string>(water_u4);
	std::u16string water_u16 = unicode_cast<std::u16string>(water_u4);

	BOOST_CHECK_EQUAL(water_u4[0], static_cast<char32_t>(water_u16[0]));
#if defined(_WIN32) || defined(_WIN64)
	// Windows complains it can't be represented in the currentl code-page.
	// So instead, check directly for its UTF-8 representation.
	BOOST_CHECK_EQUAL(water_u8, "\xE6\xB0\xB4");
#else
	BOOST_CHECK_EQUAL(water_u8, "\u6C34");
#endif

#if defined(_WIN32) || defined(_WIN64)
	// Same as above.
	std::string nonbmp_u8("\xF0\x90\x80\x80");
#else
	std::string nonbmp_u8("\U00010000");
#endif
	std::u32string nonbmp_u4 = unicode_cast<std::u32string>(nonbmp_u8);
	std::u16string nonbmp_u16 = unicode_cast<std::u16string>(nonbmp_u4);

	BOOST_CHECK_EQUAL(nonbmp_u8.size(), 4u);
	BOOST_CHECK_EQUAL(nonbmp_u4[0], 0x10000u);
	BOOST_CHECK_EQUAL(nonbmp_u16[0], 0xD800);
	BOOST_CHECK_EQUAL(nonbmp_u16[1], 0xDC00);
	BOOST_CHECK_EQUAL(nonbmp_u8, unicode_cast<std::string>(nonbmp_u4));
	BOOST_CHECK_EQUAL(nonbmp_u8, unicode_cast<std::string>(nonbmp_u16));
	BOOST_CHECK(nonbmp_u16 == unicode_cast<std::u16string>(nonbmp_u4));
	BOOST_CHECK(nonbmp_u4 == unicode_cast<std::u32string>(nonbmp_u16));
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
	BOOST_CHECK(utils::wildcard_string_match(str, "+bar+"));

	BOOST_CHECK(!utils::wildcard_string_match(str, "*BAR*"));
	BOOST_CHECK(!utils::wildcard_string_match(str, "+BAR+"));
	BOOST_CHECK(!utils::wildcard_string_match(str, "bar"));

	BOOST_CHECK(utils::wildcard_string_match(str, "*ba? b*"));
	BOOST_CHECK(utils::wildcard_string_match(str, "+ba? b+"));
	BOOST_CHECK(utils::wildcard_string_match(str, "*?a?*"));
	BOOST_CHECK(utils::wildcard_string_match(str, "+?a?+"));

	BOOST_CHECK(!utils::wildcard_string_match(str, "foo? "));
	BOOST_CHECK(!utils::wildcard_string_match(str, "?foo"));

	std::string superfluous_mask;

	superfluous_mask = std::string(str.length(), '?');
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask));
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask + '?'));

	superfluous_mask = std::string(str.length(), '*');
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask));
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask + '*'));

	superfluous_mask = std::string(str.length(), '+');
	BOOST_CHECK(utils::wildcard_string_match(str, superfluous_mask));
	BOOST_CHECK(!utils::wildcard_string_match(str, superfluous_mask + '+'));

	BOOST_CHECK(utils::wildcard_string_match("", ""));
	BOOST_CHECK(!utils::wildcard_string_match(str, ""));

	BOOST_CHECK(utils::wildcard_string_match("", "*"));
	BOOST_CHECK(utils::wildcard_string_match("", "***"));
	BOOST_CHECK(!utils::wildcard_string_match("", "+"));
	BOOST_CHECK(!utils::wildcard_string_match("", "*bar"));
	BOOST_CHECK(!utils::wildcard_string_match("", "***?**"));
	BOOST_CHECK(!utils::wildcard_string_match("", "+++?++"));
	BOOST_CHECK(!utils::wildcard_string_match("", "?"));
	BOOST_CHECK(!utils::wildcard_string_match("", "???"));
}

BOOST_AUTO_TEST_CASE( test_base64_encodings )
{
	const std::vector<uint8_t> empty;
	const std::string empty_b64;
	const std::string empty_c64;
	const std::vector<uint8_t> foo = {'f', 'o', 'o'};
	const std::string foo_b64 = "Zm9v";
	const std::string foo_c64 = "axqP";
	const std::vector<uint8_t> foob = {'f', 'o', 'o', 'b'};
	const std::string foob_b64 = "Zm9vYg==";
	const std::string foob_c64 = "axqPW/";

	std::vector<uint8_t> many_bytes;

	many_bytes.resize(1024);
	for(int i = 0; i < 1024; ++i) {
		many_bytes[i] = i % 256;
	}

	BOOST_CHECK(base64::encode({empty.data(), empty.size()}).empty());
	BOOST_CHECK_EQUAL(base64::encode({foo.data(), foo.size()}), foo_b64);
	BOOST_CHECK_EQUAL(base64::encode({foob.data(), foob.size()}), foob_b64);

	BOOST_CHECK(base64::decode(empty_b64).empty());
	// Not using CHECK_EQUAL because vector<uint8_t> is not printable
	BOOST_CHECK(base64::decode(foo_b64) == foo);
	BOOST_CHECK(base64::decode(foob_b64) == foob);

	BOOST_CHECK(crypt64::encode({empty.data(), empty.size()}).empty());
	BOOST_CHECK_EQUAL(crypt64::encode({foo.data(), foo.size()}), foo_c64);
	BOOST_CHECK_EQUAL(crypt64::encode({foob.data(), foob.size()}), foob_c64);

	BOOST_CHECK(crypt64::decode(empty_c64).empty());
	// Not using CHECK_EQUAL because vector<uint8_t> is not printable
	BOOST_CHECK(crypt64::decode(foo_c64) == foo);
	BOOST_CHECK(crypt64::decode(foob_c64) == foob);

	BOOST_CHECK_EQUAL(crypt64::decode('.'), 0);
	BOOST_CHECK_EQUAL(crypt64::decode('z'), 63);
	BOOST_CHECK_EQUAL(crypt64::encode(0), '.');
	BOOST_CHECK_EQUAL(crypt64::encode(63), 'z');

	BOOST_CHECK(base64::decode(base64::encode({many_bytes.data(), many_bytes.size()})) == many_bytes);
	BOOST_CHECK(crypt64::decode(crypt64::encode({many_bytes.data(), many_bytes.size()})) == many_bytes);
}

BOOST_AUTO_TEST_SUITE_END()
