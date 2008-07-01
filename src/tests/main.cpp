/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Karol Nowak <grywacz@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <game_errors.hpp>
#include <network.hpp>
#include <config.hpp>

/**
 * @todo: add all others exception handlers too
 **/

void exception_translator_config(const config::error& e)
{
	throw boost::execution_exception(boost::execution_exception::cpp_exception_error, "config:error: " + e.message);
}

void exception_translator_network(const network::error& e)
{
	throw boost::execution_exception(boost::execution_exception::cpp_exception_error, "network::error: " + e.message);
}

void exception_translator_game(const game::error& e)
{
	throw boost::execution_exception(boost::execution_exception::cpp_exception_error, "game::error: " + e.message);
}

struct wesnoth_global_fixture {
	wesnoth_global_fixture() 
	{
		boost::unit_test::unit_test_log.set_threshold_level( boost::unit_test::log_messages );
		BOOST_MESSAGE("Initializing test!");
		boost::unit_test::unit_test_monitor.register_exception_translator<game::error>(&exception_translator_game);
		boost::unit_test::unit_test_monitor.register_exception_translator<network::error>(&exception_translator_network);
		boost::unit_test::unit_test_monitor.register_exception_translator<config::error>(&exception_translator_config);
	}
	~wesnoth_global_fixture() 
	{
	}
};

BOOST_GLOBAL_FIXTURE( wesnoth_global_fixture );


/*
 * This is a main compilation unit for the test program.
 * main() function is defined by the framework.
 *
 * Please don't put your tests in this file.
 */

/* vim: set ts=4 sw=4: */

