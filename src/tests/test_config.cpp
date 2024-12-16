/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
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
#include <cmath>

#include "config.hpp"
#include "variable_info.hpp"

BOOST_AUTO_TEST_SUITE(test_config)

BOOST_AUTO_TEST_CASE(test_config_attribute_value)
{
	config c;
	config c1;
	config c2;
	const config& cc = c;
	int x_int;
	std::string x_str;
	long long x_sll;
	double x_dbl;

// compare identical assigned int vs string
	c1["x"] = 6;
	c2["x"] = "6";
	BOOST_CHECK_EQUAL(c1["x"], c2["x"]);

// compare identical assigned int vs floating point
	c1["x"] = 6;
	c2["x"] = 6.0;
	BOOST_CHECK_EQUAL(c1["x"], c2["x"]);

// compare identical assigned int-string vs floating point
	c1["x"] = "6";
	c2["x"] = 6.0;
	BOOST_CHECK_EQUAL(c1["x"], c2["x"]);

// compare identical assigned floating point-string vs int
	c1["x"] = 6;
	c2["x"] = "6.0";
	BOOST_CHECK_NE(c1["x"], c2["x"]);

// compare identical assigned floating point vs string
	c1["x"] = 6.0;
	c2["x"] = "6.0";
	BOOST_CHECK_NE(c1["x"], c2["x"]);

// check what happens when trying to get a numeric result from a non-numeric value
	c["x"] = "1aaaa";
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "1aaaa");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 1);
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 1ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1.0);

	c["x"] = "1.7aaaa";
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "1.7aaaa");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 1);
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 1ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1.7);

	c["x"] = "aaaa1";
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "aaaa1");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 0);
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 0.0);

// check type conversion when assigned as int
	c["x"] = 1;
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "1");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 1);
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 1ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1.0);

// check type conversion when assigned as int (again)
	c["x"] = 10000000;
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 10000000);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "10000000");
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 10000000ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1e7);

// check type conversion when assigned aan empty string
	c["x"] = "";
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 0);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 0.0);

// check type conversion when assigned as a hex string
	c["x"] = "0x11";
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 0);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "0x11");
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 0.0);

// check type conversion when assigned as a hex string (again)
	c["x"] = "0xab";
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 0);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "0xab");
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 0.0);

// check type conversion when assigned as a string with leading zeroes
	c["x"] = "00001111";
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 1111);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "00001111");
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 1111ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1.111e3);

// check type conversion when assigned as a string with only zeroes
	c["x"] = "000000";
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 0);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "000000");
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 0.0);

// check type conversion when assigned as a string with leading zeroes and is too large to fit in an int
	c["x"] = "01234567890123456789";
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 1234567890123456789ll);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "01234567890123456789");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 0);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1.23456789012345678e18);

// check type conversion when assigned as a string with no leading zeroes and is too large to fit in an int
	c["x"] = "99999999999999999999";
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 0ll);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "99999999999999999999");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 0);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1e20);

// check type conversion when assigned as a floating point
	c["x"] = 1.499;
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 1ll);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "1.499");
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, 1);
	x_dbl = c["x"].to_double();
	BOOST_CHECK(std::abs(x_dbl - 1.499) < 1e-6);

// check type conversion when assigned as a long long (int overflows)
	c["x"] = 123456789123ll;
	x_int = c["x"].to_int();
	BOOST_CHECK_EQUAL(x_int, -1097262461);
	x_dbl = c["x"].to_double();
	BOOST_CHECK_EQUAL(x_dbl, 1.23456789123e11);
	x_sll = c["x"].to_long_long();
	BOOST_CHECK_EQUAL(x_sll, 123456789123ll);
	x_str = c["x"].str();
	BOOST_CHECK_EQUAL(x_str, "123456789123");

// check heterogeneous comparison
	c["x"] = 987654321;
	BOOST_CHECK_EQUAL(c["x"], 987654321);
	c["x"] = "1";
	BOOST_CHECK_EQUAL(c["x"], "1");
	c["x"] = 222;
	BOOST_CHECK_EQUAL(c["x"], "222");
	c["x"] = "test";
	BOOST_CHECK_EQUAL(c["x"], "test");
	c["x"] = "33333";
	BOOST_CHECK_EQUAL(c["x"], 33333);
	c["x"] = "yes";
	BOOST_CHECK_EQUAL(c["x"], true);
	c["x"] = false;
	BOOST_CHECK_EQUAL(c["x"], "no");
	c["x"] = 1.23456789;
	BOOST_CHECK_EQUAL(c["x"], 1.23456789);
#if 1 // FIXME: this should work. it doesn't work. looks like it's getting stored as 9.8765432099999995
	c["x"] = "9.87654321";
	BOOST_CHECK_EQUAL(c["x"], 9.87654321);
#endif
	c["x"] = "sfvsdgdsfg";
	BOOST_CHECK_NE(c["x"], 0);
	BOOST_CHECK_NE(c["x"], true);
	BOOST_CHECK_NE(c["x"], "a random string");

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

