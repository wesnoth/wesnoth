/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

#include "filesystem.hpp"
#include <iosfwd>
#include <map>
#include <vector>
#include <boost/optional.hpp>

#include "exceptions.hpp"
#include "version.hpp"
#include "deprecation.hpp"

class config_writer;
class config;

typedef std::map<std::string, struct preproc_define> preproc_map;

struct preproc_define
{
	preproc_define()
		: value()
		, arguments()
		, optional_arguments()
		, textdomain()
		, linenum(0)
		, location()
	{
	}

	explicit preproc_define(const std::string& val)
		: value(val)
		, arguments()
		, optional_arguments()
		, textdomain()
		, linenum(0)
		, location()
	{
	}

	preproc_define(const std::string& val,
			const std::vector<std::string>& args,
			const std::map<std::string, std::string>& optargs,
			const std::string& domain,
			int line,
			const std::string& loc,
			const std::string& dep_msg,
			DEP_LEVEL dep_lvl, const version_info& dep_ver)
		: value(val)
		, arguments(args)
		, optional_arguments(optargs)
		, textdomain(domain)
		, linenum(line)
		, location(loc)
		, deprecation_message(dep_msg)
		, deprecation_level(dep_lvl)
		, deprecation_version(dep_ver)
	{
	}

	std::string value;

	std::vector<std::string> arguments;

	std::map<std::string, std::string> optional_arguments;

	std::string textdomain;

	int linenum;

	std::string location;

	std::string deprecation_message;

	boost::optional<DEP_LEVEL> deprecation_level;

	version_info deprecation_version;

	bool is_deprecated() const {
		return deprecation_level != boost::none;
	}

	void write(config_writer&, const std::string&) const;
	void write_argument(config_writer&, const std::string&) const;

	void read(const config&);
	void read_argument(const config&);

	static preproc_map::value_type read_pair(const config&);

	bool operator==(const preproc_define&) const;

	bool operator<(const preproc_define&) const;

	bool operator!=(const preproc_define& v) const
	{
		return !operator==(v);
	}
};

std::ostream& operator<<(std::ostream& stream, const preproc_define& def);

struct preproc_config
{
	struct error : public game::error
	{
		error(const std::string& message)
			: game::error(message)
		{
		}
	};
};

std::string lineno_string(const std::string& lineno);

std::ostream& operator<<(std::ostream& stream, const preproc_map::value_type& def);

/**
 * Function to use the WML preprocessor on a file.
 *
 * @param defines                 A map of symbols defined.
 *
 * @returns                       The resulting preprocessed file data.
 */
filesystem::scoped_istream preprocess_file(const std::string& fname, preproc_map* defines = nullptr);

void preprocess_resource(const std::string& res_name,
		preproc_map* defines_map,
		bool write_cfg = false,
		bool write_plain_cfg = false,
		const std::string& target_directory = "");
