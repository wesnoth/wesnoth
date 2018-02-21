/*
   Copyright (C) 2008 - 2018 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
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
#include "exceptions.hpp"
#include "bcrypt/bcrypt.h"

namespace utils {

struct hash_error : public game::error
{
	hash_error(const std::string& message) : game::error(message) {}
};

class hash_base
{
public:
	virtual std::string base64_digest() const = 0;
	virtual std::string hex_digest() const = 0;
	virtual ~hash_base() {}
};

template<size_t sz, typename T = uint8_t>
class hash_digest : public hash_base
{
protected:
	std::array<T, sz> hash;
public:
	static const int DIGEST_SIZE = sz;
	std::array<T, sz> raw_digest() const {return hash;}
};

class md5 : public hash_digest<16>
{
public:
	static int get_iteration_count(const std::string& hash);
	static std::string get_salt(const std::string& hash);
	static bool is_valid_prefix(const std::string& hash);
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

class bcrypt : public hash_digest<BCRYPT_HASHSIZE, char>
{
	bcrypt() {}
	bcrypt(const std::string& input);

public:
	static bcrypt from_salted_salt(const std::string& input);
	static bcrypt from_hash_string(const std::string& input);
	static bcrypt hash_pw(const std::string& password, bcrypt& salt);

	std::size_t iteration_count_delim_pos;

	static bool is_valid_prefix(const std::string& hash);
	std::string get_salt() const;
	virtual std::string hex_digest() const override;
	virtual std::string base64_digest() const override;
};

} // namespace utils
