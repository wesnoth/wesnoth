/*
   Copyright (C) 2012 - 2013 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "addon/validation.hpp"

BOOST_AUTO_TEST_SUITE( addons )

BOOST_AUTO_TEST_CASE( validation )
{
	BOOST_CHECK( !addon_filename_legal("") );
	BOOST_CHECK( !addon_filename_legal(".") );
	BOOST_CHECK( !addon_filename_legal("invalid/slash") );
	BOOST_CHECK( !addon_filename_legal("invalid\\backslash") );
	BOOST_CHECK( !addon_filename_legal("invalid:colon") );
	BOOST_CHECK( !addon_filename_legal("invalid~tilde") );
	BOOST_CHECK( !addon_filename_legal("invalid/../parent") );
}

BOOST_AUTO_TEST_SUITE_END()
