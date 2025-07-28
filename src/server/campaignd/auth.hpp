/*
	Copyright (C) 2015 - 2025
	by Iris Morelle <shadowm2006@gmail.com>
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
 * campaignd authentication API.
 */

#pragma once

#include <string>
#include <utility>

namespace campaignd
{

namespace auth
{

/**
 * Verifies the specified plain text passphrase against a salted hash.
 *
 * @param passphrase           Passphrase (user input).
 * @param salt                 Salt string.
 * @param hash                 Base64-encoded salted MD5 hash.
 */
bool verify_passphrase(const std::string& passphrase, const std::string& salt, const std::string& hash);

/**
 * Generates a salted hash from the specified passphrase.
 *
 * @param passphrase           Passphrase (user input).
 *
 * @return A pair consisting of the salt (in @a first) and Base64-encoded MD5
 *         hash (in @a second).
 */
std::pair<std::string, std::string> generate_hash(const std::string& passphrase);

} // end namespace auth

} // end namespace campaignd
