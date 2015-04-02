/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
#include <boost/foreach.hpp>
#include <cmath>
#include <set>
#include <vector>

#include "config.hpp"
#include "config_assign.hpp"
#include "make_enum.hpp"
#include "utils/config_class.tpp"
#include "utils/convert.hpp"
#include "utils/convert_comma_list.hpp"
#include "utils/convert_lexical.hpp"

CONFIG_CLASS( test_info,
	CONFIG_VAL( int,		i,	convert::to_int(3))
	CONFIG_VAL( int,		i2,	convert::to_int())
	CONFIG_VAL( bool, 		b,	convert::to_bool(true))
	CONFIG_VAL( bool, 		b2,	convert::to_bool(false))
	CONFIG_VAL( std::string,	s,	convert::str())
)

CONFIG_CLASS( test_comma_list_struct,
	CONFIG_VAL( std::vector<std::string>, 	vec, convert::comma_list<std::vector<std::string> >() )
	CONFIG_VAL( std::set<std::string>, 	set, convert::comma_list<std::set<std::string> >() )
)

CONFIG_CLASS( test_comma_list_struct_templ,
	CONFIG_VAL_T( std::vector<std::string>, vec, convert::comma_list)
	CONFIG_VAL_T( std::set<std::string>, 	set, convert::comma_list)
)


namespace foo {
	MAKE_ENUM( ey,
		(FOO, "foo")
		(BAR, "bar")
		(BAZ, "baz")
	)

	MAKE_ENUM_STREAM_OPS1(ey)

	ey warning_suppressor = string_to_ey_default("asdf", FOO);
}

CONFIG_CLASS( test_lexical_struct,
	CONFIG_VAL( foo::ey,	widget,		convert::lexical<foo::ey>())
	CONFIG_VAL( foo::ey,	gadget,		convert::lexical<foo::ey>(foo::BAZ))
)

BOOST_AUTO_TEST_SUITE ( test_config_convert )

BOOST_AUTO_TEST_CASE( test_info_1 )
{
	const config cfg = config_of
		("i2", 2)
		("b", "no")
		("s", "huzzah");

	test_info t = from_config<test_info>(cfg);

	BOOST_CHECK_EQUAL(t.i_, 3);
	BOOST_CHECK_EQUAL(t.i2_,2);
	BOOST_CHECK_EQUAL(t.b_, false);
	BOOST_CHECK_EQUAL(t.b2_,false);
	BOOST_CHECK_EQUAL(t.s_, "huzzah");
}

BOOST_AUTO_TEST_CASE( test_comma_list )
{
	const config cfg = config_of
		("vec", "1,1,2,3,5")
		("set", "2,4,6,8");

	test_comma_list_struct t = from_config<test_comma_list_struct>(cfg);

	const config cfg2 = to_config<test_comma_list_struct> (t);

	BOOST_CHECK_EQUAL(cfg2["vec"].str(), "1,1,2,3,5");
	BOOST_CHECK_EQUAL(cfg2["set"].str(), "2,4,6,8");
}

BOOST_AUTO_TEST_CASE( test_comma_list_templ )
{
	const config cfg = config_of
		("vec", "1,1,2,3,5")
		("set", "2,4,6,8");

	test_comma_list_struct_templ t = from_config<test_comma_list_struct_templ>(cfg);

	const config cfg2 = to_config<test_comma_list_struct_templ> (t);

	BOOST_CHECK_EQUAL(cfg2["vec"].str(), "1,1,2,3,5");
	BOOST_CHECK_EQUAL(cfg2["set"].str(), "2,4,6,8");
}

BOOST_AUTO_TEST_CASE( test_lexical )
{
	const config cfg = config_of
		("widget", "bar");

	test_lexical_struct t = from_config<test_lexical_struct>(cfg);

	BOOST_CHECK_EQUAL(t.widget_, foo::BAR);
	BOOST_CHECK_EQUAL(t.gadget_, foo::BAZ);
}

BOOST_AUTO_TEST_SUITE_END()
