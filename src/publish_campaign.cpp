/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
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

namespace {

const std::string& campaign_dir()
{
	static const std::string res = get_user_data_dir() + "/data/campaigns";
	return res;
}

void setup_dirs()
{
	make_directory(get_user_data_dir() + "/data");
	make_directory(campaign_dir());
}

}

void get_campaign_info(const std::string& campaign_name, config& cfg)
{
	scoped_istream stream = istream_file(campaign_dir() + "/" + campaign_name + ".pbl");
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
		const std::string cfg_file = *i + ".cfg";
		const std::string publish_file = *i + ".pbl";
		if(std::find(files.begin(),files.end(),cfg_file) != files.end() &&
		   std::find(files.begin(),files.end(),publish_file) != files.end()) {
			res.push_back(*i);
		}
	}

	return res;
}

namespace {

const char escape_char = 1;
bool needs_escaping(char c) { return c == 0 || c == escape_char; }

std::string encode_binary(const std::string& str)
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

std::string unencode_binary(const std::string& str)
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

void archive_file(const std::string& path, const std::string& fname, config& cfg)
{
	cfg["name"] = fname;
	cfg["contents"] = encode_binary(read_file(path + '/' + fname));
}

void archive_dir(const std::string& path, const std::string& dirname, config& cfg)
{
	cfg["name"] = dirname;
	const std::string dir = path + '/' + dirname;

	std::vector<std::string> files, dirs;
	get_files_in_dir(dir,&files,&dirs);
	for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		archive_file(dir,*i,cfg.add_child("file"));
	}

	for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
		archive_dir(dir,*j,cfg.add_child("dir"));
	}
}

}

void archive_campaign(const std::string& campaign_name, config& cfg)
{
	archive_file(campaign_dir(),campaign_name + ".cfg",cfg.add_child("file"));
	archive_dir(campaign_dir(),campaign_name,cfg.add_child("dir"));
}

namespace {

void unarchive_file(const std::string& path, const config& cfg)
{
	write_file(path + '/' + cfg["name"].str(), unencode_binary(cfg["contents"]));
}

void unarchive_dir(const std::string& path, const config& cfg)
{
	const std::string dir = path + '/' + cfg["name"].str();
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

}

void unarchive_campaign(const config& cfg)
{
	setup_dirs();
	unarchive_dir(campaign_dir(),cfg);
}

namespace {
	bool two_dots(char a, char b) { return a == '.' && b == '.'; }
}

bool campaign_name_legal(const std::string& name)
{
	if(name == "" || strlen(name.c_str()) == 0 ||
	   std::find(name.begin(),name.end(),'/') != name.end() ||
	   std::find(name.begin(),name.end(),'\\') != name.end() ||
	   std::find(name.begin(),name.end(),':') != name.end() ||
	   std::adjacent_find(name.begin(),name.end(),two_dots) != name.end()) {
		return false;
	} else {
		return true;
	}
}
