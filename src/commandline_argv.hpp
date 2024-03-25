/*
	Copyright (C) 2020 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include <string>
#include <vector>

/**
 * Reads argv into a vector of STL strings.
 *
 * @note Both parameters are ignored on Windows in order to obtain guaranteed
 *       Unicode-safe versions through the Win32 API. Do NOT try to pass values
 *       other than the ones you got from main() here.
 */
std::vector<std::string> read_argv(int argc, char** argv);
