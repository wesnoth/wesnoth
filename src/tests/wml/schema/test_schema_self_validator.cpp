/*
	Copyright (C) 2003 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
	COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string/predicate.hpp>

#include "serialization/schema_validator.hpp"
#include "wml_exception.hpp"

BOOST_AUTO_TEST_SUITE(test_schema_self_validator)

BOOST_AUTO_TEST_CASE(test_schema_super_self_reference)
{
	strict_validation_enabled = true;

	constexpr auto schema_path = "src/tests/wml/schema/test_schema_self_validator/test_schema_super_self_reference.cfg";

	BOOST_CHECK_EXCEPTION(
		schema_validation::schema_validator(schema_path, true), wml_exception, [](const wml_exception& e) {
			return boost::algorithm::contains(e.dev_message, "Inheritance loop super=main found");
		});
}

BOOST_AUTO_TEST_CASE(test_schema_super_cycle)
{
	strict_validation_enabled = true;

	constexpr auto schema_path = "src/tests/wml/schema/test_schema_self_validator/test_schema_super_cycle.cfg";

	BOOST_CHECK_EXCEPTION(
		schema_validation::schema_validator(schema_path, true), wml_exception, [](const wml_exception& e) {
			return boost::algorithm::contains(e.dev_message, "Inheritance loop super=main/first found");
		});
}

BOOST_AUTO_TEST_SUITE_END()
