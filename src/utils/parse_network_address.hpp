/*
	Copyright (C) 2019 - 2025
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

#pragma once

#include <string>

/**
 * Parse a host:port style network address, supporting [] notation for ipv6 addresses
 * @param address
 * @param default_port the port to return if address doesn't have it specified
 */
std::pair<std::string, std::string> parse_network_address(const std::string& address, const std::string& default_port);

