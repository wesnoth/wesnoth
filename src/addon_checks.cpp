/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2011 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
#include "foreach.hpp"

#include <cstring>

const unsigned short default_campaignd_port = 15001;

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
	   std::find(name.begin(),name.end(),'~') != name.end() ||
	   std::adjacent_find(name.begin(),name.end(),two_dots) != name.end()) {
		return false;
	} else {
		return true;
	}
}

bool check_names_legal(const config& dir)
{
	BOOST_FOREACH (const config &path, dir.child_range("file")) {
		if (!addon_name_legal(path["name"])) return false;
	}
	BOOST_FOREACH (const config &path, dir.child_range("dir")) {
		if (!addon_name_legal(path["name"])) return false;
		if (!check_names_legal(path)) return false;
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
	else if (str == "other")
		return ADDON_OTHER;
	else
		return ADDON_UNKNOWN;
}

void find_scripts(config &cfg, const std::string &extension,
                  std::vector<config *> &scripts)
{
	BOOST_FOREACH (config &i, cfg.child_range("dir"))
	{
		BOOST_FOREACH (config &j, cfg.child_range("file"))
		{
			std::string filename = j["name"];
			if (filename.length() > extension.length()) {
				if (filename.substr(filename.length() - extension.length()) ==
					extension) {
					scripts.push_back(&j);
				}
			}
		}
		// Recursively look for files in sub directories.
		find_scripts(i, extension, scripts);
	}
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


