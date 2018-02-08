/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
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
#include <cmath>

#include "config.hpp"
#include "variable_info.hpp"

BOOST_AUTO_TEST_SUITE ( test_config )

BOOST_AUTO_TEST_CASE ( test_config_attribute_value )
{
	config c;
	const config& cc = c;
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
		BOOST_CHECK(std::abs(x_dbl - 1.499) < 1e-6);


	c["x"] = 123456789123ll;
	x_int = c["x"].to_int();
		BOOST_CHECK_EQUAL(x_int, -1097262461);
	x_dbl = c["x"].to_double();
		BOOST_CHECK_EQUAL(x_dbl, 1.23456789123e11);
	x_sll = c["x"].to_long_long();
		BOOST_CHECK_EQUAL(x_sll, 123456789123ll);
	x_str = c["x"].str();
		BOOST_CHECK_EQUAL(x_str, "123456789123");

	// blank != "" test.
	c.clear();
	BOOST_CHECK(cc["x"] != "");
	BOOST_CHECK(cc["x"].empty());
	BOOST_CHECK(cc["x"].blank());

	BOOST_CHECK(c["x"] != "");
	BOOST_CHECK(c["x"].empty());
	BOOST_CHECK(c["x"].blank());

	BOOST_CHECK_EQUAL(cc["x"], c["x"]);

	c["x"] = "";
	BOOST_CHECK(cc["x"].empty());
	BOOST_CHECK(cc["x"].empty());
	BOOST_CHECK(!cc["x"].blank());

	BOOST_CHECK(c["x"].empty());
	BOOST_CHECK(c["x"].empty());
	BOOST_CHECK(!c["x"].blank());

	BOOST_CHECK_EQUAL(cc["x"], c["x"]);
}

BOOST_AUTO_TEST_CASE ( test_variable_info )
{
	config c;
	{
		variable_access_const access("", c);
		// We dotn allow empty keys
		BOOST_CHECK_THROW (access.as_scalar(), invalid_variablename_exception);
	}
	{
		variable_access_const access("some_non_existent.", c);
		// We dotn allow empty keys
		BOOST_CHECK_THROW (access.as_scalar(), invalid_variablename_exception);
	}
	{
		variable_access_const access("some_non_existent[0]value", c);
		// We expect '.' after ']'
		BOOST_CHECK_THROW (access.as_scalar(), invalid_variablename_exception);
	}
	{
		variable_access_const access("some_non_existent", c);
		// we return empty be default
		BOOST_CHECK (!access.exists_as_container());
		BOOST_CHECK_EQUAL (access.as_container(), config());
		BOOST_CHECK (!access.exists_as_attribute());
		BOOST_CHECK_EQUAL (access.as_scalar(), config::attribute_value());
	}
	{
		variable_access_const access("a.b[0].c[1].d.e.f[2]", c);
		// we return empty be default
		BOOST_CHECK (!access.exists_as_container());
		BOOST_CHECK_EQUAL (access.as_container(), config());
		// Explicit indexes can never be an attribute
		BOOST_CHECK_THROW (access.as_scalar(), invalid_variablename_exception);
	}
	BOOST_CHECK (c.empty());
	{
		config c2;
		variable_access_create access("a.b[0].c[1].d.e.f[2].g", c2);
		access.as_scalar() = 84;
		BOOST_CHECK_EQUAL (variable_access_const("a.length", c2).as_scalar(), 1);
		BOOST_CHECK_EQUAL (variable_access_const("a.b.length", c2).as_scalar(), 1);
		BOOST_CHECK_EQUAL (variable_access_const("a.b.c.length", c2).as_scalar(), 2);
		BOOST_CHECK_EQUAL (variable_access_const("a.b.c[1].d.e.f.length", c2).as_scalar(), 3);
		// we set g as a scalar
		BOOST_CHECK_EQUAL (variable_access_const("a.b.c[1].d.e.f[2].g.length", c2).as_scalar(), 0);
		BOOST_CHECK_EQUAL (variable_access_const("a.b.c[1].d.e.f[2].g", c2).as_scalar(), 84);
	}
	{
		config c2;
		variable_access_throw access("a.b[9].c", c2);
		BOOST_CHECK_THROW(access.as_scalar(), invalid_variablename_exception);
	}
	{
		const config nonempty {
			"tag1", config(),
			"tag1", config {
				"tag2", config(),
				"tag2", config(),
				"tag2", config {
					"atribute1", 88,
					"atribute2", "value",
				},
			},
			"tag1", config(),
		};
		/** This is the config:
		[tag1]
		[/tag1]
		[tag1]
			[tag2]
			[/tag2]
			[tag2]
			[/tag2]
			[tag2]
				atribute1 = 88
				atribute2 = "value"
			[/tag2]
		[/tag1]
		[tag1]
		[/tag1]
		*/
		BOOST_CHECK_EQUAL (variable_access_const("tag1.length", nonempty).as_scalar(), 3);
		BOOST_CHECK_EQUAL (variable_access_const("tag1.tag2.length", nonempty).as_scalar(), 0);
		BOOST_CHECK_EQUAL (variable_access_const("tag1[1].tag2.length", nonempty).as_scalar(), 3);
		BOOST_CHECK_EQUAL (variable_access_const("tag1[1].tag2[2].atribute1", nonempty).as_scalar().to_int(), 88);
		int count = 0;
		for(const config& child : variable_access_const("tag1", nonempty).as_array())
		{
			//silences unused variable warning.
			UNUSED(child);
			++count;
		}
		BOOST_CHECK_EQUAL (count, 3);
		count = 0;
		for(const config& child : variable_access_const("tag1.tag2", nonempty).as_array())
		{
			//silences unused variable warning.
			UNUSED(child);
			++count;
		}
		BOOST_CHECK_EQUAL (count, 0);
		count = 0;
		// explicit indexes as range always return a one element range, whether they exist or not.
		for(const config& child : variable_access_const("tag1.tag2[5]", nonempty).as_array())
		{
			//silences unused variable warning.
			UNUSED(child);
			++count;
		}
		BOOST_CHECK_EQUAL (count, 1);
	}
}

BOOST_AUTO_TEST_SUITE_END()
