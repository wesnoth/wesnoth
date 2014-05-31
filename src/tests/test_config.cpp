/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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

#include <boost/test/unit_test.hpp>

#include "config.hpp"

BOOST_AUTO_TEST_SUITE ( test_config )

BOOST_AUTO_TEST_CASE ( test_config_attribute_value )
{
	config c;
	int x_int;
	std::string x_str;
	long long x_sll;
	double x_dbl;

	c["x"] = 1;
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "1");
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 1);
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 1ll);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 1.0);


	c["x"] = 10000000;
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 10000000);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "10000000");
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 10000000ll);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 1e7);

	c["x"] = "";
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "");
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 0);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 0.0);


	c["x"] = "0x11";
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 0);
	x_str = c["x"].str();
		BOOST_CHECK_EQUAL(x_str, "0x11");
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 0.0);


	c["x"] = "0xab";
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 0);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "0xab");
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 0.0);


	c["x"] = "00001111";
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 1111);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "00001111");
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 1111ll);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 1.111e3);


	c["x"] = "000000";
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 0);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str,"000000");
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 0.0);


	c["x"] = "01234567890123456789";
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll,1234567890123456789ll);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str,"01234567890123456789");
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 0);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 1.23456789012345678e18);


	c["x"] = "99999999999999999999";
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "99999999999999999999");
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 0);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 1e20);

	c["x"] = 1.499;
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 1ll);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "1.499");
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, 1);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK(abs(x_dbl - 1.499) < 1e-6);


	c["x"] = 123456789123ll;
	x_int = c["x"].to_int(); 
		BOOST_CHECK_EQUAL(x_int, -1097262461);
	x_dbl = c["x"].to_double(); 
		BOOST_CHECK_EQUAL(x_dbl, 1.23456789123e11);
	x_sll = c["x"].to_long_long(); 
		BOOST_CHECK_EQUAL(x_sll, 123456789123ll);
	x_str = c["x"].str(); 
		BOOST_CHECK_EQUAL(x_str, "123456789123");
}

BOOST_AUTO_TEST_SUITE_END()
