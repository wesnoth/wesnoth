/*
	Copyright (C) 2020 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "commandline_argv.hpp"

#ifdef _WIN32

#include "serialization/unicode_cast.hpp"

#include <windows.h>

namespace {

bool win32_parse_single_arg(const char*& next, const char* end, std::string& res)
{
	// strip leading whitespace
	while(next != end && *next == ' ') {
		++next;
	}

	if(next == end) {
		return false;
	}

	bool is_escaped = false;

	for(; next != end; ++next) {
		if(*next == ' ' && !is_escaped) {
			break;
		} else if(*next == '"' && !is_escaped) {
			is_escaped = true;
			continue;
		} else if(*next == '"' && is_escaped && next + 1 != end && *(next + 1) == '"') {
			res.push_back('"');
			++next;
			continue;
		} else if(*next == '"' && is_escaped) {
			is_escaped = false;
			continue;
		} else {
			res.push_back(*next);
		}
	}

	return true;
}

std::vector<std::string> win32_read_argv(const std::string& input)
{
	const char* start = &input[0];
	const char* end = start + input.size();

	std::string buffer;
	std::vector<std::string> res;

	while(win32_parse_single_arg(start, end, buffer)) {
		res.emplace_back().swap(buffer);
	}

	return res;
}

}

#endif

std::vector<std::string> read_argv([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
#ifdef _WIN32
	// On Windows, argv is ANSI-encoded by default. Wesnoth absolutely needs to
	// work with UTF-8 values in order to avoid losing or corrupting
	// information from the command line.
	auto flat_cmdline = unicode_cast<std::string>(std::wstring{GetCommandLineW()});
	return win32_read_argv(flat_cmdline);
#else
	std::vector<std::string> args;
	args.reserve(argc);
	for(int i = 0; i < argc; ++i) {
		args.emplace_back(argv[i]);
	}
	return args;
#endif
}
