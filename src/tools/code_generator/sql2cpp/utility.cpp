/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "tools/code_generator/sql2cpp/utility.hpp"
#include <fstream>
#include <sstream>

// Why not using read_file from filesystem.cpp?
// Because it adds too many dependencies for a single function...
std::string file2string(const std::string& filename)
{
	std::ifstream s(filename.c_str(), std::ios_base::binary);
	std::stringstream ss;
	ss << s.rdbuf();
	return ss.str();
}
