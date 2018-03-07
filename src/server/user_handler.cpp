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

#include "server/user_handler.hpp"
#include "config.hpp"
#include "random.hpp"
#include <openssl/rand.h>

#include <ctime>
#include <sstream>

bool user_handler::send_mail(const std::string& to_user,
		const std::string& /*subject*/, const std::string& /*message*/) {

	//If this user is registered at all
	if(!user_exists(to_user)) {
		throw error("Could not send email. No user with the name '" + to_user + "' exists.");
	}

	// If this user did not provide an email
	if(get_mail(to_user).empty()) {
		throw error("Could not send email. The email address of the user '" + to_user + "' is empty.");
	}

	throw user_handler::error("This server is configured not to send email.");
}

void user_handler::init_mailer(const config &) {
}

std::string user_handler::create_unsecure_nonce(int length) {
	srand(static_cast<unsigned>(time(nullptr)));

	std::stringstream ss;

	for(int i = 0; i < length; i++) {
		ss << randomness::rng::default_instance().get_random_int(0, 9);
	}

	return  ss.str();
}

// TODO - This really should be a common function.
// This is duplicated in two or three other places.
// Some are virtual member functions.
namespace {
	const std::string itoa64 = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ;

	std::string encode_hash(const unsigned char* input, unsigned int len) {
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

	class RAND_bytes_exception: public std::exception
	{
	};
}

std::string user_handler::create_secure_nonce()
{
	// Must be full base64 encodings (3 bytes = 4 chars) else we skew the PRNG results
	unsigned char buf [((3 * 32) / 4)];

	if(!RAND_bytes(buf, sizeof(buf))) {
		throw RAND_bytes_exception();
	}

	return encode_hash(buf, sizeof(buf));
}

