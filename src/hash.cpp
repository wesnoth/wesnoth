/*
	Copyright (C) 2008 - 2024
	by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "hash.hpp"

#include "serialization/base64.hpp"

#include <string>
#include <sstream>
#include <string.h>
#include <assert.h>

extern "C" {
#include "crypt_blowfish/crypt_blowfish.h"
}

#ifndef __APPLE__

#include <openssl/evp.h>

#else

#include <CommonCrypto/CommonDigest.h>

static_assert(utils::md5::DIGEST_SIZE == CC_MD5_DIGEST_LENGTH, "Constants mismatch");

#endif

namespace {

const std::string hash_prefix = "$H$";

template<std::size_t len>
std::string encode_hash(const std::array<uint8_t, len>& bytes) {
	utils::byte_string_view view{bytes.data(), len};
	return crypt64::encode(view);
}

template<std::size_t len>
std::string hexencode_hash(const std::array<uint8_t, len>& input) {
	std::ostringstream sout;
	sout << std::hex;
	for(uint8_t c : input) {
		sout << static_cast<int>(c);
	}
	return sout.str();
}

}

namespace utils {

md5::md5(const std::string& input) {

#ifndef __APPLE__
	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	unsigned int md5_digest_len = EVP_MD_size(EVP_md5());
	assert(utils::md5::DIGEST_SIZE == md5_digest_len);

	// MD5_Init
	EVP_DigestInit_ex(mdctx, EVP_md5(), nullptr);

	// MD5_Update
	EVP_DigestUpdate(mdctx, input.c_str(), input.size());

	// MD5_Final
	EVP_DigestFinal_ex(mdctx, hash.data(), &md5_digest_len);
	EVP_MD_CTX_free(mdctx);
#else
	CC_MD5(input.data(), static_cast<CC_LONG>(input.size()), hash.data());
#endif

}

int md5::get_iteration_count(const std::string& hash) {
	return crypt64::decode(hash[3]);
}

std::string md5::get_salt(const std::string& hash) {
	return hash.substr(4,8);
}

bool md5::is_valid_prefix(const std::string& hash)
{
	return hash.substr(0,3) == hash_prefix;
}

bool md5::is_valid_hash(const std::string& hash) {
	if(hash.size() != 34) return false;
	if(!is_valid_prefix(hash)) return false;

	const int iteration_count = get_iteration_count(hash);
	if(iteration_count < 7 || iteration_count > 30) return false;

	return true;
}

md5::md5(const std::string& password, const std::string& salt, int iteration_count)
{
	iteration_count = 1 << iteration_count;

	hash = md5(salt + password).raw_digest();
	do {
		hash = md5(std::string(hash.begin(), hash.end()).append(password)).raw_digest();
	} while(--iteration_count);
}

std::string md5::hex_digest() const
{
	return hexencode_hash<DIGEST_SIZE>(hash);
}

std::string md5::base64_digest() const
{
	return encode_hash<DIGEST_SIZE>(hash);
}

bcrypt::bcrypt(const std::string& input)
{
	assert(is_valid_prefix(input));

	iteration_count_delim_pos = input.find('$', 4);
	if(iteration_count_delim_pos == std::string::npos)
		throw hash_error("hash string malformed");
}

bcrypt bcrypt::from_salted_salt(const std::string& input)
{
	bcrypt hash { input };
	std::string bcrypt_salt = input.substr(0, hash.iteration_count_delim_pos + 23);
	if(bcrypt_salt.size() >= BCRYPT_HASHSIZE)
		throw hash_error("hash string too large");
	strcpy(hash.hash.data(), bcrypt_salt.c_str());

	return hash;
}

bcrypt bcrypt::from_hash_string(const std::string& input)
{
	bcrypt hash { input };
	if(input.size() >= BCRYPT_HASHSIZE)
		throw hash_error("hash string too large");
	strcpy(hash.hash.data(), input.c_str());

	return hash;
}

bcrypt bcrypt::hash_pw(const std::string& password, bcrypt& salt)
{
	bcrypt hash;
	if(!php_crypt_blowfish_rn(password.c_str(), salt.hash.data(), hash.hash.data(), BCRYPT_HASHSIZE))
		throw hash_error("failed to hash password");

	return hash;
}

bool bcrypt::is_valid_prefix(const std::string& hash) {
	return ((hash.compare(0, 4, "$2a$") == 0)
	     || (hash.compare(0, 4, "$2b$") == 0)
	     || (hash.compare(0, 4, "$2x$") == 0)
	     || (hash.compare(0, 4, "$2y$") == 0));
}

std::string bcrypt::get_salt() const
{
	std::size_t salt_pos = iteration_count_delim_pos + 23;
	if(salt_pos >= BCRYPT_HASHSIZE)
		throw hash_error("malformed hash");
	return std::string(hash.data(), salt_pos);
}

std::string bcrypt::hex_digest() const
{
	return std::string(hash.data());
}

std::string bcrypt::base64_digest() const
{
	return std::string(hash.data());
}

} // namespace utils
