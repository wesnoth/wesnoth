/*
	Copyright (C) 2008 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "filesystem.hpp"
#include "gettext.hpp"
#include "hash.hpp"

#include <algorithm>
#include <array>
#include <boost/algorithm/string.hpp>

const unsigned short default_campaignd_port = 15019;

namespace
{

const std::array<std::string, ADDON_TYPES_COUNT> addon_type_strings {{
	"unknown", "core", "campaign", "scenario", "campaign_sp_mp", "campaign_mp",
	"scenario_mp", "map_pack", "era", "faction", "mod_mp", "media",	"theme", "other"
}};

struct addon_name_char_illegal
{
	/**
	 * Returns whether the given add-on name char is not whitelisted.
	 */
	inline bool operator()(char c) const
	{
		switch(c) {
			case '-':		// hyphen-minus
			case '_':		// low line
				return false;
			default:
				return !isalnum(c);
		}
	}
};

} // end unnamed namespace

bool addon_name_legal(const std::string& name)
{
	if(name.empty() ||
	   std::find_if(name.begin(), name.end(), addon_name_char_illegal()) != name.end()) {
		return false;
	} else {
	   return true;
	}
}

bool addon_filename_legal(const std::string& name)
{
	// Currently just a wrapper for filesystem::is_legal_user_file_name().
	// This is allowed to change in the future. Do not remove this wrapper.
	// I will hunt you down if you do.
	return filesystem::is_legal_user_file_name(name, false);
}

