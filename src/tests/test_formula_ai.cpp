/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "utils/test_support.hpp"



#include "tests/utils/play_scenario.hpp"


BOOST_AUTO_TEST_SUITE( formula_ai )

BOOST_AUTO_TEST_CASE( test_move )
{
	test_utils::play_scenario scenario("formula");

	scenario.add_formula_command("move(loc(11,3), loc(15,5))");

	scenario.play();

	// Remember that map_location() handles automaticaly transition to 1 based from 0 based locations
	BOOST_CHECK_EQUAL(scenario.find_unit_loc("side_1_leader"), map_location(14,4));
}

BOOST_AUTO_TEST_CASE( test_move_teleport_bug )
{
	test_utils::play_scenario scenario("formula");

	scenario.add_formula_command("move(loc(11,3), loc(19,7))");

	scenario.play();

	// Remember that map_location() handles automaticaly transition to 1 based from 0 based locations
	// we should land to same place as previus test as 19,7 is 2 moves to same direction
	BOOST_CHECK_EQUAL(scenario.find_unit_loc("side_1_leader"), map_location(14,4));
}


BOOST_AUTO_TEST_SUITE_END();
