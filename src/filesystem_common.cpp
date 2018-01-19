/*
   Copyright (C) 2017-2018 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <fstream>

#include "filesystem.hpp"
#include "wesconfig.h"

#include "config.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"

static lg::log_domain log_filesystem("filesystem");
#define LOG_FS LOG_STREAM(info, log_filesystem)
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace filesystem
{

std::string get_prefs_file()
{
	return get_user_config_dir() + "/preferences";
}

std::string get_credentials_file()
{
	return get_user_config_dir() + "/credentials";
}

std::string get_default_prefs_file()
{
#ifdef HAS_RELATIVE_DEFPREF
	return game_config::path + "/" + game_config::default_preferences_path;
#else
	return game_config::default_preferences_path;
#endif
}

std::string get_save_index_file()
{
	return get_user_data_dir() + "/save_index";
}

std::string get_saves_dir()
{
	const std::string dir_path = get_user_data_dir() + "/saves";
	return get_dir(dir_path);
}

std::string get_addons_dir()
{
	const std::string dir_path = get_user_data_dir() + "/data/add-ons";
	return get_dir(dir_path);
}

std::string get_intl_dir()
{
#ifdef _WIN32
	return get_cwd() + "/translations";
#else

#ifdef USE_INTERNAL_DATA
	return get_cwd() + "/" LOCALEDIR;
#endif

#if HAS_RELATIVE_LOCALEDIR
	std::string res = game_config::path + "/" LOCALEDIR;
#else
	std::string res = LOCALEDIR;
#endif

	return res;
#endif
}

std::string get_screenshot_dir()
{
	const std::string dir_path = get_user_data_dir() + "/screenshots";
	return get_dir(dir_path);
}

bool looks_like_pbl(const std::string& file)
{
	return utils::wildcard_string_match(utf8::lowercase(file), "*.pbl");
}

file_tree_checksum::file_tree_checksum()
	: nfiles(0), sum_size(0), modified(0)
{}

file_tree_checksum::file_tree_checksum(const config& cfg) :
	nfiles	(cfg["nfiles"].to_size_t()),
	sum_size(cfg["size"].to_size_t()),
	modified(cfg["modified"].to_time_t())
{
}

void file_tree_checksum::write(config& cfg) const
{
	cfg["nfiles"] = nfiles;
	cfg["size"] = sum_size;
	cfg["modified"] = modified;
}

bool file_tree_checksum::operator==(const file_tree_checksum &rhs) const
{
	return nfiles == rhs.nfiles && sum_size == rhs.sum_size &&
		modified == rhs.modified;
}

bool ends_with(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && std::equal(suffix.begin(),suffix.end(),str.end()-suffix.size());
}

std::string read_map(const std::string& name)
{
	std::string res;
	std::string map_location = get_wml_location("maps/" + name);
	if(!map_location.empty()) {
		res = read_file(map_location);
	}

	if (res.empty()) {
		res = read_file(get_user_data_dir() + "/editor/maps/" + name);
	}

	return res;
}

static void get_file_tree_checksum_internal(const std::string& path, file_tree_checksum& res)
{

	std::vector<std::string> dirs;
	get_files_in_dir(path,nullptr,&dirs, ENTIRE_FILE_PATH, SKIP_MEDIA_DIR, DONT_REORDER, &res);

	for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
		get_file_tree_checksum_internal(*j,res);
	}
}

const file_tree_checksum& data_tree_checksum(bool reset)
{
	static file_tree_checksum checksum;
	if (reset)
		checksum.reset();
	if(checksum.nfiles == 0) {
		get_file_tree_checksum_internal("data/",checksum);
		get_file_tree_checksum_internal(get_user_data_dir() + "/data/",checksum);
		LOG_FS << "calculated data tree checksum: "
			   << checksum.nfiles << " files; "
			   << checksum.sum_size << " bytes" << std::endl;
	}

	return checksum;
}

}
