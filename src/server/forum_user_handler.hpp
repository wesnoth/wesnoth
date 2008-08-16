#ifndef FORUM_USER_HANDLER_HPP_INCLUDED
#define FORUM_USER_HANDLER_HPP_INCLUDED

#include "user_handler.hpp"

#include <mysql++/mysql++.h>
#include "../md5.hpp"

//! @class A user_handler implementation to link the server
//! with a phpbb3 forum.

// The [user_handler] section in the server configuration
// file could look like this:
//
//[user_handler]
//	db_name=phpbb3
//	db_host=localhost
//	db_user=root
//	db_password=secret
//[/user_handler]


class fuh : public user_handler {
	public:
		fuh(config c);
		~fuh() {}

		void add_user(const std::string& name, const std::string& mail, const std::string& password);
		void remove_user(const std::string& name);

		void clean_up();

		bool login(const std::string& name, const std::string& password, const std::string& seed);
		void user_logged_in(const std::string& name);

		bool user_exists(const std::string& name);

		void password_reminder(const std::string& name);

		std::string user_info(const std::string& name);

		void set_user_detail(const std::string& user, const std::string& detail, const std::string& value);
		std::string get_valid_details();

		// Needed because the hashing algorithm used by phpbb requires some info
		// from the original hash to recreate the same hash
		//! index = 0 returns the hash seed
		//! index = 1 return the salt
		std::string create_pepper(const std::string& name, int index);

	private:
		std::string get_hash(const std::string& user);
		std::string get_mail(const std::string& user);
		time_t get_lastlogin(const std::string& user);
		time_t get_registrationdate(const std::string& user);

		void set_lastlogin(const std::string& user, const time_t& lastlogin);

		std::string db_name_, db_host_, db_user_, db_password_;

		mysqlpp::StoreQueryResult db_query(const std::string& query);
		std::string db_query_to_string(const std::string& query);
		mysqlpp::Connection db_interface_;
};

#endif //FORUM_USER_HANDLER_HPP_INCLUDED
