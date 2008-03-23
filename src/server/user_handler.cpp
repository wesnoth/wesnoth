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
#include <string>


user_handler::user_handler(const std::string& users_file) :
        users_file_(users_file),
        cfg_(read_config()),
        user_data_(users_file_ + ".db")
{
    load_config();
    load_users();
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
        config& mail = *(cfg_.child("email"));

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
}

void user_handler::load_users() {
    //Create the table with the user details
    //If it already exists this will just fail
    user_data_.exec("create table users ("
            "name text primary key,"
            "password text,"
            "email text,"
            "realname text,"
            "registration_date text,"
            "last_login text"
            ")");

    std::vector<std::string> data;
    user_data_.exec("select * from users", &data);

    assert(data.size() % (uh::MAX_VALUE + 1) == 0);

    int i = 0;
    while(i < data.size()) {
        std::string* u = new std::string[uh::MAX_VALUE];
        users_.insert(std::pair<std::string,std::string*>(data[i++], u));

        u[uh::PASSWORD]          = data[i+uh::PASSWORD];
        u[uh::EMAIL]             = data[i+uh::EMAIL];
        u[uh::REALNAME]          = data[i+uh::REALNAME];
        u[uh::REGISTRATION_DATE] = data[i+uh::REGISTRATION_DATE];
        u[uh::LAST_LOGIN]        = data[i+uh::LAST_LOGIN];

        i += uh::MAX_VALUE;
    }
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

    config& mail = *(cfg_.child("email"));

    jwsmtp::mailer m(to_address, mail["from_address"].c_str(), subject, message,
            mail["mail_server"].c_str(), mail_port_, false);
    if(!(mail["mail_user"].empty())) {
        m.username(mail["mail_user"]);
        m.password( mail["mail_password"]);
    }

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

    for(std::map<std::string,std::string*>::const_iterator i = users_.begin(); i != users_.end(); ++i) {
        time_t last_login = lexical_cast_default<time_t>(i->second[uh::LAST_LOGIN]);
        if((now - last_login) > limit) {
            std::cout << "User '" << i->first << "' exceeds expiration date: ";
            remove_user(i->first);
        }
    }
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
        for(std::map<std::string,std::string*>::const_iterator i = users_.begin(); i != users_.end(); ++i) {
            if(i->second[uh::EMAIL] == mail) {
                throw error("Could not add new user. The email address '" + mail + "' is already in use.");
            }
        }
    }

    std::string now = lexical_cast_default<std::string>(time(NULL));

    sql_query("insert into users (name,password,email,realname,registration_date,last_login) values ('" +
            name + "','" + password + "','" + mail + "','','" + now + "','" + now + "')");

    users_.insert(std::pair<std::string,std::string*>(name,NULL));
    users_[name] = new std::string[uh::MAX_VALUE];

    users_[name][uh::PASSWORD] = password;
    users_[name][uh::EMAIL]    = mail;
    users_[name][uh::REGISTRATION_DATE] = now;
    users_[name][uh::LAST_LOGIN] = now;

    //I don't think we need to send the user details via email,
    //we don't require any account activation anyways.

    std::cout << "Created new user '" << name << "'\n";
}



void user_handler::password_reminder(const std::string& name) {
    #ifndef NO_MAIL

    if(!user_exists(name)) {
        throw error("Could not send password reminder. No user with the name '" + name + "' exists.");
    }

    std::string* u = users_[name];

    if(u[uh::EMAIL].empty()) {
        throw error("Could not send password reminder. The email address of the user '" + name + "' is empty.");
    }

    std::stringstream msg;
    msg << "Hello " << name << ",\n\n" <<
            "Your password is '" << u[uh::PASSWORD] << "'.\n\n" <<
            "Have fun playing at Wesnoth!";

    //If sending does not return true warn that no message was sent.
    if(!(send_mail(u[uh::EMAIL].c_str(), "Wesnoth Multiplayer Server Password Reminder", msg.str().c_str()))) {
        throw error("Could not send password reminder. There was an error sending the reminder email.");
    }
    return;

    #endif //NO_MAIL

    throw error("Could not send password reminder. This server is configured not to send emails.");

}

