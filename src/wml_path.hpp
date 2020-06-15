/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
 * Declarations for File-IO.
 */

#pragma once

#include <algorithm>
#include <ctime>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "exceptions.hpp"
#include "serialization/string_utils.hpp"

namespace filesystem {

class wml_path {
public:

	wml_path()
		: base_()
		, path_()
	{

	}

	explicit wml_path(const std::string& path)
		: base_()
		, path_(path)
	{}
	/// "unsafe" constructor
	static wml_path from_absolute(const std::string& path);
	/// helper function, also used by lua
	static bool canonical_path(std::string& filename, utils::string_view currentdir);
	/// returns an abolute path to open the file.
	std::string get_abolute_path() const;
	///
	/// 
	wml_path get_relative_path(const std::string& path) const;
	wml_path append(const std::string& path) const;
	void get_files_in_dir(std::vector<wml_path>* files, std::vector<wml_path>* dirs, bool do_reroder = true)  const;
	const std::string& safe_path() const { return path_; }
	///
	bool exists() const;
private:
	wml_path(utils::string_view base, utils::string_view path);
	std::string base_;
	std::string path_;
};
}
using wml_path = filesystem::wml_path;
