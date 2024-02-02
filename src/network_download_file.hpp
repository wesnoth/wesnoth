/*
	Copyright (C) 2003 - 2024
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

namespace network
{
	/**
	 * Initiates a standalone download of a single file from an HTTPS URL.
	 *
	 * @param url The URL of the file to download.
	 * @param local_path The path on the local machine to store the file at.
	 */
    void download(const std::string& url, const std::string& local_path);
}
