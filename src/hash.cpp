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

#include "hash.hpp"

#include <iostream>
#include <string>
#include <SDL_platform.h>

#ifndef __IPHONEOS__

#include <openssl/sha.h>
#include <openssl/md5.h>

static_assert(utils::md5::DIGEST_SIZE == MD5_DIGEST_LENGTH, "Constants mismatch");
static_assert(utils::sha1::DIGEST_SIZE == SHA_DIGEST_LENGTH, "Constants mismatch");

#else

#include <CommonCrypto/CommonDigest.h>

static_assert(utils::md5::DIGEST_SIZE == CC_MD5_DIGEST_LENGTH, "Constants mismatch");
static_assert(utils::sha1::DIGEST_SIZE == CC_SHA1_DIGEST_LENGTH, "Constants mismatch");

#endif

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

}

namespace utils {

md5::md5(const std::string& input) {

#ifndef __IPHONEOS__
	MD5_CTX md5_worker;
	MD5_Init(&md5_worker);
	MD5_Update(&md5_worker, input.data(), input.size());
	MD5_Final(hash.data(), &md5_worker);
#else
	CC_MD5(input.data(), (CC_LONG) input.size(), hash.data());
#endif

}

int md5::get_iteration_count(const std::string& hash) {
	return itoa64.find_first_of(hash[3]);
}

std::string md5::get_salt(const std::string& hash) {
	return hash.substr(4,8);
}

bool md5::is_valid_hash(const std::string& hash) {
	if(hash.size() != 34) return false;
	if(hash.substr(0,3) != hash_prefix) return false;

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
	return encode_hash<DIGEST_SIZE>(hash);
}

sha1::sha1(const std::string& str)
{
#ifndef __IPHONEOS__
	SHA_CTX hasher;
	SHA1_Init(&hasher);
	SHA1_Update(&hasher, str.data(), str.size());
	SHA1_Final(hash.data(), &hasher);
#else
	CC_MD5(str.data(), (CC_LONG) str.size(), hash.data());
#endif
}

std::string sha1::hex_digest() const
{
	return encode_hash<DIGEST_SIZE>(hash);
}

} // namespace utils
