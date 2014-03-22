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
	
	utf8::string unicode = "ünicod€ check";
	BOOST_CHECK( utf8::size(unicode) == 13 );
	
	int euro = utf8::index(unicode,6);
	BOOST_CHECK( str.substr(euro,utf8::index(unicode,7)-euro) == "€" );
	
	BOOST_CHECK( utf8::truncate(unicode,3) == "üni");
}

