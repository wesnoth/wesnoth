/*
	Copyright (C) 2019 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <boost/test/unit_test.hpp>

#include "formula/format_timespan.hpp"
#include "serialization/chrono.hpp"

BOOST_AUTO_TEST_SUITE( formula_timespan )

BOOST_AUTO_TEST_CASE( test_formula_timespan )
{
	using namespace std::chrono_literals;

	BOOST_CHECK_EQUAL("1 second",  utils::format_timespan(std::chrono::seconds{1}, true));
	BOOST_CHECK_EQUAL("2 seconds", utils::format_timespan(std::chrono::seconds{2}, true));
	BOOST_CHECK_EQUAL("1 minute",  utils::format_timespan(std::chrono::minutes{1}, true));
	BOOST_CHECK_EQUAL("2 minutes", utils::format_timespan(std::chrono::minutes{2}, true));
	BOOST_CHECK_EQUAL("1 hour",    utils::format_timespan(std::chrono::hours{1}, true));
	BOOST_CHECK_EQUAL("2 hours",   utils::format_timespan(std::chrono::hours{2}, true));
	BOOST_CHECK_EQUAL("1 day",     utils::format_timespan(chrono::days{1}, true));
	BOOST_CHECK_EQUAL("2 days",    utils::format_timespan(chrono::days{2}, true));
	BOOST_CHECK_EQUAL("1 week",    utils::format_timespan(chrono::weeks{1}, true));
	BOOST_CHECK_EQUAL("2 weeks",   utils::format_timespan(chrono::weeks{2}, true));
	BOOST_CHECK_EQUAL("1 year",    utils::format_timespan(chrono::years{1}, true));
	BOOST_CHECK_EQUAL("2 years",   utils::format_timespan(chrono::years{2}, true));

	BOOST_CHECK_EQUAL(utils::format_timespan(0s),       utils::format_timespan(0min));
	BOOST_CHECK_EQUAL(utils::format_timespan(0s, true), utils::format_timespan(0min));
	BOOST_CHECK_EQUAL(utils::format_timespan(0s),       utils::format_timespan(-10000min));
	BOOST_CHECK_EQUAL(utils::format_timespan(0s, true), utils::format_timespan(-10000min));

	{
		constexpr auto time = chrono::years{2} + chrono::months{5} + chrono::weeks{2} + chrono::days{3} +
			std::chrono::hours{23} + std::chrono::minutes{1} + std::chrono::seconds{12};

		BOOST_CHECK_EQUAL("2 years, 5 months, 2 weeks, 3 days, 23 hours, 1 minute, and 12 seconds", utils::format_timespan(time, true));
		BOOST_CHECK_EQUAL("2 years", utils::format_timespan(time));
	}

	{
		constexpr auto time = chrono::days{2} + std::chrono::hours{1} + std::chrono::seconds{4};
		BOOST_CHECK_EQUAL("2 days, 1 hour, and 4 seconds", utils::format_timespan(time, true));
		BOOST_CHECK_EQUAL("2 days", utils::format_timespan(time));
	}

	{
		constexpr auto time = chrono::years{4} + chrono::weeks{2} + chrono::days{4} + std::chrono::minutes{40};
		BOOST_CHECK_EQUAL("4 years, 2 weeks, 4 days, and 40 minutes", utils::format_timespan(time, true));
		BOOST_CHECK_EQUAL("4 years", utils::format_timespan(time));
	}

	{
		constexpr auto time = chrono::years{4} + chrono::months{3} + std::chrono::hours{1};
		BOOST_CHECK_EQUAL("4 years, 3 months, and 1 hour", utils::format_timespan(time, true));
		BOOST_CHECK_EQUAL("4 years", utils::format_timespan(time));
	}

	{
		constexpr auto time = chrono::months{2} + std::chrono::seconds{10};
		BOOST_CHECK_EQUAL("2 months and 10 seconds", utils::format_timespan(time, true));
		BOOST_CHECK_EQUAL("2 months", utils::format_timespan(time));
	}
}

BOOST_AUTO_TEST_SUITE_END()
