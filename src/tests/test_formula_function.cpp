/* $Id$ */
/*
   Copyright (C) 2010 by Mark de Wever <koraq@xs4all.nl>
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

#include "formula_function.hpp"

BOOST_AUTO_TEST_SUITE(formula_function)

BOOST_AUTO_TEST_CASE(test_formula_function_substring)
{
	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', 0)")
				.evaluate().as_string()
			, "hello world");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', 6)")
				.evaluate().as_string()
			, "world");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', 11)")
				.evaluate().as_string()
			, "");


	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', -1)")
				.evaluate().as_string()
			, "d");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', -5)")
				.evaluate().as_string()
			, "world");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', -11)")
				.evaluate().as_string()
			, "hello world");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', -12)")
				.evaluate().as_string()
			, "hello world");


	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', 0, 0)")
				.evaluate().as_string()
			, "");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', 0, -1)")
				.evaluate().as_string()
			, "");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', 5, 1)")
				.evaluate().as_string()
			, " ");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', 1, 9)")
				.evaluate().as_string()
			, "ello worl");

	BOOST_CHECK_EQUAL(
			game_logic::formula("substring('hello world', -10, 9)")
				.evaluate().as_string()
			, "ello worl");

}

BOOST_AUTO_TEST_SUITE_END()

