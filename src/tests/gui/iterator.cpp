/*
   Copyright (C) 2011 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include <boost/test/unit_test.hpp>

#include "config.hpp"
#include "config_cache.hpp"
#include "gui/auxiliary/iterator/iterator.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/grid.hpp"

#include <iostream>
#include <sstream>
#include <typeinfo>

/*
 * In the unit tests we use a widget tree that looks like:
 *
 * [0]
 *  \
 *   [1|2|3|4]
 *    \
 *    [5|6|7|8]
 *
 * Where widgets 0 and 1 are a grid and the rest of the widgets a label.
 * The unit tests traverse the tree.
 */

static std::string top_down_t_t_t_result()
{
	static const std::string result =
		"At '0'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: proceed. Down and visit '1'.\n"
		"At '1'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: proceed. Down and visit '5'.\n"
		"At '5'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: reached the end. Up widget '5'. "
			"Iterate: reached '6'. Down and visit '6'.\n"
		"At '6'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: reached the end. Up widget '6'. "
			"Iterate: reached '7'. Down and visit '7'.\n"
		"At '7'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: reached the end. Up widget '7'. "
			"Iterate: reached '8'. Down and visit '8'.\n"
		"At '8'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: reached the end. Up widget '8'. "
			"Iterate: failed. Up widget '1'. "
			"Iterate: reached '2'. Down and visit '2'.\n"
		"At '2'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: reached the end. Up widget '2'. "
			"Iterate: reached '3'. Down and visit '3'.\n"
		"At '3'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: reached the end. Up widget '3'. "
			"Iterate: reached '4'. Down and visit '4'.\n"
		"At '4'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: reached the end. Up widget '4'. "
			"Iterate: failed. Finished iteration.\n";

	return result;
}

static std::string bottom_up_t_t_t_result()
{
	static const std::string result =
		"Constructor:  Down widget '1'. Down widget '5'. Finished at '5'.\n"
		"At '5'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '1'. Iterate child: visit '1'. "
			"Down widget '6'. Visit '6'.\n"
		"At '6'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '1'. Iterate child: visit '1'. "
			"Down widget '7'. Visit '7'.\n"
		"At '7'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '1'. Iterate child: visit '1'. "
			"Down widget '8'. Visit '8'.\n"
		"At '8'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '1'. Iterate child: reached the end. Visit '1'.\n"
		"At '1'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '0'. Iterate child: visit '0'. "
			"Down widget '2'. Visit '2'.\n"
		"At '2'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '0'. Iterate child: visit '0'. "
			"Down widget '3'. Visit '3'.\n"
		"At '3'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '0'. Iterate child: visit '0'. "
			"Down widget '4'. Visit '4'.\n"
		"At '4'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Up '0'. Iterate child: reached the end. Visit '0'.\n"
		"At '0'. Iterate widget: reached the end. Iterate grid: failed. "
			"Iterate child: Finished iteration.\n";

	return result;
}

static void add_widget(gui2::grid& grid
		, gui2::widget* widget
		, const std::string& id
		, const unsigned row
		, const unsigned column)
{
	BOOST_REQUIRE_NE(widget, static_cast<gui2::widget*>(nullptr));

	widget->set_id(id);
	grid.set_child(widget
			, row
			, column
			, gui2::grid::VERTICAL_GROW_SEND_TO_CLIENT
				| gui2::grid::HORIZONTAL_GROW_SEND_TO_CLIENT
			, 0);
}

