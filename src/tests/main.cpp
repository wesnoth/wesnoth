/*
	Copyright (C) 2007 - 2025
	by Karol Nowak <grywacz@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>
#include <boost/test/unit_test_parameters.hpp>
#include <boost/test/results_reporter.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>

#include <SDL2/SDL.h>

#include "events.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "gui/core/event/handler.hpp"
#include "gui/gui.hpp"
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
		using namespace std::literals;
		events::set_main_thread();
		boost::filesystem::path file("boost_test_result.xml");
		for(int i = 1; i < framework::master_test_suite().argc; i++) {
			if(framework::master_test_suite().argv[i - 1] == "--output_file"s) {
				file = framework::master_test_suite().argv[i];
				break;
			}
		}

		reporter.open(file.string());
		assert( reporter.is_open() );

		results_reporter::set_stream(reporter);
//		lg::set_log_domain_severity("all",lg::debug());
		game_config::path = filesystem::get_cwd();
		filesystem::set_user_data_dir(std::string());

		// declare this here so that it will always be at the front of the event queue.
		events::event_context global_context;

		// Initialize unit tests
		SDL_Init(SDL_INIT_TIMER);
		test_utils::set_test_resolution(1024, 768);

		gui2::init();
		gui2::switch_theme("default");
		static const gui2::event::manager gui_event_manager;

		// TODO: For some reason this fails on MacOS and prevents any tests from running
		// It's not crucial to change the log levels though, so just skip over it.
#ifndef __APPLE__
		// Set more report as default
		if(runtime_config::get<log_level>(runtime_config::btrt_log_level) == invalid_log_level)
			unit_test_log.set_threshold_level(log_messages);
		if(runtime_config::get<report_level>(runtime_config::btrt_report_level) == INV_REPORT_LEVEL)
			results_reporter::set_level(SHORT_REPORT);
#endif
		unit_test_monitor.register_exception_translator<game::error>(&exception_translator_game);
		unit_test_monitor.register_exception_translator<config::error>(&exception_translator_config);
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
