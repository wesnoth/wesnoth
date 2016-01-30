/*
   Copyright (C) 2008 - 2016 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <iostream>
#include <string>

#include "md5.hpp"
#include "hash.hpp"

namespace util {

const std::string itoa64 = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ;
const std::string hash_prefix = "$H$";

unsigned char* md5(const std::string& input) {
	MD5 md5_worker;
	md5_worker.update(const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(input.c_str())), input.size());
	md5_worker.finalize();
	return md5_worker.raw_digest();
}

int get_iteration_count(const std::string& hash) {
	return itoa64.find_first_of(hash[3]);
}

std::string get_salt(const std::string& hash) {
	return hash.substr(4,8);
}

bool is_valid_hash(const std::string& hash) {
	if(hash.size() != 34) return false;
	if(hash.substr(0,3) != hash_prefix) return false;

	const int iteration_count = get_iteration_count(hash);
	if(iteration_count < 7 || iteration_count > 30) return false;

	return true;
}

std::string encode_hash(unsigned char* input) {
	std::string encoded_hash;

	unsigned int i = 0;
	do {
		unsigned value = input[i++];
		encoded_hash.append(itoa64.substr(value & 0x3f,1));
		if(i < 16)
			value |= static_cast<int>(input[i]) << 8;
		encoded_hash.append(itoa64.substr((value >> 6) & 0x3f,1));
		if(i++ >= 16)
			break;
		if(i < 16)
			value |= static_cast<int>(input[i]) << 16;
		encoded_hash.append(itoa64.substr((value >> 12) & 0x3f,1));
		if(i++ >= 16)
			break;
		encoded_hash.append(itoa64.substr((value >> 18) & 0x3f,1));
	} while (i < 16);

	return encoded_hash;
}

std::string create_hash(const std::string& password, const std::string& salt, int iteration_count) {
	iteration_count = 1 << iteration_count;

	unsigned char* output = md5(salt + password);
	do {
		output = md5(std::string(reinterpret_cast<char*>(output), reinterpret_cast<char*>(output) + 16).append(password));
	} while(--iteration_count);

	return encode_hash(output);
}

} // namespace util
