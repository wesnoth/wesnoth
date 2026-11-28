/*
	Copyright (C) 2026
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

#include "log.hpp"

#include <boost/test/unit_test.hpp>

#include <string>

BOOST_AUTO_TEST_SUITE(log_domains)

BOOST_AUTO_TEST_CASE(test_list_log_domains_severity)
{
	const std::string name = "test/logdomains_severity";
	[[maybe_unused]] const lg::log_domain domain(name.c_str());

	const std::string with_warning = lg::list_log_domains(name, true);
	BOOST_CHECK_EQUAL(with_warning, name + " warning\n");

	const std::string names_only = lg::list_log_domains(name);
	BOOST_CHECK_EQUAL(names_only, name + "\n");

	BOOST_REQUIRE(lg::set_log_domain_severity(name, lg::severity::LG_NONE));
	const std::string with_none = lg::list_log_domains(name, true);
	BOOST_CHECK_EQUAL(with_none, name + " none\n");
}

BOOST_AUTO_TEST_SUITE_END()
