#ifndef MAIL_HPP_INCLUDED
#define MAIL_HPP_INCLUDED

#include "../global.hpp"

#include "../config.hpp"
#include "jwsmtp/jwsmtp.h"

#include <string>

// The [mail] section in the server configuration
// file could look like this:
//
//[mail]
//	server=localhost
//	username=user
//	password=secret
//	from_address=noreply@wesnoth.org
//[/mail]

/**
 * A helper classe for sending email using the jwsmtp library.
 */
class mailer {
	public:
		mailer(const config& c) :
			mail_cfg()
		{
			load_mail_cfg(c);
		}

		struct tmail_cfg {

			tmail_cfg() :
				from_address(),
				server(),
				port(0),
				username(),
				password()
			{
			}

			std::string from_address;
			std::string server;
			unsigned short port;
			std::string username;
			std::string password;
		};

		tmail_cfg mail_cfg;

		void load_mail_cfg(const config& c);
		bool send_mail(const std::string& to_address, const std::string& subject, const std::string& message);
};

#endif //MAIL_HPP_INCLUDED
