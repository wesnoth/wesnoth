/*
	Copyright (C) 2015 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <boost/test/unit_test.hpp>

#include "config_cache.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"

#if 0
namespace {

template<typename T>
void dump(const T& v)
{
	for(typename T::const_iterator k = v.begin(); k != v.end(); ++k) {
		PLAIN_LOG << " * " << *k;
	}
}

}
#endif

BOOST_AUTO_TEST_SUITE( filesystem ); // implicit namespace filesystem

const std::string& gamedata = game_config::path;

BOOST_AUTO_TEST_CASE( test_fs_game_path_reverse_engineering )
{
	const std::string maincfg = "_main.cfg";

	std::string gamedata_rev = get_wml_location("_main.cfg").value();

	const std::size_t strip_len = (maincfg + "/data/").length();
	BOOST_REQUIRE(gamedata_rev.length() > strip_len);
	gamedata_rev.resize(gamedata_rev.length() - strip_len);

	BOOST_CHECK_EQUAL( gamedata_rev, gamedata );
}

BOOST_AUTO_TEST_CASE( test_fs_base )
{
	BOOST_CHECK( is_root("/") );
	BOOST_CHECK( is_root("////") );
	BOOST_CHECK( is_root("/../") );
	BOOST_CHECK( is_root("/../.././") );
	BOOST_CHECK( is_root("/.././../.") );
	BOOST_CHECK( is_root("/.") );

	BOOST_CHECK( is_relative(".") );
	BOOST_CHECK( is_relative("..") );
	BOOST_CHECK( is_relative("../foo") );
	BOOST_CHECK( is_relative("foo") );
	BOOST_CHECK( !is_relative("/./../foo/..") );
	BOOST_CHECK( !is_relative("/foo/..") );
	BOOST_CHECK( !is_relative("/foo") );
	BOOST_CHECK( !is_relative("///foo") );

	BOOST_CHECK( is_directory("/") );
	BOOST_CHECK( is_directory("/.") );
	BOOST_CHECK( is_directory("/./././.") );
	BOOST_CHECK( is_directory("/..") );

	BOOST_CHECK( is_directory(".") );
	BOOST_CHECK( is_directory("..") );
	BOOST_CHECK( is_directory("./././.") );

	BOOST_CHECK( is_directory(gamedata + "/data/core/../../data/././../data/././core") );

	BOOST_CHECK( file_exists("/") );
	BOOST_CHECK( file_exists("/.") );
	BOOST_CHECK( file_exists("/./././.") );
	BOOST_CHECK( file_exists("/..") );

	BOOST_CHECK_EQUAL( base_name("foo/bar/baz.cfg"), "baz.cfg" );
	// FIXME: BFS gives "." for this, Unix basename gives "bar"!
	//BOOST_CHECK_EQUAL( base_name("foo/bar/"), "bar" );
	BOOST_CHECK_EQUAL( base_name("foo/bar"), "bar" );
	BOOST_CHECK_EQUAL( base_name("/"), "/" );
	BOOST_CHECK_EQUAL( base_name(""), "" );

	BOOST_CHECK_EQUAL( directory_name("foo/bar/baz.cfg"), "foo/bar" );
	BOOST_CHECK_EQUAL( directory_name("foo/bar/"), "foo/bar" );
	BOOST_CHECK_EQUAL( directory_name("foo/bar"), "foo" );
	BOOST_CHECK_EQUAL( directory_name("/"), "" );
	BOOST_CHECK_EQUAL( directory_name(""), "" );

	// TODO normalize_path

	//BOOST_CHECK_EQUAL( normalize_path(gamedata + "/data/core/../../data/././../data/././core"),
	//                   gamedata + "/data/core" );
}

BOOST_AUTO_TEST_CASE( test_fs_enum )
{
	const std::string path = "data/test/test/filesystem/enum";

	const std::vector<std::string> expected_filenames {
		"_initial.cfg",
		"A1.cfg",
		"A2.cfg",
		"A3.cfg",
		"B1.cfg",
		"B2.cfg",
		"B3.cfg",
		"_final.cfg"};
	const std::vector<std::string> expected_dirnames {
		"D1",
		"D2",
		"D3"};

	std::vector<std::string> files, dirs;
	std::vector<std::string> expected_filepaths, expected_dirpaths;

	for(const std::string& n : expected_filenames) {
		expected_filepaths.push_back(gamedata + "/" + path + "/" + n);
	}

	for(const std::string& n : expected_dirnames) {
		expected_dirpaths.push_back(gamedata + "/" + path + "/" + n);
	}

	// FIXME: get_files_in_dir with mode == FILE_NAME_ONLY will fail to reorder
	//        entries because the sorting code looks for forward slashes.
	//        This affects both the BFS-based and legacy implementations.
	get_files_in_dir(path, &files, &dirs, name_mode::ENTIRE_FILE_PATH, filter_mode::NO_FILTER, reorder_mode::DO_REORDER);

	BOOST_CHECK( files == expected_filepaths );
	BOOST_CHECK( dirs  == expected_dirpaths  );
}

BOOST_AUTO_TEST_CASE( test_fs_binary_path )
{
	config main_config;
	game_config_view game_config_view_ = game_config_view::wrap(main_config);
	game_config::config_cache& cache = game_config::config_cache::instance();

	cache.clear_defines();
	cache.add_define("EDITOR");
	cache.add_define("MULTIPLAYER");
	cache.get_config(game_config::path +"/data", main_config);

	const filesystem::binary_paths_manager bin_paths_manager(game_config_view_);

	//load_language_list();
	game_config::load_config(main_config.mandatory_child("game_config"));

	BOOST_CHECK_EQUAL( get_binary_dir_location("images", ".").value(), gamedata + "/images/." );

	BOOST_CHECK_EQUAL( get_binary_file_location("images", "wesnoth-icon.png").value(),
	                   gamedata + "/data/core/images/wesnoth-icon.png" );

	BOOST_CHECK_EQUAL( get_binary_file_location("music", "silence.ogg").value(),
	                   gamedata + "/data/core/music/silence.ogg" );

	BOOST_CHECK_EQUAL( get_binary_file_location("sounds", "explosion.ogg").value(),
	                   gamedata + "/data/core/sounds/explosion.ogg" );

	BOOST_CHECK_EQUAL( get_independent_binary_file_path("images", "wesnoth-icon.png").value(),
	                   "data/core/images/wesnoth-icon.png" );

	// Inexistent paths are resolved empty.
	BOOST_CHECK( !get_binary_dir_location("images", "").has_value() );
	BOOST_CHECK( !get_binary_dir_location("inexistent_resource_type", "").has_value() );
	BOOST_CHECK( !get_binary_file_location("image", "wesnoth-icon.png").has_value() );
	BOOST_CHECK( !get_binary_file_location("images", "bunnies_and_ponies_and_rainbows_oh_em_gee.psd").has_value() );
	BOOST_CHECK( !get_binary_file_location("music", "this_track_does_not_exist.aiff").has_value() );
	BOOST_CHECK( !get_binary_file_location("sounds", "rude_noises.aiff").has_value() );
	BOOST_CHECK( !get_independent_binary_file_path("images", "dopefish.txt").has_value() );
}

BOOST_AUTO_TEST_CASE( test_fs_wml_path )
{
	const std::string& userdata = get_user_data_dir();

	BOOST_CHECK_EQUAL( get_wml_location("").value_or(""), "" );

	BOOST_CHECK_EQUAL( get_wml_location("_main.cfg").value_or(""), gamedata + "/data/_main.cfg" );
	BOOST_CHECK_EQUAL( get_wml_location("core/_main.cfg").value_or(""), gamedata + "/data/core/_main.cfg" );
	BOOST_CHECK_EQUAL( get_wml_location(".", std::string("")).value_or(""), "." );

	BOOST_CHECK_EQUAL( get_wml_location("~/").value_or(""), userdata + "/data/" );

	// Inexistent paths are resolved empty.
	BOOST_CHECK( !get_wml_location("why_would_anyone_ever_name_a_file_like_this").has_value() );
}

BOOST_AUTO_TEST_CASE( test_fs_search )
{
	const std::string& userdata = get_user_data_dir();

	BOOST_CHECK_EQUAL( nearest_extant_parent(userdata + "/THIS_DOES_NOT_EXIST/foo/bar"), userdata );

	BOOST_CHECK_EQUAL( nearest_extant_parent(gamedata + "/THIS_DOES_NOT_EXIST_EITHER/foo"), gamedata );
	BOOST_CHECK_EQUAL( nearest_extant_parent(gamedata + "/data/_main.cfg"), gamedata + "/data" );
	BOOST_CHECK_EQUAL( nearest_extant_parent(gamedata + "/data/core/THIS_DOES_NOT_EXIST/test"), gamedata + "/data/core" );

	BOOST_CHECK_EQUAL( nearest_extant_parent("/THIS_HOPEFULLY_DOES_NOT_EXIST"), "/" );
	BOOST_CHECK_EQUAL( nearest_extant_parent("/THIS_HOPEFULLY_DOES_NOT_EXIST/foo/bar"), "/" );
	BOOST_CHECK_EQUAL( nearest_extant_parent("/THIS_HOPEFULLY_DOES_NOT_EXIST/foo/bar/.."), "/" );
}

BOOST_AUTO_TEST_CASE( test_fs_fluff )
{
	BOOST_CHECK( looks_like_pbl("foo.pbl") );
	BOOST_CHECK( looks_like_pbl("FOO.PBL") );
	BOOST_CHECK( looks_like_pbl("Foo.Pbl") );
	BOOST_CHECK( !looks_like_pbl("foo.pbl.cfg") );

	BOOST_CHECK( is_gzip_file("foo.gz") );
	BOOST_CHECK( !is_gzip_file("foo.gz.bz2") );
	BOOST_CHECK( is_bzip2_file("foo.bz2") );
	BOOST_CHECK( !is_bzip2_file("foo.bz2.gz") );

	BOOST_CHECK( is_compressed_file("foo.gz") );
	BOOST_CHECK( is_compressed_file("foo.bz2") );
	BOOST_CHECK( !is_compressed_file("foo.txt") );

	// FIXME: Is this even intended?
	BOOST_CHECK( !is_gzip_file("foo.GZ") );
	BOOST_CHECK( !is_bzip2_file("foo.BZ2") );
	BOOST_CHECK( !is_compressed_file("foo.GZ") );
	BOOST_CHECK( !is_compressed_file("foo.BZ2") );
}

BOOST_AUTO_TEST_SUITE_END()
