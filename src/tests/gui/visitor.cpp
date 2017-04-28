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

#include "config_cache.hpp"
#include "gui/auxiliary/iterator/walker.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/grid.hpp"

#include <iostream>
#include <typeinfo>

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

template<class T>
static void test_control()
{
	//std::cerr << __func__ << ": " << typeid(T).name() << ".\n";

	T control;
	const std::unique_ptr<gui2::iteration::walker_base> visitor(control.create_walker());

	BOOST_REQUIRE_NE(visitor.get(), static_cast<void*>(nullptr));

	/***** INITIAL STATE *****/

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::self), false);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::internal), true);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::child), true);

	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::self), &control);
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::internal), static_cast<void*>(nullptr));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::child), static_cast<void*>(nullptr));

	/***** VISITING WIDGET *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::self), gui2::iteration::walker_base::invalid);
	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::internal), gui2::iteration::walker_base::fail);
	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::child), gui2::iteration::walker_base::fail);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::self), true);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::internal), true);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::child), true);

	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::self), static_cast<void*>(nullptr));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::internal), static_cast<void*>(nullptr));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::child), static_cast<void*>(nullptr));

	/***** POST END *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::self), gui2::iteration::walker_base::fail);
}

static void test_control()
{
	/* Could add more widgets to the list. */
	test_control<gui2::label>();

}

static void test_grid()
{
	/* An empty grid behaves the same as a control so test here. */
	test_control<gui2::grid>();

	//std::cerr << __func__ << ": Detailed test.\n";

	/* Test the child part here. */
	gui2::grid grid(2 ,2);
	add_widget(grid, new gui2::label(), "(1,1)", 0, 0);
	add_widget(grid, new gui2::label(), "(1,2)", 0, 1);
	add_widget(grid, new gui2::label(), "(2,1)", 1, 0);
	add_widget(grid, new gui2::label(), "(2,2)", 1, 1);

	const std::unique_ptr<gui2::iteration::walker_base> visitor(grid.create_walker());

	/***** LABEL 1,1 *****/

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iteration::walker_base::child), static_cast<void*>(nullptr));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::child)->id(), "(1,1)");

	/***** LABEL 2,1 *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::child), gui2::iteration::walker_base::valid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iteration::walker_base::child), static_cast<void*>(nullptr));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::child)->id(), "(2,1)");

	/***** LABEL 1,2 *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::child), gui2::iteration::walker_base::valid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iteration::walker_base::child), static_cast<void*>(nullptr));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::child)->id(), "(1,2)");

	/***** LABEL 2,2 *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::child), gui2::iteration::walker_base::valid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iteration::walker_base::child), static_cast<void*>(nullptr));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::child)->id(), "(2,2)");

	/***** END *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::child), gui2::iteration::walker_base::invalid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iteration::walker_base::child), true);

	BOOST_CHECK_EQUAL(visitor->get(gui2::iteration::walker_base::child), static_cast<void*>(nullptr));

	/***** POST END *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iteration::walker_base::child), gui2::iteration::walker_base::fail);
}

BOOST_AUTO_TEST_CASE(test_gui2_visitor)
{

	/**** Initialize the environment. *****/
	game_config::config_cache& cache = game_config::config_cache::instance();

	cache.clear_defines();
	cache.add_define("EDITOR");
	cache.add_define("MULTIPLAYER");

	test_control();
	test_grid();
}


