
/**
 * @file
 * @brief Implementation of the client side proxy connection for the ana project.
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

#include <sstream>

#include "asio_proxy_connection.hpp"

proxy_connection::proxy_connection(tcp::socket&      socket,
                                   proxy_information pi,
                                   ana::address      address,
                                   ana::port         port,
                                   ana::timer*       timer) :
    socket_(socket),
    proxy_info_(pi),
    address_(address),
    port_(port),
    manager_( NULL ),
    conn_handler_( NULL ),
    authenticating_( false ),
    timer_( timer )
{
}


std::string* proxy_connection::generate_connect_request() const
{
    return new std::string
    (
        "CONNECT " + address_ + ":" + port_ + " HTTP/1.0\n"
        "User-agent: ana 0.1 \n"
        "\n"
    );
}

std::string* proxy_connection::generate_base64_credentials() const
{
    const std::string user_and_pass( proxy_info_.user_name + ':' + proxy_info_.password );
    return new std::string
    (
        "CONNECT " + address_ + ":" + port_ + " HTTP/1.0\n"
        "User-agent: ana 0.1 \n"
        "Proxy-Connection: keep-alive\n"
        "Proxy-Authorization: Basic " + base64_encode(user_and_pass.c_str(),
                                                      user_and_pass.size() ) + "\n"
        "\n"
    );
}

std::string proxy_connection::base64_encode(char const* bytes_to_encode, unsigned int in_len) const
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    const char base64_chars[(1 << 6) + 2]
               = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0]  & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2]   & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;
}

bool proxy_connection::finds( const std::string& source, char const* pattern )
{
    const size_t find_pos = source.find( std::string( pattern ) );

    return find_pos < source.size();
}

void proxy_connection::handle_response(boost::asio::streambuf*          buf,
                                       const boost::system::error_code& ec,
                                       size_t                           /*bytes_read*/)
{
    std::stringstream ss;
    ss << buf;
    delete buf;

    if ( ec )
        manager_->handle_proxy_connection( ec, conn_handler_, timer_ );
    else
    {
        if ( finds( ss.str(), "200" ) )
            manager_->handle_proxy_connection( ec, conn_handler_, timer_ );
        else
        {
            if ( ( ! authenticating_ ) && finds( ss.str(), "407" ) )
            {
                if ( finds( ss.str(), "Proxy-Authenticate: Basic" ) )
                {
                    authenticating_ = true;
                    socket_.close();

                    do_connect( );
                }
                else //TODO: digest authentication support here
                    manager_->handle_proxy_connection( ana::generic_error, conn_handler_, timer_);

            }
            else //Can't connect, wrong password or wasn't offered the possibility to authenticate
                manager_->handle_proxy_connection( ana::generic_error, conn_handler_, timer_);
        }
    }
}

void proxy_connection::handle_sent_request(const boost::system::error_code& /*ec*/,
                                           std::string*                     request)
{
    delete request;

    boost::asio::streambuf* buf = new boost::asio::streambuf( );

    boost::asio::async_read_until(socket_, *buf,
                                  "\r\n\r\n",
                                  boost::bind(&proxy_connection::handle_response, this,
                                              buf, boost::asio::placeholders::error,_2));
}

void proxy_connection::handle_connect(const boost::system::error_code& ec,
                                      tcp::resolver::iterator          endpoint_iterator)
{
    if ( ! ec )
    {
        std::string* request( NULL );

        if ( authenticating_ )
            request = generate_base64_credentials();
        else
            request = generate_connect_request();

        socket_.async_send(boost::asio::buffer( *request ),
                                 boost::bind(&proxy_connection::handle_sent_request,this,
                                             boost::asio::placeholders::error,
                                             request));
    }
    else
    {
        if ( endpoint_iterator == tcp::resolver::iterator() ) // could not connect to proxy
            manager_->handle_proxy_connection( ec, conn_handler_, timer_ );
        else
        {
            //retry
            socket_.close();

            tcp::endpoint endpoint = *endpoint_iterator;
            socket_.async_connect(endpoint,
                                boost::bind(&proxy_connection::handle_connect, this,
                                            boost::asio::placeholders::error, ++endpoint_iterator));
        }
    }
}

void proxy_connection::do_connect()
{
    try
    {
        tcp::resolver resolver( socket_.get_io_service() );
        tcp::resolver::query query(proxy_info_.proxy_address.c_str(),
                                   proxy_info_.proxy_port.c_str() );
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::endpoint endpoint = *endpoint_iterator;

        socket_.async_connect(endpoint,
                              boost::bind(&proxy_connection::handle_connect, this,
                                          boost::asio::placeholders::error, ++endpoint_iterator));
    }
    catch (const std::exception&)
    {
        manager_->handle_proxy_connection(ana::generic_error, conn_handler_, timer_ );
    }
}

void proxy_connection::connect( proxy_connection_manager* manager,
                                ana::connection_handler* handler )
{
    manager_        = manager;
    conn_handler_   = handler;
    authenticating_ = false;

    do_connect();
}
