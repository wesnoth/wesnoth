/*
	Copyright (C) 2017 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "filesystem.hpp"
#include "wesconfig.h"

#include "config.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"

#include <boost/algorithm/string.hpp>

static lg::log_domain log_filesystem("filesystem");
#define LOG_FS LOG_STREAM(info, log_filesystem)
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace filesystem
{

bool is_legal_user_file_name(const std::string& name, bool allow_whitespace)
{
	//
	// IMPORTANT NOTE:
	//
	// If you modify this function you must be aware that it is used by the
	// add-on server validation routines both on the client and server sides.
	// The addition or removal of any criteria here should be carefully
	// evaluated with this in mind.
	//

	if(name.empty() || name.back() == '.' || name.find("..") != std::string::npos || name.size() > 255) {
		return false;
	}

	// Reserved DOS device names on Windows.
	static const std::set<std::string> dos_device_names = {
		// Hardware devices
		"NUL", "CON", "AUX", "PRN",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
		// Console API pseudo-devices
		"CONIN$", "CONOUT$",
	};

	// We can't use filesystem::base_name() here, because it returns the
	// filename up to the *last* dot. "CON.foo.bar" is still redirected to
	// "CON" on Windows, although "foo.CON.bar" and "foo.bar.CON" are not.
	//
	// Do also note that we're relying on the char-by-char check further below
	// to flag the name as illegal if it contains a colon ':', the reason
	// being that is valid to refer to DOS device names with a trailing colon
	// (e.g. "CON:" is synonymous with "CON").

	const auto& first_name =
		boost::algorithm::to_upper_copy(name.substr(0, name.find('.')), std::locale::classic());

	if(dos_device_names.count(first_name)) {
		return false;
	}

	const auto& name_ucs4 = unicode_cast<std::u32string>(name);
	if(name != unicode_cast<std::string>(name_ucs4)){
		return false; // name is an invalid UTF-8 sequence
	}

	return name_ucs4.end() == std::find_if(name_ucs4.begin(), name_ucs4.end(), [=](char32_t c)
	{
		switch(c) {
			case ' ':
				return !allow_whitespace;
			case '"':
			case '*':
			case '/':
			case ':':
			case '<':
			case '>':
			case '?':
			case '\\':
			case '|':
			case '~':
			case 0x7F: // DEL
				return true;
			default:
				return c < 0x20 ||                  // C0 control characters
					   (c >= 0x80 && c < 0xA0) ||   // C1 control characters
					   (c >= 0xD800 && c < 0xE000); // surrogate pairs
		}
	});
}

void blacklist_pattern_list::remove_blacklisted_files_and_dirs(std::vector<std::string>& files, std::vector<std::string>& directories) const
{
	files.erase(
		std::remove_if(files.begin(), files.end(), [this](const std::string& name) { return match_file(name); }),
		files.end());
	directories.erase(
		std::remove_if(directories.begin(), directories.end(), [this](const std::string& name) { return match_dir(name); }),
		directories.end());
}

bool blacklist_pattern_list::match_file(const std::string& name) const
{
	return std::any_of(file_patterns_.begin(), file_patterns_.end(),
					   std::bind(&utils::wildcard_string_match, std::ref(name), std::placeholders::_1));
}

bool blacklist_pattern_list::match_dir(const std::string& name) const
{
	return std::any_of(directory_patterns_.begin(), directory_patterns_.end(),
					   std::bind(&utils::wildcard_string_match, std::ref(name), std::placeholders::_1));
}

std::string autodetect_game_data_dir(std::string exe_dir)
{
	std::string auto_dir;

	// scons leaves the resulting binaries at the root of the source
	// tree by default.
	if(filesystem::file_exists(exe_dir + "/data/_main.cfg")) {
		auto_dir = std::move(exe_dir);
	}
	// cmake encourages creating a subdir at the root of the source
	// tree for the build, and the resulting binaries are found in it.
	else if(filesystem::file_exists(exe_dir + "/../data/_main.cfg")) {
		auto_dir = filesystem::normalize_path(exe_dir + "/..");
	}
	// Allow using the current working directory as the game data dir
	else if(filesystem::file_exists(filesystem::get_cwd() + "/data/_main.cfg")) {
		auto_dir = filesystem::get_cwd();
	}
#ifdef _WIN32
	// In Windows builds made using Visual Studio and its CMake
	// integration, the EXE is placed a few levels below the game data
	// dir (e.g. .\out\build\x64-Debug).
	else if(filesystem::file_exists(exe_dir + "/../../build") && filesystem::file_exists(exe_dir + "/../../../out")
		&& filesystem::file_exists(exe_dir + "/../../../data/_main.cfg")) {
		auto_dir = filesystem::normalize_path(exe_dir + "/../../..");
	}
#endif

	return auto_dir;
}

std::string get_synced_prefs_file()
{
	return get_sync_dir() + "/preferences";
}

std::string get_unsynced_prefs_file()
{
	return get_user_data_dir() + "/preferences";
}

std::string get_credentials_file()
{
	return get_user_data_dir() + "/credentials-aes";
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

std::string get_lua_history_file()
{
	return get_sync_dir() + "/lua_command_history";
}

std::string get_sync_dir()
{
	return get_user_data_dir() + "/sync";
}

std::string get_saves_dir()
{
	const std::string dir_path = get_sync_dir() + "/saves";
	return get_dir(dir_path);
}

std::string get_addons_data_dir()
{
	const std::string dir_path = get_user_data_dir() + "/data";
	return get_dir(dir_path);
}

std::string get_addons_dir()
{
	const std::string dir_path = get_addons_data_dir() + "/add-ons";
	return get_dir(dir_path);
}

std::string get_wml_persist_dir()
{
	const std::string dir_path = get_sync_dir() + "/persist";
	return get_dir(dir_path);
}

std::string get_legacy_editor_dir()
{
	const std::string dir_path = get_sync_dir() + "/editor";
	return get_dir(dir_path);
}

std::string get_current_editor_dir(const std::string& addon_id)
{
	if(addon_id == "mainline") {
		return get_dir(game_config::path) + "/data/multiplayer";
	} else {
		return get_addons_dir() + "/" + addon_id;
	}
}

std::string get_core_images_dir()
{
	return get_dir(game_config::path + "/data/core/images");
}

std::string get_intl_dir()
{
#ifdef _WIN32
	return game_config::path + "/" LOCALEDIR;
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

std::string read_map(const std::string& name)
{
	std::string res;
	auto map_location = get_wml_location(name);
	if(!map_location) {
		// Consult [binary_path] for maps as well.
		map_location = get_binary_file_location("maps", name);
	}
	if(map_location) {
		res = read_file(map_location.value());
	}

	if(res.empty()) {
		res = read_file(get_user_data_dir() + "/editor/maps/" + name);
	}

	return res;
}

std::string read_scenario(const std::string& name)
{
	std::string res;
	auto file_location = get_wml_location(name);
	if(!file_location) {
		// Consult [binary_path] for scenarios as well.
		file_location = get_binary_file_location("scenarios", name);
	}
	if(file_location) {
		res = read_file(file_location.value());
	}

	if(res.empty()) {
		res = read_file(get_user_data_dir() + "/editor/scenarios/" + name);
	}

	return res;
}

static void get_file_tree_checksum_internal(const std::string& path, file_tree_checksum& res)
{

	std::vector<std::string> dirs;
	get_files_in_dir(path,nullptr,&dirs, name_mode::ENTIRE_FILE_PATH, filter_mode::SKIP_MEDIA_DIR, reorder_mode::DONT_REORDER, &res);

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
			   << checksum.sum_size << " bytes";
	}

	return checksum;
}

}
