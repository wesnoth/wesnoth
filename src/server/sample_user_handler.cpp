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

#include "server/sample_user_handler.hpp"

#include "config.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"

#include <iostream>

suh::suh(config c)
	: user_expiration_(0)
	, users_()
{

	if(c["user_expiration"].empty()) {
		user_expiration_ = 60;
	} else {
		try {
			user_expiration_ = lexical_cast_default<int>(c["user_expiration"]);
		} catch (bad_lexical_cast) {
			std::cerr << "Bad lexical cast reading 'user_expiration', using default value.\n";
			user_expiration_ = 60;
		}
	}
}

void suh::add_user(const std::string& name, const std::string& mail, const std::string& password) {
	if(user_exists(name)) throw error("This nickname is already registered");

	users_.insert(std::pair<std::string, user>(name, user()));

	set_password(name, password);
	set_mail(name, mail);

	user_logged_in(name);
}

void suh::remove_user(const std::string& name) {
	if(!user_exists(name)) throw error("This nickname is not registered");

	users_.erase(name);
}

bool suh::user_exists(const std::string& name) {
	std::map<std::string,user>::const_iterator u = users_.find(name);

	return (u != users_.end());
}

bool suh::user_is_active(const std::string& /*name*/) {
	// FIXME: add support for inactive users maybe?
	return true;
}

std::vector<std::string> suh::users() {
	std::vector<std::string> users;
	for(std::map<std::string,user>::const_iterator u = users_.begin(); u != users_.end(); ++u) {
		users.push_back(u->first);
	}
	return users;
}

void suh::set_user_detail(const std::string& user, const std::string& detail, const std::string& value) {
	if(detail == "mail") {
		set_mail(user, value);
	} else if (detail == "password") {
		set_password(user, value);
	} else if (detail == "realname") {
		set_realname(user, value);
	} else {
		throw error("Invalid usersdetail '" + detail + "'. Valid details are: " + get_valid_details());
	}
}

std::string suh::get_valid_details() {
	return "'mail', 'password', 'realname';";
}


bool suh::user_is_moderator(const std::string& name) {
	if(!user_exists(name)) return false;
	return users_[name].is_moderator;
}

void suh::set_is_moderator(const std::string& name, const bool& is_moderator) {
	if(!user_exists(name)) return;
	users_[name].is_moderator = is_moderator;
}

void suh::set_mail(const std::string& user, const std::string& mail) {
	check_mail(mail);
	users_[user].mail = mail;
}

void suh::set_password(const std::string& user, const std::string& password) {
	check_password(password);
	users_[user].password = password;
}

void suh::set_realname(const std::string& user, const std::string& realname) {
	check_realname(realname);
	users_[user].realname = realname;
}

//--
// set_lastlogin() is not called by the server via set_user_detail()
// and thus must not throw user_handler::error

void suh::set_lastlogin(const std::string& user, const time_t& lastlogin) {
	users_[user].lastlogin = lastlogin;
}

//---

std::string suh::get_mail(const std::string& user) {
	return users_[user].mail;
}

std::string suh::get_password(const std::string& user) {
	return users_[user].password;
}

std::string suh::get_realname(const std::string& user) {
	return users_[user].realname;
}

time_t suh::get_lastlogin(const std::string& user) {
	return users_[user].lastlogin;
}

time_t suh::get_registrationdate(const std::string& user) {
	return users_[user].registrationdate;
}

//---

void suh::check_name(const std::string& name) {
	if(!utils::isvalid_username(name)) {
		throw error("This username contains invalid "
				"characters. Only alpha-numeric characters, underscores and hyphens"
				"are allowed.");
	}
	if(name.size() > 20) {
		throw error("This username is too long. Usernames must be 20 characters or less.");
	}
}

void suh::check_mail(const std::string& /*mail*/) {
	return;
}

void suh::check_password(const std::string& password) {
	if(!utils::isvalid_username(password)) {
		throw error("Password contains invalid characters");
	}
}

void suh::check_realname(const std::string& realname) {
	if(realname.size() > 50) {
		throw error("This name is too long. Names must be 50 characters or less");
	}
}


void suh::clean_up() {
	// Remove users that have not logged in for user_expiration_ days:
	// Check if the last login of this user exceeds the
	// expiration limit

	//The expiration time set to 0 means no expiration limit
	if(!user_expiration_) {
		return;
	}

	time_t now = time(nullptr);

	//A minute has 60 seconds, an hour 60 minutes and
	//a day 24 hours.
	time_t limit = user_expiration_ * 60 * 60 * 24;

	std::vector<std::string> us = users();
	for(std::vector<std::string>::const_iterator u = us.begin(); u != us.end(); ++u) {
		if((now - get_lastlogin(*u)) > limit) {
			std::cout << "User '" << *u << "' exceeds expiration limit.\n";
			remove_user(*u);
		}
	}
}

bool suh::login(const std::string& name, const std::string& password, const std::string&) {
	return password == get_password(name);
}

void suh::user_logged_in(const std::string& name) {
	set_lastlogin(name, time(nullptr));
}

void suh::password_reminder(const std::string& name) {
	std::stringstream msg;
	msg << "Hello " << name << ",\nyour password is '" << get_password(name) << "'.\n\nHave fun playing Wesnoth :)";
	send_mail(name, "Wesnoth Password Reminder", msg.str());
}

std::string suh::user_info(const std::string& name) {
	if(!user_exists(name)) throw error("No user with the name '" + name + "' exists.");

	time_t reg_date = get_registrationdate(name);
	time_t ll_date = get_lastlogin(name);

	std::string reg_string = ctime(&reg_date);
	std::string ll_string;

	if(ll_date) {
		ll_string = ctime(&ll_date);
	} else {
		ll_string = "Never\n";
	}

	std::stringstream info;
	info << "Name: " << name << "\n"
		 << "Real name: " << get_realname(name) << "\n"
		 << "Registered: " << reg_string
		 << "Last login: " << ll_string;
	if(!user_is_active(name)) {
		info << "This account is currently inactive.\n";
	}
	return info.str();
}
