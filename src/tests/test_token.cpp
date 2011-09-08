/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <boost/test/unit_test.hpp>


#include "token.hpp"

// #include "config_cache.hpp"
// #include "config.hpp"
// #include "game_config.hpp"
// #include "language.hpp"
// #include "version.hpp"

#include <boost/bind.hpp>


/*
./test --report_level=detailed --log_level=all --run_test=token_suite

 */

typedef n_token::t_token t_token;

BOOST_AUTO_TEST_SUITE( token_suite )

BOOST_AUTO_TEST_CASE( test_token1 ) {
	t_token z_one("one");
	t_token z_one2("one");
	t_token z_one3(z_one);
	t_token z_two("two");
	t_token z_three("three");

	BOOST_CHECK(t_token::z_empty() == "");
	BOOST_CHECK(t_token::z_empty() != "asdfsdaf");
	BOOST_CHECK(t_token::z_empty() == std::string(""));
	BOOST_CHECK(t_token::z_empty() != std::string("asdfsdaf"));

	BOOST_CHECK(z_one == "one");
	BOOST_CHECK(z_one != "asdfsdaf");
	BOOST_CHECK("one" == z_one);
	BOOST_CHECK("asdfsdaf" != z_one);

	BOOST_CHECK(z_one == z_one2);
	BOOST_CHECK(z_one2 == z_one);

	BOOST_CHECK(z_one == z_one3);
	BOOST_CHECK(z_one3 == z_one);

	BOOST_CHECK(z_one != z_two);
	BOOST_CHECK(z_two != z_one);

	BOOST_CHECK(t_token::z_empty().empty());
	BOOST_CHECK(!z_one.empty());

	t_token z_one_copy;
	BOOST_CHECK(z_one_copy == t_token::z_empty());
	BOOST_CHECK(z_one_copy.empty());
	z_one_copy = z_one2;
	BOOST_CHECK(z_one == z_one_copy);

}

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()

