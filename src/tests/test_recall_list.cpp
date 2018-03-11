/*
   Copyright (C) 2014 - 2018 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <boost/test/unit_test.hpp>

#include "config.hpp"
#include "recall_list_manager.hpp"
#include "tests/utils/game_config_manager.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"

BOOST_AUTO_TEST_SUITE( recall_list_suite )

BOOST_AUTO_TEST_CASE( test_1 ) {
	config game_config(test_utils::get_test_config());

	config orc_config {
		"id",			"Orcish Grunt",
		"random_traits", 	false,
		"animate",		false,
	};

	unit_type orc_type(orc_config);

	unit_types.build_unit_type(orc_type, unit_type::FULL);

	unit_ptr orc1(new unit(orc_type, 1, false));
	unit_ptr orc2(new unit(orc_type, 1, false));

	orc1->set_name("Larry");
	orc2->set_name("Moe");

	orc1->set_id("larry");
	orc2->set_id("moe");

	recall_list_manager recall_man;
	BOOST_CHECK_EQUAL(recall_man.size(), 0);
	BOOST_CHECK_MESSAGE(recall_man.begin() == recall_man.end(), "failed begin() == end() for an empty container");

	recall_man.add(orc1);
	BOOST_CHECK_EQUAL(recall_man.size(), 1);
	BOOST_CHECK_MESSAGE(recall_man[0] == orc1, "unexpected result at index [0]");
	BOOST_CHECK_MESSAGE(recall_man.find_if_matches_id("larry") == orc1, "found something unexpected");
	BOOST_CHECK_MESSAGE(!recall_man.find_if_matches_id("moe"), "found something unexpected");

	recall_man.add(orc2);
	BOOST_CHECK_EQUAL(recall_man.size(), 2);
	BOOST_CHECK_MESSAGE(recall_man[0] == orc1, "unexpected result at index [0]");
	BOOST_CHECK_MESSAGE(recall_man[1] == orc2, "unexpected result at index [1]");
	BOOST_CHECK_MESSAGE(recall_man.find_if_matches_id("larry") == orc1, "found something unexpected");
	BOOST_CHECK_MESSAGE(recall_man.find_if_matches_id("moe") == orc2, "found something unexpected");

	recall_man.erase_if_matches_id("larry");
	BOOST_CHECK_EQUAL(recall_man.size(), 1);
	BOOST_CHECK_MESSAGE(recall_man[0] == orc2, "unexpected result at index [0]");
	BOOST_CHECK_MESSAGE(!recall_man.find_if_matches_id("larry"), "found something unexpected");
	BOOST_CHECK_MESSAGE(recall_man.find_if_matches_id("moe") == orc2, "found something unexpected");

	recall_man.add(orc1);
	BOOST_CHECK_EQUAL(recall_man.size(), 2);
	BOOST_CHECK_MESSAGE(recall_man[0] == orc2, "unexpected result at index [0]");
	BOOST_CHECK_MESSAGE(recall_man[1] == orc1, "unexpected result at index [1]");
	BOOST_CHECK_MESSAGE(recall_man.find_if_matches_id("larry") == orc1, "found something unexpected");
	BOOST_CHECK_MESSAGE(recall_man.find_if_matches_id("moe") == orc2, "found something unexpected");

}

BOOST_AUTO_TEST_SUITE_END()
