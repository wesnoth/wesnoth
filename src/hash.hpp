/*
   Copyright (C) 2008 - 2017 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "global.hpp"

namespace utils {

class hash_base
{
public:
	virtual std::string base64_digest() const = 0;
	virtual std::string hex_digest() const = 0;
	virtual ~hash_base() {}
};

template<size_t sz>
class hash_digest : public hash_base
{
protected:
	std::array<uint8_t, sz> hash;
public:
	static const int DIGEST_SIZE = sz;
	std::array<uint8_t, sz> raw_digest() const {return hash;}
};

class md5 : public hash_digest<16>
{
public:
	static int get_iteration_count(const std::string& hash);
	static std::string get_salt(const std::string& hash);
	static bool is_valid_hash(const std::string& hash);
	explicit md5(const std::string& input);
	md5(const std::string& input, const std::string& salt, int iteration_count = 10);
	virtual std::string base64_digest() const override;
	virtual std::string hex_digest() const override;
};

class sha1 : public hash_digest<20>
{
public:
	explicit sha1(const std::string& input);
	virtual std::string base64_digest() const override;
	virtual std::string hex_digest() const override;
};

} // namespace utils
