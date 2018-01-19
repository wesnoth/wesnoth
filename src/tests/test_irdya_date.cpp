/*
Copyright (C) 2003 - 2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.

See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "utils/irdya_datetime.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_irdya_datetime)

BOOST_AUTO_TEST_CASE(test_irdya_date_parse) {
	irdya_date BW_423 = irdya_date::read_date("   423   BW  ");
	irdya_date YW_123 = irdya_date::read_date("   123   YW  ");
	irdya_date BF_109 = irdya_date::read_date("   109   BF  ");
	irdya_date AF_928 = irdya_date::read_date("   928   AF  ");

	BOOST_CHECK_EQUAL(BW_423.get_epoch(), irdya_date::EPOCH::BEFORE_WESNOTH);
	BOOST_CHECK_EQUAL(BW_423.get_year(), 423);
	BOOST_CHECK_EQUAL(YW_123.get_epoch(), irdya_date::EPOCH::WESNOTH);
	BOOST_CHECK_EQUAL(YW_123.get_year(), 123);
	BOOST_CHECK_EQUAL(BF_109.get_epoch(), irdya_date::EPOCH::BEFORE_FALL);
	BOOST_CHECK_EQUAL(BF_109.get_year(), 109);
	BOOST_CHECK_EQUAL(AF_928.get_epoch(), irdya_date::EPOCH::AFTER_FALL);
	BOOST_CHECK_EQUAL(AF_928.get_year(), 928);
}

BOOST_AUTO_TEST_CASE(test_irdya_date_equal) {
	irdya_date first(irdya_date::EPOCH::WESNOTH, 12);
	irdya_date second(irdya_date::EPOCH::WESNOTH, 12);
	BOOST_CHECK_EQUAL(first, second);
}

BOOST_AUTO_TEST_CASE(test_irdya_date_ordering) {
	irdya_date BW_34(irdya_date::EPOCH::BEFORE_WESNOTH, 34), BW_12(irdya_date::EPOCH::BEFORE_WESNOTH, 12), YW_40(irdya_date::EPOCH::WESNOTH, 40), YW_52(irdya_date::EPOCH::WESNOTH, 52);
	irdya_date BF_29(irdya_date::EPOCH::BEFORE_FALL, 29), BF_42(irdya_date::EPOCH::BEFORE_FALL, 42), AF_12(irdya_date::EPOCH::AFTER_FALL, 12), AF_102(irdya_date::EPOCH::AFTER_FALL, 102), Y0;

	BOOST_CHECK(BW_34 < BW_12);
	BOOST_CHECK(BW_34 < YW_40);
	BOOST_CHECK(BW_34 < YW_52);
	BOOST_CHECK(BW_34 < BF_42);
	BOOST_CHECK(BW_34 < BF_29);
	BOOST_CHECK(BW_34 < AF_12);
	BOOST_CHECK(BW_34 < AF_102);
	BOOST_CHECK(BW_34 < Y0);

	BOOST_CHECK(BW_12 < YW_40);
	BOOST_CHECK(BW_12 < YW_52);
	BOOST_CHECK(BW_12 < BF_42);
	BOOST_CHECK(BW_12 < BF_29);
	BOOST_CHECK(BW_12 < AF_12);
	BOOST_CHECK(BW_12 < AF_102);
	BOOST_CHECK(BW_12 < Y0);

	BOOST_CHECK(YW_40 < YW_52);
	BOOST_CHECK(YW_40 < BF_42);
	BOOST_CHECK(YW_40 < BF_29);
	BOOST_CHECK(YW_40 < AF_12);
	BOOST_CHECK(YW_40 < AF_102);
	BOOST_CHECK(YW_40 < Y0);

	BOOST_CHECK(YW_52 < BF_42);
	BOOST_CHECK(YW_52 < BF_29);
	BOOST_CHECK(YW_52 < AF_12);
	BOOST_CHECK(YW_52 < AF_102);
	BOOST_CHECK(YW_52 < Y0);

	BOOST_CHECK(BF_42 < BF_29);
	BOOST_CHECK(BF_42 < AF_12);
	BOOST_CHECK(BF_42 < AF_102);
	BOOST_CHECK(BF_42 < Y0);

	BOOST_CHECK(BF_29 < AF_12);
	BOOST_CHECK(BF_29 < AF_102);
	BOOST_CHECK(BF_29 < Y0);

	BOOST_CHECK(AF_12 < AF_102);
	BOOST_CHECK(AF_12 < Y0);
	BOOST_CHECK(AF_102 < Y0);
}

BOOST_AUTO_TEST_SUITE_END()
