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

std::string user_handler::create_salt(int length) {
	srand(static_cast<unsigned>(time(nullptr)));

	std::stringstream ss;

	for(int i = 0; i < length; i++) {
		ss << randomness::rng::default_instance().get_random_int(0, 9);
	}

	return  ss.str();
}
