/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

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
