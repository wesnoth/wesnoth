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

#ifndef USER_HANDLER_HPP_INCLUDED
#define USER_HANDLER_HPP_INCLUDED

class config;

#include "exceptions.hpp"

#include <string>

/**
 * An interface class to handle nick registration
 * To activate it put a [user_handler] section into the
 * server configuration file
 */
class user_handler {

	// public functions are called by the server
	//
	// private functions are for convenience as they
	// will probably be the same for all user_handler
	// implementations

	public:
		user_handler()
		{
		}

		virtual ~user_handler()
		{
		}

		/**
		 * Adds a user.
		 *
		 * Throws an error containing the error message if adding fails
		 * (e.g. because a user with the same name already exists).
		 */
		virtual void add_user(const std::string& name, const std::string& mail, const std::string& password) =0;

		/**
		 * Removes a user.
		 *
		 * Throws an error containing the error message if removing fails
		 * (e.g. no user with the given name exists).
		 */
		virtual void remove_user(const std::string& name) =0;

		/**
		 * Called by the server once a day.
		 *
		 * Could for example be used for removing users that have not logged in
		 * for a certain amount of time.
		 */
		virtual void clean_up() =0;

		/**
		 * Send a password reminder email to the given user.
		 *
		 * Should throw user_handler::error if sending fails
		 * (e.g. because we cannot send email).
		 */
		virtual void password_reminder(const std::string& name) =0;

		/**
		 * Return true if the given password matches the password for the given user.
		 *
		 * Password could also be a hash
		 * Seed is not needed for clear text log ins
		 * Currently the login procedure in the server and client code is hardcoded
		 * for the forum_user_handler implementation
		 */
		virtual bool login(const std::string& name, const std::string& password, const std::string& seed) =0;

		/** Executed when the user with the given name logged in. */
		virtual void user_logged_in(const std::string& name) =0;

		/**
		 * Returns a string containing info like the last login of this user.
		 *
		 * Formatted for user readable output.
		 */
		virtual std::string user_info(const std::string& name) =0;

		/**
		 * Set data for a given user name.
		 *
		 * Should throw an error on invalid data.
		 */
		virtual void set_user_detail(const std::string& user, const std::string& detail, const std::string& value) =0;

		/** List of details that can be set for this user_handler. */
		virtual std::string get_valid_details() =0;

		/** Returns true if a user with the given name exists. */
		virtual bool user_exists(const std::string& name) =0;

		/** Returns true if the specified user account is usable for logins. */
		virtual bool user_is_active(const std::string& name) =0;

		/** Returns true if this user is a moderator on this server */
		virtual bool user_is_moderator(const std::string& name) =0;

		/** Mark this user as a moderator */
		virtual void set_is_moderator(const std::string& name, const bool& is_moderator) =0;

		struct error : public game::error {
			error(const std::string& message) : game::error(message) {}
		};

		/** Initiate the mailer object. */
		void init_mailer(const config &c);

		/** Create a random string of digits for password encryption. */
		std::string create_salt(int length =8);

		/**
		 * Create custom salt.
		 *
		 * If not needed let it return and empty string or whatever you feel like.
		 */
		virtual std::string create_pepper(const std::string& username) =0;

		/**
		 * Does this user_handler want passwords passed encrypted using phpbb's algorithm?
		 *
		 * Let it return true if it does and false if it does not.
		 */
		virtual bool use_phpbb_encryption() const =0;

	protected:

		/**
		 * Sends an email to the specified address. Requires access to an SMTP server.
		 *
		 * Throws an error if the mail could not be sent.
		 */
		bool send_mail(const std::string& to_user, const std::string& subject, const std::string& message);

		/**
		 * Used in send_mail().
		 *
		 * Should return an empty string when not used.
		 */
		virtual std::string get_mail(const std::string& user) =0;
};

#endif //USER_HANDLER_HPP_INCLUDED
