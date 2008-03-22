#include "user_handler.hpp"

#include "../log.hpp"
#include "../filesystem.hpp"
#include "../serialization/parser.hpp"
#include "../serialization/preprocessor.hpp"
#include "../serialization/string_utils.hpp"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <vector>


user_handler::user_handler(const std::string& users_file) :
        users_file_(users_file),
        cfg_(read_config())
{
    load_config();
    clean_up();
}

user_handler::~user_handler() {
    save_config();
}

config user_handler::read_config() const {
	config configuration;
	if (users_file_ == "") return configuration;
	scoped_istream stream = preprocess_file(users_file_);
	std::string errors;
	try {
		read(configuration, *stream, &errors);
		if (errors.empty()) {
			std::cout << "Read user_handler configuration from file: '" << users_file_
				<< "' read.\n";
		} else {
			std::cerr << "ERROR: Errors reading configuration file: '"
				<< errors << "'.\n";
		}
	} catch(config::error& e) {
		std::cerr << "ERROR: Could not read configuration file: '"
			<< users_file_ << "': '" << e.message << "'.\n";
	}
	return configuration;
}

void user_handler::load_config() {

    //Load configuration and initialize with default values if we don't find values

        if(cfg_["username_expiration_limit"].empty()) {
            unsigned short default_time = 60;
            username_expiration_limit_ = default_time;
            cfg_["username_expiration_limit"] = lexical_cast_default<std::string>(default_time);
        } else {
            try {
                username_expiration_limit_ = lexical_cast_default<unsigned short>(cfg_["username_expiration_limit"]);
            } catch (bad_lexical_cast) {
                std::cerr << "Bad lexical cast reading 'username_expiration_limit', using default value.\n";
                unsigned short default_time = 60;
                username_expiration_limit_ = default_time;
                cfg_["username_expiration_limit"] = lexical_cast_default<std::string>(default_time);
            }
        }

    #ifndef NO_MAIL

    if(mail()) {
        config& mail = *(cfg_.child("mail"));

        mail["from_address"] = mail["from_address"].empty() ? "NOREPLY@wesnoth.org" : mail["from_address"];
        mail["mail_server"] = mail["mail_server"].empty() ? "127.0.0.1" : mail["mail_server"];

        //"mail_user" and "mail_password" may stay empty

        if(mail["mail_port"].empty()) {
            unsigned short default_port = jwsmtp::mailer::SMTP_PORT;
            mail_port_ = default_port;
            mail["mail_port"] = lexical_cast_default<std::string>(default_port);
        } else {
            try {
                mail_port_ = lexical_cast_default<unsigned short>(mail["mail_port"]);
            } catch (bad_lexical_cast) {
                std::cerr << "Bad lexical cast reading the 'mail_port', using default port.\n";
                unsigned short default_port = jwsmtp::mailer::SMTP_PORT;
                mail_port_ = default_port;
                mail["mail_port"] = lexical_cast_default<std::string>(default_port);
            }
        }
    }

    #endif //NO_MAIL

    //Check if we already have users and
    //if we don't create a child for them
    if(cfg_.child("users")) {
        users_ = cfg_.child("users");
    } else {
        users_ = &(cfg_.add_child("users"));
    }

    std::cout << cfg_ << std::endl;
}

void user_handler::save_config() {
	try {
		scoped_ostream users_file = ostream_file(users_file_);
		write(*users_file , cfg_);
		std::cout << "Users file written to '" << users_file_ << "'\n";
	} catch(io_exception&) {
		std::cerr << "ERROR: Writing to users file '" << users_file_ << "'\n";
	}
}

bool user_handler::send_mail(const char* to_address, const char* subject, const char* message) {

    #ifndef NO_MAIL

    //if we cannot send emails at all we of course also cannot send this one
    if(!mail()) {
        return false;
    }

    config& mail = *(cfg_.child("mail"));

    jwsmtp::mailer m(to_address, mail["from_address"].c_str(), subject, message,
            mail["mail_server"].c_str(), mail_port_, false);
    if(!(mail["mail_user"].empty())) {
        m.username(mail["mail_user"]);
        m.password( mail["mail_password"]);
    }
    //! @todo Sending the mail in a new thread
    //! (as suggested on http://johnwiggins.net/jwsmtp/)
    //! might be a very good idea.
    //! To bad I am not familiar with boost::thread

    m.send();

    std::cout << "Sent email to " << to_address << " with response '" << m.response() << "'\n";

    if(m.response().substr(0,3) != "250") {
        return false;
    }

    return true;

    #endif //NO_MAIL

    return false;

}

void user_handler::clean_up() {
    std::cout << "User handler clean up...\n";
    remove_dead_users();
    std::cout << "Clean up finished\n";
}

void user_handler::remove_dead_users() {
    //username_expiration_limit_ set to 0 means
    //no expiration limit
    if(!username_expiration_limit_) {
        return;
    }

    time_t now = time(NULL);
    //A minute has 60 seconds, an hour 60 minutes and
    //a day 24 hours.
    //Thus a day has 60 * 60 * 24 = 86400 seconds
    time_t limit = username_expiration_limit_ * 86400;

    const config::child_map& user_childs = users_->all_children();
    for(config::child_map::const_iterator i = user_childs.begin(); i != user_childs.end(); ++i) {
        const config& user = *(users_->child(i->first));
        time_t last_login = lexical_cast_default<time_t>(user["last_login"]);
        if((now - last_login) > limit) {
            std::cout << "User '" << i->first << "' exceeds expiration date: ";
            remove_user(i->first);
        }
    }

    save_config();
}

