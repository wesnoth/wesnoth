#ifndef MAIL_HPP_INCLUDED
#define MAIL_HPP_INCLUDED

#include "../global.hpp"

#include "../config.hpp"
#include "jwsmtp/jwsmtp.h"

#include <string>

//! @class A helper classe for sending email using the jwsmtp library

// The [mail] section in the server configuration
// file could look like this:
//
//[mail]
//	server=localhost
//	username=user
//	password=secret
//	from_address=noreply@wesnoth.org
//[/mail]

class mailer {
	public:
		mailer(const config& c) {
			load_mail_cfg(c);
		}

		struct {
			std::string from_address;
			std::string server;
			unsigned short port;
			std::string username;
			std::string password;
		} mail_cfg;

		void load_mail_cfg(const config& c);
		bool send_mail(const std::string& to_address, const std::string& subject, const std::string& message);
};

#endif //MAIL_HPP_INCLUDED
