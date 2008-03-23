#ifndef USER_HANDLER_HPP_INCLUDED
#define USER_HANDLER_HPP_INCLUDED

#include "../global.hpp"

#include "../config.hpp"

#ifndef NO_MAIL
#include <jwsmtp/jwsmtp.h>
#endif

#include "sqlite.hpp"


#include <vector>
#include <string>
#include <map>

namespace uh {

    enum {
        PASSWORD = 0,
        EMAIL,
        REALNAME,
        REGISTRATION_DATE,
        LAST_LOGIN,
        MAX_VALUE
    };

}

class user_handler {
    public:
        user_handler(const std::string& users_file);
        ~user_handler();

        config read_config() const;
        void load_config();
        void load_users();
        void save_config();

        void clean_up();

        //! Removes users that have not logged in for a certain amount of time
        void remove_dead_users();

        //! Adds a user.
        //! Returns false if adding fails (e.g. because a user with the same name already exists).
        void add_user(const std::string& name, const std::string& mail, const std::string& password);

        //! Removes a user.
        //! Returns false if the user does not exist
        void remove_user(const std::string& name);

        //! Send a password reminder email to the given user
        void password_reminder(const std::string& name);

        void set_mail(const std::string& user, const std::string& mail);
        void set_password(const std::string& user, const std::string& password);
        void set_realname(const std::string& user, const std::string& realname);

        //! Returns true if the given password equals the password for the given user
        bool login(const std::string& name, const std::string& password);

        //! Returns true if a user with the given name exists
        bool user_exists(const std::string& name);

        //! The user with the given name logged in
        void user_logged_in(const std::string& name);

        //! Returns a string containing info like the last login of this user
        std::string user_info(const std::string& name);

        //! Returns true if we can send emails
        bool mail();

        struct error {
            error(const std::string& msg) : message(msg) {}
            std::string message;
        };

    private:
        //! Sends an email to a specified address. Requires access to an SMTP server.
        //! Returns false if the mail could not be sent.
        bool send_mail(const char* to_address, const char* subject, const char* message);

        void set_user_attribute(const std::string& name,
                const std::string& attribute, const std::string& value);

        void check_mail(const std::string& mail);
        void check_password(const std::string& password);

        //! A wrapper for user_data_.exec()
        //! Throws an exception if the the query fails
        //! User generated querys (e.g. registration of
        //! a new nick, etc.) should use this.
        void sql_query(const std::string query, std::vector<std::string>* data =NULL);

        std::string users_file_;
        unsigned short mail_port_;
        unsigned short username_expiration_limit_;

        config cfg_;
        sqlite_database user_data_;
        std::map<std::string,std::string*> users_;
};

#endif
