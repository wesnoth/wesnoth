/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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

#include "config.hpp"
#include "lexical_cast.hpp"
#include "utils/make_enum.hpp"

namespace foo {

	MAKE_ENUM(enumname,
	        (con1, "name1")
	        (con2, "name2")
	        (con3, "name3")
	)
}


//generates an enum foo::enumname with lexical casts
/*
namespace foo {
enum enumname {con1, con2 ,con3}
}

foo::enumname lexical_cast<std::string> ( std::string str ) throws bad_lexical_cast
{
    ...
}
*/

class bar {
public:
	MAKE_ENUM(another,
	        (val1, "name1")
	        (val2, "name2")
	        (val3, "name3")
	)
};


/** Tests begin **/

BOOST_AUTO_TEST_SUITE ( test_make_enum )

BOOST_AUTO_TEST_CASE ( test_make_enum_namespace )
{
	foo::enumname e = foo::enumname::string_to_enum("name2", foo::enumname::con1); //returns con2

	BOOST_CHECK_EQUAL ( e, foo::enumname::con2 );

	std::string str = foo::enumname::enum_to_string(foo::enumname::con1); //returns "name1"

	BOOST_CHECK_EQUAL ( str, "name1" );

	std::string str2 = lexical_cast<std::string> (e); //returns "name2" since e is con2

	BOOST_CHECK_EQUAL ( str2, "name2" );

	bool threw_exception_when_it_wasnt_supposed_to = false;

	try {
		e = lexical_cast<foo::enumname> ("name3"); //returns con3
	} catch (bad_lexical_cast & /*e*/) {
		//std::cerr << "enum lexical cast didn't work!" << std::endl;
		threw_exception_when_it_wasnt_supposed_to = true;
	}

	BOOST_CHECK( !threw_exception_when_it_wasnt_supposed_to );

	bool threw_exception_when_it_was_supposed_to = false;

	try {
		e = lexical_cast<foo::enumname> ("name4"); //throw bad_lexical_cast
	} catch (bad_lexical_cast & /*e*/) {
		//std::cerr << "enum lexical cast worked!" << std::endl;
		threw_exception_when_it_was_supposed_to = true;
	}

	BOOST_CHECK( threw_exception_when_it_was_supposed_to );

	std::stringstream ss;
	ss << e;
	BOOST_CHECK_EQUAL (ss.str(), "name3");
}

BOOST_AUTO_TEST_CASE ( test_make_enum_class )
{
	bar::another e = bar::another::string_to_enum("name2", bar::another::val1); //returns val2

	BOOST_CHECK_EQUAL ( e, bar::another::val2 );

	std::string str = bar::another::enum_to_string(bar::another::val1); //returns "name1"

	BOOST_CHECK_EQUAL ( str, "name1" );

	std::string str2 = lexical_cast<std::string> (e); //returns "name2" since e is val2

	BOOST_CHECK_EQUAL ( str2, "name2" );

	bool threw_exception_when_it_wasnt_supposed_to = false;

	try {
		e = lexical_cast<bar::another> ("name3"); //returns val3
	} catch (bad_lexical_cast & /*e*/) {
		//std::cerr << "enum lexical cast didn't work!" << std::endl;
		threw_exception_when_it_wasnt_supposed_to = true;
	}

	BOOST_CHECK( !threw_exception_when_it_wasnt_supposed_to );

	bool threw_exception_when_it_was_supposed_to = false;

	try {
		e = lexical_cast<bar::another> ("name4"); //throw bad_lexical_cast
	} catch (bad_lexical_cast & /*e*/) {
		//std::cerr << "enum lexical cast worked!" << std::endl;
		threw_exception_when_it_was_supposed_to = true;
	}

	BOOST_CHECK( threw_exception_when_it_was_supposed_to );

	std::stringstream ss;
	ss << e;
	BOOST_CHECK_EQUAL (ss.str(), "name3");
}

BOOST_AUTO_TEST_CASE ( test_make_enum_config )
{
	config cfg;
	foo::enumname e1 = foo::enumname::con1;
	foo::enumname e2 = foo::enumname::con2;
	foo::enumname e3 = foo::enumname::con3;

	cfg["t1"] = e2;
	cfg["t2"] = foo::enumname::enum_to_string(e2);
	cfg["t3"] = "name2";
	cfg["t4"] = "345646";

	BOOST_CHECK_EQUAL(cfg["t1"].to_enum<foo::enumname>(foo::enumname::con1) , e2);
	BOOST_CHECK_EQUAL(cfg["t2"].to_enum<foo::enumname>(foo::enumname::con1) , e2);
	BOOST_CHECK_EQUAL(cfg["t3"].to_enum<foo::enumname>(foo::enumname::con1) , e2);

	BOOST_CHECK_EQUAL(cfg["t1"].to_enum(e1) , e2);
	BOOST_CHECK_EQUAL(cfg["t2"].to_enum(e1) , e2);
	BOOST_CHECK_EQUAL(cfg["t3"].to_enum(e1) , e2);


	BOOST_CHECK_EQUAL(cfg["t4"].to_enum<foo::enumname>(foo::enumname::con3) , e3);

	BOOST_CHECK_EQUAL(cfg["t1"].str() , "name2");
}


BOOST_AUTO_TEST_CASE ( test_make_enum_parse )
{
	config cfg;
	foo::enumname e1 = foo::enumname::con1;
	foo::enumname e2 = foo::enumname::con2;
	foo::enumname e3 = foo::enumname::con3;

	cfg["t1"] = e2;
	cfg["t2"] = foo::enumname::enum_to_string(e1);
	cfg["t3"] = e1.to_string();
	cfg["t4"] = "name3";
	cfg["t5"] = "345646";

	foo::enumname dummy = foo::enumname::con1;

	BOOST_CHECK_EQUAL(dummy, e1);
	BOOST_CHECK_EQUAL(dummy.parse("name1"), true);
	BOOST_CHECK_EQUAL(dummy, e1);
	BOOST_CHECK_EQUAL(dummy.parse("name2"), true);
	BOOST_CHECK_EQUAL(dummy, e2);
	BOOST_CHECK_EQUAL(dummy.parse("name3"), true);
	BOOST_CHECK_EQUAL(dummy, e3);
	BOOST_CHECK_EQUAL(dummy.parse("name2 "), false);
	BOOST_CHECK_EQUAL(dummy, e3);
	BOOST_CHECK_EQUAL(dummy.parse("kdfg89"), false);
	BOOST_CHECK_EQUAL(dummy, e3);
	BOOST_CHECK_EQUAL(dummy.parse("NAME2"), false);
	BOOST_CHECK_EQUAL(dummy, e3);
	BOOST_CHECK_EQUAL(dummy.parse(""), false);
	BOOST_CHECK_EQUAL(dummy, e3);
	BOOST_CHECK_EQUAL(dummy.parse("name2"), true);
	BOOST_CHECK_EQUAL(dummy, e2);

	BOOST_CHECK_EQUAL(foo::enumname::name(), "enumname");


}


BOOST_AUTO_TEST_SUITE_END()