template<class T, typename... T2>
static void test_control(T2&&... args)
{
	T control(std::forward<T2>(args)...);

	{
		gui2::iteration::iterator< gui2::iteration::policy::order::top_down<
				true
				, true
				, true> >
			iterator(control);

		/***** INITIAL STATE *****/

		BOOST_CHECK_EQUAL(iterator.at_end(), false);

		BOOST_CHECK_EQUAL(&*iterator, &control);

		/***** POST END *****/

		BOOST_CHECK_EQUAL(iterator.next(), false);

		BOOST_CHECK_EQUAL(iterator.at_end(), true);

	}
	{
		gui2::iteration::iterator< gui2::iteration::policy::order::top_down<
				false
				, true
				, true> >
			iterator(control);

		/***** INITIAL STATE *****/

		BOOST_CHECK_EQUAL(iterator.at_end(), true);
	}

	{
		gui2::iteration::iterator<gui2::iteration::policy::order::bottom_up<true, true, true> > iterator(control);
		BOOST_CHECK_EQUAL(iterator.at_end(), false);
	}
	{
		gui2::iteration::iterator<gui2::iteration::policy::order::bottom_up<false, false, false> > iterator(control);
		BOOST_CHECK_EQUAL(iterator.at_end(), true);
	}
}

static void test_control()
{
	/* Could add more widgets to the list. */
	test_control<gui2::label>(gui2::implementation::builder_label(config()));

}

static void test_grid()
{
	/* An empty grid behaves the same as a control so test here. */
	test_control<gui2::grid>();

	/* Test the child part here. */
	gui2::grid grid(2 ,2);
	grid.set_id("0");

	gui2::grid* g = new gui2::grid(2, 2);
	add_widget(grid, g, "1", 0, 0);
	add_widget(grid, new gui2::label(gui2::implementation::builder_label(config())), "2", 1, 0);
	add_widget(grid, new gui2::label(gui2::implementation::builder_label(config())), "3", 0, 1);
	add_widget(grid, new gui2::label(gui2::implementation::builder_label(config())), "4", 1, 1);

	add_widget(*g, new gui2::label(gui2::implementation::builder_label(config())), "5", 0, 0);
	add_widget(*g, new gui2::label(gui2::implementation::builder_label(config())), "6", 1, 0);
	add_widget(*g, new gui2::label(gui2::implementation::builder_label(config())), "7", 0, 1);
	add_widget(*g, new gui2::label(gui2::implementation::builder_label(config())), "8", 1, 1);

	{
		std::stringstream sstr;
		lg::redirect_output_setter redirect_output(sstr);

		gui2::iteration::iterator<gui2::iteration::policy::order::top_down<
				true
				, true
				, true> >
			iterator(grid);

		while(iterator.next()) {
			/* DO NOTHING */
		}

		BOOST_CHECK_EQUAL(top_down_t_t_t_result(), sstr.str());
	}
	{
		std::stringstream sstr;
		lg::redirect_output_setter redirect_output(sstr);

		gui2::iteration::iterator<gui2::iteration::policy::order::top_down<
				true
				, true
				, true> >
			iterator(grid);

		for( ; !iterator.at_end(); ++iterator) {
			/* DO NOTHING */
		}

		BOOST_CHECK_EQUAL(top_down_t_t_t_result(), sstr.str());
	}
	{
		std::stringstream sstr;
		lg::redirect_output_setter redirect_output(sstr);

		gui2::iteration::iterator<gui2::iteration::policy::order::bottom_up<
				true
				, true
				, true> >
			iterator(grid);

		while(iterator.next()) {
			/* DO NOTHING */
		}

		BOOST_CHECK_EQUAL(bottom_up_t_t_t_result(), sstr.str());
	}
	{
		std::stringstream sstr;
		lg::redirect_output_setter redirect_output(sstr);

		gui2::iteration::iterator<gui2::iteration::policy::order::bottom_up<
				true
				, true
				, true> >
			iterator(grid);

		for( ; !iterator.at_end(); ++iterator) {
			/* DO NOTHING */
		}

		BOOST_CHECK_EQUAL(bottom_up_t_t_t_result(), sstr.str());
	}
}

BOOST_AUTO_TEST_CASE(test_gui2_iterator)
{
	/**** Initialize the environment. *****/
	game_config::config_cache& cache = game_config::config_cache::instance();

	cache.clear_defines();
	cache.add_define("EDITOR");
	cache.add_define("MULTIPLAYER");

	lg::set_log_domain_severity("gui/iterator", lg::debug());
	lg::timestamps(false);

	std::stringstream sstr;
	lg::redirect_output_setter redirect_output(sstr);

	test_control();
	test_grid();
}

