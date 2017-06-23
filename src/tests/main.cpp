/*
   Copyright (C) 2007 - 2017 by Karol Nowak <grywacz@gmail.com>
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

#ifndef BOOST_TEST_DYN_LINK
#error Compiler misconfiguration - must define BOOST_TEST_DYN_LINK
#endif

#include <boost/version.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#if BOOST_VERSION >= 106000
#include <boost/test/unit_test_parameters.hpp>
#else
#include <boost/test/detail/unit_test_parameters.hpp>
#endif
#include <boost/test/results_reporter.hpp>


#include <fstream>

#include <SDL.h>

#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "gui/core/event/handler.hpp"
#include "gui/widgets/helper.hpp"
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

static void exception_translator_game(const game::error& e)
{
	throw "game::error: " + e.message;
}

std::ofstream reporter;

struct wesnoth_global_fixture {
	wesnoth_global_fixture()
	{
		using namespace boost::unit_test;
		reporter.open("boost_test_result.xml");
		assert( reporter.is_open() );

		results_reporter::set_stream(reporter);
//		lg::set_log_domain_severity("all",lg::debug());
		game_config::path = filesystem::get_cwd();


		// Initialize unit tests
		SDL_Init(SDL_INIT_TIMER);
		test_utils::get_fake_display(1024, 768);

		gui2::init();
		static const gui2::event::manager gui_event_manager;



		// Set more report as default
#if BOOST_VERSION >= 106400
		if(runtime_config::get<log_level>(runtime_config::btrt_log_level) == invalid_log_level)
			unit_test_log.set_threshold_level(log_messages);
		if(runtime_config::get<report_level>(runtime_config::btrt_report_level) == INV_REPORT_LEVEL)
			results_reporter::set_level(SHORT_REPORT);
		unit_test_monitor.register_exception_translator<game::error>(&exception_translator_game);
		unit_test_monitor.register_exception_translator<config::error>(&exception_translator_config);
#elif BOOST_VERSION >= 106000
		if(runtime_config::get<log_level>(runtime_config::LOG_LEVEL) == invalid_log_level)
			unit_test_log.set_threshold_level(log_messages);
		if(runtime_config::get<report_level>(runtime_config::REPORT_LEVEL) == INV_REPORT_LEVEL)
			results_reporter::set_level(SHORT_REPORT);
		unit_test_monitor.register_exception_translator<game::error>(&exception_translator_game);
		unit_test_monitor.register_exception_translator<config::error>(&exception_translator_config);
#else
		if(runtime_config::log_level() == invalid_log_level)
			unit_test_log.set_threshold_level(log_messages);
		if(runtime_config::report_level() == INV_REPORT_LEVEL)
			results_reporter::set_level(SHORT_REPORT);
		unit_test_monitor.register_exception_translator<game::error>(&exception_translator_game);
		unit_test_monitor.register_exception_translator<config::error>(&exception_translator_config);
#endif
	}
	~wesnoth_global_fixture()
	{
		SDL_Quit();
	}
};

BOOST_GLOBAL_FIXTURE(wesnoth_global_fixture);

/*
 * This is a main compilation unit for the test program.
 * main() function is defined by the framework.
 *
 * Please don't put your tests in this file.
 */

/* vim: set ts=4 sw=4: */

