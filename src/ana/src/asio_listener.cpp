/* $Id$ */

/**
 * @file asio_listener.cpp
 * @brief Implementation of a listener for the ana project.
 *
 * ana: Asynchronous Network API.
 * Copyright (C) 2010 Guillermo Biset.
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

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "asio_listener.hpp"

using boost::asio::ip::tcp;

asio_listener::asio_listener( ) :
    listener_( NULL )
{
}

asio_listener::~asio_listener()
{
}

void asio_listener::disconnect( ana::listener_handler* listener, boost::system::error_code error)
{
    listener->handle_disconnect( error, id() );
    disconnect_listener();
}


void asio_listener::handle_body( ana::detail::read_buffer buf,
                                 const boost::system::error_code& ec, ana::listener_handler* listener )
{
    try
    {
        if (ec)
            disconnect(listener, ec);
        else
        {
            log_receive( buf );
            
            listener->handle_message( ec, id(), buf );

            listen_one_message();
        }
    }
    catch(const std::exception& e)
    {
        disconnect(listener, ec);
    }
}


void asio_listener::handle_header(char* header, const boost::system::error_code& ec, ana::listener_handler* listener )
{
    try
    {
        if (ec)
            disconnect(listener, ec);
        else
        {
            ana::serializer::bistream input( std::string(header, ana::HEADER_LENGTH) );

            size_t   size;
            input >> size;

            if (size != 0)
            {
                ana::detail::read_buffer read_buf( new ana::detail::read_buffer_implementation( size ) );

                boost::asio::async_read(socket(), boost::asio::buffer( read_buf->base(), read_buf->size() ),
                                        boost::bind(&asio_listener::handle_body,
                                                    this, read_buf,
                                                    boost::asio::placeholders::error,
                                                    listener));
            }
        }
    }
    catch(const std::exception& e)
    {
        disconnect(listener, ec);
    }
}

void asio_listener::set_listener_handler( ana::listener_handler* listener )
{
    listener_ = listener;
}

void asio_listener::run_listener( )
{
    listen_one_message();
}

void asio_listener::listen_one_message()
{
    try
    {
        boost::asio::async_read(socket(), boost::asio::buffer(header_, ana::HEADER_LENGTH),
                                boost::bind(&asio_listener::handle_header, this,
                                            header_, boost::asio::placeholders::error, listener_));
    }
    catch(const std::exception& e)
    {
        disconnect(listener_, boost::system::error_code(1,boost::system::system_category) );
    }
}