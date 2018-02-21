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

#include "hash.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <string.h>
#include <assert.h>

#include <openssl/sha.h>
#include <openssl/md5.h>

static_assert(utils::md5::DIGEST_SIZE == MD5_DIGEST_LENGTH, "Constants mismatch");
static_assert(utils::sha1::DIGEST_SIZE == SHA_DIGEST_LENGTH, "Constants mismatch");

namespace {

const std::string itoa64 = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ;
const std::string hash_prefix = "$H$";

template<size_t len>
std::string encode_hash(const std::array<uint8_t, len>& input) {
	std::string encoded_hash;

	unsigned int i = 0;
	do {
		unsigned value = input[i++];
		encoded_hash.append(itoa64.substr(value & 0x3f,1));
		if(i < len)
			value |= static_cast<int>(input[i]) << 8;
		encoded_hash.append(itoa64.substr((value >> 6) & 0x3f,1));
		if(i++ >= len)
			break;
		if(i < len)
			value |= static_cast<int>(input[i]) << 16;
		encoded_hash.append(itoa64.substr((value >> 12) & 0x3f,1));
		if(i++ >= len)
			break;
		encoded_hash.append(itoa64.substr((value >> 18) & 0x3f,1));
	} while (i < len);

	return encoded_hash;
}

template<size_t len>
std::string hexencode_hash(const std::array<uint8_t, len>& input) {
	std::ostringstream sout;
	sout << std::hex;
	for(uint8_t c : input) {
		sout << int(c);
	}
	return sout.str();
}

}

namespace utils {

md5::md5(const std::string& input) {
	MD5_CTX md5_worker;
	MD5_Init(&md5_worker);
	MD5_Update(&md5_worker, input.data(), input.size());
	MD5_Final(hash.data(), &md5_worker);
}

int md5::get_iteration_count(const std::string& hash) {
	return itoa64.find_first_of(hash[3]);
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

sha1::sha1(const std::string& str)
{
	SHA_CTX hasher;
	SHA1_Init(&hasher);
	SHA1_Update(&hasher, str.data(), str.size());
	SHA1_Final(hash.data(), &hasher);
}

std::string sha1::hex_digest() const
{
	return hexencode_hash<DIGEST_SIZE>(hash);
}

std::string sha1::base64_digest() const
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
	if(bcrypt_hashpw(password.c_str(), salt.hash.data(), hash.hash.data()) != 0)
		throw hash_error("failed to hash password");

	return hash;
}

bool bcrypt::is_valid_prefix(const std::string& hash) {
	return hash.compare(0, 4, "$2y$") == 0;
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
