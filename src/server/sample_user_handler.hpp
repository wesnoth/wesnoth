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

#pragma once

#include "user_handler.hpp"

#include <map>
#include <vector>

/**
 * An example of how to implement user_handler.
 * If you use this on anything real, you are insane.
 */
class suh : public user_handler {
	public:
		suh(config c);

		void add_user(const std::string& name, const std::string& mail, const std::string& password);
		void remove_user(const std::string& name);

		void clean_up();

		bool login(const std::string& name, const std::string& password, const std::string&);
		void user_logged_in(const std::string& name);

		bool user_exists(const std::string& name);
		bool user_is_active(const std::string& name);

		bool user_is_moderator(const std::string& name);
		void set_is_moderator(const std::string& name, const bool& is_moderator);

		ban_info user_is_banned(const std::string& name, const std::string&);

		std::string user_info(const std::string& name);

		struct user {
			user() :
					password(),
					realname(),
					mail(),
					lastlogin(time(nullptr)),
					registrationdate(time(nullptr)),
					is_moderator(false) {}
			std::string password;
			std::string realname;
			std::string mail;
			time_t lastlogin;
			time_t registrationdate;
			bool is_moderator;
		};

		void set_user_detail(const std::string& user, const std::string& detail, const std::string& value);
		std::string get_valid_details();

		std::string extract_salt(const std::string&) { return ""; }
		bool use_phpbb_encryption() const { return false; }

		std::string get_uuid();
		void db_insert_game_info(const std::string&, int, const std::string&, const std::string&, const std::string&, const std::string&, int, int, int, int, const std::string&, const std::string&, const std::string&, const std::string&){}
		void db_update_game_end(const std::string&, int, const std::string&){}
		void db_insert_game_player_info(const std::string&, int, const std::string&, int, int, const std::string&, const std::string&, const std::string&, const std::string&){}
		void db_insert_modification_info(const std::string&, int, const std::string&, const std::string&, const std::string&){}
		void db_set_oos_flag(const std::string&, int){}

	private:
		std::string get_mail(const std::string& user);
		std::string get_password(const std::string& user);
		std::string get_realname(const std::string& user) ;
		time_t get_lastlogin(const std::string& user);
		time_t get_registrationdate(const std::string& user);

		void check_name(const std::string& name);
		void check_mail(const std::string& mail);
		void check_password(const std::string& password);
		void check_realname(const std::string& realname);

		void set_mail(const std::string& user, const std::string& mail);
		void set_password(const std::string& user, const std::string& password);
		void set_realname(const std::string& user, const std::string& realname);

		void set_lastlogin(const std::string& user, const time_t& lastlogin);

		int user_expiration_;

		std::map <std::string,user> users_;
		std::vector<std::string> users();
};
