/*
	Copyright (C) 2024 - 2024
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
		auto obj_moved = std::move(obj);
		BOOST_CHECK(ptr);
		BOOST_CHECK_EQUAL(ptr.get_ptr(), &obj_moved);
		BOOST_CHECK_EQUAL(ptr->value, "test");
		vec.clear();
	}
	BOOST_CHECK(!ptr);
}
