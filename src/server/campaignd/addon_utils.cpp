/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Copyright (C) 2013 - 2015 by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "server/campaignd/addon_utils.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "addon/validation.hpp"

#include <boost/algorithm/string.hpp>

static lg::log_domain log_network("network");
#define LOG_CS LOG_STREAM_NAMELESS(err, log_network)

namespace {

typedef std::map<std::string, std::string> plain_string_map;

/**
 * Quick and dirty alternative to @a utils::interpolate_variables_into_string
 * that doesn't require formula AI code. It is definitely NOT safe for normal
 * use since it doesn't do strict checks on where variable placeholders
 * ("$foobar") end and doesn't support pipe ("|") terminators.
 *
 * @param str     The format string.
 * @param symbols The symbols table.
 */
std::string fast_interpolate_variables_into_string(const std::string &str, const plain_string_map * const symbols)
{
	std::string res = str;

	if(symbols) {
		for(const plain_string_map::value_type& sym : *symbols) {
			boost::replace_all(res, "$" + sym.first, sym.second);
		}
	}

	return res;
}

} // end anonymous namespace

namespace campaignd {

// Markup characters recognized by GUI1 code. These must be
// the same as the constants defined in marked-up_text.cpp.
const std::string illegal_markup_chars = "*`~{^}|@#<&";

std::string format_addon_feedback_url(const std::string& format, const config& params)
{
	if(!format.empty() && !params.empty()) {
		plain_string_map escaped;

		// Percent-encode parameter values for URL interpolation. This is
		// VERY important since otherwise people could e.g. alter query
		// strings from the format string.
		for(const auto& [key, value] : params.attribute_range()) {
			escaped[key] = utils::urlencode(value.str());
		}

		// FIXME: We cannot use utils::interpolate_variables_into_string
		//        because it is implemented using a lot of formula AI junk
		//        that really doesn't belong in campaignd.
		const std::string& res =
			fast_interpolate_variables_into_string(format, &escaped);

		if(res != format) {
			return res;
		}

		// If we get here, that means that no interpolation took place; in
		// that case, the parameters table probably contains entries that
		// do not match the format string expectations.
	}

	return std::string();
}

void support_translation(config& addon, const std::string& locale_id)
{
	config* locale = addon.find_child("translation", "language", locale_id).ptr();
	if(!locale) {
		locale = &addon.add_child("translation");
		(*locale)["language"] = locale_id;
	}
	(*locale)["supported"] = true;
}

void find_translations(const config& base_dir, config& addon)
{
	for(const config& file : base_dir.child_range("file")) {
		const std::string& fn = file["name"].str();
		if(boost::algorithm::ends_with(fn, ".po")) {
			support_translation(addon, filesystem::base_name(fn, true));
		}
	}

	for(const config &dir : base_dir.child_range("dir"))
	{
		if(dir["name"] == "LC_MESSAGES") {
			support_translation(addon, base_dir["name"]);
		} else {
			find_translations(dir, addon);
		}
	}
}

void add_license(config& cfg)
{
	auto dir = cfg.optional_child("dir");

	// No top-level directory? Hm..
	if(!dir) {
		LOG_CS << "Could not find toplevel [dir] tag";
		return;
	}

	// Don't add if it already exists.
	if(dir->find_child("file", "name", "COPYING.txt")
	   || dir->find_child("file", "name", "COPYING")
	   || dir->find_child("file", "name", "copying.txt")
	   || dir->find_child("file", "name", "Copying.txt")
	   || dir->find_child("file", "name", "COPYING.TXT"))
	{
		return;
	}

	// Copy over COPYING.txt
	const std::string& contents = filesystem::read_file("COPYING.txt");
	if (contents.empty()) {
		LOG_CS << "Could not find COPYING.txt, path is \"" << game_config::path << "\"";
		return;
	}

	config& copying = dir->add_child("file");
	copying["name"] = "COPYING.txt";
	copying["contents"] = contents;
}

std::map<version_info, config> get_version_map(config& addon)
{
	std::map<version_info, config> version_map;

	for(config& version : addon.child_range("version")) {
		version_map.emplace(version_info(version["version"]), version);
	}

	return version_map;
}

bool data_apply_removelist(config& data, const config& removelist)
{
	for(const config& f : removelist.child_range("file")) {
		data.remove_children("file", [&f](const config& d) { return f["name"] == d["name"]; });
	}

	for(const config& dir : removelist.child_range("dir")) {
		auto data_dir = data.find_child("dir", "name", dir["name"]);
		if(data_dir && !data_apply_removelist(*data_dir, dir)) {
			// Delete empty directories
			data.remove_children("dir", [&dir](const config& d) { return dir["name"] == d["name"]; });
		}
	}

	return data.has_child("file") || data.has_child("dir");
}

void data_apply_addlist(config& data, const config& addlist)
{
	for(const config& f : addlist.child_range("file")) {
		// Just add it since we have already checked the data for duplicates
		data.add_child("file", f);
	}

	for(const config& dir : addlist.child_range("dir")) {
		config* data_dir = data.find_child("dir", "name", dir["name"]).ptr();
		if(!data_dir) {
			data_dir = &data.add_child("dir");
			(*data_dir)["name"] = dir["name"];
		}
		data_apply_addlist(*data_dir, dir);
	}
}

} // end namespace campaignd
