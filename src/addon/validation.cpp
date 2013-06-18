/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2013 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "addon/validation.hpp"
#include "config.hpp"

#include <boost/foreach.hpp>

#include <cstring>

const unsigned short default_campaignd_port = 15002;

static bool two_dots(char a, char b)
{
	return a == '.' && b == '.';
}

namespace {
	struct addon_name_char_illegal
	{
		/**
		 * Returns whether the given add-on name char is not whitelisted.
		 */
		inline bool operator()(char c)
		{
			switch(c)
			{
				case '-':		// hyphen-minus
				case '_':		// low line
				return false;
				default:
					return !isalnum(c);
			}
		}
	};
}

bool addon_name_legal(const std::string& name)
{
	if(name.empty() || name == "." ||
	   std::find_if(name.begin(), name.end(), addon_name_char_illegal()) != name.end() ||
	   name.find("..") != std::string::npos) {
		return false;
	} else {
	   return true;
	}
}

bool addon_filename_legal(const std::string& name)
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
	BOOST_FOREACH(const config &path, dir.child_range("file")) {
		if (!addon_filename_legal(path["name"])) return false;
	}
	BOOST_FOREACH(const config &path, dir.child_range("dir")) {
		if (!addon_filename_legal(path["name"])) return false;
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


