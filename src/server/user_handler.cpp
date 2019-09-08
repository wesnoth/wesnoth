/*
   Copyright (C) 2008 - 2018 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "serialization/base64.hpp"

#ifndef __APPLE__
#include <openssl/rand.h>
#else
#include <cstdlib>
#endif

#include <array>
#include <ctime>
#include <sstream>

std::string user_handler::create_unsecure_nonce(int length) {
	srand(static_cast<unsigned>(std::time(nullptr)));

	std::stringstream ss;

	for(int i = 0; i < length; i++) {
		ss << randomness::rng::default_instance().get_random_int(0, 9);
	}

	return  ss.str();
}

#ifndef __APPLE__
namespace {
	class RAND_bytes_exception: public std::exception
	{
	};
}
#endif

std::string user_handler::create_secure_nonce()
{
	// Must be full base64 encodings (3 bytes = 4 chars) else we skew the PRNG results
	std::array<unsigned char, (3 * 32) / 4> buf;

#ifndef __APPLE__
	if(!RAND_bytes(buf.data(), buf.size())) {
		throw RAND_bytes_exception();
	}
#else
	arc4random_buf(buf.data(), buf.size());
#endif

	return base64::encode({buf.data(), buf.size()});
}

