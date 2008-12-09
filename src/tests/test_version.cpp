/* $Id$ */
/*
   Copyright (C) 2008 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

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
	
	
}

BOOST_AUTO_TEST_SUITE_END()
