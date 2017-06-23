/*
   Copyright (C) 2008 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
#include "version.hpp"

BOOST_AUTO_TEST_SUITE( version )

BOOST_AUTO_TEST_CASE( test_version_info )
{
	version_info empty;

	BOOST_CHECK( empty == version_info(0, 0, 0) );
	BOOST_CHECK( empty.str() == "0.0.0" );

	version_info dots1("........");
	version_info dots2("...hullo");

	BOOST_CHECK( dots1 == empty);
	BOOST_CHECK( dots2.str() == "0.0.0hullo" );

	version_info canonical("1.2.3");

	BOOST_CHECK( canonical.is_canonical() );

	version_info canonical_suffixed("1.2.3+dev");

	BOOST_CHECK( canonical_suffixed > canonical );
	BOOST_CHECK( canonical < canonical_suffixed );

	version_info non_canonical("1.2.3.4.5.7.8.9");

	BOOST_CHECK( !non_canonical.is_canonical() );

	version_info non_canonical_suffixed("1.2.3.4.5.7.8.9+dev");

	BOOST_CHECK( non_canonical_suffixed > non_canonical );
	BOOST_CHECK( non_canonical < non_canonical_suffixed );

	version_info right_zero("1.2.0");
	version_info no_right_zero("1.2");

	BOOST_CHECK( right_zero == no_right_zero );

	version_info left_zero("0.1.4");
	version_info no_left_zero("1.4");

	BOOST_CHECK( left_zero != no_left_zero );

	const std::string bad_version_info_string_1 = "Viva la revolución!";
	const std::string bad_version_info_string_2 = "To infinity and beyond!";

	const version_info bad_version_info1( bad_version_info_string_1 );
	const version_info bad_version_info2( bad_version_info_string_2 );

	BOOST_CHECK( bad_version_info1.str() == ("0.0.0"+bad_version_info_string_1) );
	BOOST_CHECK( bad_version_info2.str() == ("0.0.0"+bad_version_info_string_2) );

	version_info somewhat_complex("1.5.10-1.6beta2");
	BOOST_CHECK( somewhat_complex.major_version() == 1 && somewhat_complex.minor_version() == 5 && somewhat_complex.revision_level() == 10 && somewhat_complex.special_version() == "1.6beta2" && somewhat_complex.special_version_separator() == '-' );
}

BOOST_AUTO_TEST_SUITE_END()
