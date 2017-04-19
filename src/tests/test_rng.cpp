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
#include "random_new_synced.hpp"
#include "random_new_deterministic.hpp"
#include "config.hpp"
#include <sstream>
#include <iomanip>

BOOST_AUTO_TEST_SUITE( rng )

/* this test adapted from validation routine at
   http://www.boost.org/doc/libs/1_38_0/libs/random/random_test.cpp
*/
BOOST_AUTO_TEST_CASE( validate_mt19937 )
{
	std::mt19937 rng;
	for (int i = 0; i < 9999 ; i++) {
		rng();
	}
	unsigned long val = rng();
	BOOST_CHECK_EQUAL( val , 4123659995U );
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

BOOST_AUTO_TEST_CASE( test_mt_rng_reproducibility )
{
	config cfg;
	cfg["random_seed"] = "5eedf00d";
	cfg["random_calls"] = 0;

	rand_rng::mt_rng rng1(cfg);
	rand_rng::mt_rng rng2(cfg);

	BOOST_CHECK(rng1 == rng2);
	for (int i = 0; i < 10 ; i++) {
		BOOST_CHECK(rng1.get_next_random() == rng2.get_next_random());
	}
}

BOOST_AUTO_TEST_CASE( test_mt_rng_reproducibility2 )
{
	config cfg;
	cfg["random_seed"] = "18da5eed";
	cfg["random_calls"] = 9999;

	rand_rng::mt_rng rng1(cfg);
	rand_rng::mt_rng rng2(cfg);

	BOOST_CHECK(rng1 == rng2);
	for (int i = 0; i < 10 ; i++) {
		BOOST_CHECK(rng1.get_next_random() == rng2.get_next_random());
	}
}

BOOST_AUTO_TEST_CASE( test_mt_rng_reproducibility3 )
{
	rand_rng::mt_rng rng1;
	config cfg;
	cfg["random_seed"] = rng1.get_random_seed_str();
	cfg["random_calls"] = rng1.get_random_calls();

	rand_rng::mt_rng rng2(cfg);

	BOOST_CHECK(rng1 == rng2);
	for (int i = 0; i < 10 ; i++) {
		BOOST_CHECK(rng1.get_next_random() == rng2.get_next_random());
	}
}

BOOST_AUTO_TEST_CASE( test_mt_rng_reproducibility4 )
{
	rand_rng::mt_rng rng1;

	for (int i = 0; i < 5; i++) {
		rng1.get_next_random();
	}

	config cfg;
	cfg["random_seed"] = rng1.get_random_seed_str();
	cfg["random_calls"] = rng1.get_random_calls();

	rand_rng::mt_rng rng2(cfg);

	BOOST_CHECK(rng1 == rng2);
	BOOST_CHECK(rng1.get_next_random() == rng2.get_next_random());
}

BOOST_AUTO_TEST_CASE( test_mt_rng_reproducibility5 )
{
	config cfg;
	cfg["random_seed"] = "5eedc0de";
	cfg["random_calls"] = 0;

	rand_rng::mt_rng rng(cfg);

	for (int i = 0; i < 9999 ; i++) {
		rng.get_next_random();
	}

	config cfg2;
	cfg2["random_seed"] = rng.get_random_seed_str();
	cfg2["random_calls"] = rng.get_random_calls();

	rand_rng::mt_rng rng2(cfg2);

	uint32_t result1 = rng.get_next_random();
	uint32_t result2 = rng2.get_next_random();

	BOOST_CHECK (rng == rng2);
	BOOST_CHECK (rng.get_random_seed_str() == rng2.get_random_seed_str());
	BOOST_CHECK (rng.get_random_calls() == rng2.get_random_calls());
	BOOST_CHECK (result1 == result2);

	config cfg_save;
	cfg_save["random_seed"] = rng.get_random_seed_str();
	cfg_save["random_calls"] = rng.get_random_calls();

	uint32_t result3 = rng.get_next_random();

	rand_rng::mt_rng rng3(cfg_save);
	uint32_t result4 = rng3.get_next_random();

	BOOST_CHECK (rng == rng3);
	BOOST_CHECK (rng.get_random_seed_str() == rng3.get_random_seed_str());
	BOOST_CHECK (rng.get_random_calls() == rng3.get_random_calls());
	BOOST_CHECK (result3 == result4);
}

namespace {

void validate_seed_string(std::string seed_str)
{
	config cfg;
	cfg["random_seed"] = seed_str;
	cfg["random_calls"] = 0;

	rand_rng::mt_rng rng1(cfg);

	for (int i = 0; i < 9999 ; i++) {
		rng1.get_next_random();
	}

	config cfg2;
	cfg2["random_seed"] = rng1.get_random_seed_str();
	cfg2["random_calls"] = rng1.get_random_calls();

	rand_rng::mt_rng rng2(cfg2);

	for (int i = 0; i < 9999 ; i++) {
		rng1.get_next_random();
		rng2.get_next_random();
	}

	BOOST_CHECK(rng1 == rng2);
	BOOST_CHECK(rng1.get_next_random() == rng2.get_next_random());

}

}

BOOST_AUTO_TEST_CASE( test_mt_rng_reproducibility_coverage )
{
	validate_seed_string("0000badd");
	validate_seed_string("00001234");
	validate_seed_string("deadbeef");
	validate_seed_string("12345678");
	validate_seed_string("00009999");
	validate_seed_string("ffffaaaa");
	validate_seed_string("11110000");
	validate_seed_string("10101010");
	validate_seed_string("aaaa0000");
}

namespace {

std::string validate_get_random_int_seed_generator()
{
	return "dada5eed";
}

}

#define validation_get_random_int_num_draws 19999

#define validation_get_random_int_max 32000

#define validation_get_random_int_correct_answer 10885

/**
 *  This test and the next validate that we are getting the correct values
 *  from the get_random_int function, in the class random_new.
 *  We test both subclasses of random_new.
 *  If these tests fail but the seed manipulation tests all pass,
 *  and validate_mt19937 passes, then it suggests that the implementation
 *  of get_random_int may not be working properly on your platform.
 */
BOOST_AUTO_TEST_CASE( validate_get_random_int )
{
	config cfg;
	cfg["random_seed"] = validate_get_random_int_seed_generator();
	cfg["random_calls"] = validation_get_random_int_num_draws;

	rand_rng::mt_rng mt_(cfg);

	std::shared_ptr<random_new::rng> gen_ (new random_new::rng_deterministic(mt_));

	int val = gen_->get_random_int(0, validation_get_random_int_max);
	BOOST_CHECK_EQUAL ( val , validation_get_random_int_correct_answer );
}

BOOST_AUTO_TEST_CASE( validate_get_random_int2 )
{
	std::shared_ptr<random_new::rng> gen_ (new random_new::synced_rng(validate_get_random_int_seed_generator));

	for (int i = 0; i < validation_get_random_int_num_draws; i++) {
		gen_->next_random();
	}

	int val = gen_->get_random_int(0,validation_get_random_int_max);
	BOOST_CHECK_EQUAL ( val , validation_get_random_int_correct_answer );
}


BOOST_AUTO_TEST_SUITE_END()
