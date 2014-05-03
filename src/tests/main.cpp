/*
   Copyright (C) 2007 - 2014 by Karol Nowak <grywacz@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"


#define BOOST_TEST_MODULE wesnoth unit tests master suite
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/detail/unit_test_parameters.hpp>
#include <boost/test/results_reporter.hpp>

#include <fstream>

#include "SDL.h"

#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "gui/auxiliary/event/handler.hpp"
#include "gui/widgets/helper.hpp"
#include "network.hpp"
#include "config.hpp"
#include "log.hpp"

#include "tests/utils/fake_display.hpp"
#include "game_display.hpp"
/**
 * @todo add all others exception handlers too
 **/

static void exception_translator_config(const config::error& e)
{
	throw "config:error: " + e.message;
}

static void exception_translator_network(const network::error& e)
{
	throw "network::error: " + e.message;
}



static void exception_translator_game(const game::error& e)
{
	throw "game::error: " + e.message;
}

std::ofstream reporter;

struct wesnoth_global_fixture {
	wesnoth_global_fixture()
	{
		reporter.open("boost_test_result.xml");
		assert( reporter.is_open() );

		boost::unit_test::results_reporter::set_stream(reporter);
//		lg::set_log_domain_severity("all",lg::debug);
		game_config::path = get_cwd();


		// Initialize unit tests
		SDL_Init(SDL_INIT_TIMER);
		test_utils::get_fake_display(1024, 768);

		gui2::init();
		static const gui2::event::tmanager gui_event_manager;



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

BOOST_GLOBAL_FIXTURE(wesnoth_global_fixture)

/*
 * This is a main compilation unit for the test program.
 * main() function is defined by the framework.
 *
 * Please don't put your tests in this file.
 */

/* vim: set ts=4 sw=4: */

