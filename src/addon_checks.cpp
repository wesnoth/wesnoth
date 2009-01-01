/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "addon_checks.hpp"
#include "config.hpp"

#include <cstring>

static bool two_dots(char a, char b)
{
	return a == '.' && b == '.';
}

bool addon_name_legal(const std::string& name)
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
			if (!addon_name_legal((**i)["name"])) return false;
	}
	const config::child_list& dirs = dir.get_children("dir");
	{
		for(config::child_list::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
				if (!addon_name_legal((**i)["name"])) return false;
				if (!check_names_legal(**i)) return false;
		}
	}
	return true;
}

ADDON_TYPE get_addon_type(const std::string& str)
{
	if (str.empty())
		return ADDON_UNKNOWN;
	else if (str == "campaign")
		return ADDON_SP_CAMPAIGN;
	else if (str == "scenario")
		return ADDON_SP_SCENARIO;
	else if (str == "era")
		return ADDON_MP_ERA;
	else if (str == "faction")
		return ADDON_MP_FACTION;
	else if (str == "map_pack")
		return ADDON_MP_MAPS;
	else if (str == "scenario_mp")
		return ADDON_MP_SCENARIO;
	else if (str == "campaign_mp")
		return ADDON_MP_CAMPAIGN;
	else if (str == "media")
		return ADDON_MEDIA;
// 	else if (str == "mod")
// 		return ADDON_MOD;
// 	else if (str == "gui")
// 		return ADDON_GUI;
	else
		return ADDON_UNKNOWN;
}

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
	const char escape_char = '\x01'; /**< Binary escape char. */
} // end unnamed namespace 2

bool needs_escaping(char c) {
	switch(c) {
		case '\x00':
		case escape_char:
		case '\x0D': //Windows -- carriage return
		case '\xFE': //Parser code -- textdomain or linenumber&filename
			return true;
		default:
			return false;
	}
}

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


