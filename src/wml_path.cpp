/*
   Copyright (C) 2020 by the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "wml_path.hpp"
#include "filesystem.hpp"

#include "config.hpp"
#include "log.hpp"


namespace {
	utils::string_view name_of_directory(const std::string& path)
	{
		size_t pos = path.rfind('/');
		if(pos != std::string::npos) {
			return utils::string_view(path).substr(0, pos);
		}
		else {
			return utils::string_view();
		}

	}
}


namespace filesystem {


wml_path::wml_path(utils::string_view base, utils::string_view path)
	: base_(base.to_string())
	, path_(path.to_string())
{

}

wml_path wml_path::from_absolute(const std::string& path)
{

	std::string dir = filesystem::directory_name(path);
	std::string base = "./" +  filesystem::base_name(path);

	std::cerr << "wml_path::from_absolute: " << path << " to " << dir << " and " << base << "\n";
	return wml_path(utils::string_view(dir), utils::string_view(base));
}

std::string wml_path::get_abolute_path() const
{
	return get_wml_location(path_, base_);
}

wml_path wml_path::get_relative_path(const std::string& path) const
{
	std::string path2 = path;
	canonical_path(path2, name_of_directory(path_));

	wml_path res = wml_path(utils::string_view(base_), utils::string_view());

	res.path_ = std::move(path2);
	return res;
}

wml_path wml_path::append(const std::string& path) const
{
	std::string path2 = "./" + path;
	canonical_path(path2, path_);

	wml_path res = wml_path(utils::string_view(base_), utils::string_view());

	res.path_ = std::move(path2);
	return res;
}

void wml_path::get_files_in_dir(std::vector<wml_path>* files, std::vector<wml_path>* dirs, bool) const
{
	std::vector<std::string> file_names;
	std::vector<std::string> dir_names;
	filesystem::get_files_in_dir(get_abolute_path(), files ? &file_names : nullptr , dirs ? &dir_names : nullptr, FILE_NAME_ONLY, NO_FILTER, DO_REORDER);
	std::cerr << "get_files_in_dir:\n";
	for(const auto& str: file_names) {
		if(str == "_main.cfg") {
			std::cerr << "    " << str << " " << get_abolute_path() << "\n";
		}
		files->push_back(append(str));
	}
	for(const auto& str: dir_names) {
		dirs->push_back(append(str));
	}
}

bool wml_path::exists() const
{
	return !get_abolute_path().empty();
}

bool wml_path::canonical_path(std::string& filename, utils::string_view currentdir)
{
	if(filename.size() < 2) {
		return false;
	}
	if(filename[0] == '.' && filename[1] == '/') {
		filename = currentdir.to_string() + filename.substr(1);
	}
	if(std::find(filename.begin(), filename.end(), '\\') != filename.end()) {
		return false;
	}
	//resolve /./
	while(true) {
		std::size_t pos = filename.find("/./");
		if(pos == std::string::npos) {
			break;
		}
		filename = filename.replace(pos, 2, "");
	}
	//resolve //
	while(true) {
		std::size_t pos = filename.find("//");
		if(pos == std::string::npos) {
			break;
		}
		filename = filename.replace(pos, 1, "");
	}
	//resolve /../
	while(true) {
		std::size_t pos = filename.find("/..");
		if(pos == std::string::npos) {
			break;
		}
		std::size_t pos2 = filename.find_last_of('/', pos - 1);
		if(pos2 == std::string::npos || pos2 >= pos) {
			return false;
		}
		filename = filename.replace(pos2, pos- pos2 + 3, "");
	}
	if(filename.find("..") != std::string::npos) {
		return false;
	}
	return true;
}

}