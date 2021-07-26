/*
	Copyright (C) 2019 - 2021
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

#include "formula/string_utils.hpp"
#include "tstring.hpp"

#include <algorithm>

BOOST_AUTO_TEST_SUITE( formula_timespan )

using std::time_t;

namespace {

enum TIME_FACTORS
{
	YEAR = 31104000, /* 12 months */
	MONTH = 2592000, /* 30 days */
	WEEK = 604800,
	DAY = 86400,
	HOUR = 3600,
	MIN = 60,
	SEC = 1,
};

inline std::string minifmt(time_t t, const std::string& singular, const std::string& plural)
{
	return t ? std::to_string(t) + " " + (t > 1 ? plural : singular) : "";
}

typedef std::tuple<
	time_t /*sec*/,
	time_t /*min*/,
	time_t /*hr*/,
	time_t /*day*/,
	time_t /*wk*/,
	time_t /*mo*/,
	time_t /*yr*/> time_detailed;

inline time_t gen_as_time_t(const time_detailed& params)
{
	auto [sec, min, hr, day, wk, mo, yr] = params;

	return YEAR*yr + MONTH*mo + WEEK*wk + DAY*day + HOUR*hr + MIN*min + SEC*sec;
}

inline std::string gen_as_str(const time_detailed& params)
{
	auto [sec, min, hr, day, wk, mo, yr] = params;

	std::vector<t_string> bits;
	std::string res;

	bits.emplace_back(minifmt(yr, "year", "years"));
	bits.emplace_back(minifmt(mo, "month", "months"));
	bits.emplace_back(minifmt(wk, "week", "weeks"));
	bits.emplace_back(minifmt(day, "day", "days"));
	bits.emplace_back(minifmt(hr, "hour", "hours"));
	bits.emplace_back(minifmt(min, "minute", "minutes"));
	bits.emplace_back(minifmt(sec, "second", "seconds"));

	// Drop zeroes
	auto p = std::remove_if(bits.begin(), bits.end(), [](const t_string& t) { return t.empty(); });
	if(p != bits.end()) {
		bits.erase(p);
	}

	return utils::format_conjunct_list("expired", bits);
}

}

BOOST_AUTO_TEST_CASE( test_formula_timespan )
{
	time_detailed t;

	t = time_detailed{ 1, 0, 0, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL("1 second", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 2, 0, 0, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL("2 seconds", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 1, 0, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL("1 minute", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 2, 0, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL("2 minutes", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 1, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL("1 hour", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 2, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL("2 hours", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 1, 0, 0, 0 };
	BOOST_CHECK_EQUAL("1 day", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 2, 0, 0, 0 };
	BOOST_CHECK_EQUAL("2 days", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 0, 1, 0, 0 };
	BOOST_CHECK_EQUAL("1 week", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 0, 2, 0, 0 };
	BOOST_CHECK_EQUAL("2 weeks", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 0, 0, 1, 0 };
	BOOST_CHECK_EQUAL("1 month", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 0, 0, 2, 0 };
	BOOST_CHECK_EQUAL("2 months", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 0, 0, 0, 1 };
	BOOST_CHECK_EQUAL("1 year", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 0, 0, 0, 2 };
	BOOST_CHECK_EQUAL("2 years", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 12, 1, 23, 3, 2, 5, 2 };
	BOOST_CHECK_EQUAL(gen_as_str(t), utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 0, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL(utils::format_timespan(gen_as_time_t(t)), utils::format_timespan(0));
	BOOST_CHECK_EQUAL(utils::format_timespan(gen_as_time_t(t)), utils::format_timespan(-10000));

	t = time_detailed{ 4, 0, 49, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL("2 days, 1 hour, and 4 seconds", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 40, 0, 11, 1, 0, 4 };
	BOOST_CHECK_EQUAL("4 years, 2 weeks, 4 days, and 40 minutes", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 0, 0, 1, 0, 0, 3, 4 };
	BOOST_CHECK_EQUAL("4 years, 3 months, and 1 hour", utils::format_timespan(gen_as_time_t(t)));

	t = time_detailed{ 10, 0, 0, 0, 0, 2, 0 };
	BOOST_CHECK_EQUAL("2 months and 10 seconds", utils::format_timespan(gen_as_time_t(t)));
}

BOOST_AUTO_TEST_SUITE_END()
