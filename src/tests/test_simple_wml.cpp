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

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()
