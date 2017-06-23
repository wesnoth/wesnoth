/*
   Copyright (C) 2008 - 2017 by Pauli Nieminen <paniemin@cc.hut.fi>
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

#define GETTEXT_DOMAIN "wesnoth-test"


#include "config_cache.hpp"
#include "config.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "version.hpp"

#include "tests/utils/game_config_manager.hpp"

#include "utils/functional.hpp"


static preproc_map setup_test_preproc_map()
{
	preproc_map defines_map;

#if defined(__APPLE__)
	defines_map["APPLE"] = preproc_define();
#endif

	defines_map["WESNOTH_VERSION"] = preproc_define(game_config::wesnoth_version.str());

	return defines_map;

}


/**
 * Used to make distinct singleton for testing it
 * because other tests will need original one to load data
 **/
class test_config_cache : public game_config::config_cache {
	test_config_cache() : game_config::config_cache() {}

	public:
	static test_config_cache& instance() ;

	void set_force_invalid_cache(bool force)
	{
		game_config::config_cache::set_force_invalid_cache(force);
	}
};

test_config_cache & test_config_cache::instance() {
	static test_config_cache * cache_  = new test_config_cache;
	return *cache_;
}

/**
 * Used to redirect defines settings to test cache
 **/
typedef game_config::scoped_preproc_define_internal<test_config_cache> test_scoped_define;

struct config_cache_fixture {
	config_cache_fixture() : cache(test_config_cache::instance()), old_locale(get_language()), test_def("TEST")
	{
		test_utils::get_test_config_ref();
	}
	~config_cache_fixture()
	{
		set_language(old_locale);
	}
	test_config_cache& cache;
	language_def old_locale;
	test_scoped_define test_def;
};

BOOST_AUTO_TEST_CASE( test_preproc_defines )
{
	test_config_cache& cache = test_config_cache::instance();
	const preproc_map& test_defines = cache.get_preproc_map();
	preproc_map defines_map(setup_test_preproc_map());

	// check initial state
	BOOST_REQUIRE_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(),
			defines_map.begin() ,defines_map.end());

	// scoped
	{
		test_scoped_define test("TEST");
		defines_map["TEST"] = preproc_define();

		BOOST_CHECK_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(),
				defines_map.begin() ,defines_map.end());
		defines_map.erase("TEST");
	}
	// Check scoped remove

	BOOST_CHECK_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(),
			defines_map.begin() ,defines_map.end());

	// Manual add define
	cache.add_define("TEST");
	defines_map["TEST"] = preproc_define();
	BOOST_CHECK_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(),
			defines_map.begin() ,defines_map.end());

	// Manual remove define
	cache.remove_define("TEST");
	defines_map.erase("TEST");
	BOOST_CHECK_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(),
			defines_map.begin() ,defines_map.end());
}

BOOST_AUTO_TEST_CASE( test_config_cache_defaults )
{
	test_config_cache& cache = test_config_cache::instance();
	preproc_map defines_map(setup_test_preproc_map());

	const preproc_map& test_defines = cache.get_preproc_map();
	BOOST_CHECK_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(),
			defines_map.begin() ,defines_map.end());
}


BOOST_FIXTURE_TEST_SUITE( config_cache, config_cache_fixture )

	const std::string test_data_path("data/test/test/_main.cfg");

static config setup_test_config()
{
	config test_config;
	config* child = &test_config.add_child("textdomain");
	(*child)["name"] = GETTEXT_DOMAIN;

	child = &test_config.add_child("test_key");
	(*child)["define"] = "test";
	return test_config;
}


BOOST_AUTO_TEST_CASE( test_load_config )
{

	config test_config = setup_test_config();
	config cached_config;
	cache.get_config(test_data_path, cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);

	config &child = test_config.add_child("test_key2");
	child["define"] = t_string("testing translation reset.", GETTEXT_DOMAIN);


	test_scoped_define test_define_def("TEST_DEFINE");
	cached_config.clear();
	cache.get_config(test_data_path, cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);

	BOOST_CHECK_EQUAL(test_config.child("test_key2")["define"].str(), cached_config.child("test_key2")["define"].str());
}

