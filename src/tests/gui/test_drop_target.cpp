/*
   Copyright (C) 2008 - 2014 by Pauli Nieminen <paniemin@cc.hut.fi>
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

#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>

#include "sdl/rect.hpp"
#include "sdl/utils.hpp"
#include "widgets/drop_target.hpp"

BOOST_AUTO_TEST_SUITE( test_drop_target )

	/**
	 * Specialized testing class so unit test
	 * can call protected member functions to
	 * simulate drop operation
	 **/
	class test_drop_target : public gui::drop_target {
		public:
		test_drop_target(gui::drop_group_manager_ptr group, const SDL_Rect& loc) : gui::drop_target(group, loc)
		{}

		int handle_drop() {
			return gui::drop_target::handle_drop();
		}
		static bool empty() {
			return gui::drop_target::empty();
		}
	};

BOOST_AUTO_TEST_CASE( test_create_group )
{

	gui::drop_group_manager group0;
	gui::drop_group_manager* group1 = new gui::drop_group_manager();

	BOOST_CHECK_EQUAL(group0.get_group_id(), 0);
	BOOST_CHECK_EQUAL(group1->get_group_id(), 1);

	delete group1;

	gui::drop_group_manager_ptr group2(new gui::drop_group_manager());


	BOOST_CHECK_EQUAL(group2->get_group_id(), 2);
}

typedef std::vector<gui::drop_target_ptr> target_store;

static void create_drop_targets(const SDL_Rect& loc, gui::drop_group_manager_ptr group, target_store& targets, int& id_counter)
{
	gui::drop_target_ptr new_target(new test_drop_target(group, loc));
	BOOST_CHECK_EQUAL(id_counter++, new_target->get_id());
	// Test that drop gives -1 correctly for non overlapping
	// targets
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(new_target.get())->handle_drop(), -1);
	targets.push_back(new_target);
}

BOOST_AUTO_TEST_CASE( test_create_drop_targets )
{
	gui::drop_group_manager_ptr group(new gui::drop_group_manager());
	BOOST_CHECK(group->get_group_id() > 0);

	typedef std::vector<SDL_Rect> location_store;
	location_store locations;

	// Create rectangles for drop targets
	locations.push_back(sdl::create_rect(50,50,20,20));
	locations.push_back(sdl::create_rect(50,100,20,20));
	locations.push_back(sdl::create_rect(50,150,20,20));
	locations.push_back(sdl::create_rect(50,200,20,20));
	locations.push_back(sdl::create_rect(50,250,20,20));
	locations.push_back(sdl::create_rect(50,300,20,20));

	target_store targets;

	int id_counter = 0;

	std::for_each(locations.begin(), locations.end(),
			boost::bind(create_drop_targets,_1, group, boost::ref(targets), boost::ref(id_counter)));

	BOOST_CHECK_EQUAL(targets.size(), locations.size());

	// Modify 3rd rectangle to overlap with 4th
	locations[2].y = 190;

	// Check for correct drop results
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[2].get())->handle_drop(), 3);
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[3].get())->handle_drop(), 2);
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[1].get())->handle_drop(), -1);
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[4].get())->handle_drop(), -1);
}

BOOST_AUTO_TEST_CASE( check_memory_leaks )
{
	BOOST_CHECK(test_drop_target::empty());
}

BOOST_AUTO_TEST_CASE( test_multiple_drop_groups )
{
	gui::drop_group_manager_ptr group(new gui::drop_group_manager());
	gui::drop_group_manager_ptr group2(new gui::drop_group_manager());
	BOOST_CHECK(group->get_group_id() > 0);
	BOOST_CHECK(group2->get_group_id() > 0);

	typedef std::vector<SDL_Rect> location_store;
	location_store locations;
	location_store locations2;

	// Create rectangles for drop targets
	locations.push_back(sdl::create_rect(50,50,20,20));
	locations.push_back(sdl::create_rect(50,100,20,20));
	locations.push_back(sdl::create_rect(50,150,20,20));
	locations.push_back(sdl::create_rect(50,200,20,20));
	locations.push_back(sdl::create_rect(50,250,20,20));
	locations.push_back(sdl::create_rect(50,300,20,20));

	locations2.push_back(sdl::create_rect(50,50,20,20));
	locations2.push_back(sdl::create_rect(100,50,20,20));
	locations2.push_back(sdl::create_rect(150,50,20,20));
	locations2.push_back(sdl::create_rect(200,50,20,20));
	locations2.push_back(sdl::create_rect(250,50,20,20));
	locations2.push_back(sdl::create_rect(300,50,20,20));


	target_store targets;
	target_store targets2;

	int id_counter = 0;

	std::for_each(locations.begin(), locations.end(),
			boost::bind(create_drop_targets,_1, group, boost::ref(targets), boost::ref(id_counter)));
	id_counter = 0;
	std::for_each(locations2.begin(), locations2.end(),
			boost::bind(create_drop_targets,_1, group2, boost::ref(targets2), boost::ref(id_counter)));

	BOOST_CHECK_EQUAL(targets.size(), locations.size());
	BOOST_CHECK_EQUAL(targets2.size(), locations2.size());

	// Modify 3rd rectangle to overlap with 4th
	locations[2].y = 190;

	// Check for correct drop results
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[2].get())->handle_drop(), 3);
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[3].get())->handle_drop(), 2);
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[1].get())->handle_drop(), -1);
	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets[4].get())->handle_drop(), -1);

	locations2[2].y = 180;
	locations2[2].x = 50;

	BOOST_CHECK_EQUAL(static_cast<test_drop_target*>(targets2[2].get())->handle_drop(), -1);

}

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()
