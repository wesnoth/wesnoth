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
#include "mt_rng.hpp"
#include "config.hpp"
#include <sstream>
#include <iomanip>

BOOST_AUTO_TEST_SUITE( rng )

/* this test adapted from validation routine at 
   http://www.boost.org/doc/libs/1_38_0/libs/random/random_test.cpp
*/
BOOST_AUTO_TEST_CASE( validate_mt19937 )
{
	boost::mt19937 rng;
	for (int i = 0; i < 9999 ; i++) {
		rng();
	}
	unsigned long val = rng();
	bool result = ( val == 4123659995U );
	BOOST_CHECK(result);
}

/* this test checks the soundness of mt_rng string manipulations */
BOOST_AUTO_TEST_CASE( test_mt_rng_seed_manip )
{
	uint32_t seed = 42;
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << seed;

	std::string seed_str = stream.str();

	rand_rng::mt_rng rng;
	rng.seed_random(seed_str);

	BOOST_CHECK (rng.get_random_seed() == seed);
	BOOST_CHECK (rng.get_random_seed_str() == seed_str);

	std::string seed_str2 = rng.get_random_seed_str();
	rng.seed_random(seed_str2);

	BOOST_CHECK (rng.get_random_seed() == seed);
	BOOST_CHECK (rng.get_random_seed_str() == seed_str);


	uint32_t seed3 = 1123581321; //try the same with a different number
	std::stringstream stream2;
	stream2 << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << seed3;
	std::string seed_str3 = stream2.str();

	rng.seed_random(seed_str3);
	BOOST_CHECK (rng.get_random_seed() == seed3);
	BOOST_CHECK (rng.get_random_seed_str() == seed_str3);

	std::string seed_str4 = rng.get_random_seed_str();
	rng.seed_random(seed_str4);

	BOOST_CHECK (rng.get_random_seed() == seed3);
	BOOST_CHECK (rng.get_random_seed_str() == seed_str3);


	//now check that the results that shouldn't match don't
	BOOST_CHECK (seed != seed3);
	BOOST_CHECK (seed_str != seed_str3);

}	

BOOST_AUTO_TEST_CASE( test_mt_rng_config_seed_manip )
{
	uint32_t seed = 42;
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << seed;
	std::string seed_str = stream.str();

	config cfg;
	cfg["random_seed"] = seed_str;
	cfg["random_calls"] = 0;

	rand_rng::mt_rng rng(cfg);

	BOOST_CHECK (rng.get_random_seed() == seed);
	BOOST_CHECK (rng.get_random_seed_str() == seed_str);

	std::string seed_str2 = rng.get_random_seed_str();
	rng.seed_random(seed_str2);

	BOOST_CHECK (rng.get_random_seed() == seed);
	BOOST_CHECK (rng.get_random_seed_str() == seed_str);


	uint32_t seed3 = 1123581321; //try the same with a different number
	std::stringstream stream2;
	stream2 << std::setfill('0') << std::setw(sizeof(uint32_t)*2) << std::hex << seed3;
	std::string seed_str3 = stream2.str();

	config cfg2;
	cfg2["random_seed"] = seed_str3;
	cfg2["random_calls"] = 0;

	rand_rng::mt_rng rng2(cfg2);

	BOOST_CHECK (rng2.get_random_seed() == seed3);
	BOOST_CHECK (rng2.get_random_seed_str() == seed_str3);

	std::string seed_str4 = rng2.get_random_seed_str();
	rng2.seed_random(seed_str4);

	BOOST_CHECK (rng2.get_random_seed() == seed3);
	BOOST_CHECK (rng2.get_random_seed_str() == seed_str3);


	//now check that the results that shouldn't match don't
	BOOST_CHECK (seed != seed3);
	BOOST_CHECK (seed_str != seed_str3);
}

BOOST_AUTO_TEST_SUITE_END();
