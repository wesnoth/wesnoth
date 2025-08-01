/*
	Copyright (C) 2024 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
	COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "scripting/lua_ptr.hpp"
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

struct dummy_object : public enable_lua_ptr<dummy_object> {
	std::string value;
	dummy_object(const std::string& s) : enable_lua_ptr<dummy_object>(this), value(s) {}
};

BOOST_AUTO_TEST_CASE(test_lua_ptr) {
	std::vector<dummy_object> vec;
	auto& obj = vec.emplace_back("test");
	BOOST_CHECK_EQUAL(obj.value, "test");
	lua_ptr<dummy_object> ptr(obj);
	BOOST_CHECK(ptr);
	BOOST_CHECK_EQUAL(ptr.get_ptr(), &obj);
	{
		// test move constructor
		auto obj2 = std::move(obj);
		BOOST_CHECK(ptr);
		BOOST_CHECK_EQUAL(ptr.get_ptr(), &obj2);
		BOOST_CHECK_EQUAL(ptr->value, "test");

		// NOLINTNEXTLINE(bugprone-use-after-move)
		lua_ptr<dummy_object> ptr_from_moved_object(obj);
		BOOST_CHECK(!ptr_from_moved_object);

		// test move assignment
		dummy_object obj3{"different"};
		obj3 = std::move(obj2);
		BOOST_CHECK(ptr);
		BOOST_CHECK_EQUAL(ptr.get_ptr(), &obj3);
		BOOST_CHECK_EQUAL(ptr->value, "test");

		vec.clear();
		BOOST_CHECK(ptr);
		BOOST_CHECK_EQUAL(ptr->value, "test");
		BOOST_CHECK(!ptr_from_moved_object);
	}
	BOOST_CHECK(!ptr);
}
