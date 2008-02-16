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
#ifdef WESNOTH_BOOST_AUTO_TEST_MAIN
	#define BOOST_AUTO_TEST_MAIN
	#include <boost/test/auto_unit_test.hpp>
#else
	#define BOOST_TEST_MAIN
	#include <boost/test/unit_test.hpp>
#endif

#if WESNOTH_BOOST_TEST_MAIN

struct wesnoth_global_fixture {
	wesnoth_global_fixture() 
	{
		boost::unit_test::unit_test_log.set_threshold_level( boost::unit_test::log_messages );
		BOOST_MESSAGE("Initializing test!");
	}
	~wesnoth_global_fixture() 
	{
	}
};

BOOST_GLOBAL_FIXTURE( wesnoth_global_fixture );

#endif

/*
 * This is a main compilation unit for the test program.
 * main() function is defined by the framework.
 *
 * Please don't put your tests in this file.
 */

/* vim: set ts=4 sw=4: */

