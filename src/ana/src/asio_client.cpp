/* $Id$ */

/**
 * @file asio_client.cpp
 * @brief Implementation of the client side of the ana project.
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
 * the Free Software Foundation, either version 3 of the License, or
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

#include <iostream>

#include <memory>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "asio_client.hpp"

using boost::asio::ip::tcp;

asio_client::asio_client(ana::address address, ana::port pt) :
    io_service_(),
    socket_(io_service_),
    asio_listener(io_service_, socket_),
    address_(address),
    port_(pt),
    proxy_( NULL ),
    use_proxy_( false )
{
}

asio_client::~asio_client()
{
}

ana::client* ana::client::create(ana::address address, ana::port pt)
{
    return new asio_client(address, pt);
}

void asio_client::run()
{
    if ( ! use_proxy_ ) // If I will connect through a proxy, defer this call until connected.
        run_listener( );

    boost::thread t( boost::bind( &boost::asio::io_service::run, &io_service_) );
}

ana::client_id asio_client::id() const
{
    return 0;
}

void asio_client::handle_proxy_connection(const boost::system::error_code& ec, ana::connection_handler* handler)
{
    handler->handle_connect( ec, 0 );

    if ( ! ec )
        run_listener();
        
    delete proxy_;
}

void asio_client::handle_connect(const boost::system::error_code& ec,
                                 tcp::resolver::iterator endpoint_iterator,
                                 ana::connection_handler* handler )
{
    if ( ! ec )
        handler->handle_connect( ec, 0 );
    else
    {
        if ( endpoint_iterator == tcp::resolver::iterator() ) // finished iterating, not connected
            handler->handle_connect( ec, 0 );
        else
        {
            //retry
            socket_.close();

            tcp::endpoint endpoint = *endpoint_iterator;
            socket_.async_connect(endpoint,
                                boost::bind(&asio_client::handle_connect, this,
                                            boost::asio::placeholders::error, ++endpoint_iterator,
                                            handler));
        }
    }
}

void asio_client::connect( ana::connection_handler* handler )
{
    try
    {
        tcp::resolver resolver(io_service_);
        tcp::resolver::query query(address_.c_str(), port_.c_str() );
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        tcp::endpoint endpoint = *endpoint_iterator;
        socket_.async_connect(endpoint,
                              boost::bind(&asio_client::handle_connect, this,
                                          boost::asio::placeholders::error, ++endpoint_iterator,
                                          handler));
    }
    catch (const std::exception& e)
    {
        handler->handle_connect( boost::system::error_code(1,boost::system::system_category ), 0 );
        std::cerr << "Client: An error ocurred, " << e.what() << std::endl;
    }
}

void asio_client::connect_through_proxy(ana::proxy::authentication_type auth_type,
                                        std::string                     proxy_address,
                                        std::string                     proxy_port,
                                        ana::connection_handler*        handler,
                                        std::string                     user_name,
                                        std::string                     password)
{
    use_proxy_ = true;

    proxy_information proxy_info;

    proxy_info.auth_type     = auth_type;
    proxy_info.proxy_address = proxy_address;
    proxy_info.proxy_port    = proxy_port;
    proxy_info.user_name     = user_name;
    proxy_info.password      = password;

    proxy_ = new proxy_connection( socket_, proxy_info, address_, port_);

    proxy_->connect( this, handler );
}

void asio_client::send(boost::asio::const_buffer buffer, ana::send_handler* handler, ana::send_type copy_buffer )
{
    try
    {
        ana::detail::shared_buffer s_buf(
          new ana::detail::copying_buffer(buffer, copy_buffer ) ); // it's a boost::shared_ptr

        ana::serializer::bostream* output_stream = new ana::serializer::bostream();
        (*output_stream) << s_buf->size();

        //write the header first in a separate operation, then send the full buffer
        boost::asio::async_write(socket_, boost::asio::buffer( output_stream->str() ),
                                 boost::bind(&asio_client::handle_sent_header,this,
                                             boost::asio::placeholders::error, output_stream,
                                             s_buf, handler));
    }
    catch(std::exception& e)
    {
        disconnect_listener();
    }
}

void asio_client::handle_sent_header(const boost::system::error_code& ec,
                                     ana::serializer::bostream* bos, ana::detail::shared_buffer buffer,
                                     ana::send_handler* handler)
{
    delete bos;

    if ( ! ec )
    {
        boost::asio::async_write(socket_, boost::asio::buffer(buffer->base(), buffer->size() ),
                                    boost::bind(&asio_client::handle_send,this,
                                                boost::asio::placeholders::error,
                                                buffer, handler));
    }
}


void asio_client::handle_send(const boost::system::error_code& ec,
                              ana::detail::shared_buffer buffer,
                              ana::send_handler* handler)
{
    handler->handle_send( ec, id() );

    if ( ec )
        disconnect_listener();
}

void asio_client::disconnect_listener()
{
    io_service_.stop();
}
