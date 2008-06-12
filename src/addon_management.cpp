/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "addon_management.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <cstring>

namespace {
	void setup_addon_dirs()
	{
		make_directory(get_user_data_dir() + "/data");
		make_directory(get_addon_campaigns_dir());
	}
}

void get_addon_info(const std::string& addon_name, config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();

	// Cope with old-style or new-style file organization 
	std::string exterior = parentd + "/" + addon_name + ".pbl";
	std::string interior = parentd + "/" + addon_name + "/_server.pbl";
	const std::string pbl_file = (file_exists(exterior)? exterior : interior);

	scoped_istream stream = istream_file(pbl_file);
	read(cfg, *stream);
}

void set_addon_info(const std::string& addon_name, const config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();

	scoped_ostream stream = ostream_file(parentd + "/" + addon_name + ".pbl");
	write(*stream, cfg);
}

void remove_local_addon(const std::string& addon)
{
	const std::string addon_dir = get_addon_campaigns_dir() + "/" + addon;

	delete_directory(addon_dir);
	if (file_exists(addon_dir + ".cfg"))
		delete_directory(addon_dir + ".cfg");
}

std::vector<std::string> available_addons()
{
	std::vector<std::string> res;
	std::vector<std::string> files, dirs;
	const std::string parentd = get_addon_campaigns_dir();
	get_files_in_dir(parentd,&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		const std::string external_cfg_file = *i + ".cfg";
		const std::string internal_cfg_file = *i + "/_main.cfg";
		const std::string external_pbl_file = *i + ".pbl";
		const std::string internal_pbl_file = *i + "/_server.pbl";
		if((std::find(files.begin(),files.end(),external_cfg_file) != files.end() || file_exists(parentd + "/" + internal_cfg_file)) &&
		   (std::find(files.begin(),files.end(),external_pbl_file) != files.end() || (file_exists(parentd + "/" + internal_pbl_file)))) {
			res.push_back(*i);
		}
	}

	return res;
}

std::vector<std::string> installed_addons()
{
	std::vector<std::string> res;
	const std::string parentd = get_addon_campaigns_dir();
	std::vector<std::string> files, dirs;
	get_files_in_dir(parentd,&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		const std::string external_cfg_file = *i + ".cfg";
		const std::string internal_cfg_file = *i + "/_main.cfg";
		if(std::find(files.begin(),files.end(),external_cfg_file) != files.end() || file_exists(parentd + "/" + internal_cfg_file)) {
			res.push_back(*i);
		}
	}

	return res;
}

namespace {
	const char escape_char = 1;
}

static bool needs_escaping(char c) { return c == 0 || c == escape_char; }

bool IsCR(const char& c)
{
	return c == 13;
}
static std::string strip_cr(std::string str, bool strip)
{
	if(!strip)
		return str;
	std::string::iterator new_end = std::remove_if(str.begin(), str.end(), IsCR);
	str.erase(new_end, str.end());
	return str;
}

static std::string encode_binary(const std::string& str)
{
	std::string res;
	res.resize(str.size());
	size_t n = 0;
	for(std::string::const_iterator j = str.begin(); j != str.end(); ++j) {
		if(needs_escaping(*j)) {
			res.resize(res.size()+1);
			res[n++] = escape_char;
			res[n++] = *j + 1;
		} else {
			res[n++] = *j;
		}
	}

	return res;
}

static std::string unencode_binary(const std::string& str)
{
	std::string res;
	res.resize(str.size());

	size_t n = 0;
	for(std::string::const_iterator j = str.begin(); j != str.end(); ++j) {
		if(*j == escape_char && j+1 != str.end()) {
			++j;
			res[n++] = *j - 1;
			res.resize(res.size()-1);
		} else {
			res[n++] = *j;
		}
	}

	return res;
}

static std::pair<std::vector<std::string>, std::vector<std::string> > read_ignore_patterns(const std::string& addon_name)
{
	const std::string parentd = get_addon_campaigns_dir();

	std::pair<std::vector<std::string>, std::vector<std::string> > patterns;
	std::string exterior = parentd + "/" + addon_name + ".ign";
	std::string interior = parentd + "/" + addon_name + "/_server.ign";
	std::string ign_file;
	if (file_exists(interior)) {
		ign_file = interior;
	} else if (file_exists(exterior)) {
		ign_file = exterior;
	} else { /* default patterns */
		patterns.first.push_back("*~");
		patterns.first.push_back("*-bak");
		patterns.first.push_back("*.pbl");
		patterns.first.push_back("*.ign");
		/* 
		 * Prevent certain potential security compromises.
		 * The idea is to stop bad guys from uploading things
		 * that could become trojans if an unsuspecting user
		 * downloads them.
		 */
		patterns.first.push_back("*.exe");
		patterns.first.push_back("*.bat");
		patterns.first.push_back("*.cmd");
		patterns.first.push_back("*.com");
		patterns.first.push_back("*.scr");
		patterns.first.push_back("*.sh");
		patterns.first.push_back("*.js");
		patterns.first.push_back("*.vbs");
		patterns.first.push_back("*.o");
		/* Remove junk created by certain file manager ;) */
		patterns.first.push_back("Thumbs.db");
		return patterns;
	}
	std::istream *stream = istream_file(ign_file);
	std::string line;
	while (std::getline(*stream, line)) {
		size_t l = line.size();
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
	bool is_cfg = (fname.size() > 4 ? (fname.substr(fname.size() - 4) == ".cfg") :false);
	cfg["contents"] = encode_binary(strip_cr(read_file(path + '/' + fname),is_cfg));
}

static void archive_dir(const std::string& path, const std::string& dirname, config& cfg, std::pair<std::vector<std::string>, std::vector<std::string> >& ignore_patterns)
{
	cfg["name"] = dirname;
	const std::string dir = path + '/' + dirname;

	std::vector<std::string> files, dirs;
	get_files_in_dir(dir,&files,&dirs);
	for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		bool valid = true;
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
	const std::string parentd = get_addon_campaigns_dir();

	std::pair<std::vector<std::string>, std::vector<std::string> > ignore_patterns;
	// External .cfg may not exist; newer campaigns have a _main.cfg
	std::string external_cfg = addon_name + ".cfg";
	if (file_exists(parentd + "/" + external_cfg)) {
		archive_file(parentd, external_cfg, cfg.add_child("file"));
	}
	ignore_patterns = read_ignore_patterns(addon_name);
	archive_dir(parentd, addon_name, cfg.add_child("dir"), ignore_patterns);
}

static void unarchive_file(const std::string& path, const config& cfg)
{
	write_file(path + '/' + cfg["name"].str(), unencode_binary(cfg["contents"]));
}

static void unarchive_dir(const std::string& path, const config& cfg)
{
	std::string dir;
	if (cfg["name"].empty())
		dir = path;
	else
		dir = path + '/' + cfg["name"].str();

	make_directory(dir);

	const config::child_list& dirs = cfg.get_children("dir");
	for(config::child_list::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		unarchive_dir(dir,**i);
	}

	const config::child_list& files = cfg.get_children("file");
	for(config::child_list::const_iterator j = files.begin(); j != files.end(); ++j) {
		unarchive_file(dir,**j);
	}
}

void unarchive_addon(const config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();
	setup_addon_dirs();
	unarchive_dir(parentd, cfg);
}