bool addon_icon_too_large(const std::string& icon) {
	return icon.size() > max_icon_size;
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

bool check_case_insensitive_duplicates_internal(const config& dir, const std::string& prefix, std::vector<std::string>* badlist){
	typedef std::pair<bool, std::string> printed_and_original;
	std::map<std::string, printed_and_original> filenames;
	bool inserted;
	bool printed;
	std::string original;
	for (const config &path : dir.child_range("file")) {
		const config::attribute_value &filename = path["name"];
		const std::string lowercase = boost::algorithm::to_lower_copy(filename.str(), std::locale::classic());
		const std::string with_prefix = prefix + filename.str();
		std::tie(std::ignore, inserted) = filenames.emplace(lowercase, std::pair(false, with_prefix));
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
		std::tie(std::ignore, inserted) = filenames.emplace(lowercase, std::pair(false, with_prefix));
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
		if(!check_case_insensitive_duplicates_internal(path, with_prefix + "/", badlist) && !badlist) {
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

bool check_case_insensitive_duplicates(const config& dir, std::vector<std::string>* badlist)
{
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
	std::size_t n = 0;
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
	std::string res(str.size(), '\0');

	std::size_t n = 0;
	for(std::string::const_iterator j = str.begin(); j != str.end(); ) {
		char c = *j++;
		if((c == escape_char) && (j != str.end())) {
			c = (*j++) - 1;
		}
		res[n++] = c;
	}

	res.resize(n);
	return res;
}

static std::string file_hash_raw(const config& file)
{
	return utils::md5(file["contents"].str()).base64_digest();
}

std::string file_hash(const config& file)
{
	std::string hash = file["hash"].str();
	if(hash.empty()) {
		hash = file_hash_raw(file);
	}
	return hash;
}

bool comp_file_hash(const config& file_a, const config& file_b)
{
	return file_a["name"] == file_b["name"] && file_hash(file_a) == file_hash(file_b);
}

void write_hashlist(config& hashlist, const config& data)
{
	hashlist["name"] = data["name"];

	for(const config& f : data.child_range("file")) {
		config& file = hashlist.add_child("file");
		file["name"] = f["name"];
		file["hash"] = file_hash_raw(f);
	}

	for(const config& d : data.child_range("dir")) {
		config& dir = hashlist.add_child("dir");
		write_hashlist(dir, d);
	}
}

bool contains_hashlist(const config& from, const config& to)
{
	for(const config& f : to.child_range("file")) {
		bool found = false;
		for(const config& d : from.child_range("file")) {
			found |= comp_file_hash(f, d);
			if(found)
				break;
		}
		if(!found) {
			return false;
		}
	}

	for(const config& d : to.child_range("dir")) {
		auto origin_dir = from.find_child("dir", "name", d["name"]);
		if(origin_dir) {
			if(!contains_hashlist(*origin_dir, d)) {
				return false;
			}
		} else {
			// The case of empty new subdirectories
			const config dummy_dir = config("name", d["name"]);
			if(!contains_hashlist(dummy_dir, d)) {
				return false;
			}
		}
	}

	return true;
}

/** Surround with [dir][/dir] */
static bool write_difference(config& pack, const config& from, const config& to, bool with_content)
{
	pack["name"] = to["name"];
	bool has_changes = false;

	for(const config& f : to.child_range("file")) {
		bool found = false;
		for(const config& d : from.child_range("file")) {
			found |= comp_file_hash(f, d);
			if(found)
				break;
		}
		if(!found) {
			config& file = pack.add_child("file");
			file["name"] = f["name"];
			if(with_content) {
				file["contents"] = f["contents"];
				file["hash"] = file_hash(f);
			}
			has_changes = true;
		}
	}

	for(const config& d : to.child_range("dir")) {
		auto origin_dir = from.find_child("dir", "name", d["name"]);
		config dir;
		if(origin_dir) {
			if(write_difference(dir, *origin_dir, d, with_content)) {
				pack.add_child("dir", dir);
				has_changes = true;
			}
		} else {
			const config dummy_dir = config("name", d["name"]);
			if(write_difference(dir, dummy_dir, d, with_content)) {
				pack.add_child("dir", dir);
				has_changes = true;
			}
		}
	}

	return has_changes;
}

/**
 * &from, &to are the top directories of their structures; addlist/removelist tag is treated as [dir]
 *
 * Does it worth it to archive and write the pack on the fly using config_writer?
 * TODO: clientside verification?
 */
void make_updatepack(config& pack, const config& from, const config& to)
{
	config& removelist = pack.add_child("removelist");
	write_difference(removelist, to, from, false);
	config& addlist = pack.add_child("addlist");
	write_difference(addlist, from, to, true);
}

std::string addon_check_status_desc(unsigned int code)
{
	static const std::map<ADDON_CHECK_STATUS, std::string> message_table = {

		//
		// General errors
		//

		{
			ADDON_CHECK_STATUS::SUCCESS,
			N_("Success.")
		},
		{
			ADDON_CHECK_STATUS::UNAUTHORIZED,
			N_("Incorrect add-on passphrase.")
		},
		{
			ADDON_CHECK_STATUS::USER_DOES_NOT_EXIST,
			N_("Forum authentication was requested for a user that is not registered on the forums.")
		},
		{
			ADDON_CHECK_STATUS::DENIED,
			N_("Upload denied. Please contact the server administration for assistance.")
		},
		{
			ADDON_CHECK_STATUS::UNEXPECTED_DELTA,
			N_("Attempted to upload an update pack for a non-existent add-on.")
		},

		//
		// Structure errors
		//

		{
			ADDON_CHECK_STATUS::EMPTY_PACK,
			N_("No add-on data was supplied by the client.")
		},
		{
			ADDON_CHECK_STATUS::BAD_DELTA,
			N_("Invalid upload pack.")
		},
		{
			ADDON_CHECK_STATUS::BAD_NAME,
			N_("Invalid add-on name.")
		},
		{
			ADDON_CHECK_STATUS::NAME_HAS_MARKUP,
			N_("Formatting character in add-on name.")
		},
		{
			ADDON_CHECK_STATUS::ILLEGAL_FILENAME,
			N_("The add-on contains files or directories with illegal names.\n"
			"\n"
			"Names containing whitespace, control characters, or any of the following symbols are not allowed:\n"
			"\n"
			"    \" * / : < > ? \\ | ~\n"
			"\n"
			"Additionally, names may not be longer than 255 characters, contain '..', or end with '.'.")
		},
		{
			ADDON_CHECK_STATUS::FILENAME_CASE_CONFLICT,
			N_("The add-on contains files or directories with case conflicts.\n"
			"\n"
			"Names in the same directory may not be differently-cased versions of each other.")
		},
		{
			ADDON_CHECK_STATUS::INVALID_UTF8_NAME,
			N_("The add-on name contains an invalid UTF-8 sequence.")
		},

		//
		// .pbl errors
		//

		{
			ADDON_CHECK_STATUS::NO_TITLE,
			N_("No add-on title specified.")
		},
		{
			ADDON_CHECK_STATUS::NO_AUTHOR,
			N_("No add-on author/maintainer name specified.")
		},
		{
			ADDON_CHECK_STATUS::NO_VERSION,
			N_("No add-on version specified.")
		},
		{
			ADDON_CHECK_STATUS::NO_DESCRIPTION,
			N_("No add-on description specified.")
		},
		{
			ADDON_CHECK_STATUS::NO_EMAIL,
			N_("No add-on author/maintainer email specified.")
		},
		{
			ADDON_CHECK_STATUS::NO_PASSPHRASE,
			N_("Missing passphrase.")
		},
		{
			ADDON_CHECK_STATUS::TITLE_HAS_MARKUP,
			N_("Formatting character in add-on title.")
		},
		{
			ADDON_CHECK_STATUS::BAD_TYPE,
			N_("Invalid or unspecified add-on type.")
		},
		{
			ADDON_CHECK_STATUS::VERSION_NOT_INCREMENTED,
			N_("Version number not greater than the latest uploaded version.")
		},
		{
			ADDON_CHECK_STATUS::BAD_FEEDBACK_TOPIC_ID,
			N_("Feedback topic id is not a number.")
		},
		{
			ADDON_CHECK_STATUS::FEEDBACK_TOPIC_ID_NOT_FOUND,
			N_("Feedback topic does not exist.")
		},
		{
			ADDON_CHECK_STATUS::INVALID_UTF8_ATTRIBUTE,
			N_("The add-on publish information contains an invalid UTF-8 sequence.")
		},
		{
			ADDON_CHECK_STATUS::AUTH_TYPE_MISMATCH,
			N_("The add-on’s forum_auth attribute does not match what was previously uploaded.")
		},
		{
			ADDON_CHECK_STATUS::ICON_TOO_LARGE,
			N_("The add-on’s icon’s file size is too large.")
		},

		//
		// Server errors
		//

		{
			ADDON_CHECK_STATUS::SERVER_UNSPECIFIED,
			N_("Unspecified server error.")
		},
		{
			ADDON_CHECK_STATUS::SERVER_READ_ONLY,
			N_("Server is in read-only mode.")
		},
		{
			ADDON_CHECK_STATUS::SERVER_ADDONS_LIST,
			N_("Corrupted server add-ons list.")
		},
		{
			ADDON_CHECK_STATUS::SERVER_DELTA_NO_VERSIONS,
			N_("Empty add-on version list on the server.")
		},
		{
			ADDON_CHECK_STATUS::SERVER_FORUM_AUTH_DISABLED,
			N_("This server does not support using the forum_auth attribute in your pbl.")
		}
	};

	for(const auto& entry : message_table) {
		if(static_cast<unsigned int>(entry.first) == code) {
			return entry.second;
		}
	}

	return N_("Unspecified validation failure.");
};

std::string translated_addon_check_status(unsigned int code)
{
	return _(addon_check_status_desc(code).c_str());
}