BOOST_AUTO_TEST_CASE(test_variable_info)
{
	config c;
	{
		variable_access_const access("", c);
		// We dotn allow empty keys
		BOOST_CHECK_THROW(access.as_scalar(), invalid_variablename_exception);
	}
	{
		variable_access_const access("some_non_existent.", c);
		// We dotn allow empty keys
		BOOST_CHECK_THROW(access.as_scalar(), invalid_variablename_exception);
	}
	{
		variable_access_const access("some_non_existent[0]value", c);
		// We expect '.' after ']'
		BOOST_CHECK_THROW(access.as_scalar(), invalid_variablename_exception);
	}
	{
		variable_access_const access("some_non_existent", c);
		// we return empty be default
		BOOST_CHECK(!access.exists_as_container());
		BOOST_CHECK_EQUAL(access.as_container(), config());
		BOOST_CHECK(!access.exists_as_attribute());
		BOOST_CHECK_EQUAL(access.as_scalar(), config::attribute_value());
	}
	{
		variable_access_const access("a.b[0].c[1].d.e.f[2]", c);
		// we return empty be default
		BOOST_CHECK(!access.exists_as_container());
		BOOST_CHECK_EQUAL(access.as_container(), config());
		// Explicit indexes can never be an attribute
		BOOST_CHECK_THROW(access.as_scalar(), invalid_variablename_exception);
	}
	BOOST_CHECK(c.empty());
	{
		config c2;
		variable_access_create access("a.b[0].c[1].d.e.f[2].g", c2);
		access.as_scalar() = 84;
		BOOST_CHECK_EQUAL(variable_access_const("a.length", c2).as_scalar(), 1);
		BOOST_CHECK_EQUAL(variable_access_const("a.b.length", c2).as_scalar(), 1);
		BOOST_CHECK_EQUAL(variable_access_const("a.b.c.length", c2).as_scalar(), 2);
		BOOST_CHECK_EQUAL(variable_access_const("a.b.c[1].d.e.f.length", c2).as_scalar(), 3);
		// we set g as a scalar
		BOOST_CHECK_EQUAL(variable_access_const("a.b.c[1].d.e.f[2].g.length", c2).as_scalar(), 0);
		BOOST_CHECK_EQUAL(variable_access_const("a.b.c[1].d.e.f[2].g", c2).as_scalar(), 84);
	}
	{
		config c2;
		variable_access_throw access("a.b[9].c", c2);
		BOOST_CHECK_THROW(access.as_scalar(), invalid_variablename_exception);
	}
	{
		/* clang-format off */
		const config nonempty{
			"tag1", config(),
			"tag1", config{
				"tag2", config(),
				"tag2", config(),
				"tag2", config{
					"atribute1", 88,
					"atribute2", "value",
				},
			},
			"tag1", config(),
		};
		/* clang-format on */
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
		BOOST_CHECK_EQUAL(variable_access_const("tag1.length", nonempty).as_scalar(), 3);
		BOOST_CHECK_EQUAL(variable_access_const("tag1.tag2.length", nonempty).as_scalar(), 0);
		BOOST_CHECK_EQUAL(variable_access_const("tag1[1].tag2.length", nonempty).as_scalar(), 3);
		BOOST_CHECK_EQUAL(variable_access_const("tag1[1].tag2[2].atribute1", nonempty).as_scalar().to_int(), 88);
		int count = 0;
		for([[maybe_unused]] const config& child : variable_access_const("tag1", nonempty).as_array()) {
			++count;
		}
		BOOST_CHECK_EQUAL(count, 3);
		count = 0;
		for([[maybe_unused]] const config& child : variable_access_const("tag1.tag2", nonempty).as_array()) {
			++count;
		}
		BOOST_CHECK_EQUAL(count, 0);
		count = 0;
		// explicit indexes as range always return a one element range, whether they exist or not.
		for([[maybe_unused]] const config& child : variable_access_const("tag1.tag2[5]", nonempty).as_array()) {
			++count;
		}
		BOOST_CHECK_EQUAL(count, 1);
	}
}

BOOST_AUTO_TEST_CASE(add_child_EmptyThis_newKey_AppendAndReturnNewEmptyChild)
{
	config actual;
	const config new_child = actual.add_child("A");
	const config expected("A");
	BOOST_CHECK_EQUAL(actual, expected);
	BOOST_CHECK_EQUAL(new_child, config());
}

namespace bdata = boost::unit_test::data;
BOOST_DATA_TEST_CASE(add_child_NonEmptyThis_newOrExistingKey_lOrRValue_AppendAndReturnNewChild,
	bdata::make({"A", "B", "C"}) * bdata::make<std::string>({"lvalue_ref", "rvalue_ref"}),
	key,
	update_ref_kind) // 3 * 2 = 6 cases
{
	// Data for testing base.add_child(key, update)
	const config base{"A", config(), "a", 1};   // [A][/A] a = 1
	const config update{"B", config(), "b", 2}; // [B][/B] b = 2
	// Expected config: [A][/A] a = 1 [key]  [B][/B] b = 2  [/key]
	/* clang-format off */
	const config expected{
		// base
		"A", config(), "a", 1,
		// [key] copy of update [/key]
		key, config(update)};
	/* clang-format on */
	// Make actual
	config actual(base);
	config new_child;
	if(update_ref_kind == std::string("lvalue_ref"))
		new_child = actual.add_child(key, update);
	else
		new_child = actual.add_child(key, config(update)); // rvalue ref.

	BOOST_CHECK_EQUAL(actual, expected);
	BOOST_CHECK_EQUAL(new_child, update);
	// Assert the new child is a copy of update
	BOOST_CHECK_NE(&new_child, &update);
}

BOOST_AUTO_TEST_SUITE_END()
