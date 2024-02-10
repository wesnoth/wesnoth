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

#include "utils/irdya_datetime.hpp"
#include <boost/test/unit_test.hpp>

using irdya_date = utils::irdya_date;
using wesnoth_epoch = utils::wesnoth_epoch;

BOOST_AUTO_TEST_SUITE(test_irdya_datetime)

BOOST_AUTO_TEST_CASE(test_irdya_date_parse) {
	irdya_date BW_423 = irdya_date::read_date("   423   BW  ");
	irdya_date YW_123 = irdya_date::read_date("   123   YW  ");
	irdya_date BF_109 = irdya_date::read_date("   109   BF  ");
	irdya_date AF_928 = irdya_date::read_date("   928   AF  ");

	BOOST_CHECK_EQUAL(wesnoth_epoch::get_string(BW_423.get_epoch()), wesnoth_epoch::before_wesnoth);
	BOOST_CHECK_EQUAL(BW_423.get_year(), 423);
	BOOST_CHECK_EQUAL(wesnoth_epoch::get_string(YW_123.get_epoch()), wesnoth_epoch::wesnoth);
	BOOST_CHECK_EQUAL(YW_123.get_year(), 123);
	BOOST_CHECK_EQUAL(wesnoth_epoch::get_string(BF_109.get_epoch()), wesnoth_epoch::before_fall);
	BOOST_CHECK_EQUAL(BF_109.get_year(), 109);
	BOOST_CHECK_EQUAL(wesnoth_epoch::get_string(AF_928.get_epoch()), wesnoth_epoch::after_fall);
	BOOST_CHECK_EQUAL(AF_928.get_year(), 928);
}

BOOST_AUTO_TEST_CASE(test_irdya_date_equal) {
	irdya_date first(wesnoth_epoch::type::wesnoth, 12);
	irdya_date second(wesnoth_epoch::type::wesnoth, 12);
	BOOST_CHECK_EQUAL(first, second);
}

BOOST_AUTO_TEST_CASE(test_irdya_date_ordering) {
	irdya_date BW_34(wesnoth_epoch::type::before_wesnoth, 34), BW_12(wesnoth_epoch::type::before_wesnoth, 12), YW_40(wesnoth_epoch::type::wesnoth, 40), YW_52(wesnoth_epoch::type::wesnoth, 52);
	irdya_date BF_29(wesnoth_epoch::type::before_fall, 29), BF_42(wesnoth_epoch::type::before_fall, 42), AF_12(wesnoth_epoch::type::after_fall, 12), AF_102(wesnoth_epoch::type::after_fall, 102), Y0;

	std::vector<std::pair<irdya_date, irdya_date>> test_cases {
		{BW_34, BW_12},
		{BW_34, YW_40},
		{BW_34, YW_52},
		{BW_34, BF_42},
		{BW_34, BF_29},
		{BW_34, AF_12},
		{BW_34, AF_102},
		{BW_34, Y0},

		{BW_12, YW_40},
		{BW_12, YW_52},
		{BW_12, BF_42},
		{BW_12, BF_29},
		{BW_12, AF_12},
		{BW_12, AF_102},
		{BW_12, Y0},

		{YW_40, YW_52},
		{YW_40, BF_42},
		{YW_40, BF_29},
		{YW_40, AF_12},
		{YW_40, AF_102},
		{YW_40, Y0},

		{YW_52, BF_42},
		{YW_52, BF_29},
		{YW_52, AF_12},
		{YW_52, AF_102},
		{YW_52, Y0},

		{BF_42, BF_29},
		{BF_42, AF_12},
		{BF_42, AF_102},
		{BF_42, Y0},

		{BF_29, AF_12},
		{BF_29, AF_102},
		{BF_29, Y0},

		{AF_12, AF_102},
		{AF_12, Y0},
		{AF_102, Y0},
	};
	for(const auto& pair : test_cases) {
		BOOST_CHECK(pair.first < pair.second);
		BOOST_CHECK(!(pair.second < pair.first));
	}
}

BOOST_AUTO_TEST_SUITE_END()
