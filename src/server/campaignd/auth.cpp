/*
	Copyright (C) 2015 - 2024
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

#include "server/campaignd/auth.hpp"

#include "hash.hpp"
#include "serialization/base64.hpp"

#include <ctime>
#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

namespace campaignd
{

namespace auth
{

namespace
{

std::string generate_salt(std::size_t len)
{
	boost::mt19937 mt(std::time(nullptr));
	auto salt = std::string(len, '0');
	boost::uniform_int<> from_str(0, 63); // 64 possible values for base64
	boost::variate_generator< boost::mt19937, boost::uniform_int<>> get_char(mt, from_str);

	for(std::size_t i = 0; i < len; i++) {
		salt[i] = crypt64::encode(get_char());
	}

	return salt;
}

} // end unnamed namespace

bool verify_passphrase(const std::string& passphrase, const std::string& salt, const std::string& hash)
{
	return utils::md5(passphrase, salt).base64_digest() == hash;
}

std::pair<std::string, std::string> generate_hash(const std::string& passphrase)
{
	const auto& salt = generate_salt(16);
	return { salt, utils::md5(passphrase, salt).base64_digest() };
}

} // end namespace auth

} // end namespace campaignd
