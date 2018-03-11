/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2015 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "addon/manager.hpp"
#include "addon/manager_ui.hpp"
#include "filesystem.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon/connect.hpp"
#include "gui/dialogs/addon/manager.hpp"
#include "gui/dialogs/addon/uninstall_list.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "version.hpp"
#include "serialization/string_utils.hpp"
#include "addon/client.hpp"

#include <boost/algorithm/string.hpp>

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)
#define LOG_CFG LOG_STREAM(info, log_config)
#define WRN_CFG LOG_STREAM(warn, log_config)

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS  LOG_STREAM(err , log_filesystem)

static lg::log_domain log_network("network");
#define ERR_NET LOG_STREAM(err , log_network)
#define LOG_NET LOG_STREAM(info, log_network)

namespace {
	std::string get_pbl_file_path(const std::string& addon_name)
	{
		const std::string& parentd = filesystem::get_addons_dir();
		// Allow .pbl files directly in the addon dir
		const std::string exterior = parentd + "/" + addon_name + ".pbl";
		const std::string interior = parentd + "/" + addon_name + "/_server.pbl";
		return filesystem::file_exists(exterior) ? exterior : interior;
	}

	inline std::string get_info_file_path(const std::string& addon_name)
	{
		return filesystem::get_addons_dir() + "/" + addon_name + "/_info.cfg";
	}
}

bool have_addon_in_vcs_tree(const std::string& addon_name)
{
	static const std::string parentd = filesystem::get_addons_dir();
	return
		filesystem::file_exists(parentd+"/"+addon_name+"/.svn") ||
		filesystem::file_exists(parentd+"/"+addon_name+"/.git") ||
		filesystem::file_exists(parentd+"/"+addon_name+"/.hg");
}

bool have_addon_pbl_info(const std::string& addon_name)
{
	return filesystem::file_exists(get_pbl_file_path(addon_name));
}

config get_addon_pbl_info(const std::string& addon_name)
{
	config cfg;
	const std::string& pbl_path = get_pbl_file_path(addon_name);
	try {
		filesystem::scoped_istream stream = filesystem::istream_file(pbl_path);
		read(cfg, *stream);
	} catch(const config::error& e) {
		throw invalid_pbl_exception(pbl_path, e.message);
	}

	return cfg;
}

void set_addon_pbl_info(const std::string& addon_name, const config& cfg)
{
	filesystem::scoped_ostream stream = filesystem::ostream_file(get_pbl_file_path(addon_name));
	write(*stream, cfg);
}

bool have_addon_install_info(const std::string& addon_name)
{
	return filesystem::file_exists(get_info_file_path(addon_name));
}

void get_addon_install_info(const std::string& addon_name, config& cfg)
{
	const std::string& info_path = get_info_file_path(addon_name);
	filesystem::scoped_istream stream = filesystem::istream_file(info_path);
	try {
		read(cfg, *stream);
	} catch(const config::error& e) {
		ERR_CFG << "Failed to read add-on installation information for '"
				<< addon_name << "' from " << info_path << ":\n"
				<< e.message << std::endl;
	}
}

bool remove_local_addon(const std::string& addon)
{
	const std::string addon_dir = filesystem::get_addons_dir() + "/" + addon;

	LOG_CFG << "removing local add-on: " << addon << '\n';

	if(filesystem::file_exists(addon_dir) && !filesystem::delete_directory(addon_dir, true)) {
		ERR_CFG << "Failed to delete directory/file: " << addon_dir << '\n';
		ERR_CFG << "removal of add-on " << addon << " failed!" << std::endl;
		return false;
	}
	return true;
}

std::vector<std::string> available_addons()
{
	std::vector<std::string> res;
	std::vector<std::string> files, dirs;
	const std::string parentd = filesystem::get_addons_dir();
	filesystem::get_files_in_dir(parentd,&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		if (filesystem::file_exists(parentd + "/" + *i + "/_main.cfg") && have_addon_pbl_info(*i)) {
			res.push_back(*i);
		}
	}

	return res;
}

