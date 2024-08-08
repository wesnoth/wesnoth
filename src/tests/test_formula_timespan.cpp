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

inline void test_format_timespan(const time_detailed& tcase, const std::string& detailed, const std::string& fuzzy="")
{
	BOOST_CHECK_EQUAL(detailed, utils::format_timespan(gen_as_time_t(tcase), true));

	if(!fuzzy.empty()) {
		BOOST_REQUIRE_NE(detailed, fuzzy); // ensure test case params are not borked
		BOOST_CHECK_EQUAL(fuzzy, utils::format_timespan(gen_as_time_t(tcase)));
		BOOST_CHECK_NE(utils::format_timespan(gen_as_time_t(tcase)), utils::format_timespan(gen_as_time_t(tcase), true));
	}
}

}

BOOST_AUTO_TEST_CASE( test_formula_timespan )
{
	test_format_timespan({ 1, 0, 0, 0, 0, 0, 0 }, "1 second");

	test_format_timespan({ 2, 0, 0, 0, 0, 0, 0 }, "2 seconds");

	test_format_timespan({ 0, 1, 0, 0, 0, 0, 0 }, "1 minute");

	test_format_timespan({ 0, 2, 0, 0, 0, 0, 0 }, "2 minutes");

	test_format_timespan({ 0, 0, 1, 0, 0, 0, 0 }, "1 hour");

	test_format_timespan({ 0, 0, 2, 0, 0, 0, 0 }, "2 hours");

	test_format_timespan({ 0, 0, 0, 1, 0, 0, 0 }, "1 day");

	test_format_timespan({ 0, 0, 0, 2, 0, 0, 0 }, "2 days");

	test_format_timespan({ 0, 0, 0, 0, 1, 0, 0 }, "1 week");

	test_format_timespan({ 0, 0, 0, 0, 2, 0, 0 }, "2 weeks");

	test_format_timespan({ 0, 0, 0, 0, 0, 1, 0 }, "1 month");

	test_format_timespan({ 0, 0, 0, 0, 0, 2, 0 }, "2 months");

	test_format_timespan({ 0, 0, 0, 0, 0, 0, 1 }, "1 year");

	test_format_timespan({ 0, 0, 0, 0, 0, 0, 2 }, "2 years");

	auto t = time_detailed{ 12, 1, 23, 3, 2, 5, 2 };
	test_format_timespan(t, gen_as_str(t), "2 years");

	t = time_detailed{ 0, 0, 0, 0, 0, 0, 0 };
	BOOST_CHECK_EQUAL(utils::format_timespan(gen_as_time_t(t)), utils::format_timespan(0));
	BOOST_CHECK_EQUAL(utils::format_timespan(gen_as_time_t(t), true), utils::format_timespan(0));
	BOOST_CHECK_EQUAL(utils::format_timespan(gen_as_time_t(t)), utils::format_timespan(-10000));
	BOOST_CHECK_EQUAL(utils::format_timespan(gen_as_time_t(t), true), utils::format_timespan(-10000));

	test_format_timespan({ 4, 0, 49, 0, 0, 0, 0 }, "2 days, 1 hour, and 4 seconds", "2 days");

	test_format_timespan({ 0, 40, 0, 11, 1, 0, 4 }, "4 years, 2 weeks, 4 days, and 40 minutes", "4 years");

	test_format_timespan({ 0, 0, 1, 0, 0, 3, 4 }, "4 years, 3 months, and 1 hour", "4 years");

	test_format_timespan({ 10, 0, 0, 0, 0, 2, 0 }, "2 months and 10 seconds", "2 months");
}

BOOST_AUTO_TEST_SUITE_END()