void user_handler::user_logged_in(const std::string& name) {
    set_user_attribute(name, "last_login", lexical_cast_default<std::string>(time(NULL)));

    //Should we keep track of the IPs used for logging into this account?
}

void user_handler::remove_user(const std::string& name) {
    //Return if the user does not exist
    if(!user_exists(name)) {
        throw error("Could not remove user. No user with the name '" + name + "' exists.");
    }

    sql_query("delete from users where name='" + name + "'");

    users_.erase(users_.find(name));

    std::cout << "Removed user '" << name << "'\n";
}

bool user_handler::login(const std::string& name, const std::string& password) {
    //Return if the user does not exist
    if(!user_exists(name)) {
        return false;
    }

    std::string* u = users_[name];
    return (u[uh::PASSWORD] == password);
}

void user_handler::set_user_attribute(const std::string& name,
        const std::string& attribute, const std::string& value) {

    //Return if the user does not exist
    if(!user_exists(name)) {
        throw error("Could not set attribute '" + attribute  + "' for user '" + name +
        "'. No user with the name with this name exists.");
    }

    sql_query("update users set " + attribute + "='" + value + "' where name='" + name + "'");

    std::string* u = users_[name];

    if(attribute == "password") {
        u[uh::PASSWORD] = value;
    } else if(attribute == "email") {
        u[uh::EMAIL] = value;
    } else if(attribute == "realname") {
        u[uh::REALNAME] = value;
    } else if(attribute == "last_login") {
        u[uh::LAST_LOGIN] = value;
    } else {
        std::cerr << "Call of set_user_attribute() with unknown attribute '" << attribute << "'.\n";
    }
}

bool user_handler::user_exists(const std::string& name) {
    return (users_[name]);
}

bool user_handler::mail() {

    //We should not even get to call this when compiling with NO_MAIL
    //but some doulbe safety can never hurt :)
    #ifdef NO_MAIL
    return false;
    #endif

    return (cfg_.child("email"));
}

void user_handler::set_mail(const std::string& user, const std::string& mail) {
    check_mail(mail);
    set_user_attribute(user, "email", mail);
}

void user_handler::set_password(const std::string& user, const std::string& password) {
    check_password(password);
    set_user_attribute(user, "password", password);
}

void user_handler::set_realname(const std::string& user, const std::string& realname) {
    //Should we perform any check (e.g. max size)?
    set_user_attribute(user, "realname", realname);
}

void user_handler::check_mail(const std::string& mail) {
    if(!(mail.empty() ||utils::isvalid_email(mail))) {
        throw error("The email adress '" + mail + "' appears to be invalid.");
    }
}

void user_handler::sql_query(const std::string query, std::vector<std::string>* data) {
    int r = user_data_.exec(query);
    if(r != SQLITE_OK) {
        throw error("Error executing SQL query.");
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

    std::string* u = users_[name];

    char registration_date[99];
    char last_login[99];

    const time_t& reg_t = lexical_cast_default<time_t>(u[uh::REGISTRATION_DATE]);
    const time_t& last_t = lexical_cast_default<time_t>(u[uh::LAST_LOGIN]);

    strftime(registration_date, 99, "%c", localtime(&reg_t));
    strftime(last_login, 99, "%c", localtime(&last_t));

    std::stringstream res;
    res << "Username: " << name << "\n"
            << (u[uh::REALNAME].empty() ? "" : "Real name: " + u[uh::REALNAME] + "\n")
            << (u[uh::EMAIL].empty() ? "" : "Email: " + u[uh::EMAIL] + "\n")
            << "Registration date: " << registration_date << "\n"
            << "Last login: " << last_login;

    return res.str();
}
