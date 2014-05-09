/*
   Copyright (C) 2011 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

static void add_widget(gui2::tgrid& grid
		, gui2::twidget* widget
		, const std::string& id
		, const unsigned row
		, const unsigned column)
{
	BOOST_REQUIRE_NE(widget, static_cast<gui2::twidget*>(NULL));

	widget->set_id(id);
	grid.set_child(widget
			, row
			, column
			, gui2::tgrid::VERTICAL_GROW_SEND_TO_CLIENT
				| gui2::tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT
			, 0);
}

template<class T>
static void test_control()
{
	//std::cerr << __func__ << ": " << typeid(T).name() << ".\n";

	T control;
	boost::scoped_ptr<gui2::iterator::twalker_> visitor(control.create_walker());

	BOOST_REQUIRE_NE(visitor.get(), static_cast<void*>(NULL));

	/***** INITIAL STATE *****/

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::widget), false);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::grid), true);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::child), true);

	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::widget), &control);
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::grid), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::child), static_cast<void*>(NULL));

	/***** VISITING WIDGET *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::widget), gui2::iterator::twalker_::invalid);
	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::grid), gui2::iterator::twalker_::fail);
	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::child), gui2::iterator::twalker_::fail);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::widget), true);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::grid), true);
	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::child), true);

	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::widget), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::grid), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::child), static_cast<void*>(NULL));

	/***** POST END *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::widget), gui2::iterator::twalker_::fail);
}

static void test_control()
{
	/* Could add more widgets to the list. */
	test_control<gui2::tlabel>();

}

static void test_grid()
{
	/* An empty grid behaves the same as a control so test here. */
	test_control<gui2::tgrid>();

	//std::cerr << __func__ << ": Detailed test.\n";

	/* Test the child part here. */
	gui2::tgrid grid(2 ,2);
	add_widget(grid, new gui2::tlabel(), "(1,1)", 0, 0);
	add_widget(grid, new gui2::tlabel(), "(1,2)", 0, 1);
	add_widget(grid, new gui2::tlabel(), "(2,1)", 1, 0);
	add_widget(grid, new gui2::tlabel(), "(2,2)", 1, 1);

	boost::scoped_ptr<gui2::iterator::twalker_> visitor(grid.create_walker());

	/***** LABEL 1,1 *****/

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iterator::twalker_::child), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::child)->id(), "(1,1)");

	/***** LABEL 2,1 *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::child), gui2::iterator::twalker_::valid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iterator::twalker_::child), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::child)->id(), "(2,1)");

	/***** LABEL 1,2 *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::child), gui2::iterator::twalker_::valid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iterator::twalker_::child), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::child)->id(), "(1,2)");

	/***** LABEL 2,2 *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::child), gui2::iterator::twalker_::valid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::child), false);

	BOOST_REQUIRE_NE(visitor->get(gui2::iterator::twalker_::child), static_cast<void*>(NULL));
	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::child)->id(), "(2,2)");

	/***** END *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::child), gui2::iterator::twalker_::invalid);

	BOOST_CHECK_EQUAL(visitor->at_end(gui2::iterator::twalker_::child), true);

	BOOST_CHECK_EQUAL(visitor->get(gui2::iterator::twalker_::child), static_cast<void*>(NULL));

	/***** POST END *****/

	BOOST_CHECK_EQUAL(visitor->next(gui2::iterator::twalker_::child), gui2::iterator::twalker_::fail);
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