std::vector<std::string> installed_addons()
{
	std::vector<std::string> res;
	const std::string parentd = filesystem::get_addons_dir();
	std::vector<std::string> files, dirs;
	filesystem::get_files_in_dir(parentd,&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		if(filesystem::file_exists(parentd + "/" + *i + "/_main.cfg")) {
			res.push_back(*i);
		}
	}

	return res;
}

bool is_addon_installed(const std::string& addon_name)
{
	const std::string namestem = filesystem::get_addons_dir() + "/" + addon_name;
	return filesystem::file_exists(namestem + "/_main.cfg");
}

static inline bool IsCR(const char& c)
{
	return c == '\x0D';
}

static std::string strip_cr(std::string str, bool strip)
{
	if(!strip)
		return str;
	std::string::iterator new_end = std::remove_if(str.begin(), str.end(), IsCR);
	str.erase(new_end, str.end());
	return str;
}

namespace {
	void append_default_ignore_patterns(std::pair<std::vector<std::string>, std::vector<std::string>>& patterns)
	{
		std::vector<std::string>& files = patterns.first;
		std::vector<std::string>& dirs  = patterns.second;

		static const std::vector<std::string> default_ignored_files {
			/* Don't upload dot-files/dirs, which are hidden files in UNIX platforms */
			".*",
			"#*#",
			"*~",
			"*-bak",
			"*.swp",
			"*.pbl",
			"*.ign",
			"_info.cfg",
			"*.exe",
			"*.bat",
			"*.cmd",
			"*.com",
			"*.scr",
			"*.sh",
			"*.js",
			"*.vbs",
			"*.o",
			"*.ini",
			/* Remove junk created by certain file manager ;) */
			"Thumbs.db",
			/* Eclipse plugin */
			"*.wesnoth",
			"*.project",
		};

		static const std::vector<std::string> default_ignored_dirs {
			".*",
			/* macOS metadata-like cruft (http://floatingsun.net/2007/02/07/whats-with-__macosx-in-zip-files/) */
			"__MACOSX",
		};

		files.insert(files.end(), default_ignored_files.begin(), default_ignored_files.end());
		dirs.insert(dirs.end(), default_ignored_dirs.begin(), default_ignored_dirs.end());
    }
}

static std::pair<std::vector<std::string>, std::vector<std::string>> read_ignore_patterns(const std::string& addon_name)
{
	const std::string parentd = filesystem::get_addons_dir();
	const std::string ign_file = parentd + "/" + addon_name + "/_server.ign";

	std::pair<std::vector<std::string>, std::vector<std::string>> patterns;
	LOG_CFG << "searching for .ign file for '" << addon_name << "'...\n";
	if (!filesystem::file_exists(ign_file)) {
		LOG_CFG << "no .ign file found for '" << addon_name << "'\n"
		        << "inserting default ignore patterns...\n";
		append_default_ignore_patterns(patterns);
		return patterns; // just default patterns
	}
	LOG_CFG << "found .ign file: " << ign_file << '\n';
	auto stream = filesystem::istream_file(ign_file);
	std::string line;
	while (std::getline(*stream, line)) {
		boost::trim(line);
		const size_t l = line.size();
		// .gitignore & WML like comments
		if (l == 0 || !line.compare(0,2,"# ")) continue;
		if (line[l - 1] == '/') { // directory; we strip the last /
			patterns.second.push_back(line.substr(0, l - 1));
		} else { // file
			patterns.first.push_back(line);
		}
	}
	return patterns;
}

static void archive_file(const std::string& path, const std::string& fname, config& cfg)
{
	cfg["name"] = fname;
	const bool is_cfg = (fname.size() > 4 ? (fname.substr(fname.size() - 4) == ".cfg") : false);
	cfg["contents"] = encode_binary(strip_cr(filesystem::read_file(path + '/' + fname),is_cfg));
}

