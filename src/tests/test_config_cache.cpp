/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of thie Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth"

#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "config_cache.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "gettext.hpp"

#include "serialization/preprocessor.hpp"

BOOST_AUTO_TEST_SUITE( config_cache )

	/**
	 * Used to make distinct singleton for testing it
	 * because other tests will need original one to load data
	 **/
	class test_config_cache : public game_config::config_cache {
		test_config_cache() : game_config::config_cache() {}

		static test_config_cache cache_;

		public:
		static test_config_cache& instance() {
			return cache_;
		}

		std::string get_config_root() const {
			return game_config::config_cache::get_config_root();
		}
		std::string get_user_config_root() const {
			return game_config::config_cache::get_user_config_root();
		}

		const preproc_map& get_preproc_map() const {
			return game_config::config_cache::get_preproc_map();
		}
	};

test_config_cache test_config_cache::cache_;

static preproc_map settup_test_preproc_map()
{
	preproc_map defines_map;

#ifdef USE_TINY_GUI
	defines_map["TINY"] = preproc_define();
#endif

	if (game_config::small_gui)
		defines_map["SMALL_GUI"] = preproc_define();

#ifdef HAVE_PYTHON
	defines_map["PYTHON"] = preproc_define();
#endif

#if defined(__APPLE__)
	defines_map["APPLE"] = preproc_define();
#endif

	return defines_map;

}


BOOST_AUTO_TEST_CASE( test_config_cache_defaults )
{
	preproc_map defines_map(settup_test_preproc_map());
	test_config_cache& cache = test_config_cache::instance();

	BOOST_CHECK_EQUAL("data/", cache.get_config_root());
	BOOST_CHECK_EQUAL(get_addon_campaigns_dir(), cache.get_user_config_root());

	const preproc_map& test_defines = cache.get_preproc_map();
	BOOST_CHECK_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(), 
								 defines_map.begin() ,defines_map.end());
}

BOOST_AUTO_TEST_CASE( test_load_config )
{
	test_config_cache& cache = test_config_cache::instance();
	cache.add_define("TEST");
	
	preproc_map defines_map(settup_test_preproc_map());
	defines_map["TEST"] = preproc_define();
	const preproc_map& test_defines = cache.get_preproc_map();
	BOOST_CHECK_EQUAL_COLLECTIONS(test_defines.begin(),test_defines.end(), 
								 defines_map.begin() ,defines_map.end());
	
	std::string test_data_path("data/test/test/");
	cache.set_config_root(test_data_path);
	BOOST_CHECK_EQUAL(test_data_path, cache.get_config_root());
	test_data_path = "invalid";
	cache.set_user_config_root(test_data_path);
	BOOST_CHECK_EQUAL(test_data_path, cache.get_user_config_root());

	config test_config;
	config* child = &test_config.add_child("textdomain");
	(*child)["name"] = "wesnoth";

	child = &test_config.add_child("test_key");
	(*child)["define"] = "test";


	BOOST_CHECK_EQUAL(test_config, cache.get_config());

	cache.add_define("TEST_DEFINE");

	child = &test_config.add_child("test_key2");
	(*child)["define"] = t_string("testing translation reset.", GETTEXT_DOMAIN);
	

	BOOST_CHECK_EQUAL(test_config, cache.get_config());

	BOOST_CHECK_EQUAL((*test_config.child("test_key2"))["define"].str(), (*cache.get_config().child("test_key2"))["define"].str());
}

static bool match_german(const language_def& def)
{
	return def.localename == "de_DE";
}

BOOST_AUTO_TEST_CASE( test_translation_reload )
{
	test_config_cache& cache = test_config_cache::instance();
	config test_config;
	config* child = &test_config.add_child("textdomain");
	(*child)["name"] = "wesnoth";

	child = &test_config.add_child("test_key");
	(*child)["define"] = "test";

	child = &test_config.add_child("test_key2");
	(*child)["define"] = t_string("testing translation reset.", GETTEXT_DOMAIN);

	BOOST_CHECK_EQUAL((*test_config.child("test_key2"))["define"].str(), (*cache.get_config().child("test_key2"))["define"].str());

	// Change language
	const language_def& original_lang = get_language();
	const std::vector<language_def>& languages = get_languages();
	BOOST_CHECK_MESSAGE(languages.size()>0, "No languages found!");
	std::vector<language_def>::const_iterator German = std::find_if(languages.begin(),
									languages.end(),
									match_german); // Using German because the most active translation
	BOOST_REQUIRE_MESSAGE(German != languages.end() && German->available(), "German translation not found");
	::set_language(*German);
	cache.reload_translations();

	BOOST_CHECK_MESSAGE((*test_config.child("test_key2"))["define"].str() != (*cache.get_config().child("test_key2"))["define"].str(), "Translation reset failed to make test string different!");
	
	(*child)["define"].reset_translation();

	BOOST_CHECK_EQUAL(test_config, cache.get_config());
	BOOST_CHECK_EQUAL((*test_config.child("test_key2"))["define"].str(), (*cache.get_config().child("test_key2"))["define"].str());
	set_language(original_lang);
}

/* vim: set ts=4 sw=4: */
BOOST_AUTO_TEST_SUITE_END()

