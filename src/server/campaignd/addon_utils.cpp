/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
                 2013 - 2015 by Iris Morelle <shadowm2006@gmail.com>
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
#include "game_config.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "hash.hpp"

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

		config::const_attr_itors attrs = params.attribute_range();

		// Percent-encode parameter values for URL interpolation. This is
		// VERY important since otherwise people could e.g. alter query
		// strings from the format string.
		for(const config::attribute& a : attrs) {
			escaped[a.first] = utils::urlencode(a.second.str());
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
	config* locale = &addon.find_child("translation", "language", locale_id);
	if(!*locale) {
		locale = &addon.add_child("translation");
		(*locale)["language"] = locale_id;
	}
	(*locale)["supported"] = true;
}

void find_translations(const config& base_dir, config& addon)
{
	for(const config& file : base_dir.child_range("file")) {
		const std::string& fn = file["name"].str();
		if(filesystem::ends_with(fn, ".po")) {
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
	config& dir = cfg.find_child("dir", "name", cfg["campaign_name"]);

	// No top-level directory? Hm..
	if(!dir) {
		return;
	}

	// Don't add if it already exists.
	if(dir.find_child("file", "name", "COPYING.txt")
	   || dir.find_child("file", "name", "COPYING")
	   || dir.find_child("file", "name", "copying.txt")
	   || dir.find_child("file", "name", "Copying.txt")
	   || dir.find_child("file", "name", "COPYING.TXT"))
	{
		return;
	}

	// Copy over COPYING.txt
	const std::string& contents = filesystem::read_file("COPYING.txt");
	if (contents.empty()) {
		LOG_CS << "Could not find COPYING.txt, path is \"" << game_config::path << "\"\n";
		return;
	}

	config& copying = dir.add_child("file");
	copying["name"] = "COPYING.txt";
	copying["contents"] = contents;
}

static const std::string file_hash(const config& file)
{
	std::string hash = file["hash"].str();
	if(hash.empty()) {
		hash = utils::md5(file["contents"].str()).base64_digest();
	}
	return hash;
}

static bool comp_file_hash(const config& file_a, const config& file_b)
{
	return file_hash(file_a) == file_hash(file_b);
}

//! Surround with [dir][/dir]
static void write_difference(config& pack, const config& from, const config& to, bool with_content)
{
	pack["name"] = to["name"];

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
		}
	}

	for(const config& d : to.child_range("dir")) {
		const config& origin_dir = from.find_child("dir", "name", d["name"]);
		if(origin_dir) {
			config& dir = pack.add_child("dir");
			write_difference(dir, origin_dir, d, with_content);
		}
	}
}

/**
 * &from, &to are top-dirs of their structures; addlist/removelist are equal to [dir]
 * #TODO: make a clientside function to allow incremental uploadpacks using hash request from server
 * Does it worth it to archive and write the pack on the fly using config_writer?
 * #TODO: clientside verification?
 */
void make_updatepack(config& pack, const config& from, const config& to)
{
	config& removelist = pack.add_child("removelist");
	write_difference(removelist, to, from, false);
	config& addlist = pack.add_child("addlist");
	write_difference(addlist, from, to, true);
}

} // end namespace campaignd
