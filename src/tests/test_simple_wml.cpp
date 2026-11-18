/*
	Copyright (C) 2007 - 2024
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

#include <limits>
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "server/common/simple_wml.hpp"

BOOST_AUTO_TEST_SUITE( simple_wml )

BOOST_AUTO_TEST_CASE( simple_wml_first_test )
{
		const char* doctext = R"([test]
a="blah"
b="blah"
c="\\"
d="\"""
[/test])";
	simple_wml::document doc(doctext, INIT_STATE::INIT_COMPRESSED);

	simple_wml::node& node = doc.root();
	simple_wml::node* test_node = node.child("test");
	node.set_attr("blah", "blah");
	test_node->set_attr("e", "f");

	BOOST_CHECK(test_node);
	BOOST_CHECK((*test_node)["a"] == "blah");
	BOOST_CHECK((*test_node)["b"] == "blah");
	BOOST_CHECK((*test_node)["c"] == "\\\\");
	BOOST_CHECK((*test_node)["d"] == "\\\"\"");
	BOOST_CHECK(node["blah"] == "blah");
	BOOST_CHECK((*test_node)["e"] == "f");
}

BOOST_AUTO_TEST_CASE( simple_wml_child_limits )
{
	constexpr std::size_t max_children = std::numeric_limits<unsigned short>::max() + 1;

	// 1. Test programmatic add_child limit for identical tag names (child_list_index)
	{
		simple_wml::document doc;
		simple_wml::node& root = doc.root();

		for(std::size_t i = 0; i < max_children; ++i) {
			root.add_child("a");
		}

		BOOST_CHECK_EQUAL(root.children("a").size(), max_children);
		BOOST_CHECK_THROW(root.add_child("a"), simple_wml::error);
		BOOST_CHECK_THROW(root.add_child_at("a", 0), simple_wml::error);
	}

	// 2. Test programmatic add_child limit for distinct tag names (child_map_index)
	// node::get_children() does a linear look-up, so this part is quadratic and hits the
	// time-out limit in unoptimised Xcode builds on CI.
#if !defined(__APPLE__) || defined(__OPTIMIZE__)
	{
		simple_wml::document doc;
		simple_wml::node& root = doc.root();

		std::vector<std::string> names;
		names.reserve(max_children + 1);
		for(std::size_t i = 0; i <= max_children; ++i) {
			names.push_back("t" + std::to_string(i));
		}

		for(std::size_t i = 0; i < max_children; ++i) {
			root.add_child(names[i].c_str());
		}

		BOOST_CHECK_THROW(root.add_child(names[max_children].c_str()), simple_wml::error);
	}
#endif

	// 3. Test parser limit when reading WML text
	{
		std::string wml;
		wml.reserve((max_children + 1) * 8);
		for(std::size_t i = 0; i <= max_children; ++i) {
			wml += "[a]\n[/a]\n";
		}

		BOOST_CHECK_THROW(simple_wml::document(wml.c_str(), INIT_STATE::INIT_STATIC), simple_wml::error);
	}
}

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()