static void archive_dir(const std::string& path, const std::string& dirname, config& cfg, std::pair<std::vector<std::string>, std::vector<std::string>>& ignore_patterns)
{
	cfg["name"] = dirname;
	const std::string dir = path + '/' + dirname;

	std::vector<std::string> files, dirs;
	filesystem::get_files_in_dir(dir,&files,&dirs);
	for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		bool valid = !filesystem::looks_like_pbl(*i);
		for(std::vector<std::string>::const_iterator p = ignore_patterns.first.begin(); p != ignore_patterns.first.end(); ++p) {
			if (utils::wildcard_string_match(*i, *p)) {
				valid = false;
				break;
			}
		}
		if (valid) {
			archive_file(dir,*i,cfg.add_child("file"));
		}
	}

	for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
		bool valid = true;
		for(std::vector<std::string>::const_iterator p = ignore_patterns.second.begin(); p != ignore_patterns.second.end(); ++p) {
			if (utils::wildcard_string_match(*j, *p)) {
				valid = false;
				break;
			}
		}
		if (valid) {
			archive_dir(dir,*j,cfg.add_child("dir"),ignore_patterns);
		}
	}
}

void archive_addon(const std::string& addon_name, config& cfg)
{
	const std::string parentd = filesystem::get_addons_dir();

	std::pair<std::vector<std::string>, std::vector<std::string>> ignore_patterns;
	ignore_patterns = read_ignore_patterns(addon_name);
	archive_dir(parentd, addon_name, cfg.add_child("dir"), ignore_patterns);
}

static void unarchive_file(const std::string& path, const config& cfg)
{
	filesystem::write_file(path + '/' + cfg["name"].str(), unencode_binary(cfg["contents"]));
}

static void unarchive_dir(const std::string& path, const config& cfg)
{
	std::string dir;
	if (cfg["name"].empty())
		dir = path;
	else
		dir = path + '/' + cfg["name"].str();

	filesystem::make_directory(dir);

	for(const config &d : cfg.child_range("dir")) {
		unarchive_dir(dir, d);
	}

	for(const config &f : cfg.child_range("file")) {
		unarchive_file(dir, f);
	}
}

void unarchive_addon(const config& cfg)
{
	const std::string parentd = filesystem::get_addons_dir();
	unarchive_dir(parentd, cfg);
}

namespace {
	std::map< std::string, version_info > version_info_cache;
} // end unnamed namespace 5

void refresh_addon_version_info_cache()
{
	version_info_cache.clear();

	LOG_CFG << "refreshing add-on versions cache\n";

	const std::vector<std::string>& addons = installed_addons();
	if(addons.empty()) {
		return;
	}

	std::vector<std::string> addon_info_files(addons.size());

	std::transform(addons.begin(), addons.end(),
	               addon_info_files.begin(), get_info_file_path);

	for(size_t i = 0; i < addon_info_files.size(); ++i) {
		assert(i < addons.size());

		const std::string& addon = addons[i];
		const std::string& info_file = addon_info_files[i];

		if(filesystem::file_exists(info_file)) {
			config cfg;
			get_addon_install_info(addon, cfg);

			const config& info_cfg = cfg.child("info");
			if(!info_cfg) {
				continue;
			}

			const std::string& version = info_cfg["version"].str();
			LOG_CFG << "cached add-on version: " << addon << " [" << version << "]\n";

			version_info_cache[addon] = version;
		} else if (!have_addon_pbl_info(addon) && !have_addon_in_vcs_tree(addon)) {
			// Don't print the warning if the user is clearly the author
			WRN_CFG << "add-on '" << addon << "' has no _info.cfg; cannot read version info" << std::endl;
		}
	}
}

version_info get_addon_version_info(const std::string& addon)
{
	static const version_info nil;
	std::map< std::string, version_info >::iterator entry = version_info_cache.find(addon);
	return entry != version_info_cache.end() ? entry->second : nil;
}
