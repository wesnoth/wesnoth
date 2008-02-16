/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
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
#include "publish_campaign.hpp"
#include "serialization/parser.hpp"

#include <algorithm>
#include <cstring>

static const std::string& campaign_dir()
{
	static const std::string res = get_user_data_dir() + "/data/campaigns";
	return res;
}

static void setup_dirs()
{
	make_directory(get_user_data_dir() + "/data");
	make_directory(campaign_dir());
}

void get_campaign_info(const std::string& campaign_name, config& cfg)
{
	// Cope with old-style or new-style file organization 
	std::string exterior = campaign_dir() + "/" + campaign_name + ".pbl";
	std::string interior = campaign_dir() + "/" + campaign_name + "/_server.pbl";
	std::string pbl_file;

	if (file_exists(exterior))
		pbl_file = exterior;
	else
		pbl_file = interior;

	scoped_istream stream = istream_file(pbl_file);
	read(cfg, *stream);
}

void set_campaign_info(const std::string& campaign_name, const config& cfg)
{
	scoped_ostream stream = ostream_file(campaign_dir() + "/" + campaign_name + ".pbl");
	write(*stream, cfg);
}

std::vector<std::string> available_campaigns()
{
	std::vector<std::string> res;

	std::vector<std::string> files, dirs;
	get_files_in_dir(campaign_dir(),&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		const std::string external_cfg_file = *i + ".cfg";
		const std::string internal_cfg_file = *i + "/_main.cfg";
		const std::string external_pbl_file = *i + ".pbl";
		const std::string internal_pbl_file = *i + "/_server.pbl";
		if((std::find(files.begin(),files.end(),external_cfg_file) != files.end() || file_exists(campaign_dir() + "/" + internal_cfg_file)) &&
		   (std::find(files.begin(),files.end(),external_pbl_file) != files.end() || (file_exists(campaign_dir() + "/" + internal_pbl_file)))) {
			res.push_back(*i);
		}
	}

	return res;
}

/// Return the names of all installed campaigns.
std::vector<std::string> installed_campaigns()
{
	std::vector<std::string> res;

	std::vector<std::string> files, dirs;
	get_files_in_dir(campaign_dir(),&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		const std::string external_cfg_file = *i + ".cfg";
		const std::string internal_cfg_file = *i + "/_main.cfg";
		if(std::find(files.begin(),files.end(),external_cfg_file) != files.end() || file_exists(campaign_dir() + "/" + internal_cfg_file)) {
			res.push_back(*i);
		}
	}

	return res;
}

// Return a vector of detected scripts.
std::vector<config *> find_scripts(const config &cfg, std::string extension)
{
	std::vector<config *> python_scripts;
	const config::child_list& dirs = cfg.get_children("dir");
	config::child_list::const_iterator i;
	for(i = dirs.begin(); i != dirs.end(); ++i) {
		const config::child_list& files = (**i).get_children("file");
		config::child_list::const_iterator j;
		for(j = files.begin(); j != files.end(); ++j) {
			std::string filename = (**j)["name"].str();
			if (filename.length() > extension.length()) {
				if (filename.substr(filename.length() - extension.length()) ==
					extension) {
					python_scripts.push_back(*j);
				}
			}
		}
		// Recursively look for files in sub directories.
		std::vector<config *> childs = find_scripts(**i, extension);
		python_scripts.insert(python_scripts.end(),
			childs.begin(), childs.end());
	}
	return python_scripts;
}

namespace {

const char escape_char = 1;

}

static bool needs_escaping(char c) { return c == 0 || c == escape_char; }

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

static std::pair<std::vector<std::string>, std::vector<std::string> > read_ignore_patterns(const std::string& campaign_name) {
	std::pair<std::vector<std::string>, std::vector<std::string> > patterns;
	std::string exterior = campaign_dir() + "/" + campaign_name + ".ign";
	std::string interior = campaign_dir() + "/" + campaign_name + "/_server.ign";
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
		 * that could become trojans if an unsuspoecting user 
		 * downloads them.
		 */
		patterns.first.push_back("*.exe");
		patterns.first.push_back("*.bat");
		patterns.first.push_back("*.com");
		patterns.first.push_back("*.scr");
		patterns.first.push_back("*.sh");
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
	cfg["contents"] = encode_binary(read_file(path + '/' + fname));
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

void archive_campaign(const std::string& campaign_name, config& cfg)
{
	std::pair<std::vector<std::string>, std::vector<std::string> > ignore_patterns;
	// External .cfg may not exist; newer campaigns have a _main.cfg
	std::string external_cfg = campaign_name + ".cfg";
	if (file_exists(campaign_dir() + "/" + external_cfg))
		archive_file(campaign_dir(), external_cfg,cfg.add_child("file"));
	ignore_patterns = read_ignore_patterns(campaign_name);
	archive_dir(campaign_dir(),campaign_name,cfg.add_child("dir"),ignore_patterns);
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

void unarchive_campaign(const config& cfg)
{
	setup_dirs();
	unarchive_dir(campaign_dir(),cfg);
}

static bool two_dots(char a, char b) { return a == '.' && b == '.'; }

bool campaign_name_legal(const std::string& name)
{
	if(name == "" || strlen(name.c_str()) == 0 || name == "." ||
	   std::find(name.begin(),name.end(),'/') != name.end() ||
	   std::find(name.begin(),name.end(),'\\') != name.end() ||
	   std::find(name.begin(),name.end(),':') != name.end() ||
	   std::adjacent_find(name.begin(),name.end(),two_dots) != name.end()) {
		return false;
	} else {
		return true;
	}
}

bool check_names_legal(const config& dir)
{
        const config::child_list& files = dir.get_children("file");
        for(config::child_list::const_iterator i = files.begin(); i != files.end(); ++i) {
                if (!campaign_name_legal((**i)["name"])) return false;
        }
        const config::child_list& dirs = dir.get_children("dir");
		{
			for(config::child_list::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
					if (!campaign_name_legal((**i)["name"])) return false;
					if (!check_names_legal(**i)) return false;
			}
		}
        return true;
}