void user_handler::add_user(const std::string& name,
        const std::string& mail, const std::string& password) {

    // Check if provided values are sane
    // (e.g. the email is either empty or looks like user@domain)
    // We also check the email with NO_MAIL defined because we
    // might later decide to run the server with mailing enabled
    check_password(password);
    check_mail(mail);

    //Check if this user already exists
    if(user_exists(name)) {
        throw error("Could not add new user. A user with the name '" + name + "' already exists.");
    }

    //Check if the given email is not yet registered
    if(!mail.empty()) {
        const config::child_map& user_childs = users_->all_children();
        for(config::child_map::const_iterator i = user_childs.begin(); i != user_childs.end(); ++i) {
            if((*(users_->child(i->first)))["mail"] == mail) {
                throw error("Could not add new user. The email address '" + mail + "' is already in use.");
            }
        }
    }

    config& user = users_->add_child(name);
    user["mail"] = mail;
    user["password"] = password;

    std::string now = lexical_cast_default<std::string>(time(NULL));
    user["registration_date"] = now;
    user["last_login"] = now;

    //! @todo To save performance it we should of course not save
    //! the whole config everytime something changes
    save_config();

    std::cout << "Created new user '" << name << "'\n";

    //I don't think we need to send the user details via email,
    //we don't require any account activation anyways.
}



void user_handler::password_reminder(const std::string& name) {
    #ifndef NO_MAIL

    if(!user_exists(name)) {
        throw error("Could not send password reminder. No user with the name '" + name + "' exists.");
    }

    config& user = *(users_->child(name));

    if(user["mail"].empty()) {
        throw error("Could not send password reminder. The email address of the user '" + name + "' is empty.");
    }

    std::stringstream msg;
    msg << "Hello " << name << ",\n\n" <<
            "Your password is '" << user["password"] << "'.\n\n" <<
            "Have fun playing at Wesnoth!";

    //If sending does not return true warn that no message was sent.
    if(!(send_mail(user["mail"].c_str(), "Wesnoth Multiplayer Server Password Reminder", msg.str().c_str()))) {
        throw error("Could not send password reminder. There was an error sending the reminder email.");
    }
    return;

    #endif //NO_MAIL

    throw error("Could not send password reminder. This server is configured not to send emails.");

}

void user_handler::user_logged_in(const std::string& name) {
    if(!user_exists(name)) {
        //No exception here because this function is called by the server, not by users
        return;
    }

    config& user = *(users_->child(name));
    user["last_login"] = lexical_cast_default<std::string>(time(NULL));

    //! @todo To save performance it we should of course not save
    //! the whole config everytime something changes
    save_config();

    //Should we keep track of the IPs used for logging into this account?
}

void user_handler::remove_user(const std::string& name) {
    //Return if the user does not exist
    if(!user_exists(name)) {
        throw error("Could not remove user. No user with the name '" + name + "' exists.");
    }

    users_->remove_child(name, 0);

    //! @todo To save performance it we should of course not save
    //! the whole config everytime something changes
    save_config();

    std::cout << "Removed user '" << name << "'\n";
}

bool user_handler::login(const std::string& name, const std::string& password) {
    //Return if the user does not exist
    if(!user_exists(name)) {
        return false;
    }

    config& user = *(users_->child(name));
    return (user["password"] == password);
}

void user_handler::set_user_attribute(const std::string& name,
        const std::string& attribute, const std::string& value) {

    //Return if the user does not exist
    if(!user_exists(name)) {
        throw error("Could not set attribute '" + attribute  + "' for user '" + name +
        "'. No user with the name with this name exists.");
    }

    config& user = *(users_->child(name));
    user[attribute] = value;

    //! @todo To save performance it we should of course not save
    //! the whole config everytime something changes
    save_config();
}

bool user_handler::user_exists(const std::string& name) {
    return ((users_->child(name)));
}

bool user_handler::mail() {

    //We should not even get to call this when compiling with NO_MAIL
    //but some doulbe safety can never hurt :)
    #ifdef NO_MAIL
    return false;
    #endif

    return (cfg_.child("mail"));
}

void user_handler::set_mail(const std::string& user, const std::string& mail) {
    check_mail(mail);
    set_user_attribute(user, "mail", mail);
}

void user_handler::set_password(const std::string& user, const std::string& password) {
    check_password(password);
    set_user_attribute(user, "password", password);
}

void user_handler::set_realname(const std::string& user, const std::string& realname) {
    set_user_attribute(user, "realname", realname);
}

void user_handler::check_mail(const std::string& mail) {
    if(!(mail.empty() ||utils::isvalid_email(mail))) {
        throw error("The email adress '" + mail + "' appears to be invalid.");
    }
}

void user_handler::check_password(const std::string& password) {
    //I guess it is a good idea to have the same restrictions for password as for usernames
    if (!utils::isvalid_username(password)) {
        throw error( "This password contains invalid "
            "characters. Only alpha-numeric characters, underscores and hyphens"
			"are allowed.");
    }
}

std::string user_handler::user_info(const std::string& name) {
    if(!user_exists(name)) {
        throw error("No user with the name '" + name + "' exists.");
    }

    config& user = *(users_->child(name));

    char registration_date[99];
    char last_login[99];

    const time_t& reg_t = lexical_cast_default<time_t>(user["registration_date"]);
    const time_t& last_t = lexical_cast_default<time_t>(user["last_login"]);

    strftime(registration_date, 99, "%c", localtime(&reg_t));
    strftime(last_login, 99, "%c", localtime(&last_t));

    std::stringstream res;
    res << "Username: " << name << "\n"
            << (user["realname"].empty() ? "" : "Real name: " + user["realname"] + "\n")
            << (user["mail"].empty() ? "" : "Email: " + user["mail"] + "\n")
            << "Registration date: " << registration_date << "\n"
            << "Last login: " << last_login;

    return res.str();
}
