/*
	Copyright (C) 2019 - 2021
	by Sergey Popov <loonycyborg@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "utils/parse_network_address.hpp"

#include <regex>
#include <string>

std::pair<std::string, std::string> parse_network_address(const std::string& address, const std::string& default_port)
{
	const char* address_re = "\\[([[:xdigit:]:]*)\\](:([[:alnum:]]*))?|([[:alnum:]-_\\.]{1,253})(:([[:alnum:]]*))?";

	std::smatch m;
	std::regex_match(address, m, std::regex(address_re));

	if(!m[1].str().empty()) {
		return { m[1], m[3].str().empty() ? default_port : m[3] };
	}
	if(!m[4].str().empty()) {
		return { m[4], m[6].str().empty() ? default_port : m[6] };
	}

	throw std::runtime_error("invalid address");
}
