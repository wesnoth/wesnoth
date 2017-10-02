/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2015 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "addon/validation.hpp"
#include "config.hpp"

#include <algorithm>

const unsigned short default_campaignd_port = 15008;

namespace {
	const std::string addon_type_strings[] {
		"unknown", "core", "campaign", "scenario", "campaign_sp_mp", "campaign_mp",
		"scenario_mp", "map_pack", "era", "faction", "mod_mp", /*"gui", */ "media",
		"other", ""
	};

	struct addon_name_char_illegal
	{
		/**
		 * Returns whether the given add-on name char is not whitelisted.
		 */
		inline bool operator()(char c) const
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
	// POSIX portable filenames: a-z A-Z 0-9 . - _
	// Additionally, we use @ for translations and + for gendered sprites
	static const std::string valid_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_+@";
	if(name.empty() || name == "." ||
	   name.find_first_not_of(valid_chars) != std::string::npos ||
	   name.find("..") != std::string::npos) {
		return false;
	} else {
		return true;
	}
}

bool check_names_legal(const config& dir)
{
	for (const config &path : dir.child_range("file")) {
		if (!addon_filename_legal(path["name"])) return false;
	}
	for (const config &path : dir.child_range("dir")) {
		if (!addon_filename_legal(path["name"])) return false;
		if (!check_names_legal(path)) return false;
	}
	return true;
}

ADDON_TYPE get_addon_type(const std::string& str)
{
	if (str.empty())
		return ADDON_UNKNOWN;

	unsigned addon_type_num = 0;

	while(++addon_type_num != ADDON_TYPES_COUNT) {
		if(str == addon_type_strings[addon_type_num])  {
			return ADDON_TYPE(addon_type_num);
		}
	}

	return ADDON_UNKNOWN;
}

std::string get_addon_type_string(ADDON_TYPE type)
{
	assert(type != ADDON_TYPES_COUNT);
	return addon_type_strings[type];
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


