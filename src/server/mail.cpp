#include "mail.hpp"

#include <iostream>

void mailer::load_mail_cfg(const config& c) {
	mail_cfg.from_address = c["from_address"].empty() ? "NOREPLY@wesnoth.org" : c["from_address"];
	mail_cfg.server = c["server"].empty() ? "127.0.0.1" : c["server"];

	//"username" and "password" may stay empty
	mail_cfg.username = c["username"];
	mail_cfg.password = c["password"];

	if(c["mail_port"].empty()) {
		mail_cfg.port = jwsmtp::mailer::SMTP_PORT;
	} else {
		try {
			mail_cfg.port = lexical_cast_default<unsigned short>(c["port"]);
		} catch (bad_lexical_cast) {
			std::cerr << "Bad lexical cast reading the 'port', using default port 25.\n";
			mail_cfg.port = jwsmtp::mailer::SMTP_PORT;
		}
	}
}

bool mailer::send_mail(const std::string& to_address, const std::string& subject, const std::string& message) {
	jwsmtp::mailer m(to_address.c_str(), mail_cfg.from_address.c_str(), subject.c_str(),
		message.c_str(), mail_cfg.server.c_str(), mail_cfg.port, false);

	if(!(mail_cfg.username.empty())) {
		m.username(mail_cfg.username.c_str());
		m.password(mail_cfg.password.c_str());
	}

	m.send();

	std::cout << "Sent email to " << to_address << " with response '" << m.response() << "'\n";
	if(m.response().substr(0,3) != "250") {
		return false;
	}

	return true;
}

