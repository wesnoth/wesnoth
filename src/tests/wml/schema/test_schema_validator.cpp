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

#include "game_version.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/schema_validator.hpp"
#include "wml_exception.hpp"

BOOST_AUTO_TEST_SUITE(test_schema_validator)

BOOST_AUTO_TEST_CASE(test_super_cycle)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_cycle.cfg";
	constexpr auto config_path = "src/tests/wml/schema/test_schema_validator/test_super_cycle.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	// See: wesnoth.cpp > handle_validate_command
	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;

	BOOST_CHECK_EXCEPTION(read(result, *stream, &validator), wml_exception, [](const wml_exception& e) {
		return boost::algorithm::contains(e.dev_message, "Inheritance cycle from other/second to main/first found");
	});
}

BOOST_AUTO_TEST_CASE(test_super_cycle_only_if_used)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_cycle.cfg";
	constexpr auto config_path = "src/tests/wml/schema/test_schema_validator/test_super_cycle_only_if_used.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;
	BOOST_CHECK_NO_THROW(read(result, *stream, &validator));
}

BOOST_AUTO_TEST_CASE(test_super_cycle_crashes_on_unknown_key)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_cycle.cfg";
	constexpr auto config_path
		= "src/tests/wml/schema/test_schema_validator/test_super_cycle_crashes_on_unknown_key.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;

	BOOST_CHECK_EXCEPTION(read(result, *stream, &validator), wml_exception, [](const wml_exception& e) {
		return boost::algorithm::contains(e.dev_message, "Invalid key 'unknown=' in tag [first]");
	});
}

BOOST_AUTO_TEST_CASE(test_super_missing)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_missing.cfg";
	constexpr auto config_path = "src/tests/wml/schema/test_schema_validator/test_super_cycle.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;

	BOOST_CHECK_EXCEPTION(read(result, *stream, &validator), wml_exception, [](const wml_exception& e) {
		return boost::algorithm::contains(e.dev_message, "Super not/here not found. Needed by other/second");
	});
}

BOOST_AUTO_TEST_CASE(test_super_missing_only_if_used)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_missing.cfg";
	constexpr auto config_path = "src/tests/wml/schema/test_schema_validator/test_super_cycle_only_if_used.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;
	BOOST_CHECK_NO_THROW(read(result, *stream, &validator));
}

BOOST_AUTO_TEST_CASE(test_super_mandatory)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_mandatory_missing.cfg";
	constexpr auto config_path = "src/tests/wml/schema/test_schema_validator/test_super_mandatory.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;
	BOOST_CHECK_NO_THROW(read(result, *stream, &validator));
}

BOOST_AUTO_TEST_CASE(test_super_mandatory_missing)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_mandatory_missing.cfg";
	constexpr auto config_path = "src/tests/wml/schema/test_schema_validator/test_super_mandatory_missing.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;

	BOOST_CHECK_EXCEPTION(read(result, *stream, &validator), wml_exception, [](const wml_exception& e) {
		return boost::algorithm::contains(e.dev_message, "Missing key 'id=' in tag [campaign]");
	});
}

BOOST_AUTO_TEST_CASE(test_super_cycle_mandatory)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_super_cycle_mandatory.cfg";
	constexpr auto config_path = "src/tests/wml/schema/test_schema_validator/test_super_cycle_mandatory.cfg";

	auto validator = schema_validation::schema_validator(schema_path, false);

	validator.set_create_exceptions(true);

	preproc_map defines_map;
	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());
	defines_map["SCHEMA_VALIDATION"] = preproc_define();

	auto stream = preprocess_file(config_path, &defines_map);

	config result;
	BOOST_CHECK_NO_THROW(read(result, *stream, &validator));
}

BOOST_AUTO_TEST_CASE(test_schema_link_cycle)
{
	constexpr auto schema_path = "src/tests/wml/schema/test_schema_validator/test_schema_link_cycle.cfg";

	BOOST_CHECK_EXCEPTION(schema_validation::schema_validator(schema_path, false), abstract_validator::error, [](const abstract_validator::error& e) {
		return boost::algorithm::contains(e.message, "Link cycle from");
	});
}

BOOST_AUTO_TEST_SUITE_END()
