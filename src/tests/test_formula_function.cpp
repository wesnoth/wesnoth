/*
   Copyright (C) 2010 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "formula/function.hpp"
#include "log.hpp"

#include <cmath>

using namespace wfl;

BOOST_AUTO_TEST_SUITE(formula_function)

BOOST_AUTO_TEST_CASE(test_formula_function_substring)
{
	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 0)")
				.evaluate().as_string()
			, "hello world");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 6)")
				.evaluate().as_string()
			, "world");

	lg::set_log_domain_severity("scripting/formula", lg::err());

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 11)")
				.evaluate().as_string()
			, "");

	lg::set_log_domain_severity("scripting/formula", lg::debug());

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', -1)")
				.evaluate().as_string()
			, "d");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', -5)")
				.evaluate().as_string()
			, "world");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', -11)")
				.evaluate().as_string()
			, "hello world");

	lg::set_log_domain_severity("scripting/formula", lg::err());

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', -12)")
				.evaluate().as_string()
			, "hello world");

	lg::set_log_domain_severity("scripting/formula", lg::debug());

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 0, 0)")
				.evaluate().as_string()
			, "");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 0, -1)")
				.evaluate().as_string()
			, "h");

	lg::set_log_domain_severity("scripting/formula", lg::debug());

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 5, 1)")
				.evaluate().as_string()
			, " ");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 1, 9)")
				.evaluate().as_string()
			, "ello worl");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', -10, 9)")
				.evaluate().as_string()
			, "ello worl");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', -1, -5)")
				.evaluate().as_string()
			, "world");

	BOOST_CHECK_EQUAL(
			formula("substring('hello world', 4, -5)")
				.evaluate().as_string()
			, "hello");

}

BOOST_AUTO_TEST_CASE(test_formula_function_length)
{
	BOOST_CHECK_EQUAL(
			  formula("length('')").evaluate().as_int()
			, 0);

	BOOST_CHECK_EQUAL(
			  formula("length('hello world')").evaluate().as_int()
			, 11);
}

BOOST_AUTO_TEST_CASE(test_formula_function_concatenate)
{
	BOOST_CHECK_EQUAL(
			  formula("concatenate(100)").evaluate().as_string()
			, "100");

	BOOST_CHECK_EQUAL(
			  formula("concatenate(100, 200, 'a')")
				.evaluate().as_string()
			, "100200a");

	BOOST_CHECK_EQUAL(
			  formula("concatenate([1,2,3])")
				.evaluate().as_string()
			, "1, 2, 3");

	BOOST_CHECK_EQUAL(
			  formula(
					"concatenate([1.0, 1.00, 1.000, 1.2, 1.23, 1.234])")
						.evaluate().as_string()
			, "1.000, 1.000, 1.000, 1.200, 1.230, 1.234");
}

BOOST_AUTO_TEST_CASE(test_formula_function_math)
{
	BOOST_CHECK_EQUAL(formula("abs(5)").evaluate().as_int(), 5);
	BOOST_CHECK_EQUAL(formula("abs(-5)").evaluate().as_int(), 5);
	BOOST_CHECK_EQUAL(formula("abs(5.0)").evaluate().as_int(), 5);
	BOOST_CHECK_EQUAL(formula("abs(-5.0)").evaluate().as_int(), 5);

	BOOST_CHECK_EQUAL(formula("min(3,5)").evaluate().as_int(), 3);
	BOOST_CHECK_EQUAL(formula("min(5,2)").evaluate().as_int(), 2);
	BOOST_CHECK_EQUAL(formula("max(3,5)").evaluate().as_int(), 5);
	BOOST_CHECK_EQUAL(formula("max(5,2)").evaluate().as_int(), 5);
	BOOST_CHECK_EQUAL(formula("max(5.5,5)").evaluate().as_decimal(),
		static_cast<int>(1000.0 * 5.5));

	BOOST_CHECK_EQUAL(formula("max(4,5,[2,18,7])").evaluate().as_int(), 18);

	BOOST_CHECK_EQUAL(formula("log(8,2)").evaluate().as_int(), 3);
	BOOST_CHECK_EQUAL(formula("log(12)").evaluate().as_decimal(),
		static_cast<int>(round(1000.0 * log(12))));
	BOOST_CHECK_EQUAL(formula("exp(3)").evaluate().as_decimal(),
		static_cast<int>(round(1000.0 * exp(3))));
}

BOOST_AUTO_TEST_CASE(test_formula_function_trig)
{
	const double pi = 4. * atan(1.);

	map_formula_callable variables;

	for(size_t x = 0; x <= 360; ++x) {
		variables.add("x", variant(x));

		BOOST_CHECK_EQUAL(
			  formula("sin(x)")
				.evaluate(variables).as_decimal()
			, static_cast<int>(round(1000. * sin(x * pi / 180.))));

		BOOST_CHECK_EQUAL(
			  formula("cos(x)")
				.evaluate(variables).as_decimal()
			, static_cast<int>(round(1000. * cos(x * pi / 180.))));

		if(x % 90 == 0 && x % 180 != 0) {
			BOOST_CHECK(
				formula("tan(x)")
				.evaluate(variables).is_null());
		} else {
			BOOST_CHECK_EQUAL(
				formula("tan(x)")
				.evaluate(variables).as_decimal(),
				static_cast<int>(round(1000. * tan(x * pi / 180.))));
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()
