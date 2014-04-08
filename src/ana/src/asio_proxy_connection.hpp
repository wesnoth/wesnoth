
/**
 * @file
 * @brief Header file of the client side proxy connection for the ana project.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 - 2014 Guillermo Biset.
 *
 * This file is part of the ana project.
 *
 * System:         ana
 * Language:       C++
 *
 * Author:         Guillermo Biset
 * E-Mail:         billybiset AT gmail DOT com
 *
 * ana is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * ana is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ana.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#include "../api/ana.hpp"

#ifndef ASIO_PROXY_CONNECTION
#define ASIO_PROXY_CONNECTION

struct proxy_connection_manager
{
	virtual ~proxy_connection_manager() {}

    virtual void handle_proxy_connection(const boost::system::error_code&,
                                         ana::connection_handler*,
                                         ana::timer*)                      = 0;
};

struct proxy_information
{
    proxy_information() :
        proxy_address(),
        proxy_port(),
        user_name(),
        password()
    {
    }

    std::string                     proxy_address;
    std::string                     proxy_port;
    std::string                     user_name;
    std::string                     password;
};

class proxy_connection
{
    public:
        proxy_connection(tcp::socket&      socket,
                         proxy_information pi,
                         ana::address      address,
                         ana::port         port,
                         ana::timer*       timer);

        void connect( proxy_connection_manager* manager, ana::connection_handler* handler );

    private:
        std::string* generate_connect_request()    const;
        std::string* generate_base64_credentials() const;

        void do_connect( );

        void handle_connect(const boost::system::error_code& ec,
                            tcp::resolver::iterator endpoint_iterator);

        void handle_sent_request(const boost::system::error_code& ec, std::string* request);

        void handle_response( boost::asio::streambuf* buf,
                              const boost::system::error_code&,
                              size_t );

        bool finds( const std::string& source, char const* pattern );

        std::string base64_encode(char const* bytes_to_encode, unsigned int in_len) const;

        // Attributes
        tcp::socket&              socket_;

        const proxy_information   proxy_info_;

        ana::address              address_;
        ana::port                 port_;

        proxy_connection_manager* manager_;
        ana::connection_handler*  conn_handler_;

        bool                      authenticating_;
        ana::timer*               timer_;
};

#endif
