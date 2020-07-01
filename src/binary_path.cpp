/*
   Copyright (C) 2020 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * File-IO
 */
#define GETTEXT_DOMAIN "wesnoth-lib"

#include "filesystem.hpp"
#include "binary_path.hpp"

#include "config.hpp"
#include "deprecation.hpp"
#include "game_config.hpp"
#include "game_version.hpp"
#include "gettext.hpp"
#include "log.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "game_config_view.hpp"


static lg::log_domain log_filesystem("filesystem");
#define DBG_FS LOG_STREAM(debug, log_filesystem)
#define LOG_FS LOG_STREAM(info, log_filesystem)
#define WRN_FS LOG_STREAM(warn, log_filesystem)
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace bfs = boost::filesystem;
using boost::system::error_code;


/**
 *  The paths manager is responsible for recording the various paths
 *  that binary files may be located at.
 *  It should be passed a config object which holds binary path information.
 *  This is in the format
 *@verbatim
 *    [binary_path]
 *      path=<path>
 *    [/binary_path]
 *  Binaries will be searched for in [wesnoth-path]/data/<path>/images/
 *@endverbatim
 */
namespace
{
std::set<wml_path> binary_paths;

typedef std::map<std::string, std::vector<wml_path>> paths_map;
paths_map binary_paths_cache;

} // namespace

static void init_binary_paths()
{
	if(binary_paths.empty()) {
		binary_paths.insert(wml_path::from_absolute(game_config::path + "/"));
	}
}

namespace filesystem {

binary_paths_manager::binary_paths_manager()
{
}

binary_paths_manager::binary_paths_manager(const game_config_view& cfg)
{
	set_paths(cfg);
}

binary_paths_manager::~binary_paths_manager()
{
	cleanup();
}

void binary_paths_manager::set_paths(const game_config_view& cfg)
{
	cleanup();
	init_binary_paths();

	for(const config& bp : cfg.child_range("binary_path")) {
		std::string path = bp["path"].str();
		if(path.find("..") != std::string::npos) {
			ERR_FS << "Invalid binary path '" << path << "'\n";
			continue;
		}

		if(!path.empty() && path.back() != '/') {
			path += "/";
		}
		if(binary_paths.count(wml_path(path)) == 0) {
			binary_paths.insert(wml_path(path));
		}
	}
}

void binary_paths_manager::cleanup()
{
	binary_paths_cache.clear();
	binary_paths.clear();
}

void clear_binary_paths_cache()
{
	binary_paths_cache.clear();
}

/**
 * Returns a vector with all possible paths to a given type of binary,
 * e.g. 'images', 'sounds', etc,
 */
const std::vector<wml_path>& get_binary_paths(const std::string& type)
{
	const paths_map::const_iterator itor = binary_paths_cache.find(type);
	if(itor != binary_paths_cache.end()) {
		return itor->second;
	}

	if(type.find("..") != std::string::npos) {
		// Not an assertion, as language.cpp is passing user data as type.
		ERR_FS << "Invalid WML type '" << type << "' for binary paths\n";
		static std::vector<wml_path> dummy;
		return dummy;
	}

	std::vector<wml_path>& res = binary_paths_cache[type];

	init_binary_paths();

	for(const wml_path& path : binary_paths) {
		res.push_back(path.append(type + "/"));
	}
	// not found in "/type" directory, try main directory, todo: why?
	//res.push_back(wml_path::from_absolutegame_config::path + "/./"));

	return res;
}

wml_path get_binary_file_location(const std::string& type, const std::string& filename)
{
	wml_path result;
	for(const wml_path& bp : get_binary_paths(type)) {
		wml_path new_path = bp.append(filename);

		DBG_FS << "  checking '" << new_path.safe_path() << "'\n";
		if(!is_legal_file(new_path.safe_path())){
			ERR_FS << "skipping invalid path " << new_path.safe_path() << "\n";
			continue;
		}

		if(new_path.exists()) {
			DBG_FS << "  found at '" << new_path.safe_path() << "'\n";
			if(result.safe_path().empty()) {
				result = new_path;
			} else {
				WRN_FS << "Conflicting files in binary_path: '" << new_path.safe_path()
					   << "' and '" << new_path.safe_path() << "'\n";
			}
		}
	}

	return result;
}

std::string get_binary_dir_location(const std::string& type, const std::string& filename)
{
	for(const wml_path& bp : get_binary_paths(type)) {

		wml_path new_path = bp.append(filename);
		if(!is_legal_file(new_path.safe_path())){
			ERR_FS << "skipping invalid path " << new_path.safe_path() << "\n";
			continue;
		}

		DBG_FS << "  checking '" << bp.safe_path() << "'\n";
		if(is_directory(new_path.get_abolute_path())) {
			DBG_FS << "  found at '" << new_path.safe_path() << "'\n";
			return new_path.get_abolute_path();
		}
	}

	DBG_FS << "  not found\n";
	return std::string();
}

std::string get_independent_image_path(const std::string& filename)
{
	wml_path path = get_binary_file_location("images", filename);
	return path.has_abolute_part() ? path.get_abolute_path() : path.safe_path();
}

} // namespace filesystem
