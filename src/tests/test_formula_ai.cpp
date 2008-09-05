/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <boost/test/auto_unit_test.hpp>

#include "gamemap.hpp"

#include "tests/utils/game_config_manager.hpp"


BOOST_FIXTURE_TEST_SUITE( formula_ai, formula_ai_fixture );

BOOST_AUTO_TEST_CASE( test_move_teleport_bug )
{
	play_scenario scenario("formula");

	scenarion.add_formula_command("move(loc(11,30), loc(15,5))");

	scenario.play();

	BOOST_CHECK_EQUAL(scenario.find_unit_loc("side_1_leader"), gamemap::location(15,5));
}

BOOST_AUTO_TEST_SUITE_END();