BOOST_AUTO_TEST_CASE( test_non_clean_config_loading )
{

	config test_config = setup_test_config();

	// Test clean load first
	{
		config cfg;
		cache.get_config(test_data_path, cfg);
		BOOST_CHECK_EQUAL(test_config, cfg);
	}

	// test non-clean one then
	{
		config cfg;
		config &child = cfg.add_child("junk_data");
		child["some_junk"] = "hah";
		cache.get_config(test_data_path, cfg);
		BOOST_CHECK_EQUAL(test_config, cfg);
	}
}

BOOST_AUTO_TEST_CASE( test_macrosubstitution )
{
	config test_config = setup_test_config();

	config &child = test_config.add_child("test_key3");
	child["define"] = "transaction";
	config &child2 = test_config.add_child("test_key4");
	child2["defined"] = "parameter";

	// test first that macro loading works
	test_scoped_define macro("TEST_MACRO");

	// Without cache
	config cached_config;
	cache.get_config(test_data_path, cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);

	// With cache
	cached_config.clear();
	cache.get_config(test_data_path, cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);


}

BOOST_AUTO_TEST_CASE( test_transaction )
{
	config test_config = setup_test_config();

	config* child = &test_config.add_child("test_key3");
	(*child)["define"] = "transaction";
	child = &test_config.add_child("test_key4");
	(*child)["defined"] = "parameter";

	// test first that macro loading works
	test_scoped_define macro("TEST_MACRO");

	//Start transaction

	game_config::config_cache_transaction transaction;

	config cached_config;
	cache.get_config(test_data_path, cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);

	transaction.lock();
	config umc_config;
	child = &umc_config.add_child("umc");
	(*child)["test"] = "umc load";
	child = &umc_config.add_child("test_key3");
	(*child)["define"] = "transaction";
	child = &umc_config.add_child("test_key4");
	(*child)["defined"] = "parameter";
	cached_config.clear();
	cache.get_config("data/test/test/umc.cfg", cached_config);
	BOOST_CHECK_EQUAL(umc_config, cached_config);
}

BOOST_AUTO_TEST_CASE( test_define_loading )
{
	// try to load umc without valid cache
	config test_config = setup_test_config();

	config* child = &test_config.add_child("test_key3");
	(*child)["define"] = "transaction";
	child = &test_config.add_child("test_key4");
	(*child)["defined"] = "parameter";

	// test first that macro loading works
	test_scoped_define macro("TEST_MACRO");

	//Start transaction

	game_config::config_cache_transaction transaction;

	config cached_config;
	cache.get_config(test_data_path, cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);

	transaction.lock();

	cache.set_force_invalid_cache(true);
	config umc_config;
	child = &umc_config.add_child("umc");
	(*child)["test"] = "umc load";
	child = &umc_config.add_child("test_key3");
	(*child)["define"] = "transaction";
	child = &umc_config.add_child("test_key4");
	(*child)["defined"] = "parameter";
	cached_config.clear();
	cache.get_config("data/test/test/umc.cfg", cached_config);
	BOOST_CHECK_EQUAL(umc_config, cached_config);
	cache.set_force_invalid_cache(false);
}

BOOST_AUTO_TEST_CASE( test_lead_spaces_loading )
{
	config test_config;
	test_config.add_child("test_lead_space")["space"] = "empty char in middle";
	// Force reload of cache
	cache.set_force_invalid_cache(true);
	config cached_config;
	cache.get_config("data/test/test/leading_space.cfg", cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);
	cache.set_force_invalid_cache(false);
	cached_config.clear();
	cache.get_config("data/test/test/leading_space.cfg", cached_config);
	BOOST_CHECK_EQUAL(test_config, cached_config);
}

#if 0
// for profiling cache speed
BOOST_AUTO_TEST_CASE( test_performance )
{
	test_scoped_define mp("MULTIPLAYER");
	config cfg_ref;
//	cache.set_force_invalid_cache(true);
	cache.get_config("data/", cfg_ref);
//	cache.set_force_invalid_cache(false);
	for (int i=0; i < 3; ++i)
	{
		cache.get_config("data/");
	}
}
#endif

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()

