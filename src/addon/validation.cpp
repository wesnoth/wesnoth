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
#include <boost/algorithm/string.hpp>
#include <set>

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
	if(name.empty() || name == "." ||
	   name.find_first_of("/:\\~ \r\n\v\t") != std::string::npos ||
	   name.find("..") != std::string::npos) {
		return false;
	} else {
		return true;
	}
}

namespace {

bool check_names_legal_internal(const config& dir, std::string current_prefix, std::vector<std::string>* badlist)
{
	if (!current_prefix.empty()) {
		current_prefix += '/';
	}

	for(const config& path : dir.child_range("file")) {
		const std::string& filename = path["name"];

		if(!addon_filename_legal(filename)) {
			if(badlist) {
				badlist->push_back(current_prefix + filename);
			} else {
				return false;
			}
		}
	}

	for(const config& path : dir.child_range("dir")) {
		const std::string& dirname = path["name"];
		const std::string& new_prefix = current_prefix + dirname;

		if(!addon_filename_legal(dirname)) {
			if(badlist) {
				badlist->push_back(new_prefix + "/");
			} else {
				return false;
			}
		}

		// Recurse into subdir.
		if(!check_names_legal_internal(path, new_prefix, badlist) && !badlist) {
			return false;
		}
	}

	return badlist ? badlist->empty() : true;
}

bool check_case_insensitive_duplicates_internal(const config& dir, std::string prefix, std::vector<std::string>* badlist){
	typedef std::pair<bool, std::string> printed_and_original;
	std::map<std::string, printed_and_original> filenames;
	bool inserted;
	bool printed;
	std::string original;
	for (const config &path : dir.child_range("file")) {
		const config::attribute_value &filename = path["name"];
		const std::string lowercase = boost::algorithm::to_lower_copy(filename.str(), std::locale::classic());
		const std::string with_prefix = prefix + filename.str();
		std::tie(std::ignore, inserted) = filenames.emplace(lowercase, std::make_pair(false, with_prefix));
		if (!inserted){
			if(badlist){
				std::tie(printed, original) = filenames[lowercase];
				if(!printed){
					badlist->push_back(original);
					filenames[lowercase] = make_pair(true, std::string());
				}
				badlist->push_back(with_prefix);
			} else {
				return false;
			}
		}
	}
	for (const config &path : dir.child_range("dir")) {
		const config::attribute_value &filename = path["name"];
		const std::string lowercase = boost::algorithm::to_lower_copy(filename.str(), std::locale::classic());
		const std::string with_prefix = prefix + filename.str();
		std::tie(std::ignore, inserted) = filenames.emplace(lowercase, std::make_pair(false, with_prefix));
		if (!inserted) {
			if(badlist){
				std::tie(printed, original) = filenames[lowercase];
				if(!printed){
					badlist->push_back(original);
					filenames[lowercase] = make_pair(true, std::string());
				}
				badlist->push_back(with_prefix);
			} else {
				return false;
			}
		}
		if (!check_case_insensitive_duplicates_internal(path, prefix + filename + "/", badlist) && !badlist){
			return false;
		}
	}

	return badlist ? badlist->empty() : true;
}

} // end unnamed namespace 3

bool check_names_legal(const config& dir, std::vector<std::string>* badlist)
{
	// Usually our caller is passing us the root [dir] for an add-on, which
	// shall contain a single subdir named after the add-on itself, so we can
	// start with an empty display prefix and that'll reflect the addon
	// structure correctly (e.g. "Addon_Name/~illegalfilename1").
	return check_names_legal_internal(dir, "", badlist);
}

bool check_case_insensitive_duplicates(const config& dir, std::vector<std::string>* badlist){
    return check_case_insensitive_duplicates_internal(dir, "", badlist);
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


