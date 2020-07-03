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

/** @file */

#pragma once

#include <string>

std::string lineno_string(const std::string& lineno);

class include_history {
public:
	std::string& data() { return data_; }

	std::string get_location():

	static std::string get_filename(const std::string& file_code);
	static std::string get_file_code(const std::string& filename);
private:
	std::string data_;
};

