/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
                 2013 - 2015 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "campaign_server/addon_utils.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <boost/algorithm/string.hpp>

static lg::log_domain log_network("network");
#define LOG_CS if (lg::err().dont_log(log_network)) ; else lg::err()(log_network, false)

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

void find_translations(const config& base_dir, config& addon)
{
	for(const config &dir : base_dir.child_range("dir"))
	{
		if(dir["name"] == "LC_MESSAGES") {
			addon.add_child("translation")["language"] = base_dir["name"];
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

} // end namespace campaignd
