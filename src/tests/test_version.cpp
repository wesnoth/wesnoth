/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "utils/test_support.hpp"
#include "version.hpp"

BOOST_AUTO_TEST_SUITE( version )

BOOST_AUTO_TEST_CASE( test_version_info )
{
	version_info invalid("version_info");

	BOOST_CHECK( !invalid.good() );

	version_info canonical("1.2.3");

	BOOST_CHECK( canonical.is_canonical() );

	version_info canonical_suffixed("1.2.3+svn");

	BOOST_CHECK( canonical_suffixed > canonical );
	BOOST_CHECK( canonical < canonical_suffixed );

	version_info non_canonical("1.2.3.4.5.7.8.9");

	BOOST_CHECK( !non_canonical.is_canonical() );

	version_info non_canonical_suffixed("1.2.3.4.5.7.8.9+svn");

	BOOST_CHECK( non_canonical_suffixed > non_canonical );
	BOOST_CHECK( non_canonical < non_canonical_suffixed );

	version_info right_zero("1.2.0");
	version_info no_right_zero("1.2");

	BOOST_CHECK( right_zero == no_right_zero );

	version_info left_zero("0.1.4");
	version_info no_left_zero("1.4");

	BOOST_CHECK( left_zero != no_left_zero );

	bool insanity_test = true;

	try {
		const version_info bad_version_info1("Viva la revolución!");
		const version_info bad_version_info2("To infinity and beyond!");

		// The result of this test can be ignored since it should throw an exepction.
		bad_version_info1 > bad_version_info2;

		// We should have thrown an exception already...
		insanity_test = false;
	}
	catch( const version_info::not_sane_exception& ) {
		// Good.
	}

	BOOST_CHECK( insanity_test );

	// FIXME: disabled for 1.5.10 release.
	// version_info somewhat_complex("1.5.10-1.6beta2");
	// BOOST_CHECK( somewhat_complex.major_version() == 1 && somewhat_complex.minor_version() == 5 && somewhat_complex.revision_level() == 10 && somewhat_complex.special_version() == "1.6beta2" && somewhat_complex.special_version_separator() == '-' );
}

BOOST_AUTO_TEST_SUITE_END()
