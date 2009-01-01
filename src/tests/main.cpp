/* $Id$ */
/*
   Copyright (C) 2007 - 2009 by Karol Nowak <grywacz@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/version.hpp>

#define BOOST_TEST_MODULE wesnoth unit tests master suite
#if BOOST_VERSION < 103400
#include <boost/test/auto_unit_test.hpp>
#else
#include <boost/test/unit_test.hpp>
#endif
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/detail/unit_test_parameters.hpp>
#include <boost/test/results_reporter.hpp>

#if BOOST_VERSION < 103400
#include <boost/test/unit_test_log.hpp>
#include <boost/scoped_ptr.hpp>
#endif

#include "SDL.h"

#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "network.hpp"
#include "config.hpp"
#include "log.hpp"

#include "tests/utils/fake_display.hpp"
/**
 * @todo: add all others exception handlers too
 **/

static void exception_translator_config(const config::error& e)
{
	throw boost::execution_exception(boost::execution_exception::cpp_exception_error, "config:error: " + e.message);
}

static void exception_translator_network(const network::error& e)
{
	throw boost::execution_exception(boost::execution_exception::cpp_exception_error, "network::error: " + e.message);
}



static void exception_translator_game(const game::error& e)
{
	throw boost::execution_exception(boost::execution_exception::cpp_exception_error, "game::error: " + e.message);
}

struct wesnoth_global_fixture {
	wesnoth_global_fixture() 
	{
//		lg::set_log_domain_severity("all",3);
		game_config::use_dummylocales = true;
		game_config::path = get_cwd();


		// Initialize unit tests
		SDL_Init(SDL_INIT_TIMER);
		test_utils::get_fake_display();


		// Set more report as default
		if (boost::unit_test::runtime_config::log_level() == boost::unit_test::invalid_log_level)
			boost::unit_test::unit_test_log.set_threshold_level( boost::unit_test::log_messages );
		if (boost::unit_test::runtime_config::report_level() == boost::unit_test::INV_REPORT_LEVEL)
			boost::unit_test::results_reporter::set_level(boost::unit_test::SHORT_REPORT);

		boost::unit_test::unit_test_monitor.register_exception_translator<game::error>(&exception_translator_game);
		boost::unit_test::unit_test_monitor.register_exception_translator<network::error>(&exception_translator_network);
		boost::unit_test::unit_test_monitor.register_exception_translator<config::error>(&exception_translator_config);
	}
	~wesnoth_global_fixture() 
	{
		SDL_Quit();
	}
};

#if BOOST_VERSION < 103400
#include <boost/test/auto_unit_test.hpp>

#define BOOST_GLOBAL_FIXTURE(name)\
boost::scoped_ptr<name> global_fix;\
boost::unit_test::test_suite*\
init_unit_test_suite( int argc, char* argv[] ) { \
    boost::unit_test::auto_unit_test_suite_t* master_test_suite = boost::unit_test::auto_unit_test_suite();\
\
    boost::unit_test::const_string new_name = boost::unit_test::const_string( BOOST_STRINGIZE(BOOST_TEST_MODULE) );\
\
    if( !new_name.is_empty() )\
        boost::unit_test::assign_op( master_test_suite->p_name.value, new_name, 0 );\
\
    master_test_suite->argc = argc;\
    master_test_suite->argv = argv;\
\
	global_fix.reset(new name());\
    return master_test_suite;\
}

#endif

BOOST_GLOBAL_FIXTURE(wesnoth_global_fixture);

/*
 * This is a main compilation unit for the test program.
 * main() function is defined by the framework.
 *
 * Please don't put your tests in this file.
 */

/* vim: set ts=4 sw=4: */

