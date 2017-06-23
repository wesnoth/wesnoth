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

// In this domain since it compares a shared string from this domain.
#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config_cache.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/label.hpp"

#include <boost/test/unit_test.hpp>

#include "utils/functional.hpp"

#include <sstream>


static void print(std::stringstream& sstr
		, const std::string& queue
		, const std::string& id)
{
	sstr << queue << ':' << id << '\n';
}

template<gui2::event::ui_event E>
void connect_queue(
		  std::stringstream& sstr
		, gui2::widget& widget)
{
	widget.connect_signal<E>(
			  std::bind(print, std::ref(sstr), "pre", widget.id())
			, gui2::event::dispatcher::back_pre_child);

	widget.connect_signal<E>(
			  std::bind(print, std::ref(sstr), "child", widget.id())
			, gui2::event::dispatcher::back_child);

	widget.connect_signal<E>(
			  std::bind(print, std::ref(sstr), "post", widget.id())
			, gui2::event::dispatcher::back_post_child);
}

static void connect_signals(
		  std::stringstream& sstr
		, gui2::widget& widget)
{
	/** @todo Add the rest of the events. */
	connect_queue<gui2::event::DRAW>(sstr, widget);
	connect_queue<gui2::event::CLOSE_WINDOW>(sstr, widget);
	connect_queue<gui2::event::MOUSE_ENTER>(sstr, widget);
	connect_queue<gui2::event::MOUSE_LEAVE>(sstr, widget);
	connect_queue<gui2::event::LEFT_BUTTON_DOWN>(sstr, widget);
	connect_queue<gui2::event::LEFT_BUTTON_UP>(sstr, widget);
	connect_queue<gui2::event::LEFT_BUTTON_CLICK>(sstr, widget);
	connect_queue<gui2::event::LEFT_BUTTON_DOUBLE_CLICK>(sstr, widget);
	connect_queue<gui2::event::MIDDLE_BUTTON_DOWN>(sstr, widget);
	connect_queue<gui2::event::MIDDLE_BUTTON_UP>(sstr, widget);
	connect_queue<gui2::event::MIDDLE_BUTTON_CLICK>(sstr, widget);
	connect_queue<gui2::event::MIDDLE_BUTTON_DOUBLE_CLICK>(sstr, widget);
	connect_queue<gui2::event::RIGHT_BUTTON_DOWN>(sstr, widget);
	connect_queue<gui2::event::RIGHT_BUTTON_UP>(sstr, widget);
	connect_queue<gui2::event::RIGHT_BUTTON_CLICK>(sstr, widget);
	connect_queue<gui2::event::RIGHT_BUTTON_DOUBLE_CLICK>(sstr, widget);
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

static std::string set_event_order()
{
	return
		"pre:root\n"
		"pre:level 1\n"
		"pre:level 2\n"
		"child:level 2\n"
		"post:level 2\n"
		"post:level 1\n"
		"post:root\n";
}

/** @todo Add the rest of the events. */
static void validate_draw(std::stringstream& sstr)
{
	BOOST_CHECK_EQUAL(sstr.str(), set_event_order());
}

static void validate_right_button_down(std::stringstream& sstr)
{
	BOOST_CHECK_EQUAL(sstr.str(), set_event_order());
}

BOOST_AUTO_TEST_CASE(test_fire_event)
{
	/**** Initialize the environment. *****/
	game_config::config_cache& cache = game_config::config_cache::instance();

	cache.clear_defines();
	cache.add_define("EDITOR");
	cache.add_define("MULTIPLAYER");

	std::stringstream sstr;

	/**** Initialize the grid. *****/
	gui2::grid grid(1, 1);
	grid.set_id("root");
	connect_signals(sstr, grid);

	gui2::grid *child_grid = new gui2::grid(1, 1);
	add_widget(grid, child_grid, "level 1", 0, 0);
	connect_signals(sstr, *child_grid);

	gui2::widget *child = new gui2::grid(1, 1);
	add_widget(*child_grid, child, "level 2", 0, 0);
	connect_signals(sstr, *child);

	/** @todo Add the rest of the events. */
	sstr.str("");
	grid.fire(gui2::event::DRAW, *child);
	validate_draw(sstr);

	sstr.str("");
	grid.fire(gui2::event::RIGHT_BUTTON_DOWN, *child);
	validate_right_button_down(sstr);
}

