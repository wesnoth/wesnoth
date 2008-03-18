#ifndef USER_HANDLER_HPP_INCLUDED
#define USER_HANDLER_HPP_INCLUDED

#include "../global.hpp"

#include "../config.hpp"
#include "../log.hpp"
#include "../filesystem.hpp"
#include "../serialization/parser.hpp"
#include "../serialization/preprocessor.hpp"
#include "../serialization/string_utils.hpp"

#include <jwsmtp/jwsmtp.h>

//! @todo Generally write nice log/error messages for all user_handler functions

class user_handler {
    public:
        user_handler(const std::string& users_file);
        ~user_handler();

        config read_config() const;
        void load_config();
        void save_config();

        //! Remove users that registered but did never log in, etc.
        void clean_up();

        //! Adds a user.
        //! Returns false if adding fails (e.g. because a user with the same name already exists).
        void add_user(const std::string& name, const std::string& mail, const std::string& password);

        //! Removes a user-
        //! Returns false if the user does not exist
        void remove_user(const std::string& name);

        //! Send a password reminder email to the given user
        void password_reminder(const std::string& name);

        void set_mail(const std::string& user, const std::string& mail);
        void set_password(const std::string& user, const std::string& password);

        //! Returns true if the given password equals the password for the given user
        bool login(const std::string& name, const std::string& password);

        //! Returns true if a user with the given name exists
        bool user_exists(const std::string& name);


        struct error {
            error(const std::string& msg) : message(msg) {}
            std::string message;
        };

    private:
        //! Sends an e-mail to a specified address. Requires access to an SMTP server.
        //! Returns false if the mail could not be sent.
        bool send_mail(const char* to_address, const char* subject, const char* message);

        void set_user_attribute(const std::string& name,
                const std::string& attribute, const std::string& value);

        void check_mail(const std::string& mail);
        void check_password(const std::string& password);

        std::string users_file_;
        unsigned short mail_port_;

        config cfg_;
        config* users_;
};

#endif
