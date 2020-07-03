/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Guillaume Melquiond <guillaume.melquiond@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * WML preprocessor.
 */

#include "serialization/preprocessor.hpp"

#include "buffered_istream.hpp"
#include "config.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "game_version.hpp"
#include "wesconfig.h"
#include "deprecation.hpp"

#include <stdexcept>
#include <deque>

static lg::log_domain log_preprocessor("preprocessor");
#define ERR_PREPROC LOG_STREAM(err, log_preprocessor)
#define WRN_PREPROC LOG_STREAM(warn, log_preprocessor)
#define LOG_PREPROC LOG_STREAM(info, log_preprocessor)
#define DBG_PREPROC LOG_STREAM(debug, log_preprocessor)

static const std::string current_file_str = "CURRENT_FILE";
static const std::string current_dir_str = "CURRENT_DIRECTORY";
static const std::string left_curly_str = "LEFT_BRACE";
static const std::string right_curly_str = "RIGHT_BRACE";

// map associating each filename encountered to a number
static std::map<std::string, int> file_number_map;

static bool encode_filename = true;

static std::string preprocessor_error_detail_prefix = "\n    ";

static const char OUTPUT_SEPARATOR = '\xFE';

// get filename associated to this code
static std::string get_filename(const std::string& file_code)
{
	if(!encode_filename) {
		return file_code;
	}

	std::stringstream s;
	s << file_code;
	int n = 0;
	s >> std::hex >> n;

	for(const auto& p : file_number_map) {
		if(p.second == n) {
			return p.first;
		}
	}

	return "<unknown>";
}

// Get code associated to this filename
static std::string get_file_code(const std::string& filename)
{
	if(!encode_filename) {
		return filename;
	}

	// Current number of encountered filenames
	static int current_file_number = 0;

	int& fnum = file_number_map[utils::escape(filename, " \\")];
	if(fnum == 0) {
		fnum = ++current_file_number;
	}

	std::ostringstream shex;
	shex << std::hex << fnum;

	return shex.str();
}

// decode the filenames placed in a location
static std::string get_location(const std::string& loc)
{
	std::string res;
	std::vector<std::string> pos = utils::quoted_split(loc, ' ');

	if(pos.empty()) {
		return res;
	}

	std::vector<std::string>::const_iterator i = pos.begin(), end = pos.end();
	while(true) {
		res += get_filename(*(i++));

		if(i == end) {
			break;
		}

		res += ' ';
		res += *(i++);

		if(i == end) {
			break;
		}

		res += ' ';
	}

	return res;
}
